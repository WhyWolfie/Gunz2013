#include "pch.h"

#include "MMatchObject.h"
#include "MMatchObject_Constant.h"

#include "MMatchItem.h"

#include "MMessageID.h"

#include "MMatchServer_OnCommand.h"
#include "MMatchServer_Etc.h"

#include "MMatchDBMgr.h"
#include "MAsyncDBProcess.h"

#include "MMatchGambleItem.h"

#include "MMatchBlitzKrieg.h"

void CheckExpiredCharItem(MMatchObject *pObj)
{
	// clears expired char rent item. but for additional, also clears account rent item.
	
	/*
	vector<DbData_ExpiredItemNode> vtItemInfo;
	if(Db_ClearExpiredItem(pObj->m_Char.nCID, &vtItemInfo) == false) return;
	
	vector<DbData_ExpiredItemNode> vtAItemInfo;
	if(Db_ClearExpiredAccountItem(pObj->m_Account.nAID, &vtAItemInfo) == false) return;
	
	if(vtItemInfo.empty() == true && vtAItemInfo.empty() == true) return;
	
	MCmdWriter Cmd;
	
	Cmd.StartBlob(sizeof(unsigned long));
	// char item.
	for(vector<DbData_ExpiredItemNode>::iterator i = vtItemInfo.begin(); i != vtItemInfo.end(); i++)
	{
		DbData_ExpiredItemNode *pNode = &(*i);
		
		pObj->ItemExpired(pNode->nCIID);
		Cmd.WriteULong((unsigned long)pNode->nItemID);
	}
	// acc item.
	for(vector<DbData_ExpiredItemNode>::iterator i = vtAItemInfo.begin(); i != vtAItemInfo.end(); i++)
	{
		DbData_ExpiredItemNode *pNode = &(*i);
		
		pObj->AccItemExpired(pNode->nCIID);	// = AIID.
		Cmd.WriteULong((unsigned long)pNode->nItemID);
	}
	Cmd.EndBlob();
	
	Cmd.Finalize(MC_MATCH_EXPIRED_RENT_ITEM, MCFT_END);
	SendToClient(&Cmd, pObj->GetUID());
	
	OnCharacterItemList(pObj->GetUID());
	OnAccountItemList(pObj->GetUID());
	*/
	
	AsyncDb_ClearExpiredItem(pObj->GetUID(), pObj->m_Account.nAID, pObj->m_Char.nCID);
}

// global funcs for friend.
int CheckFriendAddable(MMatchObject *pObj, MMatchObject *pTargetObj)
{
	if(pTargetObj->m_bCharInfoExist == false)
	{
		return MATCHNOTIFY_GENERAL_USER_NOTFOUND;
	}
	
	if(pObj->CheckFriendExists(pTargetObj->m_Char.nCID) == true)
	{
		return MATCHNOTIFY_FRIEND_ALREADY_EXIST;
	}
	
	if(pObj->IsFriendListMax() == true)
	{
		return MATCHNOTIFY_FRIEND_TOO_MANY_ADDED;
	}
	
	return MSG_OK;
}

/*
int CheckFriendRemovable(MMatchObject *pObj, int nFriendCID)
{
	if(pObj->CheckFriendExists(nFriendCID) == false)
	{
		return MATCHNOTIFY_FRIEND_NOT_EXIST;
	}
	
	return MSG_OK;
}
*/
// ----------.

void OnUserOption(const MUID &uidPlayer, unsigned long nFlag)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	pObj->m_nOptionFlag = nFlag;
}

