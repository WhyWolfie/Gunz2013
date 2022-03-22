#include "pch.h"

#include <set>
using namespace std;

#include "MMatchStage.h"
#include "MMatchObject.h"
#include "MMatchChannel.h"

#include "MMessageID.h"

#include "MMatchGame.h"
#include "MMatchExp.h"

#include "MMatchMap.h"

#include "MIPReplacer.h"

#include "MMatchServer_Etc.h"
#include "MMatchClan.h"

#include "MMatchQuest.h"

#include "MMatchCashShop.h"

#include "MMatchServer_OnCommand.h"

void SendChannelPlayerList(const MUID &uidChannel);	// from channel command.
void SendStageList(const MUID &uidChannel);

// from quest command.
void SendQuestStageInfoToStage(const MUID &uidStage, int nScenarioID);
void SendQuestStageInfoToClient(const MUID &uidPlayer, int nScenarioID);

// from char command.
void CheckExpiredCharItem(MMatchObject *pObj);

#define MAX_CHANNEL_STAGE_COUNT	100

int AssignStageNumber(const MUID &uidChannel)	// return value -1 = error.
{
	multiset<int> StageNumbers;
	
	for(list<MMatchStage *>::iterator i = g_StageMgr.Begin(); i != g_StageMgr.End(); i++)
	{
		MMatchStage *pCurr = (*i);
		
		if(pCurr->GetChannelUID() == uidChannel)
			StageNumbers.insert(pCurr->GetNumber());
	}
	
	int nRes = -1;
	multiset<int>::iterator itEnd = StageNumbers.end();
	
	for(int i = 1; i <= MAX_CHANNEL_STAGE_COUNT; i++)
	{
		if(StageNumbers.find(i) != itEnd) continue;
		nRes = i; break;
	}
	
	return nRes;
}

// uidDestPlayer can be MUID(0, 0) - all players.
void SendObjectCache(int nCacheType, MMatchStage *pSrcStage, const MUID &uidDestPlayer)
{
	MCmdWriter Cmd;
	Cmd.WriteUChar((unsigned char)nCacheType);
	
	Cmd.StartBlob(sizeof(MTD_ObjectCache) + 4);
	for(map<MUID, MMatchObject *>::iterator it = pSrcStage->ObjBegin(); it != pSrcStage->ObjEnd(); it++)
	{
		MMatchObject *pCurrObj = (*it).second;
		
		char szClanName[DB_CLANNAME_LEN] = "";
		unsigned int nEmblemChecksum = 0;
		
		MMatchClan *pClan = g_ClanMgr.Get(pCurrObj->m_Clan.nCLID);
		if(pClan != NULL)
		{
			strcpy(szClanName, pClan->GetName());
			nEmblemChecksum = pClan->GetEmblemChecksum();
		}
		
		MTD_ObjectCache cache;
		ZeroInit(&cache, sizeof(MTD_ObjectCache));
		
		cache.uidObject = pCurrObj->GetUID();
		strcpy(cache.szName, pCurrObj->m_Char.szName);
		strcpy(cache.szClanName, szClanName);
		cache.nLevel = (char)pCurrObj->m_Exp.nLevel;
		cache.nUGrade = pCurrObj->m_Account.nUGradeID;
		cache.nPGrade = pCurrObj->m_Account.nPGradeID;
		cache.nPlayerFlags = (unsigned char)pCurrObj->m_nPlayerFlag;
		cache.nCLID = pCurrObj->m_Clan.nCLID;
		cache.nEmblemChecksum = nEmblemChecksum;
		cache.nSex = pCurrObj->m_Char.nSex;
		cache.nHair = pCurrObj->m_Char.nHair;
		cache.nFace = pCurrObj->m_Char.nFace;
		for(int i = 0; i < MMCIP_END; i++)
		{
			MMatchCharItem *pItem = pCurrObj->GetCharItemBySlotIndex(i);
			if(pItem == NULL) continue;
			
			cache.nEquipedItemID[i] = pItem->nID;
		}
		cache.nRank = (unsigned int)pCurrObj->m_Exp.nRanking;
		cache.nKillCount = pCurrObj->m_Exp.nKill;
		cache.nDeathCount = pCurrObj->m_Exp.nDeath;
		cache.nDTGrade = pCurrObj->m_DuelTournament.nClass;
		
		Cmd.WriteSkip(4);	// don't remove for a reason.
		Cmd.WriteData(&cache, sizeof(MTD_ObjectCache));
	}
	Cmd.EndBlob();
	
	Cmd.Finalize(MC_MATCH_OBJECT_CACHE, MCFT_END);
	
	if(uidDestPlayer != MUID(0, 0))
		SendToClient(&Cmd, uidDestPlayer);
	else
		SendToStage(&Cmd, pSrcStage->GetUID());
}

void SendObjectCache(int nCacheType, MMatchObject *pSrcObj, const MUID &uidDestStage)
{
	MCmdWriter Cmd;
	Cmd.WriteUChar((unsigned char)nCacheType);
	
	Cmd.StartBlob(sizeof(MTD_ObjectCache) + 4);
	
	char szClanName[DB_CLANNAME_LEN] = "";
	unsigned int nEmblemChecksum = 0;
	
	MMatchClan *pClan = g_ClanMgr.Get(pSrcObj->m_Clan.nCLID);
	if(pClan != NULL)
	{
		strcpy(szClanName, pClan->GetName());
		nEmblemChecksum = pClan->GetEmblemChecksum();
	}
	
	MTD_ObjectCache cache;
	ZeroInit(&cache, sizeof(MTD_ObjectCache));
		
	cache.uidObject = pSrcObj->GetUID();
	strcpy(cache.szName, pSrcObj->m_Char.szName);
	strcpy(cache.szClanName, szClanName);
	cache.nLevel = (char)pSrcObj->m_Exp.nLevel;
	cache.nUGrade = pSrcObj->m_Account.nUGradeID;
	cache.nPGrade = pSrcObj->m_Account.nPGradeID;
	cache.nPlayerFlags = (unsigned char)pSrcObj->m_nPlayerFlag;
	cache.nCLID = pSrcObj->m_Clan.nCLID;
	cache.nEmblemChecksum = nEmblemChecksum;
	cache.nSex = pSrcObj->m_Char.nSex;
	cache.nHair = pSrcObj->m_Char.nHair;
	cache.nFace = pSrcObj->m_Char.nFace;
	for(int i = 0; i < MMCIP_END; i++)
	{
		MMatchCharItem *pItem = pSrcObj->GetCharItemBySlotIndex(i);
		if(pItem == NULL) continue;
		
		cache.nEquipedItemID[i] = pItem->nID;
	}
	cache.nRank = (unsigned int)pSrcObj->m_Exp.nRanking;
	cache.nKillCount = pSrcObj->m_Exp.nKill;
	cache.nDeathCount = pSrcObj->m_Exp.nDeath;
	cache.nDTGrade = pSrcObj->m_DuelTournament.nClass;
		
	Cmd.WriteSkip(4);	// don't remove for a reason.
	Cmd.WriteData(&cache, sizeof(MTD_ObjectCache));
	
	Cmd.EndBlob();
	
	Cmd.Finalize(MC_MATCH_OBJECT_CACHE, MCFT_END);
	SendToStage(&Cmd, uidDestStage);
}

void CreateStageSettingCommand(MMatchStage *pStage, MCommandWriter *pOut)
{
	pOut->WriteMUID(pStage->GetUID());	// param 1 : stage uid.
	
	// param 2 : stage setting.
	MSTAGE_SETTING_NODE stagenode;
	ZeroInit(&stagenode, sizeof(MSTAGE_SETTING_NODE));
	
	stagenode.uidStage = pStage->GetUID();
	if(pStage->IsRelayMapStage() == false || pStage->IsGameLaunched() == false)
		strcpy(stagenode.szMapName, pStage->GetMapName());
	else
	{
		int nMapsetType = g_MapMgr.GetMapsetFromGameType(pStage->GetGameType());
		
		MMatchMap *pMap = g_MapMgr.GetMapFromIndex(pStage->GetCurrRelayMapID(), nMapsetType);
		if(pMap != NULL) strcpy(stagenode.szMapName, pMap->szName);
		else strcpy(stagenode.szMapName, g_MapMgr.GetDefaultMapName(MMMST_GENERAL));
	}
	stagenode.nMapIndex = (char)pStage->GetMapIndex();
	stagenode.nGameType = pStage->GetGameType();
	stagenode.nRoundMax = pStage->GetRound();
	stagenode.nLimitTime = pStage->GetTimeLimit();
	#if _GAME_VERSION < 2013
	if(MMatchStage::IsUnLimitedTime(stagenode.nLimitTime) == false)
	{
		stagenode.nLimitTime /= 60 * 1000;
	}
	#endif
	stagenode.nLimitLevel = pStage->GetLimitLevel();
	stagenode.nMaxPlayers = pStage->GetMaxPlayer();
	stagenode.bTeamKillEnabled = pStage->IsFriendlyFireEnabled();
	stagenode.bTeamWinThePoint = false;	// always false.
	stagenode.bForcedEntryEnabled = pStage->IsForcedEntryEnabled();
	stagenode.bAutoTeamBalancing = pStage->IsTeamBalancingEnabled();
	stagenode.bIsRelayMap = pStage->IsRelayMapStage();
	stagenode.bIsStartRelayMap = false;	// always false.
	stagenode.nRelayMapListCount = pStage->GetRelayMapSetting()->GetMapCount();
	for(int i = 0; i < MAX_RELAYMAP_ELEMENT_COUNT; i++)
		stagenode.nMapList[i] = pStage->GetRelayMapSetting()->GetMapIDFromIndex(i);
	stagenode.nRelayMapType = pStage->GetRelayMapSetting()->GetType();
	stagenode.nRelayMapRepeatCount = pStage->GetRelayMapSetting()->GetRepeat();
	
	pOut->StartBlob(sizeof(MSTAGE_SETTING_NODE));
	pOut->WriteData(&stagenode, sizeof(MSTAGE_SETTING_NODE));
	pOut->EndBlob();
	
	// param 3 : char setting.
	pOut->StartBlob(sizeof(MSTAGE_CHAR_SETTING_NODE));
	for(map<MUID, MMatchObject *>::iterator i = pStage->ObjBegin(); i != pStage->ObjEnd(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		
		MSTAGE_CHAR_SETTING_NODE charnode;
		charnode.uidChar = pCurr->GetUID();
		charnode.nTeam = pCurr->m_GameInfo.nTeam;
		charnode.nState = pCurr->m_nStageState;
		
		pOut->WriteData(&charnode, sizeof(MSTAGE_CHAR_SETTING_NODE));
	}
	pOut->EndBlob();
	
	pOut->WriteInt(pStage->GetState());	// param 4 : stage state.
	pOut->WriteMUID(pStage->GetMasterUID());	// param 5 : master uid.
	
	pOut->Finalize(MC_MATCH_RESPONSE_STAGESETTING);
	
	// relaymap element.
	pOut->WriteMUID(pStage->GetUID());
	pOut->WriteInt(pStage->GetRelayMapSetting()->GetType());
	pOut->WriteInt(pStage->GetRelayMapSetting()->GetRepeat());
	pOut->Finalize(MC_MATCH_STAGE_RELAY_MAP_ELEMENT_UPDATE, MCFT_END);
}

void SendStageSettingToStage(MMatchStage *pStage)
{
	MCmdWriter Cmd;
	CreateStageSettingCommand(pStage, &Cmd);
	SendToStage(&Cmd, pStage->GetUID(), true);
	
	if(pStage->GetGameType() == (int)MMGT_QUEST)
	{
		SendQuestStageInfoToStage(pStage->GetUID(), pStage->GetQuestScenarioID());
	}
	else if(pStage->GetGameType() == (int)MMGT_SPY)
	{
		pStage->SendSpyMapInfo();
	}
}

void ReserveStageAgent(const MUID &uidStage)
{
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidStage);
	Cmd.Finalize(MC_AGENT_STAGE_RESERVE, MCFT_END);
	SendToAgent(&Cmd);
}

void UnbindAgentClient(const MUID &uidPlayer)
{
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidPlayer);
	Cmd.Finalize(MC_AGENT_PEER_UNBIND, MCFT_END);
	SendToAgent(&Cmd);
}

