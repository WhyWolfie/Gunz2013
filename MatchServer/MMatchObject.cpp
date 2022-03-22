#include "pch.h"

#include "MMatchObject.h"
#include "MMatchConstant.h"

#include "MMatchItem.h"

#include "MMatchChannel.h"
#include "MMatchStage.h"

#include "MMatchServer_OnCommand.h"

#include "MMatchDBMgr.h"
#include "MAsyncDBProcess.h"

#include "MMatchClan.h"

#include "MMatchChatRoom.h"

#include "MMatchDuelTournament.h"
#include "MMatchBlitzKrieg.h"

#include "MServerNetwork.h"

MMatchObjectManager g_ObjectMgr;

MMatchObject::MMatchObject(Socket::socket_type s, const MUID &uid, const unsigned char *pCryptKey)
{
	m_Socket = s;
	m_uid = uid;
	memcpy(&m_nCryptKey[0], &pCryptKey[0], sizeof(unsigned char) * ENCRYPTIONKEY_LENGTH);
	
	m_bDisconnected = false;
	
	m_bCharInfoExist = false;
	
	ZeroInit(&m_Account, sizeof(MMatchAccountInfo));
	ZeroInit(&m_Char, sizeof(MMatchCharInfo));
	ZeroInit(&m_Exp, sizeof(MMatchCharExp));
	// ZeroInit(m_Item, sizeof(MMatchCharItem) * MMCIP_END);
	ZeroInit(&m_Survival, sizeof(MMatchCharSurvivalInfo));
	ZeroInit(&m_DuelTournament, sizeof(MMatchCharDuelTournamentInfo));
	ZeroInit(&m_GameInfo, sizeof(MMatchCharGameInfo));
	ZeroInit(&m_Clan, sizeof(MMatchCharClanInfo));
	ZeroInit(&m_BlitzKrieg, sizeof(MMatchCharBlitzKriegInfo));
	
	m_nPlayerFlag = 0;
	m_nOptionFlag = 0;
	m_nGameFlag = MMOGF_NONE;
	
	m_nPlace = 0;
	m_nStageState = 0;
	
	ZeroUID(m_uidEquippedItem, MMCIP_END);
	
	m_uidChannel = MUID(0, 0);
	m_uidStage = MUID(0, 0);
	
	m_nLastChannelPlayerListPage = 0;
	m_nLastStageListPage = 0;
	
	m_nRequestedChannelListType = -1;
	
	m_nLastChannelPlayerListChecksum = 0;
	m_nLastStageListChecksum = 0;
	
	m_nSpawnTimer = 0;
	
	m_bVoted = false;
	
	m_nChatRoomID = 0;
	
	m_bAgentUser = false;
	
	m_bFlooding = false;
	
	m_nBlitzGroupID = 0;
	
	m_bDistribMedalBonus = false;
	m_nNextMedalBonusTime = 0;
	
	m_nAddedBonusMedal = 0;
	
	m_bStageKickReserved = false;
	m_nStageKickTime = 0;
	
	// init socket address.
	Socket::address_type addr;
	Socket::GetPeerName(s, &addr);
	
	char szIP[64];
	strcpy(szIP, Socket::InetNtoa(&addr));
	
	strcpy(m_szIP, szIP);
	m_nIP = Socket::InetAddr(szIP);
	m_nPort = Socket::Ntohs(addr.sin_port);
	m_nUDPPort = 0;
}

MMatchObject::~MMatchObject()
{
	Clean();
	
	// clear all account items.
	ClearAccItemList();
	
	// leave from all chat room.
	for(vector<unsigned long>::iterator i = m_vtChatRoomID.begin(); i != m_vtChatRoomID.end(); i++)
	{
		unsigned long nCurrID = (*i);
		
		MMatchChatRoom *pCurrRoom = g_ChatRoomMgr.Get(nCurrID);
		if(pCurrRoom != NULL)
		{
			pCurrRoom->Leave(this);
			
			if(pCurrRoom->IsEmpty() == true)
			{
				g_ChatRoomMgr.Remove(pCurrRoom->GetID());
			}
		}
	}
	m_vtChatRoomID.clear();
	
	m_nChatRoomID = 0;
	
	AsyncDb_SetAccountCash(m_uid, m_Account.nAID, m_Account.nCash);
}

void MMatchObject::Disconnect()
{
	ReserveDisconnect(m_Socket);
	m_bDisconnected = true;
}

void MMatchObject::Update(unsigned long nTime)
{
	if(m_bDisconnected == true) return;
	
	if(m_bFlooding == true)
	{
		mlog("[AID %d - UID %lu:%lu] Disconnecting this client for command flooding...", 
			  m_Account.nAID, m_uid.ulHighID, m_uid.ulLowID);
		Disconnect();
		return;
	}
	
	#define BLITZ_MEDAL_BONUS_TIME_INTERVAL	60000	// 1 min.
	if(m_bDistribMedalBonus == true && m_nNextMedalBonusTime <= nTime)
	{
		// m_BlitzKrieg.nMedal++;
		if(m_nAddedBonusMedal < 80)	// 80 : waiting medal bonus limit.
		{
			m_nAddedBonusMedal++;
		}
		
		// don't do (nTime + BLITZ_MEDAL_BONUS_TIME_INTERVAL), it makes time difference.
		m_nNextMedalBonusTime += BLITZ_MEDAL_BONUS_TIME_INTERVAL;
	}
	
	if(m_bStageKickReserved == true && m_nStageKickTime <= nTime)
	{
		OnStageLeave(m_uid);
		m_bStageKickReserved = false;
	}
}