void OnUserWhisper(const MUID &uidSender, const char *pszTarget, const char *pszMessage)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidSender);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	MCmdWriter Cmd;
	
	if(strlen(pszTarget) == 0)
	{
		Cmd.WriteUInt(MATCHNOTIFY_GENERAL_USER_NOTFOUND);
		Cmd.Finalize(MC_MATCH_NOTIFY, MCFT_END);
		SendToClient(&Cmd, uidSender);
		return;
	}
	
	MMatchObject *pTargetObj = g_ObjectMgr.Get(pszTarget);
	if(pTargetObj == NULL)
	{
		Cmd.WriteUInt(MATCHNOTIFY_GENERAL_USER_NOTFOUND);
		Cmd.Finalize(MC_MATCH_NOTIFY, MCFT_END);
		SendToClient(&Cmd, uidSender);
		return;
	}
	
	if(pTargetObj->m_bCharInfoExist == false)
	{
		Cmd.WriteUInt(MATCHNOTIFY_GENERAL_USER_NOTFOUND);
		Cmd.Finalize(MC_MATCH_NOTIFY, MCFT_END);
		SendToClient(&Cmd, uidSender);
		return;
	}
	
	if((pTargetObj->m_nOptionFlag & MBITFLAG_USEROPTION_REJECT_WHISPER) != 0)
	{
		Cmd.WriteUInt(MATCHNOTIFY_USER_WHISPER_REJECTED);
		Cmd.Finalize(MC_MATCH_NOTIFY, MCFT_END);
		SendToClient(&Cmd, uidSender);
		return;
	}
	
	Cmd.WriteString(pObj->m_Char.szName);
	Cmd.WriteString(pTargetObj->m_Char.szName);
	Cmd.WriteString(pszMessage);
	Cmd.Finalize(MC_MATCH_USER_WHISPER, MCFT_END);
	SendToClient(&Cmd, pTargetObj->GetUID());
}

void OnCharacterItemList(const MUID &uidPlayer)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	// g_GItemMgr.SendInfo(uidPlayer);
	
	MCmdWriter Cmd;
	
	Cmd.WriteInt(pObj->m_Exp.nBP);	// bounty.
	
	// equip item.
	Cmd.StartBlob(sizeof(MUID));
	for(int i = 0; i < MMCIP_END; i++)
	{
		Cmd.WriteMUID(pObj->m_uidEquippedItem[i]);
	}
	Cmd.EndBlob();
	
	#if _GAME_VERSION >= 2012
	Cmd.StartBlob(sizeof(MTD_ItemNode2012));
	for(list<MMatchCharItem *>::iterator i = pObj->m_ItemList.begin(); i != pObj->m_ItemList.end(); i++)
	{
		MMatchCharItem *pCharItem = (*i);
		
		MMatchItem *pItem = g_ItemMgr.Get(pCharItem->nID);
		if(pItem == NULL) continue;
		
		int nRentPeriodMinute = pCharItem->CalcRentSecPeriod() / 60;
		
		int nMedalPrice = 0;
		
		MBlitzShopItem *pMedalItem = g_BlitzShop.GetItem(pCharItem->nID);
		if(pMedalItem != NULL)
		{
			nMedalPrice = pMedalItem->m_nPrice;
			if(pMedalItem->m_bCoupon == false) nMedalPrice /= 5;
		}
		
		MTD_ItemNode2012 node;
		node.uidItem = pCharItem->uid;
		node.nItemID = (unsigned long)pCharItem->nID;
		node.nRentMinutePeriodRemainder = nRentPeriodMinute == 0 ? RENT_MINUTE_PERIOD_UNLIMITED : nRentPeriodMinute;	// remain rent period. (minute.)
		node.nMaxUseHour = pItem->GetRentPeriod();
		node.nCount = pCharItem->nCount;
		node.nMedalPrice = nMedalPrice;
		
		Cmd.WriteData(&node, sizeof(MTD_ItemNode2012));
	}
	Cmd.EndBlob();
	#else
	// all item.
	Cmd.StartBlob(sizeof(MTD_ItemNode));
	for(list<MMatchCharItem *>::iterator i = pObj->m_ItemList.begin(); i != pObj->m_ItemList.end(); i++)
	{
		MMatchCharItem *pCharItem = (*i);
		
		MMatchItem *pItem = g_ItemMgr.Get(pCharItem->nID);
		if(pItem == NULL) continue;
		
		int nRentPeriodMinute = pCharItem->CalcRentSecPeriod() / 60;
		
		MTD_ItemNode node;
		node.uidItem = pCharItem->uid;
		node.nItemID = (unsigned long)pCharItem->nID;
		node.nRentMinutePeriodRemainder = nRentPeriodMinute == 0 ? RENT_MINUTE_PERIOD_UNLIMITED : nRentPeriodMinute;	// remain rent period. (minute.)
		node.nMaxUseHour = pItem->GetRentPeriod();
		node.nCount = pCharItem->nCount;
		
		Cmd.WriteData(&node, sizeof(MTD_ItemNode));
	}
	Cmd.EndBlob();
	#endif
	
	Cmd.StartBlob(sizeof(MTD_GambleItemNode));
	for(vector<MMatchCharGambleItem *>::iterator i = pObj->m_vtGambleItem.begin(); i != pObj->m_vtGambleItem.end(); i++)
	{
		MMatchCharGambleItem *pCurr = (*i);
		
		MTD_GambleItemNode node;
		ZeroInit(&node, sizeof(MTD_GambleItemNode));
		
		node.uidItem = pCurr->uid;
		node.nItemID = (unsigned int)pCurr->nItemID;
		node.nItemCnt = (unsigned int)pCurr->nCount;
		
		Cmd.WriteData(&node, sizeof(MTD_GambleItemNode));
	}
	Cmd.EndBlob();
	
	Cmd.Finalize(MC_MATCH_RESPONSE_CHARACTER_ITEMLIST, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
	
	// send quest item list.
	OnCharQuestItemList(uidPlayer);
	
	// refresh shop list and keep to show correct info.
	// OnShopItemList(uidPlayer);
}