bool LaunchRelayMapStage(MMatchStage *pStage)
{
	ReserveStageAgent(pStage->GetUID());
	
	MCmdWriter Cmd;
	if(CheckGameVersion(2013) == true)
	{
		Cmd.WriteInt(0);
		Cmd.Finalize(MC_MATCH_STAGE_START);
	}
	else
	{
		Cmd.WriteMUID(pStage->GetMasterUID());
		Cmd.WriteMUID(pStage->GetUID());
		Cmd.WriteInt(0);
		Cmd.Finalize(MC_MATCH_STAGE_START);
	}
	Cmd.Finalize();
	SendToStage(&Cmd, pStage->GetUID());
	
	char szMapName[MAX_STAGE_MAPNAME_LENGTH];
	int nMapsetType = g_MapMgr.GetMapsetFromGameType(pStage->GetGameType());
	
	MMatchMap *pMap = g_MapMgr.GetMapFromIndex(pStage->GetCurrRelayMapID(), nMapsetType);
	if(pMap != NULL)
	{
		strcpy(szMapName, pMap->szName);
	}
	else
	{
		strcpy(szMapName, g_MapMgr.GetDefaultMapName(MMMST_GENERAL));
	}
	
	for(map<MUID, MMatchObject *>::iterator i = pStage->ObjBegin(); i != pStage->ObjEnd(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		
		if(pCurr->m_nStageState == MOSS_READY)
		{
			Cmd.ResetIndex();
			Cmd.WriteMUID(pStage->GetUID());
			Cmd.WriteString(szMapName);
			Cmd.WriteBool(false);
			Cmd.Finalize(MC_MATCH_STAGE_RELAY_LAUNCH, MCFT_END);
			SendToClient(&Cmd, pCurr->GetUID());
			
			pCurr->ResetInGameInfo();
			pCurr->m_nStageState = MOSS_WAIT;
			pCurr->m_nGameFlag = MMOGF_INGAME | MMOGF_LAUNCHED;
			
			pCurr->ReserveStageKick(60000);	// 1 min.
		}
		else
		{
			Cmd.ResetIndex();
			Cmd.WriteMUID(pStage->GetUID());
			Cmd.WriteString(szMapName);
			Cmd.WriteBool(true);
			Cmd.Finalize(MC_MATCH_STAGE_RELAY_LAUNCH, MCFT_END);
			SendToClient(&Cmd, pCurr->GetUID());
		}
	}
	
	pStage->SetState((int)MMSS_RUN);
	pStage->CreateGame();
	
	SendStageSettingToStage(pStage);
	
	SendStageList(pStage->GetChannelUID());
	
	return true;
}

void MakePeerListNode(MMatchObject *pObj, MTD_PeerListNode *pOut)
{
	ZeroInit(pOut, sizeof(MTD_PeerListNode));
	
	char szClanName[DB_CLANNAME_LEN] = "";
	
	MMatchClan *pClan = g_ClanMgr.Get(pObj->m_Clan.nCLID);
	if(pClan != NULL)
	{
		strcpy(szClanName, pClan->GetName());
	}
	
	pOut->uidChar = pObj->GetUID();
	
	pOut->nIP = g_IPReplacer.Replace(pObj->GetIP());
	pOut->nPort = (unsigned int)pObj->GetUDPPort();
	
	strcpy(pOut->CharInfo.szName, pObj->m_Char.szName);
	strcpy(pOut->CharInfo.szClanName, szClanName);
	pOut->CharInfo.nClanGrade = pObj->m_Clan.nMemberGrade;
	pOut->CharInfo.nClanContPoint = 0;	// clan cont point : always 0.
	pOut->CharInfo.nCharNum = (char)0;	// TODO : charnum.
	pOut->CharInfo.nLevel = (unsigned short)pObj->m_Exp.nLevel;
	pOut->CharInfo.nSex = (char)pObj->m_Char.nSex;
	pOut->CharInfo.nHair = (char)pObj->m_Char.nHair;
	pOut->CharInfo.nFace = (char)pObj->m_Char.nFace;
	pOut->CharInfo.nXP = (unsigned long)pObj->m_Exp.nXP;
	pOut->CharInfo.nBP = pObj->m_Exp.nBP;
	pOut->CharInfo.fBonusRate = 0.0f;
	pOut->CharInfo.nPrize = 0;
	pOut->CharInfo.nHP = 0;
	pOut->CharInfo.nAP = 0;
	pOut->CharInfo.nMaxWeight = 0;	// TODO : maxwt.
	pOut->CharInfo.nSafeFalls = 0;
	pOut->CharInfo.nFR = 0;
	pOut->CharInfo.nCR = 0;
	pOut->CharInfo.nER = 0;
	pOut->CharInfo.nWR = 0;
	for(int i = 0; i < MMCIP_END; i++)
	{
		MMatchCharItem *pCharItem = pObj->GetCharItemBySlotIndex(i);
		if(pCharItem == NULL) continue;
		
		pOut->CharInfo.uidEquipedItem[i] = pCharItem->uid;
		pOut->CharInfo.nEquipedItemDesc[i] = (unsigned long)pCharItem->nID;
		pOut->CharInfo.nEquipedItemCount[i] = (unsigned long)pCharItem->nCount;
	}
	pOut->CharInfo.nUGradeID = pObj->m_Account.nUGradeID;
	pOut->CharInfo.nClanCLID = (unsigned int)pObj->m_Clan.nCLID;
	pOut->CharInfo.nDTLastWeekGrade = pObj->m_DuelTournament.nClass;
	
	pOut->ExtendInfo.nTeam = (char)pObj->m_GameInfo.nTeam;
	pOut->ExtendInfo.nPlayerFlags = (unsigned char)pObj->m_nPlayerFlag;
}

unsigned long MakeExpCommandData(unsigned long nAddXP, int nPercent)
{
	unsigned long nXP = nAddXP <= 0xFFFF ? nAddXP : 0xFFFF;
	return (unsigned long)(nXP << 16) | (unsigned long)nPercent;
}

bool CheckKickCommand(const char *pszText, char *pszOutName)
{
	if(strlen(pszText) >= 32) return false;	// source text too long.
	
	static const char szCmd[] = "/kick ";
	static const int nCmdLen = (int)strlen(szCmd);
	
	char szLower[32];
	strcpy(szLower, pszText);
	
	// strlwr(szLower);
	MStrlwr(szLower);
	
	int nMatch = 0;
	for(int i = 0; szLower[i] != '\0' && i < nCmdLen; i++)
	{
		if(szLower[i] != szCmd[i]) break;
		nMatch++;
	}
	
	if(nMatch != nCmdLen) return false;
	
	strcpy(pszOutName, &pszText[nCmdLen]);
	
	return true;
}

int CheckCallVotable(MMatchObject *pObj, MMatchObject *pTargetObj, MMatchStage *pStage)
{
	if(pStage->IsVoting() == true)
	{
		return MERR_VOTING_ALREADY;
	}
	
	// TIP : if need to prevent multiple kicks, add checks here.
	
	if(pStage->GetGameType() == (int)MMGT_DUEL_TOURNAMENT || pStage->GetGameType() == (int)MMGT_BLITZKRIEG)
	{
		return MERR_CANNOT_VOTE_ON_LADDER;
	}
	
	if(pTargetObj->IsAdmin() == true)
	{
		return MERR_VOTE_FAILED;
	}
	
	return MSG_OK;
}

void OnStageList(const MUID &uidPlayer, const MUID &uidChannel, int nStageCursor, bool bChecksum)
{
	if(uidChannel == MUID(0, 0)) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_uidChannel != uidChannel) return;
	
	MMatchChannel *pChannel = g_ChannelMgr.Get(uidChannel);
	if(pChannel == NULL) return;
	
	// TODO : skip on clan server and clan channel.
	
	if(pChannel->GetType() == MCHANNEL_TYPE_DUELTOURNAMENT || pChannel->GetType() == MCHANNEL_TYPE_BLITZKRIEG) return;
	
	MStageList list;
	g_StageMgr.RetrieveList(uidChannel, nStageCursor, &list);
	
	if(bChecksum == true)
	{
		if(list.nChecksum == pObj->m_nLastStageListChecksum) return;
	}
	pObj->m_nLastStageListChecksum = list.nChecksum;
	
	MCmdWriter Cmd;
	
	Cmd.WriteChar((char)list.nPrevStageCount);
	Cmd.WriteChar((char)list.nNextStageCount);
	
	Cmd.StartBlob(sizeof(MTD_StageListNode));
	for(int i = 0; i < list.nCount; i++)
	{
		MMatchStage *pCurr = list.pStage[i];
		
		MTD_StageListNode node;
		ZeroInit(&node, sizeof(MTD_StageListNode));
		
		unsigned int nSettingFlag = 0;
		if(pCurr->IsForcedEntryEnabled() == true) nSettingFlag |= MSTAGENODE_FLAG_FORCEDENTRY_ENABLED;
		if(pCurr->IsPrivate() == true) nSettingFlag |= MSTAGENODE_FLAG_PRIVATE;
		if(pCurr->GetLimitLevel() != 0) nSettingFlag |= MSTAGENODE_FLAG_LIMITLEVEL;
		
		node.uidStage = pCurr->GetUID();
		node.nNo = (unsigned char)pCurr->GetNumber();
		strcpy(node.szStageName, pCurr->GetName());
		node.nPlayers = (char)pCurr->GetPlayer();
		node.nMaxPlayers = (char)pCurr->GetMaxPlayer();
		node.nState = pCurr->GetState();
		node.nGameType = pCurr->GetGameType();
		node.nMapIndex = (char)pCurr->GetMapIndex();
		node.nSettingFlag = (int)nSettingFlag;
		node.nMasterLevel = pCurr->GetMasterLevel();
		node.nLimitLevel = pCurr->GetLimitLevel();
		
		Cmd.WriteData(&node, sizeof(MTD_StageListNode));
	}
	Cmd.EndBlob();
	
	Cmd.Finalize(MC_MATCH_STAGE_LIST, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
	
	pObj->m_nLastStageListPage = nStageCursor;
}

// Argument 4 - bPrivate : is not used. just in case.
void OnCreateStage(const MUID &uidPlayer, const char *pszStageName, const char *pszPassword, bool bPrivate)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	if(pObj->m_uidChannel == MUID(0, 0)) return;
	
	MMatchChannel *pChannel = g_ChannelMgr.Get(pObj->m_uidChannel);
	if(pChannel == NULL) return;
	
	// TODO : channel type check (protection for clan channel on clan server).
	if(pChannel->IsDuelTournament() == true || pChannel->IsBlitzKrieg() == true) return;
	
	if(pObj->m_uidStage != MUID(0, 0)) return;
	
	int nStageNumber = AssignStageNumber(pChannel->GetUID());
	
	if(nStageNumber == -1)
	{
		MCmdWriter err;
		err.WriteInt(MERR_CANNOT_CREATE_STAGE);
		err.Finalize(MC_MATCH_RESPONSE_STAGE_CREATE, MCFT_END);
		SendToClient(&err, uidPlayer);
		return;
	}
	
	MMatchStage *pNewStage = g_StageMgr.Create(nStageNumber);
	pNewStage->Create(pChannel->GetUID(), nStageNumber, pszStageName, pszPassword);
	
	// decide team.
	pObj->m_GameInfo.nTeam = pNewStage->GetSuitableTeam();
	
	if(pObj->IsHide() == true)
	{
		pObj->m_GameInfo.nTeam = MMT_SPECTATOR;
	}
	
	// join to a stage.
	pObj->m_uidStage = pNewStage->GetUID();
	pNewStage->Enter(pObj);
	
	pNewStage->SetMaster(uidPlayer, pObj->m_Exp.nLevel);
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidPlayer);
	Cmd.WriteMUID(pNewStage->GetUID());
	Cmd.WriteInt(pNewStage->GetNumber());
	Cmd.WriteString(pNewStage->GetName());
	Cmd.Finalize(MC_MATCH_STAGE_JOIN, MCFT_END);
	
	SendToClient(&Cmd, uidPlayer);
	
	SendObjectCache((int)MMOCT_UPDATE, pNewStage, uidPlayer);	// object cache to itself.
	
	pObj->m_nGameFlag = MMOGF_NONE;
	pObj->m_nStageState = MOSS_WAIT;
	
	pObj->m_nPlace = MMP_STAGE;
	SendChannelPlayerList(pChannel->GetUID());	// update channel player list with new place.
	
	SendStageList(pChannel->GetUID());
}