void MMatchObject::Clean()
{
	ClearCharItemList();
	ClearAllCharGambleItem();
	
	ClearFriendList();
	
	SaveExp();
	SaveQuestItem();
	
	g_DTQMgr.CancelChallenge(this);
	
	if(m_nBlitzGroupID != 0)
	{
		// m_nBlitzGroupID will be zero in this Remove() function.
		g_BKQMgr.Remove(m_nBlitzGroupID, m_Char.szName);
	}
	
	m_bCharInfoExist = false;

	/*
	MMatchStage *pStage = g_StageMgr.Get(m_uidStage);
	if(pStage != NULL)
	{
		pStage->Leave(this);
	}
	*/
	
	OnStageLeaveBattle(m_uid, false);
	OnStageLeave(m_uid);
	
	MMatchChannel *pChannel = g_ChannelMgr.Get(m_uidChannel);
	if(pChannel != NULL)
	{
		pChannel->Leave(this);
		
		if((pChannel->GetType() == MCHANNEL_TYPE_USER || pChannel->GetType() == MCHANNEL_TYPE_CLAN) && 
			pChannel->GetCurrPlayers() == 0)
		{
			g_ChannelMgr.Remove(pChannel);
		}
	}
	
	m_uidChannel = MUID(0, 0);
	m_uidStage = MUID(0, 0);
	
	m_nPlace = MMP_OUTSIDE;
	
	if(m_Clan.nCLID != 0)
	{
		MMatchClan *pClan = g_ClanMgr.Get(m_Clan.nCLID);
		if(pClan != NULL)
		{
			pClan->Leave(this);
		}
		
		m_Clan.nCLID = 0;
	}
	
	m_nLastChannelPlayerListChecksum = UINT_MAX;
	m_nLastStageListChecksum = UINT_MAX;
}

void MMatchObject::SaveExp()
{
	if(m_bCharInfoExist == true)
	{
		m_Exp.nXP = CheckPlusOver(m_Exp.nXP, m_GameInfo.nExp);
		m_Exp.nKill = CheckPlusOver(m_Exp.nKill, m_GameInfo.nKill);
		m_Exp.nDeath = CheckPlusOver(m_Exp.nDeath, m_GameInfo.nDeath);
		// Db_SetExp(m_Char.nCID, m_Exp.nLevel, m_Exp.nXP, m_Exp.nBP, m_Exp.nKill, m_Exp.nDeath);
		AsyncDb_SetExp(m_uid, m_Char.nCID, m_Exp.nLevel, m_Exp.nXP, m_Exp.nBP, m_Exp.nKill, m_Exp.nDeath);
		
		// reset game info.
		ResetInGameInfo();	
		
		// Db_SetSurvivalPoint(m_Char.nCID, m_Survival.nPoint);
		AsyncDb_SetSurvivalPoint(m_uid, m_Char.nCID, m_Survival.nPoint);
		// Db_SetDTScore(m_Char.nCID, m_DuelTournament.nTP, m_DuelTournament.nWin, m_DuelTournament.nLose, m_DuelTournament.nFinalWin);
		AsyncDb_SetDTScore(m_uid, m_Char.nCID, m_DuelTournament.nTP, m_DuelTournament.nWin, m_DuelTournament.nLose, m_DuelTournament.nFinalWin);
		
		AsyncDb_SetBlitzScore(m_uid, m_Char.nCID, m_BlitzKrieg.nWin, m_BlitzKrieg.nLose, m_BlitzKrieg.nPoint, m_BlitzKrieg.nMedal);
	}
}

void MMatchObject::SaveSpentItem()
{
	if(m_bCharInfoExist == false) return;
	
	if(m_GameInfo.nSpendItemUsed[0] > 0)
	{
		MMatchCharItem *pCharItem = GetCharItem(m_uidEquippedItem[MMCIP_CUSTOM1]);
		if(pCharItem != NULL)
		{
			pCharItem->nCount -= m_GameInfo.nSpendItemUsed[0];
			
			if(pCharItem->nCount <= 0)
			{
				// Db_TakeoffItem(m_Char.nCID, MMCIP_CUSTOM1);
				AsyncDb_TakeoffItem(m_uid, m_Char.nCID, MMCIP_CUSTOM1);
				// Db_DeleteSpentItem(pCharItem->nCIID);
				AsyncDb_DeleteSpentItem(m_uid, pCharItem->nCIID);
				
				m_uidEquippedItem[MMCIP_CUSTOM1] = MUID(0, 0);
				RemoveCharItem(pCharItem->uid);
			}
			else
			{
				// Db_UpdateItemCount(pCharItem->nCIID, pCharItem->nCount);
				AsyncDb_UpdateItemCount(m_uid, pCharItem->nCIID, pCharItem->nCount);
			}
		}
		
		m_GameInfo.nSpendItemUsed[0] = 0;
	}
	
	if(m_GameInfo.nSpendItemUsed[1] > 0)
	{
		MMatchCharItem *pCharItem = GetCharItem(m_uidEquippedItem[MMCIP_CUSTOM2]);
		if(pCharItem != NULL)
		{
			pCharItem->nCount -= m_GameInfo.nSpendItemUsed[1];
			
			if(pCharItem->nCount <= 0)
			{
				// Db_TakeoffItem(m_Char.nCID, MMCIP_CUSTOM2);
				AsyncDb_TakeoffItem(m_uid, m_Char.nCID, MMCIP_CUSTOM2);
				// Db_DeleteSpentItem(pCharItem->nCIID);
				AsyncDb_DeleteSpentItem(m_uid, pCharItem->nCIID);
				
				m_uidEquippedItem[MMCIP_CUSTOM2] = MUID(0, 0);
				RemoveCharItem(pCharItem->uid);
			}
			else
			{
				// Db_UpdateItemCount(pCharItem->nCIID, pCharItem->nCount);
				AsyncDb_UpdateItemCount(m_uid, pCharItem->nCIID, pCharItem->nCount);
			}
		}
		
		m_GameInfo.nSpendItemUsed[1] = 0;
	}
}