void OnMySimpleCharInfo(const MUID &uidPlayer)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	MCmdWriter Cmd;
	
	MTD_MySimpleCharInfo info;
	info.nLevel = (unsigned char)pObj->m_Exp.nLevel;
	info.nXP = (unsigned long)pObj->m_Exp.nXP;
	info.nBP = pObj->m_Exp.nBP;
	
	Cmd.StartBlob(sizeof(MTD_MySimpleCharInfo));
	Cmd.WriteData(&info, sizeof(MTD_MySimpleCharInfo));
	Cmd.EndBlob();
	
	Cmd.Finalize(MC_MATCH_RESPONSE_MY_SIMPLE_CHARINFO, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
}

void OnUserWhere(const MUID &uidPlayer, const char *pszTargetName)
{
	MCmdWriter Cmd;
	
	MMatchObject *pTargetObj = g_ObjectMgr.Get(pszTargetName);
	if(pTargetObj == NULL)
	{
		Cmd.WriteUInt(MATCHNOTIFY_GENERAL_USER_NOTFOUND);
		Cmd.Finalize(MC_MATCH_NOTIFY, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	if(pTargetObj->m_bCharInfoExist == false)
	{
		Cmd.WriteUInt(MATCHNOTIFY_GENERAL_USER_NOTFOUND);
		Cmd.Finalize(MC_MATCH_NOTIFY, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	// char.
	char szCharName[256] = "----";
	strcpy(szCharName, pTargetObj->m_Char.szName);
	
	// channel.
	char szChannel[256] = "?";
	
	MMatchChannel *pChannel = g_ChannelMgr.Get(pTargetObj->m_uidChannel);
	if(pChannel != NULL)
	{
		strcpy(szChannel, pChannel->GetName());
	}
	
	// stage & message.
	char szMsg[1024];
	
	MMatchStage *pStage = g_StageMgr.Get(pTargetObj->m_uidStage);
	if(pStage == NULL)
	{
		sprintf(szMsg, "[%s] %s", szCharName, szChannel);
	}
	else
	{
		sprintf(szMsg, "[%s] %s - (%d)%s", szCharName, szChannel, pStage->GetNumber(), pStage->GetName());
	}
	
	AnnounceToClient(szMsg, uidPlayer);
}

void OnFriendList(const MUID &uidPlayer)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	vector<MFRIENDLISTNODE> vtFriendListNode;
	
	for(list<MMatchFriend *>::iterator i = pObj->m_FriendList.begin(); i != pObj->m_FriendList.end(); i++)
	{
		MMatchFriend *pCurrFriend = (*i);
		
		MFRIENDLISTNODE node;
		ZeroInit(&node, sizeof(MFRIENDLISTNODE));
		
		node.nState = MMP_OUTSIDE;
		strcpy(node.szName, pCurrFriend->szName);
		
		for(list<MMatchObject *>::iterator j = g_ObjectMgr.Begin(); j != g_ObjectMgr.End(); j++)
		{
			MMatchObject *pCurrObj = (*j);
			if(pCurrObj->m_bCharInfoExist == false) continue;
			
			if(pCurrObj->m_Char.nCID == pCurrFriend->nCID)
			{
				node.nState = (unsigned char)pCurrObj->m_nPlace;
				break;
			}
		}
		
		vtFriendListNode.push_back(node);
	}
	
	MCmdWriter Cmd;
	
	Cmd.StartBlob(sizeof(MFRIENDLISTNODE));
	for(vector<MFRIENDLISTNODE>::iterator i = vtFriendListNode.begin(); i != vtFriendListNode.end(); i++)
	{
		Cmd.WriteData(&(*i), sizeof(MFRIENDLISTNODE));
	}
	Cmd.EndBlob();
	
	Cmd.Finalize(MC_MATCH_RESPONSE_FRIENDLIST, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
}

void OnFriendAdd(const MUID &uidPlayer, const char *pszCharName)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	MMatchObject *pTargetObj = g_ObjectMgr.Get(pszCharName);
	if(pTargetObj == NULL)
	{
		MCmdWriter tmp;
		tmp.WriteInt(MATCHNOTIFY_GENERAL_USER_NOTFOUND);
		tmp.Finalize(MC_MATCH_NOTIFY, MCFT_END);
		SendToClient(&tmp, uidPlayer);
		return;
	}
	
	MCmdWriter Cmd;
	
	int nRet = CheckFriendAddable(pObj, pTargetObj);
	
	if(nRet != MSG_OK)
	{
		Cmd.WriteInt(nRet);
		Cmd.Finalize(MC_MATCH_NOTIFY, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	int nFriendID;
	
	if(Db_AddFriend(pObj->m_Char.nCID, pTargetObj->m_Char.nCID, &nFriendID) == false)
	{
		// db error.
		return;
	}
	
	pObj->AddFriend(nFriendID, pTargetObj->m_Char.nCID, pTargetObj->m_Char.szName);
	
	Cmd.WriteInt(MATCHNOTIFY_FRIEND_ADD_SUCCEED);
	Cmd.Finalize(MC_MATCH_NOTIFY, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
	
	OnFriendList(uidPlayer);
}

void OnFriendRemove(const MUID &uidPlayer, const char *pszCharName)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	MMatchFriend *pFriend = pObj->GetFrind(pszCharName);
	if(pFriend == NULL)
	{
		MCmdWriter tmp;
		tmp.WriteInt(MATCHNOTIFY_FRIEND_NOT_EXIST);
		tmp.Finalize(MC_MATCH_NOTIFY, MCFT_END);
		SendToClient(&tmp, uidPlayer);
		return;
	}
	
	/*
	if(Db_RemoveFriend(pFriend->nFriendID) == false)
	{
		// db error.
		return;
	}
	*/
	AsyncDb_RemoveFriend(uidPlayer, pFriend->nFriendID);
	
	pObj->RemoveFriend(pFriend->nFriendID);
	
	MCmdWriter Cmd;
	Cmd.WriteInt(MATCHNOTIFY_FRIEND_REMOVE_SUCCEED);
	Cmd.Finalize(MC_MATCH_NOTIFY, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
	
	OnFriendList(uidPlayer);
}