void OnPrivateStageJoin(const MUID &uidPlayer, const MUID &uidStage, const char *pszPassword)
{
	if(uidStage == MUID(0, 0)) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	if(pObj->m_uidStage != MUID(0, 0)) return;
	
	MMatchStage *pStage = g_StageMgr.Get(uidStage);
	// if(pStage == NULL) return;
	
	if(pStage == NULL)
	{
		MCmdWriter temp;
		temp.WriteInt(MERR_STAGE_NOT_EXISTS);
		temp.Finalize(MC_MATCH_RESPONSE_STAGE_JOIN, MCFT_END);
		SendToClient(&temp, uidPlayer);
		return;
	}
	
	if(pStage->GetChannelUID() != pObj->m_uidChannel) return;
	
	// TODO : clan war stage check.
	if(pStage->GetGameType() == (int)MMGT_DUEL_TOURNAMENT || pStage->GetGameType() == (int)MMGT_BLITZKRIEG) return;
	
	MCmdWriter Cmd;
	
	if(pObj->IsAdmin() == false)
	{
		if(pStage->IsBannedAID(pObj->m_Account.nAID) == true)
		{
			Cmd.WriteInt(MERR_STAGE_BANNED);
			Cmd.Finalize(MC_MATCH_RESPONSE_STAGE_JOIN, MCFT_END);
			SendToClient(&Cmd, uidPlayer);
			return;
		}
		
		if(pStage->IsPrivate() == true)
		{
			if(pStage->IsPasswordMatch(pszPassword) == false)
			{
				Cmd.WriteInt(MERR_STAGE_WRONG_PASSWORD);
				Cmd.Finalize(MC_MATCH_RESPONSE_STAGE_JOIN, MCFT_END);
				SendToClient(&Cmd, uidPlayer);
				return;
			}
		}
		
		if(pStage->CheckLevelLimit(pObj->m_Exp.nLevel) == false)
		{
			Cmd.WriteInt(MERR_STAGE_LEVEL_OUT_OF_RANGE);
			Cmd.Finalize(MC_MATCH_RESPONSE_STAGE_JOIN, MCFT_END);
			SendToClient(&Cmd, uidPlayer);
			return;
		}
		
		if(pStage->IsForceJoinable() == false)
		{
			Cmd.WriteInt(MERR_FORCED_ENTRY_NOT_ALLOWED);
			Cmd.Finalize(MC_MATCH_RESPONSE_STAGE_JOIN, MCFT_END);
			SendToClient(&Cmd, uidPlayer);
			return;
		}
		
		if(pStage->IsMaxPlayersReached() == true)
		{
			Cmd.WriteInt(MERR_STAGE_MAX_PLAYER_OVER);
			Cmd.Finalize(MC_MATCH_RESPONSE_STAGE_JOIN, MCFT_END);
			SendToClient(&Cmd, uidPlayer);
			return;
		}
	}
	
	SendObjectCache((int)MMOCT_ADD, pObj, uidStage);	// send object cache to its stage players. (without sending to joiner itself.)
	
	// decide team.
	pObj->m_GameInfo.nTeam = pStage->GetSuitableTeam();
	
	if(pObj->IsHide() == true)
	{
		pObj->m_GameInfo.nTeam = MMT_SPECTATOR;
	}
	
	// join a stage.
	pObj->m_uidStage = pStage->GetUID();
	pStage->Enter(pObj);
	
	Cmd.WriteMUID(uidPlayer);
	Cmd.WriteMUID(uidStage);
	Cmd.WriteInt(pStage->GetNumber());
	Cmd.WriteString(pStage->GetName());
	Cmd.Finalize(MC_MATCH_STAGE_JOIN, MCFT_END);
	
	if(pObj->IsHide() == true)
	{
		SendToClient(&Cmd, uidPlayer);
	}
	else
	{
		SendToStage(&Cmd, uidStage);
	}
	
	SendObjectCache((int)MMOCT_UPDATE, pStage, uidPlayer);	// now send to joiner.
	
	pObj->m_nGameFlag = MMOGF_NONE;
	pObj->m_nStageState = MOSS_WAIT;
	
	pObj->m_nPlace = MMP_STAGE;
	SendChannelPlayerList(pStage->GetChannelUID());	// update channel player list with new place.
	
	SendStageList(pStage->GetChannelUID());
}

void OnStageJoin(const MUID &uidPlayer, const MUID &uidStage)
{
	OnPrivateStageJoin(uidPlayer, uidStage, NULL);
}

void OnStageLeave(const MUID &uidPlayer)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_uidStage == MUID(0, 0)) return;
	if(pObj->m_nPlace != MMP_STAGE) return;
	
	MMatchStage *pStage = g_StageMgr.Get(pObj->m_uidStage);
	if(pStage == NULL) return;
	
	pObj->CancelStageKick();
	
	// TODO : add more leave conditions?
	
	// callback leaver's quest item.
	if(pStage->GetGameType() == (int)MMGT_QUEST)
	{
		for(int i = 0; i < SACRIITEM_SLOT_COUNT; i++)
		{
			MSacriItemSlot *pSlot = pStage->GetQuestSlotItem(i);
			if(pSlot == NULL) continue;
			
			if(pSlot->uidOwner == MUID(0, 0)) continue;
			
			if(pSlot->uidOwner == uidPlayer)
			{
				OnQuestCallbackSacrificeItem(uidPlayer, i, pSlot->nItemID);
			}
		}
	}
	
	pObj->m_uidStage = MUID(0, 0);
	pStage->Leave(pObj);
	
	MCmdWriter Cmd;
	
	if(pStage->IsMaster(uidPlayer) == true)
	{
		pStage->RandomMaster();
		
		Cmd.WriteMUID(pStage->GetUID());
		Cmd.WriteMUID(pStage->GetMasterUID());
		Cmd.Finalize(MC_MATCH_STAGE_MASTER);
	}
	
	Cmd.WriteMUID(uidPlayer);
	Cmd.Finalize(MC_MATCH_STAGE_LEAVE, MCFT_END);
	
	SendToStage(&Cmd, pStage->GetUID());	// to remained stage players.
	SendToClient(&Cmd, uidPlayer);	// to leaver self.
	
	MUID uidChannel = pStage->GetChannelUID();
	
	if(pStage->GetRealPlayer() > 0)
	{
		SendObjectCache((int)MMOCT_REMOVE, pObj, pStage->GetUID());
	}
	else
	{
		pStage->DestroyGame();
		g_StageMgr.Remove(pStage);
	}
	
	pObj->m_nPlace = MMP_LOBBY;
	SendChannelPlayerList(uidChannel);
	
	SendStageList(uidChannel);
}

/*
void OnStageChat(const MUID &uidPlayer, const MUID &uidStage, const char *pszChat)
{
	if(uidStage == MUID(0, 0)) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_uidStage != uidStage) return;
	
	MMatchStage *pStage = g_StageMgr.Get(uidStage);
	if(pStage == NULL) return;
	
	// stage kick process.
	if(pStage->IsMaster(uidPlayer) == true)
	{
		char szCharName[CHARNAME_LEN];
		
		if(CheckKickCommand(pszChat, szCharName) == true)
		{
			// check object...
			MMatchObject *pTargetObj = g_ObjectMgr.Get(szCharName);
			if(pTargetObj == NULL) return;
			
			if(pTargetObj->m_bCharInfoExist == false) return;
			
			if(pTargetObj->m_uidStage != uidStage) return;
			if(pTargetObj->CheckGameFlag(MMOGF_INGAME) == true) return;
			
			// finally kick from stage.
			OnStageLeave(pTargetObj->GetUID());
			
			return;
		}
	}
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidPlayer);
	Cmd.WriteMUID(uidStage);
	Cmd.WriteString(pszChat);
	Cmd.Finalize(MC_MATCH_STAGE_CHAT, MCFT_END);
	SendToStage(&Cmd, uidStage);
}
*/

void OnStageChat(const MUID &uidPlayer, const char *pszChat)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;

	if(pObj->m_uidStage == MUID(0, 0)) return;
	
	MMatchStage *pStage = g_StageMgr.Get(pObj->m_uidStage);
	if(pStage == NULL) return;
	
	// stage kick process.
	if(pStage->IsMaster(uidPlayer) == true)
	{
		char szCharName[CHARNAME_LEN];
		
		if(CheckKickCommand(pszChat, szCharName) == true)
		{
			// check object...
			MMatchObject *pTargetObj = g_ObjectMgr.Get(szCharName);
			if(pTargetObj == NULL) return;
			
			if(pTargetObj->m_bCharInfoExist == false) return;
			
			if(pTargetObj->m_uidStage != pObj->m_uidStage) return;
			if(pTargetObj->CheckGameFlag(MMOGF_INGAME) == true) return;
			
			// finally kick from stage.
			OnStageLeave(pTargetObj->GetUID());
			
			return;
		}
	}
	
	MCmdWriter Cmd;
	
	if(CheckGameVersion(2012) == true)
	{
		Cmd.WriteMUID(uidPlayer);
		Cmd.WriteString(pszChat);
		Cmd.Finalize(MC_MATCH_STAGE_CHAT);
	}
	else
	{
		Cmd.WriteMUID(uidPlayer);
		Cmd.WriteMUID(pObj->m_uidStage);
		Cmd.WriteString(pszChat);
		Cmd.Finalize(MC_MATCH_STAGE_CHAT);
	}
	
	Cmd.Finalize();
	SendToStage(&Cmd, pObj->m_uidStage);
}

void OnStageSetting(const MUID &uidPlayer, const MUID &uidStage)
{
	if(uidStage == MUID(0, 0)) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_uidStage != uidStage) return;
	
	MMatchStage *pStage = g_StageMgr.Get(uidStage);
	if(pStage == NULL) return;
	
	MCmdWriter Cmd;
	CreateStageSettingCommand(pStage, &Cmd);
	SendToClient(&Cmd, uidPlayer);
	
	if(pStage->GetGameType() == (int)MMGT_QUEST)
	{
		SendQuestStageInfoToClient(uidPlayer, pStage->GetQuestScenarioID());
	}
	else if(pStage->GetGameType() == (int)MMGT_SPY)
	{
		pStage->SendSpyMapInfo();
	}
	
	if(pObj->IsAdmin() == true && pStage->IsPrivate() == true)
	{
		char szRoomPwd[256];
		sprintf(szRoomPwd, "^2Password of this stage is ^1%s^2.", pStage->GetPassword());
		AnnounceToClient(szRoomPwd, uidPlayer);
	}
}

void OnChangeStageSetting(const MUID &uidPlayer, const MUID &uidStage, const MSTAGE_SETTING_NODE *pNode)
{
	if(uidStage == MUID(0, 0)) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_uidStage != uidStage) return;
	
	MMatchStage *pStage = g_StageMgr.Get(uidStage);
	if(pStage == NULL) return;
	
	if(pStage->IsMaster(uidPlayer) == false) return;
	if(pStage->IsGameLaunched() == true) return;
	
	// added for prevent gameinfo access crash.
	if(pStage->CheckGameInfoExists() == true)
	{
		SendStageSettingToStage(pStage);
		return;
	}
	
	/*
	pStage->SetMap(pNode->szMapName);
	pStage->SetGameType(pNode->nGameType);
	pStage->SetRound(pNode->nRoundMax);
	pStage->SetTimeLimit(pNode->nLimitTime);
	pStage->SetLevelLimit(pNode->nLimitLevel);
	pStage->SetMaxPlayer(pNode->nMaxPlayers);
	pStage->SetTeamFriendlyKill(pNode->bTeamKillEnabled);
	pStage->SetForcedEntry(pNode->bForcedEntryEnabled);
	pStage->SetTeamBalance(pNode->bAutoTeamBalancing);
	
	pStage->GetRelayMapSetting()->SetType(pNode->nRelayMapType);
	pStage->GetRelayMapSetting()->SetRepeat(pNode->nRelayMapRepeatCount);
	
	pStage->GetRelayMapSetting()->Empty();
	for(int i = 0; i < MAX_RELAYMAP_ELEMENT_COUNT; i++)
	{
		pStage->GetRelayMapSetting()->SetMapIDFromIndex(pNode->nMapList[i], i);
	}
	pStage->GetRelayMapSetting()->Truncate();
	pStage->GetRelayMapSetting()->Shuffle();
	*/
	
	MSTAGE_SETTING_NODE node = *pNode;
	
	if(node.nGameType == (int)MMGT_DUEL_TOURNAMENT || node.nGameType == (int)MMGT_BLITZKRIEG) return;
	
	if(node.nGameType != pStage->GetGameType())	// new gametype != prev gametype (i.e. gametype changed).
	{
		if(node.nGameType == (int)MMGT_DUEL)
		{
			strcpy(node.szMapName, g_MapMgr.GetDefaultMapName(MMMST_DUEL));
		}
		else if(node.nGameType == (int)MMGT_SURVIVAL || node.nGameType == (int)MMGT_QUEST)
		{
			strcpy(node.szMapName, g_MapMgr.GetDefaultMapName(MMMST_QUEST));
		}
		else if(node.nGameType == (int)MMGT_CHALLENGE_QUEST)
		{
			strcpy(node.szMapName, g_MapMgr.GetDefaultMapName(MMMST_CHALLENGE_QUEST));
		}
		else
		{
			strcpy(node.szMapName, g_MapMgr.GetDefaultMapName(MMMST_GENERAL));
		}
		
		if(IsQuestGame(pStage->GetGameType()) == true)
		{
			// revert stage setting from quest setting.
			node.nLimitLevel = 0;	// no limit.
			node.bTeamKillEnabled = false;	// disabled.
			node.bForcedEntryEnabled = true;	// enabled.
			
			if(pStage->GetGameType() == (int)MMGT_QUEST)
			{
				// info reset.
				pStage->ResetQuestInfo();
			}
		}
		
		if(node.nGameType == (int)MMGT_SPY)
		{
			pStage->ActivateAllSpyMap();
		}
	}
	
	if(IsQuestGame(node.nGameType) == true)
	{
		node.nRoundMax = 1;
		node.nLimitTime = 99999;	// infinite.
		node.nLimitLevel = 0;
		node.bTeamKillEnabled = false;
		node.bForcedEntryEnabled = false;
	}
	
	pStage->SetGameType(node.nGameType);
	pStage->SetMap(node.szMapName);
	pStage->SetRound(node.nRoundMax);
	#if _GAME_VERSION >= 2013
	if(node.nLimitTime == -60000)	// -60000 ms : -1 min.
	{
		node.nLimitTime = 0;	// set to infinite.
	}
	#else
	if(MMatchStage::IsUnLimitedTime(node.nLimitTime) == false)
	{
		node.nLimitTime *= 60 * 1000;	// to millisecond.
	}
	#endif
	pStage->SetTimeLimit(node.nLimitTime);
	pStage->SetLevelLimit(node.nLimitLevel);
	pStage->SetMaxPlayer(node.nMaxPlayers);
	pStage->SetTeamFriendlyKill(node.bTeamKillEnabled);
	pStage->SetForcedEntry(node.bForcedEntryEnabled);
	pStage->SetTeamBalance(node.bAutoTeamBalancing);
	
	pStage->GetRelayMapSetting()->SetType(node.nRelayMapType);
	pStage->GetRelayMapSetting()->SetRepeat(node.nRelayMapRepeatCount);
	
	pStage->GetRelayMapSetting()->Empty();
	for(int i = 0; i < MAX_RELAYMAP_ELEMENT_COUNT; i++)
	{
		pStage->GetRelayMapSetting()->SetMapIDFromIndex(node.nMapList[i], i);
	}
	pStage->GetRelayMapSetting()->Truncate();
	pStage->GetRelayMapSetting()->Shuffle();
	
	SendStageSettingToStage(pStage);
	
	SendStageList(pStage->GetChannelUID());
}