void MMatchObject::ResetInGameInfo()
{
	m_GameInfo.bAlive = false;
	m_GameInfo.nKill = m_GameInfo.nDeath = 0;
	m_GameInfo.nExp = 0;
}

// equipped item list.
void MMatchObject::SetItemUID(const MUID &uidItem, int nItemSlot)
{
	if(nItemSlot < 0 || nItemSlot >= MMCIP_END) return;
	m_uidEquippedItem[nItemSlot] = uidItem;
}

bool MMatchObject::IsEquippedItem(const MUID &uidItem)
{
	for(int i = 0; i < MMCIP_END; i++)
	{
		if(m_uidEquippedItem[i] == uidItem) return true;
	}
	return false;
}

int MMatchObject::CalcCurrWeight()
{
	int nResult = 0;
	
	for(int i = 0; i < MMCIP_END; i++)
	{
		MMatchCharItem *pCharItem = GetCharItemBySlotIndex(i);
		if(pCharItem == NULL) continue;
		
		MMatchItem *pItem = g_ItemMgr.Get(pCharItem->nID);
		if(pItem == NULL) continue;
		
		nResult += pItem->GetWeight();
	}
	
	return nResult;
}

int MMatchObject::CalcMaxWeight()
{
	int nResult = 100;	// 100 = default weight.
	
	for(int i = 0; i < MMCIP_END; i++)
	{
		MMatchCharItem *pCharItem = GetCharItemBySlotIndex(i);
		if(pCharItem == NULL) continue;
		
		MMatchItem *pItem = g_ItemMgr.Get(pCharItem->nID);
		if(pItem == NULL) continue;
		
		nResult += pItem->GetMaxWt();
	}
	
	return nResult;
}

void MMatchObject::ResetDTScore()
{
	m_DuelTournament.nTP = 1000;
	m_DuelTournament.nWin = 0;
	m_DuelTournament.nLose = 0;
	m_DuelTournament.nFinalWin = 0;
	m_DuelTournament.nRanking = 0;
}

bool MMatchObject::IsAdmin()
{
	int nUGradeID = m_Account.nUGradeID;
	
	if(nUGradeID == MMUG_EVENTMASTER || 
		nUGradeID == MMUG_DEVELOPER || 
		nUGradeID == MMUG_ADMIN)
			return true;
			
	return false;
}

void MMatchObject::Hide(bool b)
{
	if(b == true)
	{
		m_nPlayerFlag |= (unsigned long)MTD_PlayerFlags_AdminHide;
	}
	else
	{
		// http://hwada.hatenablog.com/entry/20050921/1127250130
		m_nPlayerFlag &= (unsigned long)~MTD_PlayerFlags_AdminHide;
	}
}

bool MMatchObject::IsHide()
{
	return (m_nPlayerFlag & (unsigned long)MTD_PlayerFlags_AdminHide) != 0 ? true : false ;
}

unsigned int MMatchObject::MakeChecksum()
{
	// for channel player list.
	
	unsigned int nRet = 0;
	
	nRet += (unsigned int)(m_uid.ulHighID + m_uid.ulLowID);
	nRet += (unsigned int)m_Char.nCID;
	nRet += (unsigned int)m_Exp.nLevel;
	nRet += (unsigned int)m_DuelTournament.nClass;
	nRet += (unsigned int)m_Clan.nCLID;
	nRet += (unsigned int)m_nPlace;
	
	return nRet;
}

// acc item.
void MMatchObject::AddAccItem(int nAIID, int nItemID, int nItemCount, int nRentPeriod)
{
	MMatchAccItem *pNew = new MMatchAccItem;
	
	pNew->nAIID = nAIID;
	pNew->nID = nItemID;
	pNew->nCount = nItemCount;
	pNew->nRentSecPeriod = nRentPeriod;
	pNew->nMadeTime = GetTime();
	
	m_AccountItemList.push_back(pNew);
}

void MMatchObject::RemoveAccItem(int nAIID)
{
	for(list<MMatchAccItem *>::iterator i = m_AccountItemList.begin(); i != m_AccountItemList.end(); i++)
	{
		MMatchAccItem *pCurrItem = (*i);
		
		if(pCurrItem->nAIID == nAIID)
		{
			delete pCurrItem;
			m_AccountItemList.erase(i);
			break;
		}
	}
}

MMatchAccItem *MMatchObject::GetAccItemByItemID(int nItemID)
{
	for(list<MMatchAccItem *>::iterator i = m_AccountItemList.begin(); i != m_AccountItemList.end(); i++)
	{
		MMatchAccItem *pCurrItem = (*i);
		if(pCurrItem->nID == nItemID) return pCurrItem;
	}
	return NULL;
}