void OnStageMap(const MUID &uidPlayer, const MUID &uidStage, const char *pszMapName)
{
	if(uidStage == MUID(0, 0)) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_uidStage != uidStage) return;
	
	MMatchStage *pStage = g_StageMgr.Get(uidStage);
	if(pStage == NULL) return;
	
	if(pStage->IsMaster(uidPlayer) == false) return;
	if(pStage->IsGameLaunched() == true) return;
	
	pStage->SetMap(pszMapName);
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidStage);
	Cmd.WriteString(pszMapName);
	Cmd.Finalize(MC_MATCH_STAGE_MAP, MCFT_END);
	
	SendToStage(&Cmd, uidStage);
	
	if(pStage->GetGameType() == (int)MMGT_QUEST)
	{
		pStage->RefreshScenarioID();
		SendQuestStageInfoToStage(uidStage, pStage->GetQuestScenarioID());
	}
	
	SendStageList(pStage->GetChannelUID());
}

void OnRelayMapElementUpdate(const MUID &uidPlayer, const MUID &uidStage, int nRelayMapType, int nRelayMapRepeat)
{
	if(uidStage == MUID(0, 0)) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_uidStage != uidStage) return;
	
	MMatchStage *pStage = g_StageMgr.Get(uidStage);
	if(pStage == NULL) return;
	
	if(pStage->IsMaster(uidPlayer) == false) return;
	if(pStage->IsGameLaunched() == true) return;
	
	pStage->GetRelayMapSetting()->SetType(nRelayMapType);
	pStage->GetRelayMapSetting()->SetRepeat(nRelayMapRepeat);
	
	pStage->GetRelayMapSetting()->Shuffle();
	
	/*
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidStage);
	Cmd.WriteInt(nRelayMapType);
	Cmd.WriteInt(nRelayMapRepeat);
	Cmd.Finalize(MC_MATCH_STAGE_RELAY_MAP_ELEMENT_UPDATE, MCFT_END);
	
	SendToStage(&Cmd, uidStage);
	*/
	
	int nRealCount = pStage->GetRelayMapSetting()->GetMapCount();
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidStage);
	Cmd.WriteInt(nRelayMapType);
	Cmd.WriteInt(nRelayMapRepeat);
	Cmd.StartBlob(sizeof(MTD_RelayMap));
	for(int i = 0; i < nRealCount; i++)
	{
		MTD_RelayMap list;
		list.nMapID = pStage->GetRelayMapSetting()->GetMapIDFromIndex(i);
		
		Cmd.WriteData(&list, sizeof(MTD_RelayMap));
	}
	Cmd.EndBlob();
	Cmd.Finalize(MC_MATCH_STAGE_RELAY_MAP_INFO_UPDATE, MCFT_END);
	
	SendToStage(&Cmd, uidStage);
}

void OnRelayMapInfoUpdate(const MUID &uidPlayer, const MUID &uidStage, int nRelayMapType, int nRelayMapRepeat, const MTD_RelayMap *pRelayMapList, int nCount)
{
	if(uidStage == MUID(0, 0)) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_uidStage != uidStage) return;
	
	MMatchStage *pStage = g_StageMgr.Get(uidStage);
	if(pStage == NULL) return;
	
	if(pStage->IsMaster(uidPlayer) == false) return;
	if(pStage->IsGameLaunched() == true) return;
	
	pStage->GetRelayMapSetting()->SetType(nRelayMapType);
	pStage->GetRelayMapSetting()->SetRepeat(nRelayMapRepeat);
	
	pStage->GetRelayMapSetting()->Empty();
	for(int i = 0; i < nCount; i++)
	{
		pStage->GetRelayMapSetting()->SetMapIDFromIndex(pRelayMapList[i].nMapID, i);
	}
	pStage->GetRelayMapSetting()->Truncate();
	pStage->GetRelayMapSetting()->Shuffle();
	
	int nRealCount = pStage->GetRelayMapSetting()->GetMapCount();
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidStage);
	Cmd.WriteInt(nRelayMapType);
	Cmd.WriteInt(nRelayMapRepeat);
	Cmd.StartBlob(sizeof(MTD_RelayMap));
	for(int i = 0; i < nRealCount; i++)
	{
		MTD_RelayMap list;
		list.nMapID = pStage->GetRelayMapSetting()->GetMapIDFromIndex(i);
		
		Cmd.WriteData(&list, sizeof(MTD_RelayMap));
	}
	Cmd.EndBlob();
	Cmd.Finalize(MC_MATCH_STAGE_RELAY_MAP_INFO_UPDATE, MCFT_END);
	
	SendToStage(&Cmd, uidStage);
}

void OnStageTeam(const MUID &uidPlayer, const MUID &uidStage, int nTeam)
{
	if(uidStage == MUID(0, 0)) return;
	if(nTeam != MMT_RED && nTeam != MMT_BLUE) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_uidStage != uidStage) return;
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidPlayer);
	Cmd.WriteMUID(uidStage);
	Cmd.WriteInt(nTeam);
	Cmd.Finalize(MC_MATCH_STAGE_TEAM, MCFT_END);
	
	SendToStage(&Cmd, uidStage);
	
	pObj->m_GameInfo.nTeam = nTeam;
}

void OnStageState(const MUID &uidPlayer, const MUID &uidStage, int nState)
{
	if(uidStage == MUID(0, 0)) return;
	if(nState < 0 || nState >= MOSS_END) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_uidStage != uidStage) return;
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidPlayer);
	Cmd.WriteMUID(uidStage);
	Cmd.WriteInt(nState);
	Cmd.Finalize(MC_MATCH_STAGE_PLAYER_STATE, MCFT_END);
	
	SendToStage(&Cmd, uidStage);
	
	pObj->m_nStageState = nState;
}

/*
// arg 3 : nCountdown is not used.
void OnStageStart(const MUID &uidPlayer, const MUID &uidStage, int nCountdown)
{
	if(uidStage == MUID(0, 0)) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_uidStage != uidStage) return;
	
	MMatchStage *pStage = g_StageMgr.Get(uidStage);
	if(pStage == NULL) return;
	
	if(pStage->IsMaster(uidPlayer) == false) return;
	if(pStage->IsGameLaunched() == true) return;
	
	// 	NOTE : to send "not ready" message,
	// 	use this format ERRMSG:[errnum] or ERRMSG:[errnum]\a[paramstr]
	// 	with MC_MATCH_ANNOUNCE.
	
	// max player over.
	if(pObj->IsAdmin() == false)
	{
		if(pStage->GetPlayer() > pStage->GetMaxPlayer())
		{
			char szAnnounce[256];
			sprintf(szAnnounce, "ERRMSG:%d", MERR_INCREASE_STAGE_MAX_PLAYER);
			AnnounceToClient(szAnnounce, uidPlayer);
			return;
		}
	}
	
	// check not ready.
	bool bNotReady = false;
	
	for(map<MUID, MMatchObject *>::iterator i = pStage->ObjBegin(); i != pStage->ObjEnd(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		
		if(pCurr->GetUID() == uidPlayer) continue;	// skip checking stage master.
		if(pCurr->m_nStageState == MOSS_READY) continue;
		
		char szAnnounce[256];
		sprintf(szAnnounce, "ERRMSG:%d\a%s", MERR_PLAYER_NOT_READY, pCurr->m_Char.szName);
		AnnounceToStage(szAnnounce, uidStage);
		
		bNotReady = true;
	}
	
	if(bNotReady == true)
	{
		MCmdWriter temp;
		temp.WriteInt(GAMESTART_ERROR_PLAYER_NOT_READY);
		temp.WriteMUID(MUID(0, 0));
		temp.Finalize(MC_GAME_START_FAIL, MCFT_END);
		SendToClient(&temp, uidPlayer);
		
		return;
	}
	
	if(pStage->GetGameType() == (int)MMGT_SURVIVAL)
	{
		pStage->SetQuestScenarioID(g_Quest.GetSurvivalScenarioID(pStage->GetMapName()));
		
		if(pStage->GetQuestScenarioID() == 0)
		{
			MCmdWriter temp;
			temp.WriteInt(GAMESTART_ERROR_UNAVAILABLE_SCENARIO);
			temp.WriteMUID(MUID(0, 0));
			temp.Finalize(MC_GAME_START_FAIL, MCFT_END);
			SendToClient(&temp, uidPlayer);
			return;
		}
		
		MCmdWriter NPCInfoCmd;
		if(g_Quest.BuildNPCInfoListCommand(pStage->GetQuestScenarioID(), (int)MMGT_SURVIVAL, &NPCInfoCmd) == true)
		{
			SendToStage(&NPCInfoCmd, uidStage);
		}
		
		MCmdWriter QuestGameInfoCmd;
		if(g_Quest.BuildGameInfoCommand(pStage->GetQuestScenarioID(), (int)MMGT_SURVIVAL, &QuestGameInfoCmd) == true)
		{
			SendToStage(&QuestGameInfoCmd, uidStage);
		}
	}
	else if(pStage->GetGameType() == (int)MMGT_QUEST)
	{
		if(pStage->GetQuestScenarioID() == 0)
		{
			MCmdWriter temp;
			temp.WriteInt(GAMESTART_ERROR_UNAVAILABLE_SCENARIO);
			temp.WriteMUID(MUID(0, 0));
			temp.Finalize(MC_GAME_START_FAIL, MCFT_END);
			SendToClient(&temp, uidPlayer);
			return;
		}
		
		// spend sacrifice item.
		for(int i = 0; i < SACRIITEM_SLOT_COUNT; i++)
		{
			MSacriItemSlot *pSlot = pStage->GetQuestSlotItem(i);
			if(pSlot == NULL) continue;
			
			if(pSlot->uidOwner == MUID(0, 0)) continue;
			
			MMatchObject *pOwnerObj = g_ObjectMgr.Get(pSlot->uidOwner);
			if(pOwnerObj != NULL)
			{
				pOwnerObj->SubQuestItem(pSlot->nItemID, 1);
			}
		}
		
		// sync quest item list.
		for(map<MUID, MMatchObject *>::iterator i = pStage->ObjBegin(); i != pStage->ObjEnd(); i++)
		{
			OnCharQuestItemList((*i).first);
		}
		
		MCmdWriter NPCInfoCmd;
		if(g_Quest.BuildNPCInfoListCommand(pStage->GetQuestScenarioID(), (int)MMGT_QUEST, &NPCInfoCmd) == true)
		{
			SendToStage(&NPCInfoCmd, uidStage);
		}
		
		MCmdWriter QuestGameInfoCmd;
		if(g_Quest.BuildGameInfoCommand(pStage->GetQuestScenarioID(), (int)MMGT_QUEST, &QuestGameInfoCmd) == true)
		{
			SendToStage(&QuestGameInfoCmd, uidStage);
		}
	}
	
	ReserveStageAgent(uidStage);
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidPlayer);
	Cmd.WriteMUID(uidStage);
	Cmd.WriteInt(0);	// countdown : 0.
	Cmd.Finalize(MC_MATCH_STAGE_START, MCFT_END);
	SendToStage(&Cmd, uidStage);
	
	if(pStage->IsRelayMapStage() == false)
	{
		MCmdWriter Cmd2;
		Cmd2.WriteMUID(uidStage);
		Cmd2.WriteString(pStage->GetMapName());
		Cmd2.Finalize(MC_MATCH_STAGE_LAUNCH, MCFT_END);
		SendToStage(&Cmd2, uidStage);
	}
	else
	{
		pStage->AdvanceRelayMap();
		
		char szMapName[MAX_STAGE_MAPNAME_LENGTH];
		int nMapsetType = g_MapMgr.GetMapsetFromGameType(pStage->GetGameType());
		
		MMatchMap *pMap = g_MapMgr.GetMapFromIndex(pStage->GetCurrRelayMapID(), nMapsetType);
		if(pMap != NULL)
		{
			strcpy(szMapName, pMap->szName);
		}
		else
		{
			strcpy(szMapName, g_MapMgr.GetDefaultMapName(MMMST_GENERAL));
		}
		
		MCmdWriter Cmd2;
		for(map<MUID, MMatchObject *>::iterator i = pStage->ObjBegin(); i != pStage->ObjEnd(); i++)
		{
			Cmd2.ResetIndex();
			
			Cmd2.WriteMUID(uidStage);
			// Cmd2.WriteString(pStage->GetMapName());
			Cmd2.WriteString(szMapName);
			Cmd2.WriteBool(false);	// false to normal start, true to ignore this player.
			Cmd2.Finalize(MC_MATCH_STAGE_RELAY_LAUNCH, MCFT_END);
			SendToClient(&Cmd2, (*i).first);
		}
	}
	
	for(map<MUID, MMatchObject *>::iterator i = pStage->ObjBegin(); i != pStage->ObjEnd(); i++)
	{
		MMatchObject *pObj = (*i).second;
		pObj->ResetInGameInfo();
		pObj->m_nStageState = MOSS_WAIT;
		pObj->m_nGameFlag = MMOGF_INGAME | MMOGF_LAUNCHED;
		
		pObj->ReserveStageKick(60000);	// 1 min.
	}
	
	pStage->SetState((int)MMSS_RUN);
	pStage->CreateGame();
	
	SendStageList(pStage->GetChannelUID());
}
*/