MMatchAccItem *MMatchObject::GetAccItemByAIID(int nAIID)
{
	for(list<MMatchAccItem *>::iterator i = m_AccountItemList.begin(); i != m_AccountItemList.end(); i++)
	{
		MMatchAccItem *pCurrItem = (*i);
		if(pCurrItem->nAIID == nAIID) return pCurrItem;
	}
	return NULL;
}

void MMatchObject::AccItemExpired(int nAIID)
{
	RemoveAccItem(nAIID);
}

void MMatchObject::ClearAccItemList()
{
	for(list<MMatchAccItem *>::iterator i = m_AccountItemList.begin(); i != m_AccountItemList.end(); i++)
	{
		delete (*i);
	}
	m_AccountItemList.clear();
}

bool MMatchObject::MoveItemToStorage(int nCIID)
{
	/*
	MMatchCharItem *pCharItem = GetCharItemByCIID(nCIID);
	if(pCharItem == NULL) return false;
	
	int nAIID;
	if(Db_MoveCItemToAItem(nCIID, m_Account.nAID, &nAIID) == false) return false;
	
	if(nAIID == 0) return false;
	
	AddAccItem(nAIID, pCharItem->nID, pCharItem->nCount, pCharItem->nRentSecPeriod);
	RemoveCharItem(nCIID);
	*/
	
	MMatchCharItem *pCharItem = GetCharItemByCIID(nCIID);
	if(pCharItem == NULL) return false;
	
	AsyncDb_MoveCItemToAItem(m_uid, nCIID, m_Account.nAID, 0, *pCharItem);
	
	RemoveCharItem(nCIID);
	
	return true;
}

bool MMatchObject::MoveItemToStorage(int nCIID, int nCount)
{
	/*
	MMatchCharItem *pCharItem = GetCharItemByCIID(nCIID);
	if(pCharItem == NULL) return false;
	
	int nNewCount = pCharItem->nCount - nCount;
	if(nNewCount < 0) return false; // invalid : minus count.
	
	int nAIID;
	if(Db_MoveCItemToAItem(nCIID, m_Account.nAID, nCount, &nAIID) == false) return false;
	
	if(nAIID == 0) return false;
	
	MMatchAccItem *pAccItem = GetAccItemByItemID(pCharItem->nID);
	if(pAccItem == NULL)
	{
		AddAccItem(nAIID, pCharItem->nID, nCount, pCharItem->nRentSecPeriod);
	}
	else
	{
		pAccItem->nCount += nCount;
	}
	
	if(nNewCount == 0)
	{
		RemoveCharItem(nCIID);
	}
	else
	{
		pCharItem->nCount = nNewCount;
	}
	*/
	
	MMatchCharItem *pCharItem = GetCharItemByCIID(nCIID);
	if(pCharItem == NULL) return false;
	
	int nNewCount = pCharItem->nCount - nCount;
	if(nNewCount < 0) return false; // invalid : minus count.
	
	AsyncDb_MoveCItemToAItem(m_uid, nCIID, m_Account.nAID, nCount, *pCharItem);
	
	if(nNewCount == 0)
	{
		RemoveCharItem(nCIID);
	}
	else
	{
		pCharItem->nCount = nNewCount;
	}
	
	return true;
}

bool MMatchObject::MoveItemToInventory(int nAIID)
{
	/*
	MMatchAccItem *pAccItem = GetAccItemByAIID(nAIID);
	if(pAccItem == NULL) return false;
	
	int nCIID;
	if(Db_MoveAItemToCItem(nAIID, m_Char.nCID, &nCIID) == false) return false;
	
	if(nCIID == 0) return false;
	
	AddCharItem(MUID::Assign(), nCIID, pAccItem->nID, pAccItem->nCount, pAccItem->nRentSecPeriod);
	RemoveAccItem(nAIID);
	*/
	
	MMatchAccItem *pAccItem = GetAccItemByAIID(nAIID);
	if(pAccItem == NULL) return false;
	
	AsyncDb_MoveAItemToCItem(m_uid, nAIID, m_Char.nCID, 0, *pAccItem);
	
	RemoveAccItem(nAIID);
	
	return true;
}

bool MMatchObject::MoveItemToInventory(int nAIID, int nCount)
{
	/*
	MMatchAccItem *pAccItem = GetAccItemByAIID(nAIID);
	if(pAccItem == NULL) return false;
	
	int nNewCount = pAccItem->nCount - nCount;
	if(nNewCount < 0) return false;
	
	int nCIID;
	if(Db_MoveAItemToCItem(nAIID, m_Char.nCID, nCount, &nCIID) == false) return false;
	
	if(nCIID == 0) return false;
	
	MMatchCharItem *pCharItem = GetCharItemByItemID(pAccItem->nID);
	if(pCharItem == NULL)
	{
		AddCharItem(MUID::Assign(), nCIID, pAccItem->nID, nCount, pAccItem->nRentSecPeriod);
	}
	else
	{
		pCharItem->nCount += nCount;
	}
	
	if(nNewCount == 0)
	{
		RemoveAccItem(nAIID);
	}
	else
	{
		pAccItem->nCount = nNewCount;
	}
	*/
	
	MMatchAccItem *pAccItem = GetAccItemByAIID(nAIID);
	if(pAccItem == NULL) return false;
	
	int nNewCount = pAccItem->nCount - nCount;
	if(nNewCount < 0) return false;
	
	AsyncDb_MoveAItemToCItem(m_uid, nAIID, m_Char.nCID, nCount, *pAccItem);
	
	if(nNewCount == 0)
	{
		RemoveAccItem(nAIID);
	}
	else
	{
		pAccItem->nCount = nNewCount;
	}
	
	return true;
}

// char item.
void MMatchObject::AddCharItem(const MUID &uid, int nCIID, int nItemID, int nItemCount, int nRentPeriod)
{
	MMatchCharItem *pNew = new MMatchCharItem;
	
	pNew->uid = uid;
	pNew->nCIID = nCIID;
	pNew->nID = nItemID;
	pNew->nCount = nItemCount;
	pNew->nRentSecPeriod = nRentPeriod;
	pNew->nMadeTime = GetTime();
	
	m_ItemList.push_back(pNew);
}

void MMatchObject::RemoveCharItem(const MUID &uid)
{
	for(list<MMatchCharItem *>::iterator i = m_ItemList.begin(); i != m_ItemList.end(); i++)
	{
		MMatchCharItem *pCurrItem = (*i);
		
		if(pCurrItem->uid == uid)
		{
			delete pCurrItem;
			m_ItemList.erase(i);
			break;
		}
	}
}

void MMatchObject::RemoveCharItem(int nCIID)
{
	for(list<MMatchCharItem *>::iterator i = m_ItemList.begin(); i != m_ItemList.end(); i++)
	{
		MMatchCharItem *pCurrItem = (*i);
		
		if(pCurrItem->nCIID == nCIID)
		{
			delete pCurrItem;
			m_ItemList.erase(i);
			break;
		}
	}
}

MMatchCharItem *MMatchObject::GetCharItem(const MUID &uid)
{
	for(list<MMatchCharItem *>::iterator i = m_ItemList.begin(); i != m_ItemList.end(); i++)
	{
		MMatchCharItem *pCurrItem = (*i);
		if(pCurrItem->uid == uid) return pCurrItem;
	}
	return NULL;
}

MMatchCharItem *MMatchObject::GetCharItemByItemID(int nItemID)
{
	for(list<MMatchCharItem *>::iterator i = m_ItemList.begin(); i != m_ItemList.end(); i++)
	{
		MMatchCharItem *pCurrItem = (*i);
		if(pCurrItem->nID == nItemID) return pCurrItem;
	}
	return NULL;
}

MMatchCharItem *MMatchObject::GetCharItemByCIID(int nCIID)
{
	for(list<MMatchCharItem *>::iterator i = m_ItemList.begin(); i != m_ItemList.end(); i++)
	{
		MMatchCharItem *pCurrItem = (*i);
		if(pCurrItem->nCIID == nCIID) return pCurrItem;
	}
	return NULL;
}

MMatchCharItem *MMatchObject::GetCharItemBySlotIndex(int nSlotIndex)
{
	if(nSlotIndex < 0 || nSlotIndex >= MMCIP_END) return NULL;
	return GetCharItem(m_uidEquippedItem[nSlotIndex]);
}

void MMatchObject::ItemExpired(int nCIID)
{
	MMatchCharItem *pCharItem = GetCharItemByCIID(nCIID);
	if(pCharItem == NULL) return;
	
	for(int i = 0; i < MMCIP_END; i++)
	{
		if(pCharItem->uid == m_uidEquippedItem[i])
		{
			// Db_TakeoffItem(m_Char.nCID, i);
			AsyncDb_TakeoffItem(m_uid, m_Char.nCID, i);
			SetItemUID(MUID(0, 0), i);
		}
	}
	
	RemoveCharItem(nCIID);
}

void MMatchObject::ClearCharItemList()
{
	for(list<MMatchCharItem *>::iterator i = m_ItemList.begin(); i != m_ItemList.end(); i++)
	{
		delete (*i);
	}
	m_ItemList.clear();
}

void MMatchObject::AddCharGambleItem(const MUID &uid, int nGIID, int nItemID, int nCount)
{
	MMatchCharGambleItem *pNew = new MMatchCharGambleItem;
	
	pNew->uid = uid;
	pNew->nGIID = nGIID;
	pNew->nItemID = nItemID;
	pNew->nCount = nCount;
	
	m_vtGambleItem.push_back(pNew);
}

void MMatchObject::RemoveCharGambleItem(const MUID &uid)
{
	for(vector<MMatchCharGambleItem *>::iterator i = m_vtGambleItem.begin(); i != m_vtGambleItem.end(); i++)
	{
		MMatchCharGambleItem *pCurr = (*i);
		
		if(pCurr->uid == uid)
		{
			delete pCurr;
			m_vtGambleItem.erase(i);
			break;
		}
	}
}

void MMatchObject::RemoveCharGambleItem(int nGIID)
{
	for(vector<MMatchCharGambleItem *>::iterator i = m_vtGambleItem.begin(); i != m_vtGambleItem.end(); i++)
	{
		MMatchCharGambleItem *pCurr = (*i);
		
		if(pCurr->nGIID == nGIID)
		{
			delete pCurr;
			m_vtGambleItem.erase(i);
			break;
		}
	}
}