// arg 3 : nCountdown is not used.
void OnStageStart(const MUID &uidPlayer, int nCountdown)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_uidStage == MUID(0, 0)) return;
	
	MMatchStage *pStage = g_StageMgr.Get(pObj->m_uidStage);
	if(pStage == NULL) return;
	
	if(pStage->IsMaster(uidPlayer) == false) return;
	if(pStage->IsGameLaunched() == true) return;
	
	// 	NOTE : to send "not ready" message,
	// 	use this format ERRMSG:[errnum] or ERRMSG:[errnum]\a[paramstr]
	// 	with MC_MATCH_ANNOUNCE.
	
	// max player over.
	if(pObj->IsAdmin() == false)
	{
		if(pStage->GetPlayer() > pStage->GetMaxPlayer())
		{
			char szAnnounce[256];
			sprintf(szAnnounce, "ERRMSG:%d", MERR_INCREASE_STAGE_MAX_PLAYER);
			AnnounceToClient(szAnnounce, uidPlayer);
			return;
		}
	}
	
	// check not ready.
	bool bNotReady = false;
	
	for(map<MUID, MMatchObject *>::iterator i = pStage->ObjBegin(); i != pStage->ObjEnd(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		
		if(pCurr->GetUID() == uidPlayer) continue;	// skip checking stage master.
		if(pCurr->m_nStageState == MOSS_READY) continue;
		
		if(pCurr->IsHide() == true) continue;
		
		char szAnnounce[256];
		sprintf(szAnnounce, "ERRMSG:%d\a%s", MERR_PLAYER_NOT_READY, pCurr->m_Char.szName);
		AnnounceToStage(szAnnounce, pObj->m_uidStage);
		
		bNotReady = true;
	}
	
	if(bNotReady == true)
	{
		MCmdWriter temp;
		temp.WriteInt(GAMESTART_ERROR_PLAYER_NOT_READY);
		temp.WriteMUID(MUID(0, 0));
		temp.Finalize(MC_GAME_START_FAIL, MCFT_END);
		SendToClient(&temp, uidPlayer);
		
		return;
	}
	
	if(pStage->GetGameType() == (int)MMGT_SURVIVAL)
	{
		pStage->SetQuestScenarioID(g_Quest.GetSurvivalScenarioID(pStage->GetMapName()));
		
		if(pStage->GetQuestScenarioID() == 0)
		{
			MCmdWriter temp;
			temp.WriteInt(GAMESTART_ERROR_UNAVAILABLE_SCENARIO);
			temp.WriteMUID(MUID(0, 0));
			temp.Finalize(MC_GAME_START_FAIL, MCFT_END);
			SendToClient(&temp, uidPlayer);
			return;
		}
		
		MCmdWriter NPCInfoCmd;
		if(g_Quest.BuildNPCInfoListCommand(pStage->GetQuestScenarioID(), (int)MMGT_SURVIVAL, &NPCInfoCmd) == true)
		{
			SendToStage(&NPCInfoCmd, pObj->m_uidStage);
		}
		
		MCmdWriter QuestGameInfoCmd;
		if(g_Quest.BuildGameInfoCommand(pStage->GetQuestScenarioID(), (int)MMGT_SURVIVAL, &QuestGameInfoCmd) == true)
		{
			SendToStage(&QuestGameInfoCmd, pObj->m_uidStage);
		}
	}
	else if(pStage->GetGameType() == (int)MMGT_QUEST)
	{
		if(pStage->GetQuestScenarioID() == 0)
		{
			MCmdWriter temp;
			temp.WriteInt(GAMESTART_ERROR_UNAVAILABLE_SCENARIO);
			temp.WriteMUID(MUID(0, 0));
			temp.Finalize(MC_GAME_START_FAIL, MCFT_END);
			SendToClient(&temp, uidPlayer);
			return;
		}
		
		// spend sacrifice item.
		for(int i = 0; i < SACRIITEM_SLOT_COUNT; i++)
		{
			MSacriItemSlot *pSlot = pStage->GetQuestSlotItem(i);
			if(pSlot == NULL) continue;
			
			if(pSlot->uidOwner == MUID(0, 0)) continue;
			
			MMatchObject *pOwnerObj = g_ObjectMgr.Get(pSlot->uidOwner);
			if(pOwnerObj != NULL)
			{
				pOwnerObj->SubQuestItem(pSlot->nItemID, 1);
			}
		}
		
		// sync quest item list.
		for(map<MUID, MMatchObject *>::iterator i = pStage->ObjBegin(); i != pStage->ObjEnd(); i++)
		{
			OnCharQuestItemList((*i).first);
		}
		
		MCmdWriter NPCInfoCmd;
		if(g_Quest.BuildNPCInfoListCommand(pStage->GetQuestScenarioID(), (int)MMGT_QUEST, &NPCInfoCmd) == true)
		{
			SendToStage(&NPCInfoCmd, pObj->m_uidStage);
		}
		
		MCmdWriter QuestGameInfoCmd;
		if(g_Quest.BuildGameInfoCommand(pStage->GetQuestScenarioID(), (int)MMGT_QUEST, &QuestGameInfoCmd) == true)
		{
			SendToStage(&QuestGameInfoCmd, pObj->m_uidStage);
		}
	}
	
	ReserveStageAgent(pObj->m_uidStage);
	
	MCmdWriter Cmd;
	if(CheckGameVersion(2013) == true)
	{
		Cmd.WriteInt(0);	// countdown : 0.
		Cmd.Finalize(MC_MATCH_STAGE_START);
	}
	else
	{
		Cmd.WriteMUID(uidPlayer);
		Cmd.WriteMUID(pObj->m_uidStage);
		Cmd.WriteInt(0);	// countdown : 0.
		Cmd.Finalize(MC_MATCH_STAGE_START);
	}
	Cmd.Finalize();
	SendToStage(&Cmd, pObj->m_uidStage);
	
	if(pStage->IsRelayMapStage() == false)
	{
		MCmdWriter Cmd2;
		Cmd2.WriteMUID(pObj->m_uidStage);
		Cmd2.WriteString(pStage->GetMapName());
		Cmd2.Finalize(MC_MATCH_STAGE_LAUNCH, MCFT_END);
		SendToStage(&Cmd2, pObj->m_uidStage);
	}
	else
	{
		pStage->AdvanceRelayMap();
		
		char szMapName[MAX_STAGE_MAPNAME_LENGTH];
		int nMapsetType = g_MapMgr.GetMapsetFromGameType(pStage->GetGameType());
		
		MMatchMap *pMap = g_MapMgr.GetMapFromIndex(pStage->GetCurrRelayMapID(), nMapsetType);
		if(pMap != NULL)
		{
			strcpy(szMapName, pMap->szName);
		}
		else
		{
			strcpy(szMapName, g_MapMgr.GetDefaultMapName(MMMST_GENERAL));
		}
		
		MCmdWriter Cmd2;
		for(map<MUID, MMatchObject *>::iterator i = pStage->ObjBegin(); i != pStage->ObjEnd(); i++)
		{
			Cmd2.ResetIndex();
			
			Cmd2.WriteMUID(pObj->m_uidStage);
			// Cmd2.WriteString(pStage->GetMapName());
			Cmd2.WriteString(szMapName);
			Cmd2.WriteBool(false);	// false to normal start, true to ignore this player.
			Cmd2.Finalize(MC_MATCH_STAGE_RELAY_LAUNCH, MCFT_END);
			SendToClient(&Cmd2, (*i).first);
		}
	}
	
	for(map<MUID, MMatchObject *>::iterator i = pStage->ObjBegin(); i != pStage->ObjEnd(); i++)
	{
		MMatchObject *pObj = (*i).second;
		pObj->ResetInGameInfo();
		pObj->m_nStageState = MOSS_WAIT;
		pObj->m_nGameFlag = MMOGF_INGAME | MMOGF_LAUNCHED;
		
		pObj->ReserveStageKick(60000);	// 1 min.
	}
	
	pStage->SetState((int)MMSS_RUN);
	pStage->CreateGame();
	
	SendStageList(pStage->GetChannelUID());
}

void OnLoadingComplete(const MUID &uidPlayer, int nPercent)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_uidStage == MUID(0, 0)) return;
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidPlayer);
	Cmd.WriteInt(nPercent);
	Cmd.Finalize(MC_MATCH_LOADING_COMPLETE, MCFT_END);
	SendToStage(&Cmd, pObj->m_uidStage);
}

void OnTimeSync(const MUID &uidPlayer, unsigned long nLocalTimeStamp)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	unsigned long nGlobalTimeStamp = GetTime();
	
	MCmdWriter Cmd;
	Cmd.WriteULong(nLocalTimeStamp);
	Cmd.WriteULong(nGlobalTimeStamp);
	Cmd.Finalize(MC_MATCH_GAME_RESPONSE_TIMESYNC, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
}

void OnReportTimeSync(const MUID &uidPlayer, unsigned long nLocalTimeStamp, unsigned long nDataChecksum)
{
	// I don't have any checks here, so do nothing.
}

void OnStageEnterBattle(const MUID &uidPlayer, const MUID &uidStage)
{
	if(uidStage == MUID(0, 0)) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_uidStage != uidStage) return;
	
	if(pObj->CheckGameFlag(MMOGF_INGAME) == false) return;
	if(pObj->CheckGameFlag(MMOGF_ENTERED) == true) return;
	
	MMatchStage *pStage = g_StageMgr.Get(uidStage);
	if(pStage == NULL) return;
	
	// game start process will auto executed by updater thread using this entered flag.
	pObj->m_nGameFlag |= MMOGF_ENTERED;
	pObj->CancelStageKick();
	
	MCmdWriter Cmd;
	
	Cmd.WriteUChar(pObj->CheckGameFlag(MMOGF_FORCED) == false ? MCEP_NORMAL : MCEP_FORCED);
	Cmd.StartBlob(sizeof(MTD_PeerListNode));
	
	MTD_PeerListNode node;
	MakePeerListNode(pObj, &node);
	
	if(IsTeamGame(pStage->GetGameType()) == false)
	{
		node.ExtendInfo.nTeam = MMT_ALL;
	}
	
	Cmd.WriteData(&node, sizeof(MTD_PeerListNode));
	
	Cmd.EndBlob();
	Cmd.Finalize(MC_MATCH_STAGE_ENTERBATTLE, MCFT_END);
	
	// SendToBattle(&Cmd, uidStage);
	SendToStage(&Cmd, uidStage);
	
	if(pStage->CheckGameInfoExists() == true)
	{
		MMatchBaseGame *pGame = pStage->GetGame();
		
		MCmdWriter Cmd2;
		Cmd2.StartBlob(sizeof(MTD_WorldItem));
		
		for(list<MSpawnWorldItem *>::iterator i = pGame->WorldItemBegin(); i != pGame->WorldItemEnd(); i++)
		{
			MSpawnWorldItem *pCurr = (*i);
			
			MTD_WorldItem item;
			item.nUID = pCurr->nUID;
			item.nItemID = (unsigned short)pCurr->nItemID;
			item.nItemSubType = pCurr->nRemovedTime == 0 ? 
				(unsigned short)MTD_WorldItemSubType_Static : (unsigned short)MTD_WorldItemSubType_Dynamic;
			item.x = (short)pCurr->x;
			item.y = (short)pCurr->y;
			item.z = (short)pCurr->z;
			
			Cmd2.WriteData(&item, sizeof(MTD_WorldItem));
		}
		
		Cmd2.EndBlob();
		Cmd2.Finalize(MC_MATCH_SPAWN_WORLDITEM, MCFT_END);
		SendToClient(&Cmd2, uidPlayer);
	}
	
	// TODO : more roundstate send needed.
	if(pStage->IsGameLaunched() == true)
	{
		if(pStage->CheckGameInfoExists() == true)
		{
			if(pStage->GetGameType() == (int)MMGT_DEATHMATCH_TEAM || 
				pStage ->GetGameType() == (int)MMGT_GLADIATOR_TEAM || 
				pStage->GetGameType() == (int)MMGT_ASSASSINATE)
			{
				MMatchGame_BaseTeamDeathmatch *pTeamGame = (MMatchGame_BaseTeamDeathmatch *)pStage->GetGame();
				SendRoundStateToClient(uidStage, pTeamGame->GetCurrRound(), pTeamGame->GetRoundState(), pTeamGame->GetLastRoundStateArg(), uidPlayer);
			}
			else if(pStage->GetGameType() == (int)MMGT_DUEL)
			{
				MMatchGame_DuelMatch *pDuelGame = (MMatchGame_DuelMatch *)pStage->GetGame();
				
				if(pObj->IsHide() == false)
				{
					pDuelGame->AddPlayer(uidPlayer);
				}
				pDuelGame->SendDuelQueueInfo();
				
				SendRoundStateToClient(uidStage, 0, pDuelGame->GetRoundState(), 0, uidPlayer);
			}
			else if(pStage->GetGameType() == (int)MMGT_SPY)
			{
				MMatchGame_Spy *pSpyGame = (MMatchGame_Spy *)pStage->GetGame();
				
				if(pObj->IsHide() == false)
				{
					pSpyGame->JoinPlayer(uidPlayer);
				}
				
				pSpyGame->SendGameInfo();
				pSpyGame->SendScoreInfo(uidPlayer);
				
				SendRoundStateToClient(uidStage, pSpyGame->GetCurrRound(), pSpyGame->GetRoundState(), pSpyGame->GetLastRoundStateArg(), uidPlayer);
			}
			else
			{
				if(pStage->GetGame()->IsPlaying() == true)
				{
					SendRoundStateToClient(uidStage, 0, (int)MMRS_PLAY, 0, uidPlayer);
				}
			}
		}
	}
	
	pObj->m_nPlace = MMP_BATTLE;
	SendChannelPlayerList(pStage->GetChannelUID());
}