MMatchCharGambleItem *MMatchObject::GetCharGambleItem(const MUID &uid)
{
	for(vector<MMatchCharGambleItem *>::iterator i = m_vtGambleItem.begin(); i != m_vtGambleItem.end(); i++)
	{
		MMatchCharGambleItem *pCurr = (*i);
		if(pCurr->uid == uid) return pCurr;
	}
	return NULL;
}

MMatchCharGambleItem *MMatchObject::GetCharGambleItemByItemID(int nItemID)
{
	for(vector<MMatchCharGambleItem *>::iterator i = m_vtGambleItem.begin(); i != m_vtGambleItem.end(); i++)
	{
		MMatchCharGambleItem *pCurr = (*i);
		if(pCurr->nItemID == nItemID) return pCurr;
	}
	return NULL;
}

MMatchCharGambleItem *MMatchObject::GetCharGambleItemByGIID(int nGIID)
{
	for(vector<MMatchCharGambleItem *>::iterator i = m_vtGambleItem.begin(); i != m_vtGambleItem.end(); i++)
	{
		MMatchCharGambleItem *pCurr = (*i);
		if(pCurr->nGIID == nGIID) return pCurr;
	}
	return NULL;
}

void MMatchObject::ClearAllCharGambleItem()
{
	for(vector<MMatchCharGambleItem *>::iterator i = m_vtGambleItem.begin(); i != m_vtGambleItem.end(); i++)
	{
		delete (*i);
	}
	m_vtGambleItem.clear();
}

bool MMatchObject::AddFriend(int nID, int nFriendCID, const char *pszCharName)
{
	#define MAX_FRIEND_LIST_COUNT	40
	
	// if(m_FriendList.size() >= MAX_FRIEND_LIST_COUNT) return false;
	if(IsFriendListMax() == true) return false;
	
	MMatchFriend *pNew = new MMatchFriend;
	pNew->nFriendID = nID;
	pNew->nCID = nFriendCID;
	strcpy(pNew->szName, pszCharName);
	
	m_FriendList.push_back(pNew);
	
	return true;
}

bool MMatchObject::RemoveFriend(int nFriendID)
{
	for(list<MMatchFriend *>::iterator i = m_FriendList.begin(); i != m_FriendList.end(); i++)
	{
		MMatchFriend *pCurr = (*i);
		
		if(pCurr->nFriendID == nFriendID)
		{
			delete pCurr;
			m_FriendList.erase(i);
			
			return true;
		}
	}
	
	return false;
}

void MMatchObject::ClearFriendList()
{
	for(list<MMatchFriend *>::iterator i = m_FriendList.begin(); i != m_FriendList.end(); i++)
	{
		delete (*i);
	}
	
	m_FriendList.clear();
}

bool MMatchObject::IsFriendListMax()
{
	if(m_FriendList.size() >= MAX_FRIEND_LIST_COUNT) return true;
	return false;
}

bool MMatchObject::CheckFriendExists(int nFriendCID)
{
	for(list<MMatchFriend *>::iterator i = m_FriendList.begin(); i != m_FriendList.end(); i++)
	{
		MMatchFriend *pCurr = (*i);
		
		if(pCurr->nCID == nFriendCID)
		{
			return true;
		}
	}
	
	return false;
}

MMatchFriend *MMatchObject::GetFrind(const char *pszCharName)
{
	for(list<MMatchFriend *>::iterator i = m_FriendList.begin(); i != m_FriendList.end(); i++)
	{
		MMatchFriend *pCurr = (*i);
		
		if(MStricmp(pCurr->szName, pszCharName) == 0)
		{
			return pCurr;
		}
	}
	
	return NULL;
}

void MMatchObject::UpdateSpawnTime()
{
	m_nSpawnTimer = GetTime() + 7000;	// there is 7 secs to next revival.
}

bool MMatchObject::IsValidSpawn()
{
	if(m_GameInfo.bAlive == true) return true;	// allow force spawning when alive.
	if(m_nSpawnTimer <= GetTime()) return true;
	return false;
}

void MMatchObject::AttachChatRoom(unsigned long nID)
{
	if(CheckChatRoomAttached(nID) == true) return;	// already there.
	m_vtChatRoomID.push_back(nID);
}

void MMatchObject::DetachChatRoom(unsigned long nID)
{
	for(vector<unsigned long>::iterator i = m_vtChatRoomID.begin(); i != m_vtChatRoomID.end(); i++)
	{
		if((*i) == nID)
		{
			m_vtChatRoomID.erase(i);
			break;
		}
	}
}

bool MMatchObject::CheckChatRoomAttached(unsigned long nID)
{
	for(vector<unsigned long>::iterator i = m_vtChatRoomID.begin(); i != m_vtChatRoomID.end(); i++)
	{
		if((*i) == nID) return true;
	}
	return false;
}

void MMatchObject::InitQuestItem(const char *pszData)
{
	RestoreQuestItemData(pszData, &m_QuestItemList);
}

void MMatchObject::SaveQuestItem()
{
	if(m_bCharInfoExist == true)
	{
		char szData[1024];
		BuildQuestItemData(&m_QuestItemList, szData);
	
		// Db_UpdateCharQuestItem(m_Char.nCID, szData);
		AsyncDb_UpdateCharQuestItem(m_uid, m_Char.nCID, szData);
	}
}