void OnPeerList(const MUID &uidPlayer, const MUID &uidStage)
{
	if(uidStage == MUID(0, 0)) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_uidStage != uidStage) return;
	if(pObj->CheckGameFlag(MMOGF_INGAME) == false) return;
	
	MMatchStage *pStage = g_StageMgr.Get(uidStage);
	if(pStage == NULL) return;
	
	MCmdWriter Cmd;
	
	Cmd.WriteMUID(uidStage);
	
	Cmd.StartBlob(sizeof(MTD_PeerListNode));
	for(map<MUID, MMatchObject *>::iterator i = pStage->ObjBegin(); i != pStage->ObjEnd(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		if(pCurr->CheckGameFlag(MMOGF_ENTERED) == false) continue;
		
		MTD_PeerListNode node;
		MakePeerListNode(pCurr, &node);
		
		if(IsTeamGame(pStage->GetGameType()) == false)
		{
			node.ExtendInfo.nTeam = MMT_ALL;
		}
		
		Cmd.WriteData(&node, sizeof(MTD_PeerListNode));
	}
	Cmd.EndBlob();
	
	Cmd.Finalize(MC_MATCH_RESPONSE_PEERLIST, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
}

void OnStageLeaveBattle(const MUID &uidPlayer, bool bFinishLeave)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_uidStage == MUID(0, 0)) return;
	if(pObj->m_nPlace != MMP_BATTLE) return;
	
	MMatchStage *pStage = g_StageMgr.Get(pObj->m_uidStage);
	if(pStage == NULL) return;
	
	// TODO : add more conditions.
	
	if(pObj->m_bAgentUser == true)
	{
		UnbindAgentClient(uidPlayer);
		pObj->m_bAgentUser = false;
	}
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidPlayer);
	// Cmd.WriteBool(false);	// IsRelayMapPlay. : RelayMap == TRUE && bFinishLeave == TRUE.
	Cmd.WriteBool(pStage->IsRelayMapStage() == true && bFinishLeave == true ? true : false);
	Cmd.Finalize(MC_MATCH_STAGE_LEAVEBATTLE_TO_CLIENT, MCFT_END);
	SendToStage(&Cmd, pStage->GetUID());
	
	pObj->m_nGameFlag = MMOGF_NONE;
	pObj->m_nPlace = MMP_STAGE;
	
	if(pStage->GetGameType() == (int)MMGT_TRAINING || IsQuestGame(pStage->GetGameType()) == true)
	{
		// don't save kill/death when training. and quest.
		pObj->m_GameInfo.nKill = pObj->m_GameInfo.nDeath = 0;
	}
	pObj->SaveExp();
	
	// save spendable item count.
	pObj->SaveSpentItem();
	
	// sync/save cash remainder. (for in-game cash bonus.)
	pObj->SyncCash(true);
	
	// game info.
	if(pStage->CheckGameInfoExists() == true)
	{
		if(pStage->GetGameType() == (int)MMGT_BERSERKER)
		{
			MMatchGame_Berserker *pGame = (MMatchGame_Berserker *)pStage->GetGame();
			
			if(pGame->GetBerserkerUID() == uidPlayer)
			{
				pGame->AssignBerserker(MUID(0, 0));
			}
		}
		else if(pStage->GetGameType() == (int)MMGT_DUEL)
		{
			MMatchGame_DuelMatch *pGame = (MMatchGame_DuelMatch *)pStage->GetGame();
			
			if(pObj->IsHide() == false)
			{
				pGame->RemovePlayer(uidPlayer);
			}
			pGame->SendDuelQueueInfo();
		}
		else if(pStage->GetGameType() == (int)MMGT_DUEL_TOURNAMENT)
		{
			MMatchGame_DuelTournament *pGame = (MMatchGame_DuelTournament *)pStage->GetGame();
			pGame->SendGameInfo();
			
			if(pGame->IsMatchingPlayer(uidPlayer) == true)
			{
				// as a penalty, take out 20 point from the player.
				pObj->m_DuelTournament.nTP = CheckMinusOver(pObj->m_DuelTournament.nTP, 20);
			}
		}
		else if(pStage->GetGameType() == (int)MMGT_SURVIVAL || pStage->GetGameType() == (int)MMGT_QUEST)
		{
			MMatchGame_Quest *pGame = (MMatchGame_Quest *)pStage->GetGame();
			pGame->ReSelectNPCOwner();
		}
		else if(pStage->GetGameType() == (int)MMGT_CHALLENGE_QUEST)
		{
			MMatchGame_ChallengeQuest *pGame = (MMatchGame_ChallengeQuest *)pStage->GetGame();
			pGame->ReSelectActorOwner();
		}
		else if(pStage->GetGameType() == (int)MMGT_BLITZKRIEG)
		{
			MMatchGame_BlitzKrieg *pGame = (MMatchGame_BlitzKrieg *)pStage->GetGame();
			pGame->ReSelectActorOwner();
			
			if(pGame->IsFinished() == false)
			{
				pObj->m_BlitzKrieg.nLose++;
				
				static const int nPenaltyPoint = 15;
				static const int nPenaltyMedal = 10;
				
				// penalty : -15 point and -10 medal.
				pObj->m_BlitzKrieg.nPoint = CheckMinusOver(pObj->m_BlitzKrieg.nPoint, nPenaltyPoint);
				pObj->m_BlitzKrieg.nMedal = CheckMinusOver(pObj->m_BlitzKrieg.nMedal, nPenaltyMedal);
				
				pObj->BlitzPenalty(nPenaltyMedal, pObj->m_BlitzKrieg.nPoint);
			}
		}
		else if(pStage->GetGameType() == (int)MMGT_SPY)
		{
			MMatchGame_Spy *pSpyGame = (MMatchGame_Spy *)pStage->GetGame();
			
			if(pObj->IsHide() == false)
			{
				pSpyGame->LeavePlayer(uidPlayer);
			}
		}
	}
	
	SendObjectCache((int)MMOCT_UPDATE, pStage, uidPlayer);
	
	// expire item.
	CheckExpiredCharItem(pObj);
	
	// refresh item list.
	OnCharacterItemList(uidPlayer);
	
	bool bStartRelayGame = pStage->IsRelayMapStage() == true && pStage->GetRelayMapSetting()->IsUnFinish() == true ? true : false ;
	
	if(bStartRelayGame == true)
	{
		if(bFinishLeave == true)
		{
			pObj->m_nStageState = MOSS_READY;
		}
	}
	
	// delete gameinfo.
	if(pStage->CheckInGamePlayerCount() <= 0)
	{
		if(pStage->CheckGameInfoExists() == true)
		{
			if(pStage->GetGame()->IsFinished() == false)
			{
				bStartRelayGame = false;
				
				if(IsEndableGameByLeave(pStage->GetGameType()) == true)
				{
					pStage->GetRelayMapSetting()->ForceFinish();
					pStage->GetGame()->Finish();
					
					pStage->DestroyGame();
				}
			}
			else
			{
				pStage->DestroyGame();
			}
		}
	}
	else
	{
		bStartRelayGame = false;
	}
	
	if(bStartRelayGame == true)
	{
		LaunchRelayMapStage(pStage);
	}
	
	if(pStage->GetGameType() == (int)MMGT_DUEL_TOURNAMENT)
	{
		OnStageLeave(uidPlayer);
	}
}

/*
void OnForceEntry(const MUID &uidPlayer, const MUID &uidStage)
{
	if(uidStage == MUID(0, 0)) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_uidStage != uidStage) return;
	
	if(pObj->m_nPlace != MMP_STAGE) return;
	if(pObj->CheckGameFlag(MMOGF_INGAME) == true) return;
	
	MCmdWriter Cmd;
	Cmd.WriteInt(0);	// ok.
	Cmd.Finalize(MC_MATCH_STAGE_RESPONSE_FORCED_ENTRY, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
	
	pObj->ResetInGameInfo();
	pObj->m_nStageState = MOSS_WAIT;
	pObj->m_nGameFlag = MMOGF_INGAME | MMOGF_FORCED;
}
*/

void OnForceEntry(const MUID &uidPlayer)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_uidStage == MUID(0, 0)) return;
	
	if(pObj->m_nPlace != MMP_STAGE) return;
	if(pObj->CheckGameFlag(MMOGF_INGAME) == true) return;
	
	MCmdWriter Cmd;
	Cmd.WriteInt(0);	// ok.
	Cmd.Finalize(MC_MATCH_STAGE_RESPONSE_FORCED_ENTRY, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
	
	pObj->ResetInGameInfo();
	pObj->m_nStageState = MOSS_WAIT;
	pObj->m_nGameFlag = MMOGF_INGAME | MMOGF_FORCED;
}

void OnGameInfo(const MUID &uidPlayer, const MUID &uidStage)
{
	if(uidStage == MUID(0, 0)) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_uidStage != uidStage) return;
	if(pObj->CheckGameFlag(MMOGF_ENTERED) == false) return;
	
	MMatchStage *pStage = g_StageMgr.Get(uidStage);
	if(pStage == NULL) return;
	
	if(pStage->CheckGameInfoExists() == false) return;
	
	MCmdWriter Cmd;
	
	// 1 - stage UID.
	Cmd.WriteMUID(uidStage);
	
	// 2 - game info.
	Cmd.StartBlob(sizeof(MTD_GameInfo));
	
	MTD_GameInfo ginfo;
	ZeroInit(&ginfo, sizeof(MTD_GameInfo));
	
	if(pStage->GetGameType() == (int)MMGT_DEATHMATCH_TEAM || 
		pStage->GetGameType() == (int)MMGT_GLADIATOR_TEAM || 
		pStage->GetGameType() == (int)MMGT_ASSASSINATE)
	{
		MMatchGame_BaseTeamDeathmatch *pTeamGame = (MMatchGame_BaseTeamDeathmatch *)pStage->GetGame();
		
		int nRed, nBlue;
		pTeamGame->GetTeamScore(&nRed, &nBlue);
		
		ginfo.nRedTeamScore = (char)nRed;
		ginfo.nBlueTeamScore = (char)nBlue;
	}
	else if(pStage->GetGameType() == (int)MMGT_DEATHMATCH_TEAM_EXTREME)
	{
		MMatchGame_TeamDeathmatchExtreme *pTeamExGame = (MMatchGame_TeamDeathmatchExtreme *)pStage->GetGame();
		
		int nRed, nBlue;
		pTeamExGame->GetTeamKills(&nRed, &nBlue);
		
		ginfo.nRedTeamKills = (short)nRed;
		ginfo.nBlueTeamKills = (short)nBlue;
	}
	
	Cmd.WriteData(&ginfo, sizeof(MTD_GameInfo));
	Cmd.EndBlob();
	
	// 3 - rule info.
	// TODO : suitable rule info. see MTD_RuleInfo.
	if(pStage->GetGameType() == (int)MMGT_BERSERKER)
	{
		Cmd.StartBlob(sizeof(MTD_RuleInfo_Berserker));
		
		MTD_RuleInfo_Berserker rinfo;
		rinfo.nRuleType = (unsigned char)pStage->GetGameType();
		rinfo.uidBerserker = MUID(0, 0);
		
		// if(pStage->CheckGameInfoExists() == true)
		{
			rinfo.uidBerserker = ((MMatchGame_Berserker *)pStage->GetGame())->GetBerserkerUID();
		}
		
		Cmd.WriteData(&rinfo, sizeof(MTD_RuleInfo_Berserker));
		Cmd.EndBlob();
	}
	else if(pStage->GetGameType() == (int)MMGT_ASSASSINATE)
	{
		MMatchGame_Assassinate *pAssaGame = (MMatchGame_Assassinate *)pStage->GetGame();
		
		Cmd.StartBlob(sizeof(MTD_RuleInfo_Assassinate));
		
		MTD_RuleInfo_Assassinate rinfo;
		
		rinfo.nRuleType = (unsigned char)pStage->GetGameType();
		rinfo.uidRedCommander = pAssaGame->GetRedTeamCommanderUID();
		rinfo.uidBlueCommander = pAssaGame->GetBlueTeamCommanderUID();
		
		Cmd.WriteData(&rinfo, sizeof(MTD_RuleInfo_Assassinate));
		Cmd.EndBlob();
	}
	else
	{
		Cmd.StartBlob(0);
		Cmd.EndBlob();
	}
	
	// 4 - player info.
	Cmd.StartBlob(sizeof(MTD_GameInfoPlayerItem));
	for(map<MUID, MMatchObject *>::iterator i = pStage->ObjBegin(); i != pStage->ObjEnd(); i++)
	{
		MMatchObject *pCurrObj = (*i).second;
		if(pCurrObj->CheckGameFlag(MMOGF_ENTERED) == false) continue;
		
		MTD_GameInfoPlayerItem item;
		item.uidPlayer = pCurrObj->GetUID();
		item.bAlive = pCurrObj->m_GameInfo.bAlive;
		item.nKillCount = pCurrObj->m_GameInfo.nKill;
		item.nDeathCount = pCurrObj->m_GameInfo.nDeath;
		
		if(pObj->IsHide() == false)
		{
			// for solve tdm problem : when forced join, the other players looks dead.
			// if(pStage->CheckGameInfoExists() == true)
			{
				if(pStage->GetGame()->GetRoundState() != (int)MMRS_PLAY)
				{
					item.bAlive = true;
				}
			}
		}
		
		Cmd.WriteData(&item, sizeof(MTD_GameInfoPlayerItem));
	}
	Cmd.EndBlob();
	
	Cmd.Finalize(MC_MATCH_RESPONSE_GAME_INFO, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
}

void OnGameSpawn(const MUID &uidPlayer, const FloatVector *pPos, const FloatVector *pDir)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_uidStage == MUID(0, 0)) return;
	if(pObj->CheckGameFlag(MMOGF_ENTERED) == false) return;
	
	if(pObj->IsValidSpawn() == false) return;
	
	if(pObj->IsHide() == true) return;
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidPlayer);
	Cmd.WriteSVec((short)pPos->x, (short)pPos->y, (short)pPos->z);
	Cmd.WriteSVec((short)(pDir->x * 32000.0f), (short)(pDir->y * 32000.0f), (short)(pDir->z * 32000.0f));
	Cmd.Finalize(MC_MATCH_GAME_RESPONSE_SPAWN, MCFT_END);
	
	SendToBattle(&Cmd, pObj->m_uidStage);
	
	pObj->m_GameInfo.bAlive = true;
}

void OnSpawnWorldItem(const MUID &uidPlayer, int nItemID, const FloatVector *pPos, float fDropTime)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_uidStage == MUID(0, 0)) return;
	if(pObj->CheckGameFlag(MMOGF_ENTERED) == false) return;
	
	MMatchStage *pStage = g_StageMgr.Get(pObj->m_uidStage);
	if(pStage == NULL) return;
	
	// TODO : add checks.
	
	if(pStage->CheckGameInfoExists() == true)
	{
		// don't drop witem when round state is not play. : in mostly case, the world item is updated only with MMRS_PLAY.
		// if we added the item without update process, drop witem queue will be bloated.
		if(pStage->GetGame()->GetRoundState() == (int)MMRS_PLAY)
		{
			pStage->GetGame()->DropWorldItem(nItemID, pPos->x, pPos->y, pPos->z, (unsigned long)(fDropTime * 1000));
		}
	}
}

void OnObtainWorldItem(const MUID &uidPlayer, int nItemUID)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_uidStage == MUID(0, 0)) return;
	if(pObj->CheckGameFlag(MMOGF_ENTERED) == false) return;
	
	MMatchStage *pStage = g_StageMgr.Get(pObj->m_uidStage);
	if(pStage == NULL) return;
	
	// TODO : add checks.
	
	if(pStage->CheckGameInfoExists() == true)
	{
		if(pStage->GetGame()->TakeWorldItem(uidPlayer, (unsigned short)nItemUID) == false) return;
		
		MCmdWriter Cmd;
		
		Cmd.WriteMUID(uidPlayer);
		Cmd.WriteInt(nItemUID);
		Cmd.Finalize(MC_MATCH_OBTAIN_WORLDITEM /*, MCFT_END*/);
		
		if(IsQuestDerived((int)pStage->GetGameType()) == true)
		{
			MMatchGame_Quest *pQuestGame = (MMatchGame_Quest *)pStage->GetGame();
			
			int nTookBoxItemID = pQuestGame->TakeNPCDropItem((unsigned short)nItemUID);
			if(nTookBoxItemID != 0)
			{
				if(IsQuestItemID(nTookBoxItemID) == true)
				{
					Cmd.WriteUInt((unsigned int)nTookBoxItemID);
					Cmd.Finalize(MC_QUEST_OBTAIN_QUESTITEM);
				}
				else
				{
					Cmd.WriteUInt((unsigned int)nTookBoxItemID);
					Cmd.Finalize(MC_QUEST_OBTAIN_ZITEM);
				}
			}
		}
		
		Cmd.Finalize();
		SendToBattle(&Cmd, pStage->GetUID());
	}
}

// uidVictim = Sender of this command.
void OnGameKill(const MUID &uidAttacker, const MUID &uidVictim)
{
	MMatchObject *pVictimObj = g_ObjectMgr.Get(uidVictim);
	if(pVictimObj == NULL) return;
	
	if(pVictimObj->m_uidStage == MUID(0, 0)) return;
	if(pVictimObj->CheckGameFlag(MMOGF_ENTERED) == false) return;
	
	if(pVictimObj->m_GameInfo.bAlive == false) return;
	
	MMatchObject *pAttackerObj = g_ObjectMgr.Get(uidAttacker);
	if(pAttackerObj == NULL) return;
	
	if(pAttackerObj->m_uidStage != pVictimObj->m_uidStage) return;
	if(pAttackerObj->CheckGameFlag(MMOGF_ENTERED) == false) return;
	
	MMatchStage *pStage = g_StageMgr.Get(pVictimObj->m_uidStage);
	if(pStage == NULL) return;
	
	bool bSuicided = uidAttacker == uidVictim ? true : false ;
	
	int nCurrAttackerLevel = pAttackerObj->m_Exp.nLevel, nCurrVictimLevel = pVictimObj->m_Exp.nLevel;
	unsigned long nAttackerExp = 0, nAttackerBounty = 0, nVictimExp = 0, nVictimBounty = 0;
	
	// bool bWinExp = pStage->GetGameType() != (int)MMGT_TRAINING ? true : false ;
	
	bool bWinExp = true;
	
	// training : no need exp. and quest too.
	if(pStage->GetGameType() == (int)MMGT_TRAINING || IsQuestGame(pStage->GetGameType()) == true) 
		bWinExp = false;
	
	// friendly kill.
	if(IsTeamGame(pStage->GetGameType()) == true)
		if(bSuicided == false && pAttackerObj->m_GameInfo.nTeam == pVictimObj->m_GameInfo.nTeam)
			bWinExp = false;
	
	if(bWinExp == true)
	{
		if(bSuicided == false)
		{
			if(g_FormulaMgr.GetKillExp(nCurrVictimLevel, &nAttackerExp, &nAttackerBounty) == false) return;
			
			// TODO : calc more bonus.
			
			if(pStage->GetGameType() == (int)MMGT_BERSERKER)
			{
				if(pStage->CheckGameInfoExists() == true)
				{
					if(((MMatchGame_Berserker *)pStage->GetGame())->GetBerserkerUID() == uidVictim)
					{
						nAttackerExp *= 2;
						nAttackerBounty *= 2;
					}
				}
			}
			else if(pStage->GetGameType() == (int)MMGT_ASSASSINATE)
			{
				if(pStage->CheckGameInfoExists() == true)
				{
					MMatchGame_Assassinate *pGame = (MMatchGame_Assassinate *)pStage->GetGame();
					
					if(pGame->GetRedTeamCommanderUID() == uidVictim || 
						pGame->GetBlueTeamCommanderUID() == uidVictim)
					{
						nAttackerExp *= 2;
						nAttackerBounty *= 2;
					}
				}
			}
		}
		else
		{
			if(g_FormulaMgr.GetKillExp(nCurrVictimLevel, &nVictimExp, &nVictimBounty) == false) return;
			
			nVictimExp *= 2;
			nVictimBounty *= 2;
		}
	}
	
	// signed int for check minus (invalid) exp.
	int nNewAttackerTotalExp = CheckPlusOver(pAttackerObj->m_Exp.nXP, pAttackerObj->m_GameInfo.nExp + (int)nAttackerExp);
	int nNewVictimTotalExp = CheckPlusOver(pVictimObj->m_Exp.nXP, pVictimObj->m_GameInfo.nExp - (int)nVictimExp);
	int nNewAttackerLevel = g_FormulaMgr.GetLevelFromExp((int)nNewAttackerTotalExp);
	int nNewVictimLevel = g_FormulaMgr.GetLevelFromExp((int)nNewVictimTotalExp);
	
	int nAttackerExpPercent = g_FormulaMgr.GetExpPercent(nNewAttackerLevel, nNewAttackerTotalExp);
	int nVictimExpPercent = g_FormulaMgr.GetExpPercent(nNewVictimLevel, nNewVictimTotalExp);
	
	MCmdWriter Cmd;
	
	Cmd.WriteMUID(uidAttacker);
	Cmd.WriteULong(MakeExpCommandData(nAttackerExp, nAttackerExpPercent));
	Cmd.WriteMUID(uidVictim);
	Cmd.WriteULong(MakeExpCommandData(nVictimExp, nVictimExpPercent));
	Cmd.Finalize(MC_MATCH_GAME_DEAD);
	
	if(bSuicided == false)
	{
		if(nNewAttackerLevel > nCurrAttackerLevel)
		{
			Cmd.WriteMUID(uidAttacker);
			Cmd.WriteInt(nNewAttackerLevel);
			Cmd.Finalize(MC_MATCH_GAME_LEVEL_UP);
		}
		else if(nNewAttackerLevel < nCurrAttackerLevel)
		{
			Cmd.WriteMUID(uidAttacker);
			Cmd.WriteInt(nNewAttackerLevel);
			Cmd.Finalize(MC_MATCH_GAME_LEVEL_DOWN);
		}
	}
	
	if(nNewVictimLevel > nCurrVictimLevel)
	{
		Cmd.WriteMUID(uidVictim);
		Cmd.WriteInt(nNewVictimLevel);
		Cmd.Finalize(MC_MATCH_GAME_LEVEL_UP);
	}
	else if(nNewVictimLevel < nCurrVictimLevel)
	{
		Cmd.WriteMUID(uidVictim);
		Cmd.WriteInt(nNewVictimLevel);
		Cmd.Finalize(MC_MATCH_GAME_LEVEL_DOWN);
	}
	
	Cmd.Finalize();
	SendToBattle(&Cmd, pStage->GetUID());
	
	// special game type process.
	if(pStage->CheckGameInfoExists() == true)
	{
		if(pStage->GetGameType() == (int)MMGT_BERSERKER)
		{
			MMatchGame_Berserker *pGame = (MMatchGame_Berserker *)pStage->GetGame();
			
			MUID uidCurrBerserker = pGame->GetBerserkerUID();
			if(uidCurrBerserker == uidVictim || uidCurrBerserker == MUID(0, 0))
			{
				MUID uidBerserker = bSuicided == false && pAttackerObj->m_GameInfo.bAlive == true ? uidAttacker : MUID(0, 0) ;
				pGame->AssignBerserker(uidBerserker);
			}
		}
	}
	
	
	pVictimObj->m_GameInfo.bAlive = false;
	pVictimObj->m_Exp.nLevel = nNewVictimLevel;
	pVictimObj->m_GameInfo.nExp -= (int)nVictimExp;
	pVictimObj->m_Exp.nBP = CheckMinusOver(pVictimObj->m_Exp.nBP, (int)nVictimBounty);
	pVictimObj->m_GameInfo.nDeath++;
	pVictimObj->UpdateSpawnTime();	// spawn timer update for next spawn.
	
	pAttackerObj->m_Exp.nLevel = nNewAttackerLevel;
	pAttackerObj->m_GameInfo.nExp += (int)nAttackerExp;
	pAttackerObj->m_Exp.nBP += (int)nAttackerBounty;
	if(bSuicided == false) 
	{
		pAttackerObj->m_GameInfo.nKill++;
		pAttackerObj->m_Account.nCash += g_nCashBonus[CASH_BONUS_DEATHMATCH];
	}
	
	// check game finishable.
	if(IsIndividualGame(pStage->GetGameType()) == true)
	{
		if(pAttackerObj->m_GameInfo.nKill >= pStage->GetRound())
		{
			if(pStage->CheckGameInfoExists() == true)
			{
				pStage->GetGame()->Finish();
			}
		}
	}
	// increase tdm-ex kills.
	else if(pStage->GetGameType() == (int)MMGT_DEATHMATCH_TEAM_EXTREME)
	{
		if(pStage->CheckGameInfoExists() == true)
		{
			MMatchGame_TeamDeathmatchExtreme *pTeamExGame = (MMatchGame_TeamDeathmatchExtreme *)pStage->GetGame();
			
			if(pVictimObj->m_GameInfo.nTeam == MMT_RED)
			{
				pTeamExGame->RedDead();
			}
			else if(pVictimObj->m_GameInfo.nTeam == MMT_BLUE)
			{
				pTeamExGame->BlueDead();
			}
		}
	}
	// increase spy point.
	else if(pStage->GetGameType() == (int)MMGT_SPY)
	{
		if(bWinExp == true && bSuicided == false)
		{
			if(pStage->CheckGameInfoExists() == true)
			{
				MMatchGame_Spy *pSpyGame = (MMatchGame_Spy *)pStage->GetGame();
				MSpyPlayerStatus *pPlayerStatus = pSpyGame->GetPlayerStatus(uidAttacker);
				
				if(pPlayerStatus != NULL)
				{
					pPlayerStatus->m_nAddPoint += 1;
				}
			}
		}
	}
}