MMatchQuestItem *MMatchObject::GetQuestItem(int nItemID)
{
	for(list<MMatchQuestItem>::iterator i = m_QuestItemList.begin(); i != m_QuestItemList.end(); i++)
	{
		MMatchQuestItem *pCurr = &(*i);
		if(pCurr->nItemID == nItemID) return pCurr;
	}
	return NULL;
}

bool MMatchObject::AddQuestItem(int nItemID, int nCount)
{
	// #define MAX_QUEST_ITEM_COUNT	99
	
	MMatchQuestItem *pQuestItem = GetQuestItem(nItemID);
	if(pQuestItem == NULL)
	{
		// add.
		if(nCount <= 0 || nCount > MAX_QUEST_ITEM_COUNT) return false;
		
		MMatchQuestItem item = {nItemID, nCount};
		m_QuestItemList.push_back(item);
	}
	else
	{
		// update,
		int nNewCount = pQuestItem->nItemCount + nCount;
		if(nNewCount <= 0 || nNewCount > MAX_QUEST_ITEM_COUNT) return false;
		
		pQuestItem->nItemCount = nNewCount;
	}
	
	return true;
}

bool MMatchObject::SubQuestItem(int nItemID, int nCount)
{
	if(nCount <= 0 || nCount > MAX_QUEST_ITEM_COUNT) return false;
	
	for(list<MMatchQuestItem>::iterator i = m_QuestItemList.begin(); i != m_QuestItemList.end(); i++)
	{
		MMatchQuestItem *pCurr = &(*i);
		
		if(pCurr->nItemID == nItemID)
		{
			int nNewCount = pCurr->nItemCount - nCount;
			if(nNewCount < 0) return false;
			
			if(nNewCount == 0)
			{
				m_QuestItemList.erase(i);
			}
			else
			{
				pCurr->nItemCount = nNewCount;
			}
			
			return true;
		}
	}
	
	return false;
}

// for Quest Items.
bool MMatchObject::RestoreQuestItemData(const char *pszSrcData, list<MMatchQuestItem> *pDstList)
{
	pDstList->clear();
	
	// data format is, [ItemID 1]=[ItemCount 1];[ItemID 2]=[ItemCount 2]; ...
	
	int nTemp = 0;
	char szTemp[32] = "";
	
	for(int i = 0; pszSrcData[i] != '\0'; i++)
	{
		if(pszSrcData[i] != ';')
		{
			szTemp[nTemp] = pszSrcData[i];
			nTemp++;
		}
		else
		{
			szTemp[nTemp] = '\0';
			
			int nID, nCount;
			if(sscanf(szTemp, "%d=%d", &nID, &nCount) != 2)
			{
				pDstList->clear();
				return false;
			}
			
			MMatchQuestItem item = {nID, nCount};
			pDstList->push_back(item);
			
			nTemp = 0;
		}
	}
	
	return true;
}

bool MMatchObject::BuildQuestItemData(list<MMatchQuestItem> *pSrcList, char *pszDstData)
{
	pszDstData[0] = '\0';
	
	for(list<MMatchQuestItem>::iterator i = pSrcList->begin(); i != pSrcList->end(); i++)
	{
		MMatchQuestItem *pCurr = &(*i);
		
		char szTemp[32];
		sprintf(szTemp, "%d=%d;", pCurr->nItemID, pCurr->nItemCount);
		
		strcat(pszDstData, szTemp);
	}
	
	return true;
}

void MMatchObject::AddCQRecord(int nScenarioID, int nTime)
{
	MMatchChallengeQuestRecordInfo *pCurrRecord = GetCQRecord(nScenarioID);
	if(pCurrRecord == NULL)
	{
		MMatchChallengeQuestRecordInfo info = {nScenarioID, nTime};
		m_vtCQRecordInfo.push_back(info);
	}
	else
	{
		if(pCurrRecord->nTime > nTime)
		{
			pCurrRecord->nTime = nTime;
		}
	}
}

MMatchChallengeQuestRecordInfo *MMatchObject::GetCQRecord(int nScenarioID)
{
	for(vector<MMatchChallengeQuestRecordInfo>::iterator i = m_vtCQRecordInfo.begin(); i != m_vtCQRecordInfo.end(); i++)
	{
		MMatchChallengeQuestRecordInfo *pCurr = &(*i);
		if(pCurr->nScenarioID == nScenarioID) return pCurr;
	}
	
	return NULL;
}

bool MMatchObject::UpdateCmdRecvTime()
{
	#ifdef _DEBUG
		return true;
	#endif
	
	#define MAX_COMMAND_RECVTIME_RECORD_COUNT		50	// will check flooding or not when this count reached.
	#define MAX_COMMAND_RECVTIME_ALLOWED_INTERVAL	(MAX_COMMAND_RECVTIME_RECORD_COUNT * 100)	// 100 ms per one command is probably enough.
	
	if(m_bFlooding == true) return false;
	
	unsigned long nCurrTime = GetTime();
	m_CmdRecvTime.push_back(nCurrTime);
	
	if(m_CmdRecvTime.size() <= MAX_COMMAND_RECVTIME_RECORD_COUNT) return true;
	
	unsigned long nFrontTime = m_CmdRecvTime.front();
	m_CmdRecvTime.pop_front();
	
	unsigned long nTimeDiff = nCurrTime - nFrontTime;
	if(nTimeDiff < MAX_COMMAND_RECVTIME_ALLOWED_INTERVAL) m_bFlooding = true;
	
	return !m_bFlooding;
}

void MMatchObject::StartMedalBonus()
{
	m_nAddedBonusMedal = 0;
	
	m_nNextMedalBonusTime = GetTime() + BLITZ_MEDAL_BONUS_TIME_INTERVAL;
	m_bDistribMedalBonus = true;
}

void MMatchObject::StopMedalBonus()
{
	m_bDistribMedalBonus = false;
	
	m_BlitzKrieg.nMedal += m_nAddedBonusMedal;
	SyncMedal(m_nAddedBonusMedal);
}

void MMatchObject::SyncMedal()
{
	SendBlitzScore(BLITZ_SCORETYPE_MEDAL_UPDATE, m_BlitzKrieg.nMedal, 0);
}

void MMatchObject::SyncMedal(int diff)
{
	SendBlitzScore(BLITZ_SCORETYPE_MEDAL_ADD, diff, 0);
}

void MMatchObject::BlitzPenalty(int nLostMedal, int nPoint)
{
	SendBlitzScore(BLITZ_SCORETYPE_PENALTY, -nLostMedal, nPoint);
}

void MMatchObject::SendBlitzScore(int nScoreType, int nMedal, int nPoint)
{
	MCmdWriter Cmd;
	Cmd.WriteInt(nScoreType);
	Cmd.WriteInt(nMedal);
	Cmd.WriteInt(nPoint);
	Cmd.Finalize(MC_BLITZ_UPDATE_SCORE, MCFT_END);
	SendToClient(&Cmd, m_uid);
}

void MMatchObject::ReserveStageKick(unsigned long nReserveTime)
{
	m_nStageKickTime = GetTime() + nReserveTime;
	m_bStageKickReserved = true;
}

void MMatchObject::CancelStageKick()
{
	m_bStageKickReserved = false;
}

void MMatchObject::SyncCash(bool bSave)
{
	if(bSave == true)
	{
		AsyncDb_SetAccountCash(m_uid, m_Account.nAID, m_Account.nCash);
	}
	
	MCmdWriter Cmd;
	Cmd.WriteUInt((unsigned int)m_Account.nCash);
	Cmd.Finalize(MC_CASHSHOP_RESPONSE_CASH_REMAINDER, MCFT_END);
	SendToClient(&Cmd, m_uid);
}


MMatchObjectManager::MMatchObjectManager()
{
}

MMatchObjectManager::~MMatchObjectManager()
{
	EraseAll();
}

MMatchObject *MMatchObjectManager::Add(Socket::socket_type s, const MUID &uid, const unsigned char *pCryptKey)
{
	MMatchObject *pNewObj = new MMatchObject(s, uid, pCryptKey);
	m_Objects.push_back(pNewObj);
	return pNewObj;
}

MMatchObject *MMatchObjectManager::Get(Socket::socket_type s)
{
	for(list<MMatchObject *>::iterator it = m_Objects.begin();
	        it != m_Objects.end(); it++)
	{
		MMatchObject *pCurr = (*it);
		
		if(pCurr->GetSocket() == s)
			return pCurr;
	}
	return NULL;
}

MMatchObject *MMatchObjectManager::Get(const MUID &uid)
{
	for(list<MMatchObject *>::iterator it = m_Objects.begin();
	        it != m_Objects.end(); it++)
	{
		MMatchObject *pCurr = (*it);
		
		if(pCurr->GetUID() == uid)
			return pCurr;
	}
	return NULL;
}

MMatchObject *MMatchObjectManager::Get(const char *pszCharName)
{
	for(list<MMatchObject *>::iterator it = m_Objects.begin();
	        it != m_Objects.end(); it++)
	{
		MMatchObject *pCurr = (*it);
		
		if(MStricmp(pCurr->m_Char.szName, pszCharName) == 0)
			return pCurr;
	}
	return NULL;
}

void MMatchObjectManager::Erase(Socket::socket_type s)
{
	for(list<MMatchObject *>::iterator it = m_Objects.begin();
	        it != m_Objects.end(); it++)
	{
		MMatchObject *pCurr = (*it);
		
		if(pCurr->GetSocket() == s)
		{
			delete pCurr;
			m_Objects.erase(it);
			break;
		}
	}
}

void MMatchObjectManager::Erase(const MUID &uid)
{
	for(list<MMatchObject *>::iterator it = m_Objects.begin();
	        it != m_Objects.end(); it++)
	{
		MMatchObject *pCurr = (*it);
		
		if(pCurr->GetUID() == uid)
		{
			delete pCurr;
			m_Objects.erase(it);
			break;
		}
	}
}

void MMatchObjectManager::EraseAll()
{
	for(list<MMatchObject *>::iterator it = m_Objects.begin();
	        it != m_Objects.end(); it++)
		delete (*it);
		
	m_Objects.clear();
}

bool MMatchObjectManager::IsAvailable(const MUID &uid)
{
	for(list<MMatchObject *>::iterator it = m_Objects.begin(); it != m_Objects.end(); it++)
	{
		MMatchObject *pCurr = (*it);
		if(pCurr->GetUID() == uid) return true;
	}
	return false;
}

bool MMatchObjectManager::IsAvailable(MMatchObject *pObj)
{
	for(list<MMatchObject *>::iterator it = m_Objects.begin(); it != m_Objects.end(); it++)
	{
		MMatchObject *pCurr = (*it);
		if(pCurr == pObj) return true;
	}
	return false;
}