void OnGameSuicide(const MUID &uidPlayer)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_uidStage == MUID(0, 0)) return;
	if(pObj->CheckGameFlag(MMOGF_ENTERED) == false) return;
	
	if(pObj->m_GameInfo.bAlive == false) return;
	
	MMatchStage *pStage = g_StageMgr.Get(pObj->m_uidStage);
	if(pStage == NULL) return;
	
	MCmdWriter Cmd;
	Cmd.WriteInt(0);	// ok.
	Cmd.WriteMUID(uidPlayer);
	Cmd.Finalize(MC_MATCH_RESPONSE_SUICIDE, MCFT_END);
	SendToBattle(&Cmd, pObj->m_uidStage);
	
	if(pStage->GetGameType() == (int)MMGT_SURVIVAL || pStage->GetGameType() == (int)MMGT_QUEST)
	{
		OnQuestPlayerDead(uidPlayer);
	}
	else if(pStage->GetGameType() == (int)MMGT_CHALLENGE_QUEST)
	{
		OnChallengeQuestPlayerDead(uidPlayer);
	}
	else if(pStage->GetGameType() == (int)MMGT_BLITZKRIEG)
	{
		OnBlitzPlayerDead(uidPlayer, uidPlayer, NULL);
	}
	else 
	{
		OnGameKill(uidPlayer, uidPlayer);
	}
}

void OnCallvote(const MUID &uidPlayer, const char *pszDiscuss, const char *pszArg)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	if(pObj->m_uidStage == MUID(0, 0)) return;
	if(pObj->CheckGameFlag(MMOGF_ENTERED) == false) return;
	
	MMatchStage *pStage = g_StageMgr.Get(pObj->m_uidStage);
	if(pStage == NULL) return;
	
	if(MStricmp(pszDiscuss, "kick") != 0)	// discuss string : always "kick".
	{
		char szMsg[256];
		sprintf(szMsg, "ERRMSG:%d", MERR_VOTE_FAILED);
		AnnounceToClient(szMsg, uidPlayer);
		return;
	}
	
	MMatchObject *pTargetObj = g_ObjectMgr.Get(pszArg);
	if(pTargetObj == NULL) return;
	
	if(pTargetObj->m_bCharInfoExist == false) return;
	
	if(pTargetObj->m_uidStage != pObj->m_uidStage) return;
	if(pTargetObj->CheckGameFlag(MMOGF_ENTERED) == false) return;
	
	if(pObj->IsAdmin() == false)
	{
		int nRet = CheckCallVotable(pObj, pTargetObj, pStage);
		
		if(nRet != MSG_OK)
		{
			char szMsg[256];
			sprintf(szMsg, "ERRMSG:%d", nRet);
			AnnounceToClient(szMsg, uidPlayer);
			return;
		}
	
		if(pStage->LaunchVote(uidPlayer, pTargetObj->GetUID()) == false)
		{
			char szMsg[256];
			sprintf(szMsg, "ERRMSG:%d", MERR_VOTE_FAILED);
			AnnounceToClient(szMsg, uidPlayer);
			return;
		}
	}
	else
	{
		if(pTargetObj->IsAdmin() == true)
		{
			AnnounceToClient("Administrators can't kick each other administrators.", uidPlayer);
			return;
		}
		
		if(pStage->LaunchVote(uidPlayer, pTargetObj->GetUID()) == true)
		{
			pStage->AbortVoting(true);
		}
	}
}

void OnVoteYes(const MUID &uidPlayer)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	if(pObj->m_uidStage == MUID(0, 0)) return;
	if(pObj->CheckGameFlag(MMOGF_ENTERED) == false) return;
	
	MMatchStage *pStage = g_StageMgr.Get(pObj->m_uidStage);
	if(pStage == NULL) return;
	
	if(pObj->m_bVoted == true) return;
	if(pStage->IsValidVoter(uidPlayer) == false) return;
	
	pObj->m_bVoted = true;
	
	pStage->Vote(uidPlayer, true);	// yes.
}

void OnVoteNo(const MUID &uidPlayer)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	if(pObj->m_uidStage == MUID(0, 0)) return;
	if(pObj->CheckGameFlag(MMOGF_ENTERED) == false) return;
	
	MMatchStage *pStage = g_StageMgr.Get(pObj->m_uidStage);
	if(pStage == NULL) return;
	
	if(pObj->m_bVoted == true) return;
	if(pStage->IsValidVoter(uidPlayer) == false) return;
	
	pObj->m_bVoted = true;
	
	pStage->Vote(uidPlayer, false);	// no.
}

// for 2012 or higher.
void OnVote(const MUID &uidPlayer, bool bAgreed)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	if(pObj->m_uidStage == MUID(0, 0)) return;
	if(pObj->CheckGameFlag(MMOGF_ENTERED) == false) return;
	
	MMatchStage *pStage = g_StageMgr.Get(pObj->m_uidStage);
	if(pStage == NULL) return;
	
	if(pObj->m_bVoted == true) return;
	if(pStage->IsValidVoter(uidPlayer) == false) return;
	
	pObj->m_bVoted = true;
	
	pStage->Vote(uidPlayer, bAgreed);
}

void OnStageFollow(const MUID &uidPlayer, const char *pszTargetName)
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
	
	// channel.
	if(pTargetObj->m_uidChannel == MUID(0, 0)) return;
	
	MMatchChannel *pChannel = g_ChannelMgr.Get(pTargetObj->m_uidChannel);
	if(pChannel == NULL) return;
	
	if(pChannel->GetType() == MCHANNEL_TYPE_DUELTOURNAMENT || pChannel->GetType() == MCHANNEL_TYPE_BLITZKRIEG) return;
	
	// stage.
	if(pTargetObj->m_uidStage == MUID(0, 0)) return;
	
	MMatchStage *pStage = g_StageMgr.Get(pTargetObj->m_uidStage);
	if(pStage == NULL) return;
	
	if(pStage->IsPrivate() == false)
	{
		OnStageJoin(uidPlayer, pTargetObj->m_uidStage);
	}
	else
	{
		Cmd.WriteMUID(pTargetObj->m_uidStage);
		Cmd.WriteString(pStage->GetName());
		Cmd.Finalize(MC_MATCH_STAGE_REQUIRE_PASSWORD, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
	}
}

// param 2 : pParam is not used.
void OnStageQuickJoin(const MUID &uidPlayer, const MTD_QuickJoinParam *pParam)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	if(pObj->m_uidStage != MUID(0, 0)) return;
	
	MUID uidChannel = pObj->m_uidChannel;
	
	MMatchChannel *pChannel = g_ChannelMgr.Get(uidChannel);
	if(pChannel == NULL) return;
	
	// TODO : channel type check, clan war.
	if(pChannel->GetType() == MCHANNEL_TYPE_DUELTOURNAMENT || pChannel->GetType() == MCHANNEL_TYPE_BLITZKRIEG) return;
	
	vector<MUID> vtStageCandidate;
	
	for(list<MMatchStage *>::iterator i = g_StageMgr.Begin(); i != g_StageMgr.End(); i++)
	{
		MMatchStage *pCurr = (*i);
		
		if(pCurr->GetChannelUID() != uidChannel) continue;
		
		if(pCurr->IsPrivate() == true) continue;
		if(pCurr->CheckLevelLimit(pObj->m_Exp.nLevel) == false) continue;
		if(pCurr->IsForceJoinable() == false) continue;
		if(pCurr->IsMaxPlayersReached() == true) continue;
		
		vtStageCandidate.push_back(pCurr->GetUID());
	}
	
	MUID uidResult = MUID(0, 0);
	
	if(vtStageCandidate.empty() == false)
	{
		int nRand = (int)((unsigned int)RandNum() % (unsigned int)vtStageCandidate.size());
		uidResult = vtStageCandidate[nRand];
	}
	
	int nErrorID = uidResult != MUID(0, 0) ? MSG_OK : MERR_JOINABLE_STAGE_NOT_AVAILABLE ;
	
	if(CheckGameVersion(2012) == true)
	{
		MCmdWriter Cmd;
		Cmd.WriteInt(nErrorID);
		Cmd.Finalize(MC_MATCH_STAGE_RESPONSE_QUICKJOIN, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		
		OnStageJoin(uidPlayer, uidResult);
	}
	else
	{
		MCmdWriter Cmd;
		Cmd.WriteInt(nErrorID);
		Cmd.WriteMUID(uidResult);
		Cmd.Finalize(MC_MATCH_STAGE_RESPONSE_QUICKJOIN, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
	}
}

void OnStageGo(const MUID &uidPlayer, int nStageNo)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	if(pObj->m_uidStage != MUID(0, 0)) return;
	
	MUID uidChannel = pObj->m_uidChannel;
	
	MMatchChannel *pChannel = g_ChannelMgr.Get(uidChannel);
	if(pChannel == NULL) return;
	
	// TODO : clan war check.
	if(pChannel->GetType() == MCHANNEL_TYPE_DUELTOURNAMENT || pChannel->GetType() == MCHANNEL_TYPE_BLITZKRIEG) return;
	
	MMatchStage *pTargetStage = NULL;
	
	for(list<MMatchStage *>::iterator i = g_StageMgr.Begin(); i != g_StageMgr.End(); i++)
	{
		MMatchStage *pCurr = (*i);
		
		if(pCurr->GetChannelUID() != uidChannel) continue;
		if(pCurr->GetNumber() != nStageNo) continue;
		
		pTargetStage = pCurr;
		break;
	}
	
	if(pTargetStage != NULL)
	{
		if(pTargetStage->IsPrivate() == false)
		{
			OnStageJoin(uidPlayer, pTargetStage->GetUID());
		}
		else
		{
			MCmdWriter Cmd;
			Cmd.WriteMUID(pTargetStage->GetUID());
			Cmd.WriteString(pTargetStage->GetName());
			Cmd.Finalize(MC_MATCH_STAGE_REQUIRE_PASSWORD, MCFT_END);
			SendToClient(&Cmd, uidPlayer);
		}
	}
	else
	{
		MCmdWriter Cmd;
		Cmd.WriteInt(MERR_STAGE_NOT_EXISTS);
		Cmd.Finalize(MC_MATCH_RESPONSE_STAGE_JOIN, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
	}
}

void OnUseNormalSpendableItem(const MUID &uidPlayer, const MUID &uidItem)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	if(pObj->m_uidEquippedItem[MMCIP_CUSTOM1] == uidItem)
	{
		pObj->m_GameInfo.nSpendItemUsed[0]++;
	}
	else if(pObj->m_uidEquippedItem[MMCIP_CUSTOM2] == uidItem)
	{
		pObj->m_GameInfo.nSpendItemUsed[1]++;
	}
}

void OnStageKick(const MUID &uidPlayer, const char *pszTargetName)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;

	if(pObj->m_uidStage == MUID(0, 0)) return;
	
	MMatchStage *pStage = g_StageMgr.Get(pObj->m_uidStage);
	if(pStage == NULL) return;
	
	if(pStage->IsMaster(uidPlayer) == false) return;
	
	// check object...
	MMatchObject *pTargetObj = g_ObjectMgr.Get(pszTargetName);
	if(pTargetObj == NULL) return;
			
	if(pTargetObj->m_bCharInfoExist == false) return;
			
	if(pTargetObj->m_uidStage != pObj->m_uidStage) return;
	if(pTargetObj->CheckGameFlag(MMOGF_INGAME) == true) return;
			
	// finally kick from stage.
	OnStageLeave(pTargetObj->GetUID());
}


void SendStageList(const MUID &uidChannel)
{
	for(list<MMatchObject *>::iterator i = g_ObjectMgr.Begin(); i != g_ObjectMgr.End(); i++)
	{
		MMatchObject *pCurrObj = (*i);
		
		if(pCurrObj->m_uidChannel == uidChannel)
			if(pCurrObj->m_nPlace == MMP_LOBBY)
				OnStageList(pCurrObj->GetUID(), uidChannel, pCurrObj->m_nLastStageListPage, true);
	}
}