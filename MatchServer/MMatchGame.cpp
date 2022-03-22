#include "pch.h"

#include "MMatchGame.h"
#include "MMatchStage.h"
#include "MMatchServer_Etc.h"

#include "MMatchMap.h"

#include "MMatchChannel.h"

#include "MMatchQuest.h"
#include "MMatchExp.h"

#include "MMatchChallengeQuest.h"
#include "MMatchGambleItem.h"

#include "MMatchDBMgr.h"
#include "MAsyncDBProcess.h"

#include "MMatchSpy.h"

#include "MMatchCashShop.h"

#include "MMatchServer_OnCommand.h"

// from MMatchServer_Stage.cpp .
void CreateStageSettingCommand(MMatchStage *pStage, MCommandWriter *pOut);
void SendStageSettingToStage(MMatchStage *pStage);

unsigned long MakeExpCommandData(unsigned long nAddXP, int nPercent);


// game type checker.
bool IsIndividualGame(int nGameType)
{
	if(nGameType == (int)MMGT_DEATHMATCH_SOLO || 
		nGameType == (int)MMGT_GLADIATOR_SOLO || 
		nGameType == (int)MMGT_TRAINING || 
		nGameType == (int)MMGT_BERSERKER)
	{
		return true;
	}
	
	return false;
}

bool IsTeamGame(int nGameType)
{
	if(nGameType == (int)MMGT_DEATHMATCH_TEAM || 
		nGameType == (int)MMGT_GLADIATOR_TEAM || 
		nGameType == (int)MMGT_ASSASSINATE || 
		nGameType == (int)MMGT_DEATHMATCH_TEAM_EXTREME || 
		nGameType == (int)MMGT_BLITZKRIEG || 
		nGameType == (int)MMGT_SPY)
	{
		return true;
	}
	
	return false;
}

bool IsQuestGame(int nGameType)
{
	if(nGameType == (int)MMGT_SURVIVAL || 
		nGameType == (int)MMGT_QUEST || 
		nGameType == (int)MMGT_CHALLENGE_QUEST)
	{
		return true;
	}
	
	return false;
}

bool IsQuestDerived(int nGameType)
{
	if(nGameType == (int)MMGT_SURVIVAL || 
		nGameType == (int)MMGT_QUEST)
	{
		return true;
	}
	
	return false;
}

bool IsDuelGame(int nGameType)
{
	if(nGameType == (int)MMGT_DUEL || 
		nGameType == (int)MMGT_DUEL_TOURNAMENT)
	{
		return true;
	}
	
	return false;
}

bool IsEndableGameByLeave(int nGameType)
{
	if(nGameType == (int)MMGT_DEATHMATCH_SOLO || 
		nGameType == (int)MMGT_GLADIATOR_SOLO || 
		nGameType == (int)MMGT_TRAINING || 
		nGameType == (int)MMGT_SURVIVAL || 
		nGameType == (int)MMGT_QUEST || 
		nGameType == (int)MMGT_BERSERKER || 
		nGameType == (int)MMGT_DEATHMATCH_TEAM_EXTREME || 
		nGameType == (int)MMGT_CHALLENGE_QUEST)
	{
		return true;
	}
	
	return false;
}

bool IsFinishableGameByTimeLimit(int nGameType)
{
	if(nGameType == (int)MMGT_DEATHMATCH_SOLO || 
		nGameType == (int)MMGT_GLADIATOR_SOLO || 
		nGameType == (int)MMGT_TRAINING || 
		nGameType == (int)MMGT_BERSERKER)
	{
		return true;
	}
	
	return false;
}

// suitable time for next timer announce.
unsigned long GetSuitableNextAnnounceTime(unsigned long nRemained)
{
	if(nRemained <= 5000)
		return 1000;
	else if(nRemained <= 10000)
		return 5000;
	else if(nRemained <= 30000)
		return 20000;
	else if(nRemained <= 60000)
		return 30000;
	else if(nRemained <= 180000)
		return 60000;
	else if(nRemained <= 300000)
		return 120000;
	else if(nRemained <= 600000)
		return 300000;
		
	return 600000;
}

void AnnounceRemainedTime(const MUID &uidStage, unsigned long nRemainedSec)
{
	#define REMAINED_TIME_SEC_STRING_FORMAT		"%lu seconds remaining."
	#define REMAINED_TIME_MIN_SEC_STRING_FORMAT	"%lu min %lu sec remaining."
	
	char szText[256];
	if(nRemainedSec < 60)
	{
		// sprintf(szText, REMAINED_TIME_SEC_STRING_FORMAT, nRemainedSec);
		sprintf(szText, GetServerMsg(MSVRSTR_REMAINED_TIME_SEC_STRING_FORMAT), nRemainedSec);
	}
	else
	{
		// sprintf(szText, REMAINED_TIME_MIN_SEC_STRING_FORMAT, nRemainedSec / 60, nRemainedSec % 60);
		sprintf(szText, GetServerMsg(MSVRSTR_REMAINED_TIME_MIN_SEC_STRING_FORMAT), nRemainedSec / 60, nRemainedSec % 60);
	}
	
	AnnounceToStage(szText, uidStage);
}

void CreateRoundStateCommand(MCmdWriter *pOut, const MUID &uidStage, int nRound, int nState, int nArg)
{
	if(CheckGameVersion(2013) == true)
	{
		pOut->WriteMUID(uidStage);
		pOut->WriteInt(nRound);
		pOut->WriteInt(nState);
		pOut->WriteInt(nArg);
		pOut->WriteInt(0);
		pOut->Finalize(MC_MATCH_GAME_ROUNDSTATE, MCFT_END);
	}
	else
	{
		pOut->WriteMUID(uidStage);
		pOut->WriteInt(nRound);
		pOut->WriteInt(nState);
		pOut->WriteInt(nArg);
		pOut->Finalize(MC_MATCH_GAME_ROUNDSTATE, MCFT_END);
	}
}

// round state sender.
void SendRoundStateToBattle(const MUID &uidStage, int nRound, int nState, int nArg)
{
	MCmdWriter Cmd;
	CreateRoundStateCommand(&Cmd, uidStage, nRound, nState, nArg);
	
	SendToBattle(&Cmd, uidStage);
}

void SendRoundStateToClient(const MUID &uidStage, int nRound, int nState, int nArg, const MUID &uidPlayer)
{
	MCmdWriter Cmd;
	CreateRoundStateCommand(&Cmd, uidStage, nRound, nState, nArg);
	
	SendToClient(&Cmd, uidPlayer);
}

// base game class.
MMatchBaseGame::MMatchBaseGame()
{
	m_pStage = NULL;
	m_bGameStarted = false;
	m_nRoundState = (int)MMRS_PREPARE;
	m_nStartTime = 0;
	m_nEndTime = 0;
	m_nNextAnnounceTime = 0;
	m_nWorldItemUID = 0;
	m_bFinished = false;
}

MMatchBaseGame::~MMatchBaseGame()
{
	EraseAllWorldItem();
}

void MMatchBaseGame::Create(MMatchStage *pStage)
{
	m_pStage = pStage;
	OnCreate();
}

void MMatchBaseGame::OnCreate()
{
	// init map world items.
	int nGameType = m_pStage->GetGameType();
	
	if(IsDuelGame(nGameType) == false)
	{
		int nMapIndex = m_pStage->GetMapIndex();
		
		if(m_pStage->IsRelayMapStage() == true)
		{
			nMapIndex = m_pStage->GetCurrRelayMapID();
		}
		
		if(IsTeamGame(nGameType) == false)
		{
			g_MapWorldItemMgr.Get(nMapIndex, MMMWIGTID_SOLO, &m_MapWorldItemList);
		}
		else
		{
			g_MapWorldItemMgr.Get(nMapIndex, MMMWIGTID_TEAM, &m_MapWorldItemList);
		}
		
		ResetAllMapWorldItemSpawnTime(GetTime());
	}
}

void MMatchBaseGame::Update(unsigned long nTime)
{
	if(m_bFinished == true) return;
	
	if(m_nRoundState == (int)MMRS_PLAY)
	{
		UpdateWorldItem(nTime);
	}
}

void MMatchBaseGame::SetRoundState(int nState)
{
	if(m_nRoundState == nState) return;
	
	switch(nState)
	{
		case (int)MMRS_COUNTDOWN	:
		{
			m_nStartTime = GetTime() + 2000;
			EraseAllWorldItem();
		}
		break;
		
		case (int)MMRS_PLAY	:
		{
			m_nNextAnnounceTime = 0;
		}
		break;
	}
	
	SendRoundStateToBattle(m_pStage->GetUID(), 0, nState, 0);
	
	m_nRoundState = nState;
}

void MMatchBaseGame::DropWorldItem(int nItemID, float x, float y, float z, unsigned long nDropDelayTime)
{
	MDropWorldItem *pNew = new MDropWorldItem;
	
	pNew->nItemID = nItemID;
	pNew->x = x;
	pNew->y = y;
	pNew->z = z;
	pNew->nDropTime = GetTime() + nDropDelayTime;
	
	m_DropWorldItemList.push_back(pNew);
}

void MMatchBaseGame::UpdateWorldItem(unsigned long nTime)
{
	// add done item.
	list<MDropWorldItem *>::iterator i = m_DropWorldItemList.begin();
	
	while(i != m_DropWorldItemList.end())
	{
		MDropWorldItem *pCurr = (*i);
		if(pCurr->nDropTime > nTime) 
		{
			i++;
			continue;
		}
		
		AddWorldItem(pCurr->nItemID, pCurr->x, pCurr->y, pCurr->z, (int)MTD_WorldItemSubType_Static);
		
		delete pCurr;
		i = m_DropWorldItemList.erase(i);
	}
	
	// remove dynamic item + time expired. and prevent over item count.
	#define WORLDITEM_SPAWN_LIMIT	30
	
	list<MSpawnWorldItem *>::iterator j = m_SpawnWorldItemList.begin();
	
	while(j != m_SpawnWorldItemList.end())
	{
		MSpawnWorldItem *pCurr = (*j);
		
		if(m_SpawnWorldItemList.size() <= WORLDITEM_SPAWN_LIMIT)
		{
			if(pCurr->nRemovedTime == 0 || pCurr->nRemovedTime > nTime)
			{
				j++;
				continue;
			}
		}
		
		RemoveWorldItem(pCurr->nUID);
		
		ResetMapWorldItemSpawnTime(pCurr->nUID);
		
		delete pCurr;
		j = m_SpawnWorldItemList.erase(j);
	}
	
	// spawn mapitem.
	for(list<MMatchMapWorldItemSpawnNode>::iterator k = m_MapWorldItemList.begin(); k != m_MapWorldItemList.end(); k++)
	{
		MMatchMapWorldItemSpawnNode *pCurr = &(*k);
		if(pCurr->bSpawned == true || pCurr->nNextTime > nTime)
		{
			continue;
		}
		
		pCurr->nUID = AddWorldItem(pCurr->nWorldItemID, pCurr->x, pCurr->y, pCurr->z, (int)MTD_WorldItemSubType_Static);
		
		pCurr->bSpawned = true;
	}
}

// returns assigned item uid.
unsigned short MMatchBaseGame::AddWorldItem(int nItemID, float x, float y, float z, int nSubType)
{
	unsigned short nAssigned = AssignWorldItemUID();
	
	// save worlditem info.
	MSpawnWorldItem *pNew = new MSpawnWorldItem;
	pNew->nUID = nAssigned;
	pNew->nItemID = nItemID;
	pNew->nRemovedTime = nSubType == (int)MTD_WorldItemSubType_Static ? 0 : GetTime() + 8000;
	pNew->x = x;
	pNew->y = y;
	pNew->z = z;
	m_SpawnWorldItemList.push_back(pNew);
	
	// send worlditem spawn info.
	MCmdWriter Cmd;
	Cmd.StartBlob(sizeof(MTD_WorldItem));
	
	MTD_WorldItem item;
	item.nUID = nAssigned;
	item.nItemID = (unsigned short)nItemID;
	item.nItemSubType = (unsigned short)nSubType;
	item.x = (short)x;
	item.y = (short)y;
	item.z = (short)z;
	
	Cmd.WriteData(&item, sizeof(MTD_WorldItem));
	Cmd.EndBlob();
	
	Cmd.Finalize(MC_MATCH_SPAWN_WORLDITEM, MCFT_END);
	SendToBattle(&Cmd, m_pStage->GetUID());
	
	return nAssigned;
}

bool MMatchBaseGame::TakeWorldItem(const MUID &uidPlayer, unsigned short nUID, int *pOutTookItemID)
{
	for(list<MSpawnWorldItem *>::iterator i = m_SpawnWorldItemList.begin(); i != m_SpawnWorldItemList.end(); i++)
	{
		MSpawnWorldItem *pCurr = (*i);
		
		if(pCurr->nUID == nUID)
		{
			// out this itemid.
			if(pOutTookItemID != NULL) *pOutTookItemID = pCurr->nItemID;
			
			// delete from map worlditem if exists.
			ResetMapWorldItemSpawnTime(nUID);
			
			// delete item.
			delete pCurr;
			m_SpawnWorldItemList.erase(i);
			
			// send info.
			MCmdWriter Cmd;
			Cmd.WriteMUID(uidPlayer);
			Cmd.WriteInt((int)nUID);
			Cmd.Finalize(MC_MATCH_OBTAIN_WORLDITEM, MCFT_END);
			SendToBattle(&Cmd, m_pStage->GetUID());
			
			return true;
		}
	}
	
	return false;
}

// send command only. will not remove from the spawned item list.
void MMatchBaseGame::RemoveWorldItem(unsigned short nUID)
{
	MCmdWriter Cmd;
	Cmd.WriteInt((int)nUID);
	Cmd.Finalize(MC_MATCH_REMOVE_WORLDITEM, MCFT_END);
	SendToBattle(&Cmd, m_pStage->GetUID());
}

void MMatchBaseGame::EraseAllWorldItem()
{
	for(list<MDropWorldItem *>::iterator i = m_DropWorldItemList.begin(); i != m_DropWorldItemList.end(); i++)
	{
		delete (*i);
	}
	m_DropWorldItemList.clear();
	
	for(list<MSpawnWorldItem *>::iterator i = m_SpawnWorldItemList.begin(); i != m_SpawnWorldItemList.end(); i++)
	{
		delete (*i);
	}
	m_SpawnWorldItemList.clear();
	
	ResetAllMapWorldItemSpawnTime(GetTime());
}

void MMatchBaseGame::Finish()
{
	if(m_bFinished == true) return;
	
	SetRoundState((int)MMRS_FINISH);
	
	m_pStage->AdvanceRelayMap();
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(m_pStage->GetUID());
	Cmd.WriteBool(m_pStage->GetRelayMapSetting()->IsUnFinish());
	Cmd.Finalize(MC_MATCH_STAGE_FINISH_GAME, MCFT_END);
	SendToStage(&Cmd, m_pStage->GetUID());
	
	m_pStage->AbortVoting();
	
	m_pStage->SetState((int)MMSS_STANDBY);
	m_bFinished = true;
}

void MMatchBaseGame::UpdateGameTimer(unsigned long nTime)
{
	int nTimeLimit = m_pStage->GetTimeLimit();
	
	// unlimited or infinite.
	// if(nTimeLimit == 0 || nTimeLimit == 99999)	return;
	if(MMatchStage::IsUnLimitedTime(nTimeLimit) == true) return;
	
	if(m_nEndTime < nTime)
	{
		TimeLimitReached();
		return;
	}
	
	if(m_nNextAnnounceTime <= nTime)
	{
		unsigned long nRemainedTimeSec = (m_nEndTime - nTime) / 1000;
		AnnounceRemainedTime(m_pStage->GetUID(), nRemainedTimeSec);
		
		m_nNextAnnounceTime = nTime + GetSuitableNextAnnounceTime(m_nEndTime - nTime);
	}
}

void MMatchBaseGame::TimeLimitReached()
{
	Finish();
}

unsigned short MMatchBaseGame::AssignWorldItemUID()
{
	m_nWorldItemUID++;
	return m_nWorldItemUID;
}

void MMatchBaseGame::ResetAllMapWorldItemSpawnTime(unsigned long nTime)
{
	for(list<MMatchMapWorldItemSpawnNode>::iterator i = m_MapWorldItemList.begin(); i != m_MapWorldItemList.end(); i++)
	{
		MMatchMapWorldItemSpawnNode *pNode = &(*i);
		pNode->nUID = 0;
		pNode->nNextTime = nTime + (unsigned long)pNode->nTimeMSec;
		pNode->bSpawned = false;
	}
}

bool MMatchBaseGame::ResetMapWorldItemSpawnTime(unsigned short nUID)
{
	for(list<MMatchMapWorldItemSpawnNode>::iterator i = m_MapWorldItemList.begin(); i != m_MapWorldItemList.end(); i++)
	{
		MMatchMapWorldItemSpawnNode *pNode = &(*i);
		if(pNode->nUID == nUID)
		{
			pNode->nUID = 0;
			pNode->nNextTime = GetTime() + (unsigned long)pNode->nTimeMSec;
			pNode->bSpawned = false;
			return true;
		}
	}
	return false;
}

// base deathmatch.
MMatchGame_BaseDeathmatch::MMatchGame_BaseDeathmatch()
{
}

MMatchGame_BaseDeathmatch::~MMatchGame_BaseDeathmatch()
{
}

void MMatchGame_BaseDeathmatch::Create(MMatchStage *pStage)
{
	MMatchBaseGame::Create(pStage);
}

void MMatchGame_BaseDeathmatch::Update(unsigned long nTime)
{
	if(m_bFinished == true) return;
	
	MMatchStage *pStage = m_pStage;	// this stage.
	
	if(pStage->GetState() != (int)MMSS_RUN) return;
	
	// if(pStage->GetState() == (int)MMSS_RUN)
	{
		if(m_bGameStarted == false)
		{
			if(pStage->CheckGameStartable() == true)
			{
				SetRoundState((int)MMRS_COUNTDOWN);
				m_bGameStarted = true;
			}
			else
			{
				return;
			}
		}
		else
		{
			if(m_nRoundState == (int)MMRS_COUNTDOWN)
			{
				if(m_nStartTime <= nTime)
				{
					SetRoundState((int)MMRS_PLAY);
					// m_nEndTime = nTime + (unsigned long)(pStage->GetTimeLimit() * 60 * 1000);
					m_nEndTime = nTime + (unsigned long)pStage->GetTimeLimit();
				}
			}
		}
	}
	
	if(m_nRoundState == (int)MMRS_PLAY)
	{
		MMatchBaseGame::UpdateGameTimer(nTime);
	}
	
	MMatchBaseGame::Update(nTime);
}

void MMatchGame_BaseDeathmatch::SetRoundState(int nState)
{
	if(m_nRoundState == nState) return;
	
	switch(nState)
	{
		case (int)MMRS_COUNTDOWN	:
		{
			m_nStartTime = GetTime() + 2000;
			EraseAllWorldItem();
		}
		break;
		
		case (int)MMRS_PLAY	:
		{
			m_nNextAnnounceTime = 0;
		}
		break;
	}
	
	SendRoundStateToBattle(m_pStage->GetUID(), 0, nState, 0);
	
	m_nRoundState = nState;
}

void MMatchGame_BaseDeathmatch::Finish()
{
	if(m_bFinished == true) return;
	
	SetRoundState((int)MMRS_FINISH);
	
	m_pStage->AdvanceRelayMap();
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(m_pStage->GetUID());
	Cmd.WriteBool(m_pStage->GetRelayMapSetting()->IsUnFinish());
	Cmd.Finalize(MC_MATCH_STAGE_FINISH_GAME, MCFT_END);
	SendToStage(&Cmd, m_pStage->GetUID());
	
	m_pStage->AbortVoting();
	
	m_pStage->SetState((int)MMSS_STANDBY);
	m_bFinished = true;
}

void MMatchGame_BaseDeathmatch::TimeLimitReached()
{
	Finish();
}

// berserker.
MMatchGame_Berserker::MMatchGame_Berserker()
{
	m_uidBerserker = MUID(0, 0);
}

void MMatchGame_Berserker::AssignBerserker(const MUID &uidPlayer)
{
	if(m_uidBerserker == uidPlayer) return;
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidPlayer);
	Cmd.Finalize(MC_MATCH_ASSIGN_BERSERKER, MCFT_END);
	SendToBattle(&Cmd, m_pStage->GetUID());
	
	m_uidBerserker = uidPlayer;
}

// team deathmatch.
MMatchGame_BaseTeamDeathmatch::MMatchGame_BaseTeamDeathmatch()
{
	m_nRound = 0;
	m_nLastRoundStateArg = 0;
	m_nRedWins = 0;
	m_nBlueWins = 0;
	m_bWaitingForNextRound = false;
	m_nNextRoundStartTime = 0;
}

MMatchGame_BaseTeamDeathmatch::~MMatchGame_BaseTeamDeathmatch()
{
}

void MMatchGame_BaseTeamDeathmatch::Create(MMatchStage *pStage)
{
	MMatchBaseGame::Create(pStage);
}

void MMatchGame_BaseTeamDeathmatch::Update(unsigned long nTime)
{
	if(m_bFinished == true) return;
	
	MMatchStage *pStage = m_pStage;	// this stage.
	
	if(pStage->GetState() != (int)MMSS_RUN) return;
	
	// if(pStage->GetState() == (int)MMSS_RUN)
	{
		if(m_bGameStarted == false)
		{
			if(pStage->CheckGameStartable() == true)
			{
				if(CheckRoundStartable() == true)
				{
					SetRoundState((int)MMRS_COUNTDOWN);
				}
				else
				{
					SetRoundState((int)MMRS_FREE);
				}
				m_bGameStarted = true;
			}
			else
			{
				return;
			}
		}
		else
		{
			if(m_nRoundState == (int)MMRS_COUNTDOWN)
			{
				if(m_nStartTime <= nTime)
				{
					SetRoundState((int)MMRS_PLAY);
					// m_nEndTime = nTime + (unsigned long)(pStage->GetTimeLimit() * 60 * 1000);
					m_nEndTime = nTime + (unsigned long)pStage->GetTimeLimit();
				}
			}
		}
	}
	
	if(m_bWaitingForNextRound == true)
	{
		if(m_nNextRoundStartTime <= nTime)
		{
			if(CheckRoundStartable() == true)
			{
				SetRoundState((int)MMRS_COUNTDOWN);
			}
			else
			{
				SetRoundState((int)MMRS_FREE);
			}
			m_bWaitingForNextRound = false;
		}
		return;
	}
	
	if(m_nRoundState == (int)MMRS_PLAY)
	{
		// team alive checks.
		bool bRedAlive = CheckTeamMemberAlive(MMT_RED);
		bool bBlueAlive = CheckTeamMemberAlive(MMT_BLUE);
	
		if(bRedAlive == false && bBlueAlive == false)
		{
			RoundFinish(0);	// 0 for draw.
			return;
		}
		else if(bRedAlive == false)
		{
			RoundFinish(MMT_BLUE);
			return;
		}
		else if(bBlueAlive == false)
		{
			RoundFinish(MMT_RED);
			return;
		}
		// alive checks end.
		
		MMatchBaseGame::UpdateGameTimer(nTime);
	}
	else if(m_nRoundState == (int)MMRS_FREE)
	{
		if(CheckRoundStartable() == true)
		{
			SetRoundState((int)MMRS_COUNTDOWN);
		}
	}
	
	MMatchBaseGame::Update(nTime);
}

void MMatchGame_BaseTeamDeathmatch::SetRoundState(int nState)
{
	if(m_nRoundState == nState) return;
	
	switch(nState)
	{
		case (int)MMRS_COUNTDOWN	:
		{
			m_nStartTime = GetTime() + 2000;
			EraseAllWorldItem();
			
			RevivalAll();
			MemberBalancing();
		}
		break;
		
		case (int)MMRS_PLAY	:
		{
			m_nNextAnnounceTime = 0;
		}
		break;
	}
	
	if(nState == (int)MMRS_COUNTDOWN || nState == (int)MMRS_FREE)
	{
		SendRoundStateToBattle(m_pStage->GetUID(), m_nRound, (int)MMRS_PREPARE, m_nLastRoundStateArg);
	}
	
	SendRoundStateToBattle(m_pStage->GetUID(), m_nRound, nState, m_nLastRoundStateArg);
	
	m_nRoundState = nState;
}

void MMatchGame_BaseTeamDeathmatch::Finish()
{
	if(m_bFinished == true) return;
	
	m_pStage->AdvanceRelayMap();
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(m_pStage->GetUID());
	Cmd.WriteBool(m_pStage->GetRelayMapSetting()->IsUnFinish());
	Cmd.Finalize(MC_MATCH_STAGE_FINISH_GAME, MCFT_END);
	SendToStage(&Cmd, m_pStage->GetUID());
	
	m_pStage->AbortVoting();
	
	m_pStage->SetState((int)MMSS_STANDBY);
	m_bFinished = true;
}

void MMatchGame_BaseTeamDeathmatch::TimeLimitReached()
{
	RoundFinish(0);	// 0 = draw.
}

void MMatchGame_BaseTeamDeathmatch::RoundFinish(int nWinnerTeam)
{
	switch(nWinnerTeam)
	{
		case MMT_RED	: m_nLastRoundStateArg = (int)MMRSA_RED_WON;  m_nRedWins++;	 break;
		case MMT_BLUE	: m_nLastRoundStateArg = (int)MMRSA_BLUE_WON; m_nBlueWins++; break;
		default			: m_nLastRoundStateArg = (int)MMRSA_DRAW;	break;
	}
	
	SetRoundState((int)MMRS_FINISH);
	
	m_nRound++;
	
	if(m_nRound >= m_pStage->GetRound())
	{
		// max round reached.
		Finish();
	}
	else
	{
		m_nNextRoundStartTime = GetTime() + 3000;
		m_bWaitingForNextRound = true;
	}
}

bool MMatchGame_BaseTeamDeathmatch::CheckTeamMemberAlive(int nTeam)
{
	for(map<MUID, MMatchObject *>::iterator i = m_pStage->ObjBegin(); i != m_pStage->ObjEnd(); i++)
	{
		MMatchObject *pObj = (*i).second;
		if(pObj->CheckGameFlag(MMOGF_ENTERED) == true && pObj->m_GameInfo.nTeam == nTeam && pObj->m_GameInfo.bAlive == true) return true;
	}
	
	return false;
}

bool MMatchGame_BaseTeamDeathmatch::CheckRoundStartable()
{
	bool bRedExists = false, bBlueExists = false;
	
	for(map<MUID, MMatchObject *>::iterator i = m_pStage->ObjBegin(); i != m_pStage->ObjEnd(); i++)
	{
		MMatchObject *pObj = (*i).second;
		
		if(pObj->CheckGameFlag(MMOGF_ENTERED) == false) continue;
		
		if(pObj->IsHide() == false)
		{
			if(pObj->m_GameInfo.nTeam == MMT_RED) bRedExists = true;
			else if(pObj->m_GameInfo.nTeam == MMT_BLUE) bBlueExists = true;
		}
	}
	
	return bRedExists == true && bBlueExists == true ? true : false ;
}

void MMatchGame_BaseTeamDeathmatch::GetTeamScore(int *pOutRed, int *pOutBlue)
{
	*pOutRed = m_nRedWins;
	*pOutBlue = m_nBlueWins;
}

void MMatchGame_BaseTeamDeathmatch::RevivalAll()
{
	for(map<MUID, MMatchObject *>::iterator i = m_pStage->ObjBegin(); i != m_pStage->ObjEnd(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		if(pCurr->CheckGameFlag(MMOGF_ENTERED) == false) continue;
		
		if(pCurr->IsHide() == false)
		{
			pCurr->m_GameInfo.bAlive = true;
		}
	}
}

void MMatchGame_BaseTeamDeathmatch::MemberBalancing()
{
	if(m_pStage->IsTeamBalancingEnabled() == false) return;
	
	// don't do balancing after just game started.
	if(m_nRound == 0) return;
	
	// auto-member balancing each 5 rounds.
	if((m_nRound % 5) != 0) return;
	
	vector<MMatchObject *> vtObjects;
	
	for(map<MUID, MMatchObject *>::iterator i = m_pStage->ObjBegin(); i != m_pStage->ObjEnd(); i++)
	{
		MMatchObject *pObj = (*i).second;
		if(pObj->CheckGameFlag(MMOGF_ENTERED) == false) continue;
		
		if(pObj->IsHide() == false)
		{
			vtObjects.push_back(pObj);
		}
	}
	
	// http://d.hatena.ne.jp/madotubo/20081126/1227708349
	for(int i = 0; i < (int)vtObjects.size() - 1; i++)
	{
		for(int j = i + 1; j < (int)vtObjects.size(); j++)
		{
			if(vtObjects[i]->m_GameInfo.nExp < vtObjects[j]->m_GameInfo.nExp)
			{
				MMatchObject *temp = vtObjects[i];
				vtObjects[i] = vtObjects[j];
				vtObjects[j] = temp;
			}
		}
	}
	
	MCmdWriter Cmd;
	Cmd.StartBlob(sizeof(MTD_ResetTeamMembersData));
	
	int nNewTeam = MMT_RED;
	for(vector<MMatchObject *>::iterator i = vtObjects.begin(); i != vtObjects.end(); i++)
	{
		MMatchObject *pObj = (*i);
		
		pObj->m_GameInfo.nTeam = nNewTeam;
		
		MTD_ResetTeamMembersData data;
		data.uidPlayer = pObj->GetUID();
		data.nTeam = nNewTeam;
		Cmd.WriteData(&data, sizeof(MTD_ResetTeamMembersData));
		
		if(nNewTeam == MMT_RED) nNewTeam = MMT_BLUE;
		else nNewTeam = MMT_RED;
	}
	
	Cmd.EndBlob();
	Cmd.Finalize(MC_MATCH_RESET_TEAM_MEMBERS, MCFT_END);
	SendToBattle(&Cmd, m_pStage->GetUID());
	
	SendStageSettingToStage(m_pStage);
}

// assassinate.
MMatchGame_Assassinate::MMatchGame_Assassinate()
{
	m_uidRedCommander = MUID(0, 0);
	m_uidBlueCommander = MUID(0, 0);
}

void MMatchGame_Assassinate::Update(unsigned long nTime)
{
	if(m_bFinished == true) return;
	
	MMatchStage *pStage = m_pStage;	// this stage.
	
	if(pStage->GetState() != (int)MMSS_RUN) return;
	
	// if(pStage->GetState() == (int)MMSS_RUN)
	{
		if(m_bGameStarted == false)
		{
			if(pStage->CheckGameStartable() == true)
			{
				if(CheckRoundStartable() == true)
				{
					SetRoundState((int)MMRS_COUNTDOWN);
				}
				else
				{
					SetRoundState((int)MMRS_FREE);
				}
				m_bGameStarted = true;
			}
			else
			{
				return;
			}
		}
		else
		{
			if(m_nRoundState == (int)MMRS_COUNTDOWN)
			{
				if(m_nStartTime <= nTime)
				{
					SetRoundState((int)MMRS_PLAY);
					// m_nEndTime = nTime + (unsigned long)(pStage->GetTimeLimit() * 60 * 1000);
					m_nEndTime = nTime + (unsigned long)pStage->GetTimeLimit();
				}
			}
		}
	}
	
	if(m_bWaitingForNextRound == true)
	{
		if(m_nNextRoundStartTime <= nTime)
		{
			if(CheckRoundStartable() == true)
			{
				SetRoundState((int)MMRS_COUNTDOWN);
			}
			else
			{
				SetRoundState((int)MMRS_FREE);
			}
			m_bWaitingForNextRound = false;
		}
		return;
	}
	
	if(m_nRoundState == (int)MMRS_PLAY)
	{
		// team alive checks.
		bool bRedAlive = CheckRedTeamCommanderAlive();
		bool bBlueAlive = CheckBlueTeamCommanderAlive();
	
		if(bRedAlive == false && bBlueAlive == false)
		{
			RoundFinish(0);	// 0 for draw.
			return;
		}
		else if(bRedAlive == false)
		{
			RoundFinish(MMT_BLUE);
			return;
		}
		else if(bBlueAlive == false)
		{
			RoundFinish(MMT_RED);
			return;
		}
		// alive checks end.
		
		MMatchBaseGame::UpdateGameTimer(nTime);
	}
	else if(m_nRoundState == (int)MMRS_FREE)
	{
		if(CheckRoundStartable() == true)
		{
			SetRoundState((int)MMRS_COUNTDOWN);
		}
	}
	
	MMatchBaseGame::Update(nTime);
}

void MMatchGame_Assassinate::SetRoundState(int nState)
{
	if(m_nRoundState == nState) return;
	
	switch(nState)
	{
		case (int)MMRS_COUNTDOWN	:
		{
			m_nStartTime = GetTime() + 2000;
			EraseAllWorldItem();
			
			RevivalAll();
			MemberBalancing();
		}
		break;
		
		case (int)MMRS_PLAY	:
		{
			m_nNextAnnounceTime = 0;
			SetRandomTeamCommander();
		}
		break;
	}
	
	if(nState == (int)MMRS_COUNTDOWN || nState == (int)MMRS_FREE)
	{
		SendRoundStateToBattle(m_pStage->GetUID(), m_nRound, (int)MMRS_PREPARE, m_nLastRoundStateArg);
	}
	
	SendRoundStateToBattle(m_pStage->GetUID(), m_nRound, nState, m_nLastRoundStateArg);
	
	m_nRoundState = nState;
}

void MMatchGame_Assassinate::SetRandomTeamCommander()
{
	vector<MUID> vtRedCand, vtBlueCand;
	
	for(map<MUID, MMatchObject *>::iterator i = m_pStage->ObjBegin(); i != m_pStage->ObjEnd(); i++)
	{
		MMatchObject *pObj = (*i).second;
		
		if(pObj->CheckGameFlag(MMOGF_ENTERED) == false) continue;
		if(pObj->m_GameInfo.bAlive == false) continue;
		
		if(pObj->IsHide() == false)
		{
			if(pObj->m_GameInfo.nTeam == MMT_RED)
			{
				vtRedCand.push_back(pObj->GetUID());
			}
			else if(pObj->m_GameInfo.nTeam == MMT_BLUE)
			{
				vtBlueCand.push_back(pObj->GetUID());
			}
		}
	}
	
	MUID uidRedResult = MUID(0, 0), uidBlueResult = MUID(0, 0);
	
	if(vtRedCand.size() > 0)
	{
		int nRand = (int)((unsigned int)RandNum() % (unsigned int)vtRedCand.size());
		uidRedResult = vtRedCand[nRand];
	}
	
	if(vtBlueCand.size() > 0)
	{
		int nRand = (int)((unsigned int)RandNum() % (unsigned int)vtBlueCand.size());
		uidBlueResult = vtBlueCand[nRand];
	}
	
	m_uidRedCommander = uidRedResult;
	m_uidBlueCommander = uidBlueResult;
	
	// send commander info to client.
	/*
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidRedResult);
	Cmd.WriteMUID(uidBlueResult);
	Cmd.Finalize(MC_MATCH_ASSIGN_COMMANDER, MCFT_END);
	SendToBattle(&Cmd, m_pStage->GetUID());
	*/
	SendCommanderInfo();
}

bool MMatchGame_Assassinate::CheckRedTeamCommanderAlive()
{
	MMatchObject *pBossObj = g_ObjectMgr.Get(m_uidRedCommander);
	if(pBossObj == NULL) return false;
	
	if(pBossObj->m_uidStage != m_pStage->GetUID()) return false;
	if(pBossObj->CheckGameFlag(MMOGF_ENTERED) == false) return false;
	if(pBossObj->m_GameInfo.nTeam != MMT_RED) return false;
	if(pBossObj->m_GameInfo.bAlive == false) return false;
	
	return true;
}

bool MMatchGame_Assassinate::CheckBlueTeamCommanderAlive()
{
	MMatchObject *pBossObj = g_ObjectMgr.Get(m_uidBlueCommander);
	if(pBossObj == NULL) return false;
	
	if(pBossObj->m_uidStage != m_pStage->GetUID()) return false;
	if(pBossObj->CheckGameFlag(MMOGF_ENTERED) == false) return false;
	if(pBossObj->m_GameInfo.nTeam != MMT_BLUE) return false;
	if(pBossObj->m_GameInfo.bAlive == false) return false;
	
	return true;
}

void MMatchGame_Assassinate::SetRedTeamCommanderUID(const MUID &uidCommander)
{
	m_uidRedCommander = uidCommander;
	SendCommanderInfo();
}

void MMatchGame_Assassinate::SetBlueTeamCommanderUID(const MUID &uidCommander)
{
	m_uidBlueCommander = uidCommander;
	SendCommanderInfo();
}

void MMatchGame_Assassinate::SendCommanderInfo()
{
	MCmdWriter Cmd;
	Cmd.WriteMUID(m_uidRedCommander);
	Cmd.WriteMUID(m_uidBlueCommander);
	Cmd.Finalize(MC_MATCH_ASSIGN_COMMANDER, MCFT_END);
	SendToBattle(&Cmd, m_pStage->GetUID());
}

// tdm ex.
MMatchGame_TeamDeathmatchExtreme::MMatchGame_TeamDeathmatchExtreme()
{
	m_nLastRoundStateArg = 0;
	m_nRedKills = 0;
	m_nBlueKills = 0;
	m_bWaitingForFinish = false;
	m_nFinishTime = 0;
}

MMatchGame_TeamDeathmatchExtreme::~MMatchGame_TeamDeathmatchExtreme()
{
}

void MMatchGame_TeamDeathmatchExtreme::Update(unsigned long nTime)
{
	if(m_bFinished == true) return;
	
	MMatchStage *pStage = m_pStage;	// this stage.
	
	if(pStage->GetState() != (int)MMSS_RUN) return;
	
	// if(pStage->GetState() == (int)MMSS_RUN)
	{
		if(m_bGameStarted == false)
		{
			if(pStage->CheckGameStartable() == true)
			{
				SetRoundState((int)MMRS_COUNTDOWN);
				m_bGameStarted = true;
			}
			else
			{
				return;
			}
		}
		else
		{
			if(m_nRoundState == (int)MMRS_COUNTDOWN)
			{
				if(m_nStartTime <= nTime)
				{
					SetRoundState((int)MMRS_PLAY);
					// m_nEndTime = nTime + (unsigned long)(pStage->GetTimeLimit() * 60 * 1000);
					m_nEndTime = nTime + (unsigned long)pStage->GetTimeLimit();
				}
			}
		}
	}
	
	if(m_bWaitingForFinish == true)
	{
		if(m_nFinishTime <= nTime)
		{
			Finish();
		}
		return;
	}
	
	if(m_nRoundState == (int)MMRS_PLAY)
	{
		bool bRedWin = IsRedWin();
		bool bBlueWin = IsBlueWin();
		
		if(bRedWin == true && bBlueWin == true)
		{
			PreFinish(0);	// 0 for draw.
			return;
		}
		else if(bRedWin == true)
		{
			PreFinish(MMT_RED);
			return;
		}
		else if(bBlueWin == true)
		{
			PreFinish(MMT_BLUE);
			return;
		}
		
		MMatchBaseGame::UpdateGameTimer(nTime);
	}
	
	MMatchBaseGame::Update(nTime);
}

void MMatchGame_TeamDeathmatchExtreme::SetRoundState(int nState)
{
	if(m_nRoundState == nState) return;
	
	switch(nState)
	{
		case (int)MMRS_COUNTDOWN	:
		{
			m_nStartTime = GetTime() + 2000;
			EraseAllWorldItem();
		}
		break;
		
		case (int)MMRS_PLAY	:
		{
			m_nNextAnnounceTime = 0;
		}
		break;
	}
	
	SendRoundStateToBattle(m_pStage->GetUID(), 0, nState, m_nLastRoundStateArg);
	
	m_nRoundState = nState;
}

void MMatchGame_TeamDeathmatchExtreme::Finish()
{
	if(m_bFinished == true) return;
	
	m_pStage->AdvanceRelayMap();
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(m_pStage->GetUID());
	Cmd.WriteBool(m_pStage->GetRelayMapSetting()->IsUnFinish());
	Cmd.Finalize(MC_MATCH_STAGE_FINISH_GAME, MCFT_END);
	SendToStage(&Cmd, m_pStage->GetUID());
	
	m_pStage->AbortVoting();
	
	m_pStage->SetState((int)MMSS_STANDBY);
	m_bFinished = true;
}

void MMatchGame_TeamDeathmatchExtreme::TimeLimitReached()
{
	PreFinish(0);	// draw = 0.
}

void MMatchGame_TeamDeathmatchExtreme::PreFinish(int nWinnerTeam)
{
	switch(nWinnerTeam)
	{
		case MMT_RED	: m_nLastRoundStateArg = (int)MMRSA_RED_WON;  break;
		case MMT_BLUE	: m_nLastRoundStateArg = (int)MMRSA_BLUE_WON; break;
		default			: m_nLastRoundStateArg = (int)MMRSA_DRAW; break;
	}
	
	SetRoundState((int)MMRS_FINISH);
	
	m_nFinishTime = GetTime() + 3000;
	m_bWaitingForFinish = true;
}

bool MMatchGame_TeamDeathmatchExtreme::IsRedWin()
{
	return m_nRedKills >= m_pStage->GetRound() ? true : false ;
}

bool MMatchGame_TeamDeathmatchExtreme::IsBlueWin()
{
	return m_nBlueKills >= m_pStage->GetRound() ? true : false ;
}

void MMatchGame_TeamDeathmatchExtreme::GetTeamKills(int *pOutRed, int *pOutBlue)
{
	*pOutRed = m_nRedKills;
	*pOutBlue = m_nBlueKills;
}

// duel match.
MMatchGame_DuelMatch::MMatchGame_DuelMatch()
{
	m_uidLastWinner = MUID(0, 0);
	ZeroUID(m_uidFighter, 2);
	m_nWinningStreak = 0;
	m_bWaitingForNextRound = false;
	m_nNextRoundStartTime = 0;
}

MMatchGame_DuelMatch::~MMatchGame_DuelMatch()
{
	m_PlayerQueueList.clear();
}

void MMatchGame_DuelMatch::Update(unsigned long nTime)
{
	if(m_bFinished == true) return;
	
	MMatchStage *pStage = m_pStage;	// this stage.
	
	if(pStage->GetState() != (int)MMSS_RUN) return;
	
	// if(pStage->GetState() == (int)MMSS_RUN)
	{
		if(m_bGameStarted == false)
		{
			if(pStage->CheckGameStartable() == true)
			{
				SetRoundState((int)MMRS_COUNTDOWN);
				m_bGameStarted = true;
			}
			else
			{
				return;
			}
		}
		else
		{
			if(m_nRoundState == (int)MMRS_COUNTDOWN)
			{
				if(m_nStartTime <= nTime)
				{
					if(StartMatch() == false)
					{
						SetRoundState((int)MMRS_FREE);
					}
					// m_nEndTime = nTime + (unsigned long)(pStage->GetTimeLimit() * 60 * 1000);
					m_nEndTime = nTime + (unsigned long)pStage->GetTimeLimit();
				}
			}
		}
	}
	
	if(m_bWaitingForNextRound == true)
	{
		if(m_nNextRoundStartTime <= nTime)
		{
			SetRoundState((int)MMRS_COUNTDOWN);
			m_bWaitingForNextRound = false;
		}
		return;
	}
	
	if(m_nRoundState == (int)MMRS_PLAY)
	{
		bool bPlayer1Alive, bPlayer2Alive;
		CheckFighterAlive(&bPlayer1Alive, &bPlayer2Alive);
		
		if(bPlayer1Alive == false && bPlayer2Alive == false)
		{
			EndMatch();
			return;
		}
		else if(bPlayer1Alive == false)
		{
			EndMatch(m_uidFighter[1]);
			return;
		}
		else if(bPlayer2Alive == false)
		{
			EndMatch(m_uidFighter[0]);
			return;
		}
		
		MMatchBaseGame::UpdateGameTimer(nTime);
	}
	else if(m_nRoundState == (int)MMRS_FREE)
	{
		if(CheckMatchStartable() == true)
		{
			SetRoundState((int)MMRS_COUNTDOWN);
		}
	}
	
	MMatchBaseGame::Update(nTime);
}

void MMatchGame_DuelMatch::SetRoundState(int nState)
{
	if(m_nRoundState == nState) return;
	
	switch(nState)
	{
		case (int)MMRS_COUNTDOWN	:
		{
			m_nStartTime = GetTime() + 2000;
			EraseAllWorldItem();
		}
		break;
		
		case (int)MMRS_PLAY	:
		{
			m_nNextAnnounceTime = 0;
			SendDuelQueueInfo(true);
		}
		break;
		
		case (int)MMRS_FINISH	:
		{
			m_nNextRoundStartTime = GetTime() + 3000;
			m_bWaitingForNextRound = true;
		}
		break;
	}
	
	SendRoundStateToBattle(m_pStage->GetUID(), 0, nState, 0);
	
	if(CheckGameFinishable() == true)
	{
		Finish();
	}
	
	m_nRoundState = nState;
}

void MMatchGame_DuelMatch::Finish()
{
	if(m_bFinished == true) return;
	
	m_pStage->AdvanceRelayMap();
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(m_pStage->GetUID());
	Cmd.WriteBool(m_pStage->GetRelayMapSetting()->IsUnFinish());
	Cmd.Finalize(MC_MATCH_STAGE_FINISH_GAME, MCFT_END);
	SendToStage(&Cmd, m_pStage->GetUID());
	
	m_pStage->AbortVoting();
	
	m_pStage->SetState((int)MMSS_STANDBY);
	m_bFinished = true;
}

void MMatchGame_DuelMatch::TimeLimitReached()
{
	EndMatch();
}

void MMatchGame_DuelMatch::AddPlayer(const MUID &uidPlayer)
{
	m_PlayerQueueList.push_back(uidPlayer);
}

bool MMatchGame_DuelMatch::RemovePlayer(const MUID &uidPlayer)
{
	for(list<MUID>::iterator i = m_PlayerQueueList.begin(); i != m_PlayerQueueList.end(); i++)
	{
		if((*i) == uidPlayer)
		{
			m_PlayerQueueList.erase(i);
			return true;
		}
	}
	
	return false;
}

void MMatchGame_DuelMatch::ReQueuePlayer(const MUID &uidPlayer)
{
	if(RemovePlayer(uidPlayer) == true)
		AddPlayer(uidPlayer);
}

bool MMatchGame_DuelMatch::StartMatch()
{
	if(CheckMatchStartable() == false) return false;
	
	list<MUID>::iterator it = m_PlayerQueueList.begin();
	for(int i = 0; i < 2; i++)
	{
		MUID uidFighter = (*it);
		m_uidFighter[i] = uidFighter;
		
		MMatchObject *pObj = g_ObjectMgr.Get(uidFighter);
		if(pObj != NULL)
		{
			if(pObj->IsHide() == false)
			{
				pObj->m_GameInfo.bAlive = true;
			}
		}
		
		it++;
	}
	
	if(m_uidLastWinner != m_PlayerQueueList.front())
	{
		m_nWinningStreak = 0;
	}
	
	SetRoundState((int)MMRS_PLAY);
	
	return true;
}

bool MMatchGame_DuelMatch::EndMatch(const MUID &uidWinner)
{
	if(m_uidFighter[0] == uidWinner)
	{
		m_uidLastWinner = uidWinner;
		
		m_nWinningStreak++;
		ReQueuePlayer(m_uidFighter[1]);
		
		BroadcastWinningState();
		
		SetRoundState((int)MMRS_FINISH);
		
		return true;
	}
	else if(m_uidFighter[1] == uidWinner)
	{
		m_uidLastWinner = uidWinner;
		
		BroadcastWinningBreak();
		
		m_nWinningStreak = 1;
		ReQueuePlayer(m_uidFighter[0]);
		
		SetRoundState((int)MMRS_FINISH);
		
		return true;
	}
	else if(uidWinner == MUID(0, 0))	// draw game.
	{
		m_uidLastWinner = uidWinner;
		
		m_nWinningStreak = 0;
		
		ReQueuePlayer(m_uidFighter[0]);
		ReQueuePlayer(m_uidFighter[1]);
		
		SetRoundState((int)MMRS_FINISH);
		
		return true;
	}
	
	return false;
}

bool MMatchGame_DuelMatch::CheckMatchStartable()
{
	if(m_PlayerQueueList.size() < 2) return false;
	return true;
}

void MMatchGame_DuelMatch::CheckFighterAlive(bool *pOutPlayer1, bool *pOutPlayer2)
{
	bool bPlayer1Alive = false, bPlayer2Alive = false;
	
	MMatchObject *pObj1 = g_ObjectMgr.Get(m_uidFighter[0]);
	if(pObj1 != NULL)
	{
		if(pObj1->m_uidStage == m_pStage->GetUID())
		{
			if(pObj1->CheckGameFlag(MMOGF_ENTERED) == true && pObj1->m_GameInfo.bAlive == true)
				bPlayer1Alive = true;
		}
	}
	
	MMatchObject *pObj2 = g_ObjectMgr.Get(m_uidFighter[1]);
	if(pObj2 != NULL)
	{
		if(pObj2->m_uidStage == m_pStage->GetUID())
		{
			if(pObj2->CheckGameFlag(MMOGF_ENTERED) == true && pObj2->m_GameInfo.bAlive == true)
				bPlayer2Alive = true;
		}
	}
	
	*pOutPlayer1 = bPlayer1Alive;
	*pOutPlayer2 = bPlayer2Alive;
}

void MMatchGame_DuelMatch::SendDuelQueueInfo(bool bRoundEnd)
{
	MTD_DuelQueueInfo info;
	ZeroInit(&info, sizeof(MTD_DuelQueueInfo));
	
	info.uidChampion = m_uidFighter[0];
	info.uidChallenger = m_uidFighter[1];
	
	int i = 0;
	for(list<MUID>::iterator it = m_PlayerQueueList.begin(); it != m_PlayerQueueList.end(); it++)
	{
		if(i >= 2) info.uidWaitQueue[i - 2] = (*it);	// two players of front is fighter.
		
		i++;
		if(i >= 16) break;	// 16 = max players.
	}
	
	info.nQueueLength = (char)(i >= 2 ? i - 2 : 0);
	info.nVictory = (char)m_nWinningStreak;
	info.bIsRoundEnd = bRoundEnd;
	
	MCmdWriter Cmd;
	Cmd.WriteBlob(&info, sizeof(MTD_DuelQueueInfo));
	Cmd.Finalize(MC_MATCH_DUEL_QUEUEINFO, MCFT_END);
	SendToBattle(&Cmd, m_pStage->GetUID());
}

bool MMatchGame_DuelMatch::CheckGameFinishable()
{
	for(map<MUID, MMatchObject *>::iterator i = m_pStage->ObjBegin(); i != m_pStage->ObjEnd(); i++)
	{
		MMatchObject *pObj = (*i).second;
		if(pObj->CheckGameFlag(MMOGF_ENTERED) == true && pObj->m_GameInfo.nKill >= m_pStage->GetRound()) return true;
	}
	return false;
}

void MMatchGame_DuelMatch::BroadcastWinningState()
{
	if(m_pStage->IsPrivate() == true) return;	// don't send when protected stage.
	
	if(m_nWinningStreak == 0) return;	// skip on 0 streak.
	if((m_nWinningStreak % 10) != 0) return;	// broadcast each 10 streak.
	
	MMatchObject *pObj = g_ObjectMgr.Get(m_uidFighter[0]);
	if(pObj == NULL) return;
	
	MMatchChannel *pChannel = g_ChannelMgr.Get(m_pStage->GetChannelUID());
	if(pChannel == NULL) return;
	
	MCmdWriter Cmd;
	Cmd.WriteString(pObj->m_Char.szName);
	Cmd.WriteString(pChannel->GetName());
	Cmd.WriteInt(m_pStage->GetNumber());
	Cmd.WriteInt(m_nWinningStreak);
	Cmd.Finalize(MC_MATCH_BROADCAST_DUEL_RENEW_VICTORIES, MCFT_END);
	SendToAll(&Cmd);
}

void MMatchGame_DuelMatch::BroadcastWinningBreak()
{
	if(m_pStage->IsPrivate() == true) return;
	
	if(m_nWinningStreak < 10) return;	// send only when >= 10 streak.
	
	MMatchObject *pObj1 = g_ObjectMgr.Get(m_uidFighter[0]);
	if(pObj1 == NULL) return;
	
	MMatchObject *pObj2 = g_ObjectMgr.Get(m_uidFighter[1]);
	if(pObj2 == NULL) return;
	
	MCmdWriter Cmd;
	Cmd.WriteString(pObj1->m_Char.szName);
	Cmd.WriteString(pObj2->m_Char.szName);
	Cmd.WriteInt(m_nWinningStreak);
	Cmd.Finalize(MC_MATCH_BROADCAST_DUEL_INTERRUPT_VICTORIES, MCFT_END);
	SendToAll(&Cmd);
}

// duel tournament.
MMatchGame_DuelTournament::MMatchGame_DuelTournament()
{
	m_nMatchOrder = 0;
	m_nPlayerIndex = 0;
	m_nRound = 0;
	ZeroUID(m_uidFighter, 2);
	m_uidLastWinner = MUID(0, 0);
	ZeroUID(m_uidQFinalPlayer, 8);
	ZeroUID(m_uidSFinalPlayer, 4);
	ZeroUID(m_uidFinalPlayer, 2);
	m_bTimeout = false;
	m_bDrawGame = false;
	m_bMatchFinish = false;
	m_nPreCountdownEndTime = 0;
	m_nPreCountdownStartTime = 0;
	m_bWaitingForNextCountdown = false;
	m_nRoundFinishTime = 0;
	m_bWaitingForRoundFinish = false;
	ZeroInit(m_PlayerInfo, sizeof(MDuelTournamentPlayerInfo) * 2);
}

MMatchGame_DuelTournament::~MMatchGame_DuelTournament()
{
}

void MMatchGame_DuelTournament::Update(unsigned long nTime)
{
	if(m_bFinished == true) return;
	
	MMatchStage *pStage = m_pStage;	// this stage.
	
	if(pStage->GetState() != (int)MMSS_RUN) return;
	
	// if(pStage->GetState() == (int)MMSS_RUN)
	{
		if(m_bGameStarted == false)
		{
			if(pStage->CheckGameStartable() == true)
			{
				StartPreCountdown();
				m_bGameStarted = true;
			}
			else
			{
				return;
			}
		}
		else
		{
			if(m_nRoundState == (int)MMRS_COUNTDOWN)
			{
				if(m_nStartTime <= nTime)
				{
					StartMatch();
					// m_nEndTime = nTime + (unsigned long)(pStage->GetTimeLimit() * 60 * 1000);
					m_nEndTime = nTime + (unsigned long)pStage->GetTimeLimit();
				}
			}
		}
	}
	
	if(m_bWaitingForRoundFinish == true)
	{
		if(m_nRoundFinishTime <= nTime)
		{
			if(RoundFinish() == false)
			{
				StartCountdown();
			}
			m_bWaitingForRoundFinish = false;
		}
		return;
	}
	else if(m_bWaitingForNextCountdown == true)
	{
		if(m_nPreCountdownStartTime <= nTime)
		{
			StartPreCountdown();
			m_bWaitingForNextCountdown = false;
		}
		return;
	}
	
	if(m_nRoundState == (int)MMRS_PLAY)
	{
		bool bPlayer1Alive, bPlayer2Alive;
		CheckFighterAlive(&bPlayer1Alive, &bPlayer2Alive);
		
		if(bPlayer1Alive == false && bPlayer2Alive == false)
		{
			EndMatch();
			return;
		}
		else if(bPlayer1Alive == false)
		{
			EndMatch(m_uidFighter[1]);
			return;
		}
		else if(bPlayer2Alive == false)
		{
			EndMatch(m_uidFighter[0]);
			return;
		}
		
		MMatchBaseGame::UpdateGameTimer(nTime);
	}
	else if(m_nRoundState == (int)MMRS_PRE_COUNTDOWN)
	{
		if(m_nPreCountdownEndTime <= nTime)
		{
			StartCountdown();
		}
		return;
	}
	
	MMatchBaseGame::Update(nTime);
}

void MMatchGame_DuelTournament::SetRoundState(int nState)
{
	if(m_nRoundState == nState) return;
	
	switch(nState)
	{
		case (int)MMRS_COUNTDOWN	:
		{
			m_nStartTime = GetTime() + 2000;
			EraseAllWorldItem();
		}
		break;
		
		case (int)MMRS_PLAY	:
		{
			m_nNextAnnounceTime = 0;
			SendGameInfo(true);
		}
		break;
	}
	
	SendRoundStateToBattle(m_pStage->GetUID(), 0, nState, 0);
	
	m_nRoundState = nState;
}

void MMatchGame_DuelTournament::Finish()
{
	if(m_bFinished == true) return;
	
	SetRoundState((int)MMRS_EXIT);
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(m_pStage->GetUID());
	Cmd.WriteBool(m_pStage->GetRelayMapSetting()->IsUnFinish());
	Cmd.Finalize(MC_MATCH_STAGE_FINISH_GAME, MCFT_END);
	SendToStage(&Cmd, m_pStage->GetUID());
	
	m_pStage->SetState((int)MMSS_STANDBY);
	m_bFinished = true;
}

void MMatchGame_DuelTournament::TimeLimitReached()
{
	InvalidatePlayerStatus();
	
	EndMatch();
	m_bTimeout = true;
}

void MMatchGame_DuelTournament::IncPlayerIndex()
{
	m_nPlayerIndex += 2;	// 2 players.
	
	// semi final || final.
	if(m_nMatchOrder == 4 || m_nMatchOrder == 6)
	{
		m_nPlayerIndex = 0;
	}
}

void MMatchGame_DuelTournament::SetQuarterFinalPlayerUID(const MUID &uidPlayer, int nIndex)
{
	if(nIndex < 0 || nIndex >= 8) return;
	m_uidQFinalPlayer[nIndex] = uidPlayer;
}

void MMatchGame_DuelTournament::SetSemiFinalPlayerUID(const MUID &uidPlayer, int nIndex)
{
	if(nIndex < 0 || nIndex >= 4) return;
	m_uidSFinalPlayer[nIndex] = uidPlayer;
}

void MMatchGame_DuelTournament::SetFinalPlayerUID(const MUID &uidPlayer, int nIndex)
{
	if(nIndex < 0 || nIndex >= 2) return;
	m_uidFinalPlayer[nIndex] = uidPlayer;
}

int MMatchGame_DuelTournament::GetMatchType()
{
	if(m_nMatchOrder >= 6)
	{
		return MDUELTOURNAMENT_ROUNDSTATE_FINAL;
	}
	else if(m_nMatchOrder >= 4)
	{
		return MDUELTOURNAMENT_ROUNDSTATE_SEMIFINAL;
	}
	
	return MDUELTOURNAMENT_ROUNDSTATE_QUARTERFINAL;
}

int MMatchGame_DuelTournament::GetRoundCount()
{
	switch(GetMatchType())
	{
		case MDUELTOURNAMENT_ROUNDSTATE_FINAL	:
		case MDUELTOURNAMENT_ROUNDSTATE_SEMIFINAL	:
			return 2;	// semi and final is 2 round.
	}
	
	return 1;	// otherwise 1 round.
}

void MMatchGame_DuelTournament::SendGameInfo(bool bRoundEnd)
{
	MTD_DuelTournamentGameInfo info;
	ZeroInit(&info, sizeof(MTD_DuelTournamentGameInfo));
	
	int nMatchType = GetMatchType();
	
	info.uidPlayer1 = m_uidFighter[0];
	info.uidPlayer2 = m_uidFighter[1];
	info.nMatchType = nMatchType;
	info.nMatchNumber = 0;	// database log id : always 0.
	info.nRoundCount = m_nRound + 1;
	info.bIsRoundEnd = bRoundEnd;
	
	if(nMatchType == MDUELTOURNAMENT_TYPE_QUARTERFINAL)
	{
		int nPlayerQueue = 0;
		
		for(int i = 0; i < 8; i++)
		{
			if(m_uidQFinalPlayer[i] != MUID(0, 0))
			{
				info.uidWaitPlayerList[nPlayerQueue] = m_uidQFinalPlayer[i];
				nPlayerQueue++;
			}
		}
		
		info.nWaitPlayerListLength = (char)nPlayerQueue;
	}
	else if(nMatchType == MDUELTOURNAMENT_TYPE_SEMIFINAL)
	{
		int nPlayerQueue = 0;
		
		for(int i = 0; i < 4; i++)
		{
			if(m_uidSFinalPlayer[i] != MUID(0, 0))
			{
				info.uidWaitPlayerList[nPlayerQueue] = m_uidSFinalPlayer[i];
				nPlayerQueue++;
			}
		}
		
		info.nWaitPlayerListLength = (char)nPlayerQueue;
	}
	else if(nMatchType == MDUELTOURNAMENT_TYPE_FINAL)
	{
		int nPlayerQueue = 0;
		
		for(int i = 0; i < 2; i++)
		{
			if(m_uidFinalPlayer[i] != MUID(0, 0))
			{
				info.uidWaitPlayerList[nPlayerQueue] = m_uidFinalPlayer[i];
				nPlayerQueue++;
			}
		}
		
		info.nWaitPlayerListLength = (char)nPlayerQueue;
	}
	
	MCmdWriter Cmd;
	Cmd.WriteBlob(&info, sizeof(MTD_DuelTournamentGameInfo));
	Cmd.Finalize(MC_MATCH_DUELTOURNAMENT_GAME_INFO, MCFT_END);
	SendToBattle(&Cmd, m_pStage->GetUID());
}

void MMatchGame_DuelTournament::SendNextMatchInfo()
{
	MTD_DuelTournamentNextMatchPlayerInfo info;
	
	info.uidPlayer1 = m_uidFighter[0];
	info.uidPlayer2 = m_uidFighter[1];
	
	MCmdWriter Cmd;
	Cmd.WriteBlob(&info, sizeof(MTD_DuelTournamentNextMatchPlayerInfo));
	Cmd.Finalize(MC_MATCH_DUELTOURNAMENT_GAME_NEXT_MATCH_PLYAERINFO, MCFT_END);
	SendToBattle(&Cmd, m_pStage->GetUID());
}

void MMatchGame_DuelTournament::SendRoundResultInfo(const MUID &uidWinner, const MUID &uidLoser)
{
	MTD_DuelTournamentRoundResultInfo info;
	ZeroInit(&info, sizeof(MTD_DuelTournamentRoundResultInfo));
	
	info.uidWinnerPlayer = uidWinner;
	info.uidLoserPlayer = uidLoser;
	info.bIsTimeOut = m_bTimeout;
	info.bDraw = m_bDrawGame;
	info.bIsMatchFinish = m_bMatchFinish;
	
	MCmdWriter Cmd;
	Cmd.WriteBlob(&info, sizeof(MTD_DuelTournamentRoundResultInfo));
	Cmd.Finalize(MC_MATCH_DUELTOURNAMENT_GAME_ROUND_RESULT_INFO, MCFT_END);
	SendToBattle(&Cmd, m_pStage->GetUID());
}

void MMatchGame_DuelTournament::SendMatchResultInfo(const MUID &uidWinner, int nGainTP, const MUID &uidLoser, int nLoseTP)
{
	MTD_DuelTournamentMatchResultInfo info;
	
	info.nMatchNumber = 0;	// always 0.
	info.nMatchType = GetMatchType();
	info.uidWinnerPlayer = uidWinner;
	info.uidLoserPlayer = uidLoser;
	info.nGainTP = nGainTP;
	info.nLoseTP = nLoseTP;
	
	MCmdWriter Cmd;
	Cmd.WriteBlob(&info, sizeof(MTD_DuelTournamentMatchResultInfo));
	Cmd.Finalize(MC_MATCH_DUELTOURNAMENT_GAME_MATCH_RESULT_INFO, MCFT_END);
	SendToBattle(&Cmd, m_pStage->GetUID());
}

void MMatchGame_DuelTournament::StartPreCountdown()
{
	int nMatchType = GetMatchType();
	
	if(nMatchType == MDUELTOURNAMENT_ROUNDSTATE_FINAL)
	{
		m_uidFighter[0] = m_uidFinalPlayer[m_nPlayerIndex];
		m_uidFighter[1] = m_uidFinalPlayer[m_nPlayerIndex + 1];
		
		ChangeToInfiniteTime();
	}
	else if(nMatchType == MDUELTOURNAMENT_ROUNDSTATE_SEMIFINAL)
	{
		m_uidFighter[0] = m_uidSFinalPlayer[m_nPlayerIndex];
		m_uidFighter[1] = m_uidSFinalPlayer[m_nPlayerIndex + 1];
	}
	else if(nMatchType == MDUELTOURNAMENT_ROUNDSTATE_QUARTERFINAL)
	{
		m_uidFighter[0] = m_uidQFinalPlayer[m_nPlayerIndex];
		m_uidFighter[1] = m_uidQFinalPlayer[m_nPlayerIndex + 1];
	}
	else
	{
		// ...?
	}
	
	SendNextMatchInfo();
	
	// pre-countdown time based on current match progress.
	unsigned long nPreCountdownTime = 5000;
	
	if(m_nMatchOrder > 0)	// only first match should wait 5 sec (to wait NAT connection and/or alpha interval).
	{
		switch(GetMatchType())
		{
			case MDUELTOURNAMENT_ROUNDSTATE_FINAL			: nPreCountdownTime = 4000; break;	// 4 sec.
			case MDUELTOURNAMENT_ROUNDSTATE_SEMIFINAL		: nPreCountdownTime = 3000; break;
			case MDUELTOURNAMENT_ROUNDSTATE_QUARTERFINAL	: nPreCountdownTime = 2500; break;
		}
	}
	
	ResetPlayerInfo();
	
	m_nPreCountdownEndTime = GetTime() + nPreCountdownTime;
	
	m_nRound = 0;
	SetRoundState((int)MMRS_PRE_COUNTDOWN);
}

void MMatchGame_DuelTournament::StartCountdown()
{
	SetRoundState((int)MMRS_COUNTDOWN);
}

void MMatchGame_DuelTournament::StartMatch()
{
	m_bTimeout = false;
	m_bDrawGame = false;
	m_bMatchFinish = false;
	
	SpawnFighter();
	
	SetRoundState((int)MMRS_PLAY);
}

void MMatchGame_DuelTournament::EndMatch(const MUID &uidWinner)
{
	m_uidLastWinner = uidWinner;
	
	m_nRoundFinishTime = GetTime() + 3000;
	m_bWaitingForRoundFinish = true;
	
	SetRoundState((int)MMRS_FINISH);
}

void MMatchGame_DuelTournament::MatchFinish(const MUID &uidWinner, const MUID &uidLoser)
{
	/*
	// check winner & loser.
	MUID uidWinner = m_uidLastWinner;
	MUID uidLoser = MUID(0, 0);
	
	if(m_uidFighter[0] == uidWinner)
	{
		uidLoser = m_uidFighter[1];
	}
	else if(m_uidFighter[1] == uidWinner)
	{
		uidLoser = m_uidFighter[0];
	}
	*/
	
	// TP to gain/loss.
	int nTPBonus = 0;
	
	int nMatchType = GetMatchType();
	switch(nMatchType)
	{
		case MDUELTOURNAMENT_ROUNDSTATE_FINAL			: nTPBonus = 9; break;
		case MDUELTOURNAMENT_ROUNDSTATE_SEMIFINAL		: nTPBonus = 6; break;
		case MDUELTOURNAMENT_ROUNDSTATE_QUARTERFINAL	: nTPBonus = 3; break;
	}
	
	// object TP process. - present point to winner.
	MMatchObject *pWinnerObj = g_ObjectMgr.Get(uidWinner);
	if(pWinnerObj != NULL)
	{
		if(pWinnerObj->m_bCharInfoExist == true)
		{
			if(pWinnerObj->m_uidStage == m_pStage->GetUID())
			{
				pWinnerObj->m_DuelTournament.nTP = CheckPlusOver(pWinnerObj->m_DuelTournament.nTP, nTPBonus);
				pWinnerObj->m_DuelTournament.nWin++;
				if(nMatchType == MDUELTOURNAMENT_ROUNDSTATE_FINAL) pWinnerObj->m_DuelTournament.nFinalWin++;
			}
		}
	}
	
	// and take point to loser.
	MMatchObject *pLoserObj = g_ObjectMgr.Get(uidLoser);
	if(pLoserObj != NULL)
	{
		if(pLoserObj->m_bCharInfoExist == true)
		{
			if(pLoserObj->m_uidStage == m_pStage->GetUID())
			{
				pLoserObj->m_DuelTournament.nTP = CheckMinusOver(pLoserObj->m_DuelTournament.nTP, nTPBonus);
				pLoserObj->m_DuelTournament.nLose++;
			}
		}
	}
	
	// send match result to client.
	SendNextMatchInfo();
	SendRoundResultInfo(uidWinner, uidLoser);
	SendMatchResultInfo(uidWinner, nTPBonus, uidLoser, nTPBonus);

	// move winner to next match.
	Advance(uidWinner);
	
	if(m_nMatchOrder < 6)	// < final.
	{
		m_nMatchOrder++;
		IncPlayerIndex();
		
		m_nPreCountdownStartTime = GetTime() + 4000;
		m_bWaitingForNextCountdown = true;
		
		SetRoundState((int)MMRS_PRE_COUNTDOWN);
	}
	else
	{
		Finish();
	}
}

bool MMatchGame_DuelTournament::RoundFinish()
{
	DetermineWinner();
	
	// check winner & loser.
	MUID uidWinner = m_uidLastWinner;
	MUID uidLoser = MUID(0, 0);
	
	int nWinningRound = 0;
	
	if(m_uidFighter[0] == uidWinner)
	{
		uidLoser = m_uidFighter[1];
		nWinningRound = ++m_PlayerInfo[0].nWin;
	}
	else if(m_uidFighter[1] == uidWinner)
	{
		uidLoser = m_uidFighter[0];
		nWinningRound = ++m_PlayerInfo[1].nWin;
	}
	else
	{
		m_bDrawGame = true;
	}
	
	if(IsValidFighter(m_uidFighter[0]) == false || IsValidFighter(m_uidFighter[1]) == false)
	{
		// if one of fighter is out, there is no need to keep like round 2, 3, 4... anymore.
		// if both of fighter is out, the game will be infinite draw game loop.
		nWinningRound = 999;	// so, finish this round immediately by set winning round to 999 (same as force finish).
	}
	
	m_nRound++;
	
	if(nWinningRound < GetRoundCount())
	{
		m_bMatchFinish = false;
		SendRoundResultInfo(uidWinner, uidLoser);
	}
	else
	{
		m_bMatchFinish = true;
		MatchFinish(uidWinner, uidLoser);
	}
	
	return m_bMatchFinish;
}

void MMatchGame_DuelTournament::SpawnFighter()
{
	MMatchObject *pObj1 = g_ObjectMgr.Get(m_uidFighter[0]);
	if(pObj1 != NULL)
	{
		if(pObj1->m_bCharInfoExist == true)
		{
			if(pObj1->m_uidStage == m_pStage->GetUID())
			{
				if(pObj1->CheckGameFlag(MMOGF_ENTERED) == true)
				{
					if(pObj1->IsHide() == false)
					{
						pObj1->m_GameInfo.bAlive = true;
					}
				}
			}
		}
	}
	
	MMatchObject *pObj2 = g_ObjectMgr.Get(m_uidFighter[1]);
	if(pObj2 != NULL)
	{
		if(pObj2->m_bCharInfoExist == true)
		{
			if(pObj2->m_uidStage == m_pStage->GetUID())
			{
				if(pObj2->CheckGameFlag(MMOGF_ENTERED) == true)
				{
					if(pObj2->IsHide() == false)
					{
						pObj2->m_GameInfo.bAlive = true;
					}
				}
			}
		}
	}
}

void MMatchGame_DuelTournament::CheckFighterAlive(bool *pOutPlayer1, bool *pOutPlayer2)
{
	bool bPlayer1Alive = false, bPlayer2Alive = false;
	
	MMatchObject *pObj1 = g_ObjectMgr.Get(m_uidFighter[0]);
	if(pObj1 != NULL)
	{
		if(pObj1->m_uidStage == m_pStage->GetUID())
		{
			if(pObj1->CheckGameFlag(MMOGF_ENTERED) == true && pObj1->m_GameInfo.bAlive == true)
				bPlayer1Alive = true;
		}
	}
	
	MMatchObject *pObj2 = g_ObjectMgr.Get(m_uidFighter[1]);
	if(pObj2 != NULL)
	{
		if(pObj2->m_uidStage == m_pStage->GetUID())
		{
			if(pObj2->CheckGameFlag(MMOGF_ENTERED) == true && pObj2->m_GameInfo.bAlive == true)
				bPlayer2Alive = true;
		}
	}
	
	*pOutPlayer1 = bPlayer1Alive;
	*pOutPlayer2 = bPlayer2Alive;
}

bool MMatchGame_DuelTournament::IsValidFighter(const MUID &uidFighter)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidFighter);
	if(pObj == NULL) return false;
	
	if(pObj->m_uidStage != m_pStage->GetUID()) return false;
	if(pObj->CheckGameFlag(MMOGF_ENTERED) == false) return false;
	
	return true;
}

void MMatchGame_DuelTournament::Advance(const MUID &uidWinner)
{
	switch(GetMatchType())
	{
		case MDUELTOURNAMENT_ROUNDSTATE_FINAL	:
		{
			// do nothing. : no more advances.
		}
		break;
		
		case MDUELTOURNAMENT_ROUNDSTATE_SEMIFINAL	:
		{
			m_uidSFinalPlayer[m_nPlayerIndex] = m_uidSFinalPlayer[m_nPlayerIndex + 1] = MUID(0, 0);
			m_uidFinalPlayer[m_nPlayerIndex / 2] = uidWinner;
		}
		break;
		
		case MDUELTOURNAMENT_ROUNDSTATE_QUARTERFINAL	:
		{
			m_uidQFinalPlayer[m_nPlayerIndex] = m_uidQFinalPlayer[m_nPlayerIndex + 1] = MUID(0, 0);
			m_uidSFinalPlayer[m_nPlayerIndex / 2] = uidWinner;
		}
		break;
	}
}

bool MMatchGame_DuelTournament::IsMatchingPlayer(const MUID &uidPlayer)
{
	// check quarter final.
	for(int i = 0; i < 8; i++)
	{
		if(m_uidQFinalPlayer[i] == uidPlayer) 
			return true;
	}
	
	// check semi final.
	for(int i = 0; i < 4; i++)
	{
		if(m_uidSFinalPlayer[i] == uidPlayer) 
			return true;
	}
	
	// check final.
	for(int i = 0; i < 2; i++)
	{
		if(m_uidFinalPlayer[i] == uidPlayer) 
			return true;
	}
	
	return false;
}

void MMatchGame_DuelTournament::SetPlayerStatus(const MUID &uidPlayer, int nDamaged, int nRemainHP, int nRemainAP)
{
	if(m_uidFighter[0] == uidPlayer)
	{
		m_PlayerInfo[0].status.bValid = true;
		m_PlayerInfo[0].status.nDamagedPoint = nDamaged;
		m_PlayerInfo[0].status.nHP = nRemainHP;
		m_PlayerInfo[0].status.nAP = nRemainAP;
	}
	else if(m_uidFighter[1] == uidPlayer)
	{
		m_PlayerInfo[1].status.bValid = true;
		m_PlayerInfo[1].status.nDamagedPoint = nDamaged;
		m_PlayerInfo[1].status.nHP = nRemainHP;
		m_PlayerInfo[1].status.nAP = nRemainAP;
	}
}

void MMatchGame_DuelTournament::ResetPlayerInfo()
{
	for(int i = 0; i < 2; i++)
	{
		m_PlayerInfo[i].nWin = 0;
		m_PlayerInfo[i].status.bValid = false;
	}
}

void MMatchGame_DuelTournament::InvalidatePlayerStatus()
{
	for(int i = 0; i < 2; i++)
	{
		m_PlayerInfo[i].status.bValid = false;
	}
}

void MMatchGame_DuelTournament::DetermineWinner()
{
	if(m_bTimeout == false) return;
	
	if(m_PlayerInfo[0].status.bValid == false)
	{
		m_uidLastWinner = m_uidFighter[1];
		return;
	}
	else if(m_PlayerInfo[1].status.bValid == false)
	{
		m_uidLastWinner = m_uidFighter[0];
		return;
	}
	
	if(m_PlayerInfo[0].status.nDamagedPoint != m_PlayerInfo[1].status.nDamagedPoint)
	{
		if(m_PlayerInfo[0].status.nDamagedPoint < m_PlayerInfo[1].status.nDamagedPoint)
			m_uidLastWinner = m_uidFighter[0];
		else
			m_uidLastWinner = m_uidFighter[1];
			
		return;
	}
	
	if(m_PlayerInfo[0].status.nHP != m_PlayerInfo[1].status.nHP)
	{
		if(m_PlayerInfo[0].status.nHP > m_PlayerInfo[1].status.nHP)
			m_uidLastWinner = m_uidFighter[0];
		else
			m_uidLastWinner = m_uidFighter[1];
			
		return;
	}
	
	if(m_PlayerInfo[0].status.nAP != m_PlayerInfo[1].status.nAP)
	{
		if(m_PlayerInfo[0].status.nAP > m_PlayerInfo[1].status.nAP)
			m_uidLastWinner = m_uidFighter[0];
		else
			m_uidLastWinner = m_uidFighter[1];
			
		return;
	}
	
	m_uidLastWinner = m_uidFighter[0];
}

void MMatchGame_DuelTournament::ChangeToInfiniteTime()
{
	m_pStage->SetTimeLimit(0);
	
	MCmdWriter Cmd;
	CreateStageSettingCommand(m_pStage, &Cmd);
	SendToStage(&Cmd, m_pStage->GetUID());
}

// quest.
MMatchGame_Quest::MMatchGame_Quest()
{
	m_pScenario = NULL;
	m_nRoundStartTime = 0;
	m_nPortalEndTime = 0;
	m_nNextNPCSpawnTime = 0;
	m_nRound = 0;
	m_nRoundInfoIndex = 0;
	m_nCombatState = (int)MQCS_NONE;
	m_bBossRound = false;
	m_fCurrXP = 0.0f;
	m_fCurrBP = 0.0f;
	m_fRepeatBonus = 1.0f;
	m_bQuestDone = false;
	m_nFinishTime = 0;
	m_nRepeated = 0;
}

MMatchGame_Quest::~MMatchGame_Quest()
{
	m_vtRoundInfo.clear();
	m_AliveNPCList.clear();
	m_AliveBossList.clear();
	m_vtPlayerStatus.clear();
	m_vtNPCScheduledItem.clear();
	m_vtTookNPCReward.clear();
}

void MMatchGame_Quest::OnCreate()
{
	m_pScenario = g_Quest.GetScenario(m_pStage->GetQuestScenarioID());
	if(m_pScenario == NULL)
	{
		// error, abort game.
		mlog("MMatchGame_Quest::OnCreate() - not existing scenario.");
		Finish();
		return;
	}
	
	g_Quest.InitializeGameRound(m_pStage->GetQuestScenarioID(), &m_vtRoundInfo);
	
	for(map<MUID, MMatchObject *>::iterator i = m_pStage->ObjBegin(); i != m_pStage->ObjEnd(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		if(pCurr->IsHide() == true) continue;
		
		MQuestPlayerStatus info;
		info.uidPlayer = pCurr->GetUID();
		info.bNextSectorReady = false;
		
		m_vtPlayerStatus.push_back(info);
	}
	
	m_fCurrXP = (float)m_pScenario->nExp;
	m_fCurrBP = (float)m_pScenario->nBounty;
}

void MMatchGame_Quest::Update(unsigned long nTime)
{
	if(m_bFinished == true) return;
	
	MMatchStage *pStage = m_pStage;	// this stage.
	
	if(pStage->GetState() != (int)MMSS_RUN) return;
	
	// if(pStage->GetState() == (int)MMSS_RUN)
	{
		if(m_bGameStarted == false)
		{
			if(pStage->CheckGameStartable() == true)
			{
				SetRoundState((int)MMRS_COUNTDOWN);
				m_bGameStarted = true;
				
				RevivalAll();
			}
			else
			{
				return;
			}
		}
		else
		{
			if(m_nRoundState == (int)MMRS_COUNTDOWN)
			{
				if(m_nStartTime <= nTime)
				{
					SetRoundState((int)MMRS_PLAY);
				}
			}
		}
	}
	
	if(m_nCombatState == (int)MQCS_PLAY)
	{
		if(m_nRoundStartTime <= nTime)
		{
			SpawnBoss();
			
			if(m_nNextNPCSpawnTime <= nTime)
			{
				int nMaxSpawn = m_pScenario->nNPCCount / 2;	// npc same time spawn limit.
				
				if((int)m_AliveNPCList.size() < nMaxSpawn)
				{
					if(m_vtRoundInfo[m_nRoundInfoIndex].bInfinite == true)
					{
						UnLimitedSpawnNPC();
					}
					else
					{
						SpawnNPC();
					}
				}
				
				// next spawn time based on current alive npc count.
				m_nNextNPCSpawnTime = nTime + (unsigned long)RandNum(0, (int)(m_AliveNPCList.size() * 100));
			}
			
			// (no more NPCs && alive NPC is zero).
			bool bFinish = (m_vtRoundInfo[m_nRoundInfoIndex].NPCIDs.size() == 0 && m_AliveNPCList.size() == 0) ? true : false ;
			
			// (boss spawned && 
			if(m_bBossRound == true)
			{
				// alive boss is zero).
				bFinish = m_AliveBossList.size() == 0 ? true : false ;
			}
			
			if(bFinish == true)
			{
				if(m_nRound + 1 >= m_pScenario->nRound * m_pScenario->nRepeat)
				{
					if(m_bQuestDone == false)
					{
						// add finish exp & bp.
						m_fCurrXP *= 2.0f;
						m_fCurrBP *= 2.0f;
						
						// finish, wait 5 sec.
						m_nFinishTime = nTime + 5000;
						m_bQuestDone = true;
					}
				}
				else
				{
					ExpBonus();
					SetCombatState((int)MQCS_COMPLETED);
				}
			}
		}
		
		if(CheckGameOver() == true)
		{
			QuestFailed();
			return;
		}
	}
	else if(m_nCombatState == (int)MQCS_COMPLETED)
	{
		if(CheckSectorProcessable() == true || m_nPortalEndTime <= nTime)
		{
			StartSector();
		}
	}
	
	if(m_bQuestDone == true && m_nFinishTime <= nTime)
	{
		ExpBonus();
		
		QuestCompleted();
		return;
	}
	
	MMatchBaseGame::Update(nTime);
}

void MMatchGame_Quest::SetRoundState(int nState)
{
	if(m_nRoundState == nState) return;
	
	switch(nState)
	{
		case (int)MMRS_COUNTDOWN	:
		{
			m_nStartTime = GetTime() + 2000;
			EraseAllWorldItem();
			
			SetCombatState((int)MQCS_PREPARE);
		}
		break;
		
		case (int)MMRS_PLAY	:
		{
			SetCombatState((int)MQCS_PLAY);
		}
		break;
	}
	
	SendRoundStateToBattle(m_pStage->GetUID(), 0, nState, 0);
	
	m_nRoundState = nState;
}

void MMatchGame_Quest::Finish()
{
	if(m_bFinished == true) return;
	
	SurvivalFinish();
	
	m_pStage->ResetQuestInfo();
	
	SetRoundState((int)MMRS_FINISH);
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(m_pStage->GetUID());
	Cmd.WriteBool(m_pStage->GetRelayMapSetting()->IsUnFinish());
	Cmd.Finalize(MC_MATCH_STAGE_FINISH_GAME, MCFT_END);
	SendToStage(&Cmd, m_pStage->GetUID());
	
	m_pStage->AbortVoting();
	
	m_pStage->SetState((int)MMSS_STANDBY);
	m_bFinished = true;
}

void MMatchGame_Quest::SetCombatState(int nState)
{
	if(m_nCombatState == nState) return;
	
	switch(nState)
	{
		case (int)MQCS_PLAY	:
		{
			m_nRoundStartTime = GetTime() + 5000;	// 5 sec.
			RoundRefresh();
		}
		break;
		
		case (int)MQCS_COMPLETED	:
		{
			m_nPortalEndTime = GetTime() + 30000;	// 30 sec.
			
			m_nRound++;
			m_nRoundInfoIndex = m_nRound % m_pScenario->nRound;
			
			if(m_nRoundInfoIndex == 0) m_nRepeated++;
		}
		break;
	}
	
	MCmdWriter Cmd;
	Cmd.WriteChar((char)nState);
	Cmd.Finalize(MC_QUEST_COMBAT_STATE, MCFT_END);
	SendToBattle(&Cmd, m_pStage->GetUID());
	
	m_nCombatState = nState;
}

MUID MMatchGame_Quest::RandomNPCOwner()
{
	vector<MUID> vtCandUID;
	
	for(map<MUID, MMatchObject *>::iterator i = m_pStage->ObjBegin(); i != m_pStage->ObjEnd(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		if(pCurr->CheckGameFlag(MMOGF_ENTERED) == false) continue;
		
		if(pCurr->IsHide() == false)
		{
			vtCandUID.push_back(pCurr->GetUID());
		}
	}
	
	if(vtCandUID.size() == 0) return MUID(0, 0);
	
	int nRand = (int)(RandNum() % (unsigned int)vtCandUID.size());
	return vtCandUID[nRand];
}

void MMatchGame_Quest::SpawnNPC()
{
	if(m_vtRoundInfo[m_nRoundInfoIndex].NPCIDs.empty() == true) return;	// no more npc.
	
	int nNPCID = m_vtRoundInfo[m_nRoundInfoIndex].NPCIDs.front();
	
	MUID uidOwner = RandomNPCOwner();
	MUID uidNPC = MUID::Assign();
	
	MAliveNPCInfo info = {uidOwner, uidNPC};
	m_AliveNPCList.push_back(info);
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidOwner);
	Cmd.WriteMUID(uidNPC);
	Cmd.WriteUChar((unsigned char)nNPCID);
	Cmd.WriteUChar((unsigned char)RandNum(0, UCHAR_MAX));
	Cmd.Finalize(MC_QUEST_NPC_SPAWN, MCFT_END);
	SendToBattle(&Cmd, m_pStage->GetUID());
	
	ScheduleNPCDropItem(uidNPC, nNPCID);
	
	m_vtRoundInfo[m_nRoundInfoIndex].NPCIDs.pop_front();
}

void MMatchGame_Quest::SpawnBoss()
{
	if(m_vtRoundInfo[m_nRoundInfoIndex].BossNPCIDs.empty() == true) return;
	
	int nNPCID = m_vtRoundInfo[m_nRoundInfoIndex].BossNPCIDs.front();
	
	MUID uidOwner = RandomNPCOwner();
	MUID uidNPC = MUID::Assign();
	
	MAliveNPCInfo info = {uidOwner, uidNPC};
	m_AliveBossList.push_back(info);
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidOwner);
	Cmd.WriteMUID(uidNPC);
	Cmd.WriteUChar((unsigned char)nNPCID);
	Cmd.WriteUChar(0);
	Cmd.Finalize(MC_QUEST_NPC_SPAWN, MCFT_END);
	SendToBattle(&Cmd, m_pStage->GetUID());
	
	ScheduleNPCDropItem(uidNPC, nNPCID);
	
	m_vtRoundInfo[m_nRoundInfoIndex].BossNPCIDs.pop_front();
	
	m_bBossRound = true;
}

void MMatchGame_Quest::UnLimitedSpawnNPC()
{
	// if(m_vtRoundInfo[m_nRoundInfoIndex].bInfinite == false) return;
	
	int nNPCID = g_Quest.GetRandomNPC(m_pScenario->nID, m_nRoundInfoIndex);
	if(nNPCID == 0) return;
	
	MUID uidOwner = RandomNPCOwner();
	MUID uidNPC = MUID::Assign();
	
	MAliveNPCInfo info = {uidOwner, uidNPC};
	m_AliveNPCList.push_back(info);
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidOwner);
	Cmd.WriteMUID(uidNPC);
	Cmd.WriteUChar((unsigned char)nNPCID);
	Cmd.WriteUChar((unsigned char)RandNum(0, UCHAR_MAX));
	Cmd.Finalize(MC_QUEST_NPC_SPAWN, MCFT_END);
	SendToBattle(&Cmd, m_pStage->GetUID());
}

void MMatchGame_Quest::ReSelectNPCOwner()
{
	for(list<MAliveNPCInfo>::iterator i = m_AliveNPCList.begin(); i != m_AliveNPCList.end(); i++)
	{
		MAliveNPCInfo *pCurr = &(*i);
		
		MUID uidNewOwner = RandomNPCOwner();
		pCurr->uidOwner = uidNewOwner;
		
		MCmdWriter Cmd;
		Cmd.WriteMUID(uidNewOwner);
		Cmd.WriteMUID(pCurr->uidNPC);
		Cmd.Finalize(MC_QUEST_ENTRUST_NPC_CONTROL, MCFT_END);
		SendToBattle(&Cmd, m_pStage->GetUID());
	}
	
	for(list<MAliveNPCInfo>::iterator i = m_AliveBossList.begin(); i != m_AliveBossList.end(); i++)
	{
		MAliveNPCInfo *pCurr = &(*i);
		
		MUID uidNewOwner = RandomNPCOwner();
		pCurr->uidOwner = uidNewOwner;
		
		MCmdWriter Cmd;
		Cmd.WriteMUID(uidNewOwner);
		Cmd.WriteMUID(pCurr->uidNPC);
		Cmd.Finalize(MC_QUEST_ENTRUST_NPC_CONTROL, MCFT_END);
		SendToBattle(&Cmd, m_pStage->GetUID());
	}
}

bool MMatchGame_Quest::IsValidNPCOwner(const MUID &uidOwner, const MUID &uidNPC)
{
	// find normal NPC first.
	for(list<MAliveNPCInfo>::iterator i = m_AliveNPCList.begin(); i != m_AliveNPCList.end(); i++)
	{
		MAliveNPCInfo *pCurr = &(*i);
		
		if(pCurr->uidOwner == uidOwner)
		{
			if(pCurr->uidNPC == uidNPC)
				return true;
		}
	}
	
	// find boss NPC second.
	for(list<MAliveNPCInfo>::iterator i = m_AliveBossList.begin(); i != m_AliveBossList.end(); i++)
	{
		MAliveNPCInfo *pCurr = &(*i);
		
		if(pCurr->uidOwner == uidOwner)
		{
			if(pCurr->uidNPC == uidNPC)
				return true;
		}
	}
	
	return false;
}

bool MMatchGame_Quest::NPCDead(const MUID &uidKiller, const MUID &uidNPC)
{
	for(list<MAliveNPCInfo>::iterator i = m_AliveNPCList.begin(); i != m_AliveNPCList.end(); i++)
	{
		MAliveNPCInfo *pCurr = &(*i);
		
		if(pCurr->uidNPC == uidNPC)
		{
			MCmdWriter Cmd;
			Cmd.WriteMUID(uidKiller);
			Cmd.WriteMUID(uidNPC);
			Cmd.Finalize(MC_QUEST_NPC_DEAD, MCFT_END);
			SendToBattle(&Cmd, m_pStage->GetUID());
			
			m_AliveNPCList.erase(i);
			return true;
		}
	}
	
	for(list<MAliveNPCInfo>::iterator i = m_AliveBossList.begin(); i != m_AliveBossList.end(); i++)
	{
		MAliveNPCInfo *pCurr = &(*i);
		
		if(pCurr->uidNPC == uidNPC)
		{
			MCmdWriter Cmd;
			Cmd.WriteMUID(uidKiller);
			Cmd.WriteMUID(uidNPC);
			Cmd.Finalize(MC_QUEST_NPC_DEAD, MCFT_END);
			SendToBattle(&Cmd, m_pStage->GetUID());
			
			m_AliveBossList.erase(i);
			return true;
		}
	}
	
	return false;
}

MQuestPlayerStatus *MMatchGame_Quest::GetPlayerStatus(const MUID &uidPlayer)
{
	for(vector<MQuestPlayerStatus>::iterator i = m_vtPlayerStatus.begin(); i != m_vtPlayerStatus.end(); i++)
	{
		MQuestPlayerStatus *pCurr = &(*i);
		if(pCurr->uidPlayer == uidPlayer) return pCurr;
	}
	return NULL;
}

bool MMatchGame_Quest::CheckGameOver()
{
	for(map<MUID, MMatchObject *>::iterator i = m_pStage->ObjBegin(); i != m_pStage->ObjEnd(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		if(pCurr->CheckGameFlag(MMOGF_ENTERED) == true && pCurr->m_GameInfo.bAlive == true) return false;
	}
	
	return true;
}

bool MMatchGame_Quest::CheckSectorProcessable()
{
	for(vector<MQuestPlayerStatus>::iterator i = m_vtPlayerStatus.begin(); i != m_vtPlayerStatus.end(); i++)
	{
		MQuestPlayerStatus *pStatus = &(*i);
		
		if(m_pStage->ObjExists(pStatus->uidPlayer) == true)
		{
			if(pStatus->bNextSectorReady == false) return false;
		}
	}
	
	return true;
}

void MMatchGame_Quest::SectorPlayerReady(const MUID &uidPlayer)
{
	MQuestPlayerStatus *pStatus = GetPlayerStatus(uidPlayer);
	if(pStatus != NULL)
	{
		pStatus->bNextSectorReady = true;
	}
}

void MMatchGame_Quest::ExpBonus()
{
	for(map<MUID, MMatchObject *>::iterator i = m_pStage->ObjBegin(); i != m_pStage->ObjEnd(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		if(pCurr->CheckGameFlag(MMOGF_ENTERED) == false) continue;
		
		if(pCurr->IsHide() == true) continue;
		
		int nNewTotalExp = CheckPlusOver(pCurr->m_Exp.nXP, pCurr->m_GameInfo.nExp + (int)m_fCurrXP);
		int nNewLevel = g_FormulaMgr.GetLevelFromExp(nNewTotalExp);
		int nExpPercent = g_FormulaMgr.GetExpPercent(nNewLevel, nNewTotalExp);
		
		MCmdWriter Cmd;
		
		Cmd.WriteMUID(pCurr->GetUID());
		Cmd.WriteULong(MakeExpCommandData((unsigned long)m_fCurrXP, nExpPercent));
		Cmd.WriteULong((unsigned long)m_fCurrBP);
		Cmd.Finalize(MC_QUEST_SECTOR_BONUS);
		
		if(nNewLevel > pCurr->m_Exp.nLevel)
		{
			Cmd.WriteMUID(pCurr->GetUID());
			Cmd.WriteInt(nNewLevel);
			Cmd.Finalize(MC_MATCH_GAME_LEVEL_UP);
		}
		
		Cmd.Finalize();
		SendToBattle(&Cmd, m_pStage->GetUID());
		
		pCurr->m_Exp.nLevel = nNewLevel;
		pCurr->m_GameInfo.nExp += (int)m_fCurrXP;
		pCurr->m_Exp.nBP += (int)m_fCurrBP;
		
		pCurr->m_Account.nCash += g_nCashBonus[CASH_BONUS_QUEST];
	}
}

void MMatchGame_Quest::StartSector()
{
	RevivalAll();
	
	for(vector<MQuestPlayerStatus>::iterator i = m_vtPlayerStatus.begin(); i != m_vtPlayerStatus.end(); i++)
	{
		MQuestPlayerStatus *pCurr = &(*i);
		
		pCurr->bNextSectorReady = false;
	}
	
	m_AliveNPCList.clear();
	m_AliveBossList.clear();
	
	if(m_nRoundInfoIndex == 0)	// m_nRoundInfoIndex : (curr round % total round).
	{
		g_Quest.InitializeGameRound(m_pStage->GetQuestScenarioID(), &m_vtRoundInfo);
		
		// increase repeat xp/bp rate.
		m_fRepeatBonus += 0.1f;
		
		// init xp/bp with that rate.
		m_fCurrXP = (float)((float)m_pScenario->nExp * m_fRepeatBonus);
		m_fCurrBP = (float)((float)m_pScenario->nBounty * m_fRepeatBonus);
	}
	else
	{
		// add bonus exp & bp.
		m_fCurrXP *= 1.2f;
		m_fCurrBP *= 1.2f;
	}
	
	m_vtNPCScheduledItem.clear();
	
	SetCombatState((int)MQCS_PLAY);
}

void MMatchGame_Quest::RoundRefresh()
{
	m_bBossRound = false;
	
	MCmdWriter Cmd;
	
	Cmd.WriteChar((char)m_nRoundInfoIndex);
	Cmd.WriteUChar((unsigned char)m_nRepeated);
	Cmd.Finalize(MC_QUEST_SECTOR_START);
	
	Cmd.Finalize(MC_QUEST_NPC_ALL_CLEAR);
	Cmd.Finalize(MC_QUEST_REFRESH_PLAYER_STATUS, MCFT_END);
	
	SendToBattle(&Cmd, m_pStage->GetUID());
}

void MMatchGame_Quest::QuestCompleted()
{
	MCmdWriter Cmd;
	
	// quest complete.
	Cmd.StartBlob(sizeof(MTD_QuestReward));
	for(map<MUID, MMatchObject *>::iterator i = m_pStage->ObjBegin(); i != m_pStage->ObjEnd(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		if(pCurr->CheckGameFlag(MMOGF_ENTERED) == false) continue;
		
		if(pCurr->IsHide() == true) continue;
		
		MTD_QuestReward reward;
		reward.uidPlayer = pCurr->GetUID();
		reward.nXP = (int)m_fCurrXP;
		reward.nBP = (int)m_fCurrBP;
		
		Cmd.WriteData(&reward, sizeof(MTD_QuestReward));
	}
	Cmd.EndBlob();
	Cmd.Finalize(MC_QUEST_COMPLETED, MCFT_END);
	
	SendToBattle(&Cmd, m_pStage->GetUID());
	
	DistributeReward();
	
	Finish();
}

void MMatchGame_Quest::QuestFailed()
{
	MCmdWriter Cmd;
	Cmd.Finalize(MC_QUEST_FAILED, MCFT_END);
	SendToBattle(&Cmd, m_pStage->GetUID());
	
	Finish();
}

void MMatchGame_Quest::ScheduleNPCDropItem(const MUID &uidNPC, int nNPCID)
{
	MQuestNPCDropItemNode *pNode = g_Quest.GetRandomNPCItem(nNPCID);
	if(pNode == NULL) return;
	
	MQuestNPC_ScheduledItem item = {uidNPC, 0, pNode};
	m_vtNPCScheduledItem.push_back(item);
}

// returns -1 for nothing, itembox id (51) for zitem, raw world item id (can be 0) otherwise.
int MMatchGame_Quest::GetNPCDropWorldItemID(const MUID &uidNPC)
{
	for(vector<MQuestNPC_ScheduledItem>::iterator i = m_vtNPCScheduledItem.begin(); i != m_vtNPCScheduledItem.end(); i++)
	{
		MQuestNPC_ScheduledItem *pCurr = &(*i);
		
		if(pCurr->uidNPC == uidNPC)
		{
			if(pCurr->pNode->bZItem == true)
			{
				return 51;	// itembox.
			}
			else
			{
				return pCurr->pNode->nItemID;	// raw worlditem id.
			}
		}
	}
	
	return -1;
}

void MMatchGame_Quest::SetNPCDropWorldItemUID(const MUID &uidNPC, unsigned short nUID)
{
	for(vector<MQuestNPC_ScheduledItem>::iterator i = m_vtNPCScheduledItem.begin(); i != m_vtNPCScheduledItem.end(); i++)
	{
		MQuestNPC_ScheduledItem *pCurr = &(*i);
		
		if(pCurr->uidNPC == uidNPC)
		{
			pCurr->nWorldItemUID = nUID;
		}
	}
}

// returns itemid, 0 for error / non-zitem.
int MMatchGame_Quest::TakeNPCDropItem(unsigned short nWorldItemUID)
{
	for(vector<MQuestNPC_ScheduledItem>::iterator i = m_vtNPCScheduledItem.begin(); i != m_vtNPCScheduledItem.end(); i++)
	{
		MQuestNPC_ScheduledItem *pCurr = &(*i);
		
		if(pCurr->nWorldItemUID == nWorldItemUID)
		{
			if(pCurr->pNode->bZItem == true)
			{
				m_vtTookNPCReward.push_back(pCurr->pNode);
				return pCurr->pNode->nItemID;
			}
			else
			{
				return 0;
			}
		}
	}
	
	return 0;
}

void MMatchGame_Quest::DistributeReward()
{
	if(m_pStage->GetGameType() != (int)MMGT_QUEST) return;
	
	vector<MQuestGame_RewardItemObject> vtDistribInfo;
	
	// init MQuestGame_RewardItemObject array.
	for(map<MUID, MMatchObject *>::iterator i = m_pStage->ObjBegin(); i != m_pStage->ObjEnd(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		if(pCurr->CheckGameFlag(MMOGF_ENTERED) == false) continue;
		
		if(pCurr->IsHide() == false)
		{
			MQuestGame_RewardItemObject info = {pCurr, new vector<MQuestGame_RewardItemNode>};
			vtDistribInfo.push_back(info);
		}
	}
	
	if(vtDistribInfo.empty() == true) return;	// can't distribute item.
	
	// distrib all items.
	for(vector<MQuestNPCDropItemNode *>::iterator i = m_vtTookNPCReward.begin(); i != m_vtTookNPCReward.end(); i++)
	{
		MQuestNPCDropItemNode *pItem = (*i);
		
		int nRand = (int)(RandNum() % (unsigned int)vtDistribInfo.size());
		MQuestGame_RewardItemObject *pTarget = &vtDistribInfo[nRand];
		
		// assuming (id >= 200001 && id <= 200044) is quest item.
		if(IsQuestItemID(pItem->nItemID) == true)
		{
			// quest item process.
			bool bAdd = true;
			
			// found : increase, not found : add.
			for(vector<MQuestGame_RewardItemNode>::iterator j = pTarget->pvtItem->begin(); j != pTarget->pvtItem->end(); j++)
			{
				MQuestGame_RewardItemNode *pItemNode = &(*j);
				
				if(pItemNode->nItemID == pItem->nItemID)
				{
					pItemNode->nCount++;
					bAdd = false;
					break;
				}
			}
			
			if(bAdd == true)
			{
				MQuestGame_RewardItemNode node = {pItem->nItemID, 1, 0};
				pTarget->pvtItem->push_back(node);
			}
		}
		else
		{
			// normal zitem process.
			MQuestGame_RewardItemNode node = {pItem->nItemID, 1, pItem->nRentHourPeriod};
			pTarget->pvtItem->push_back(node);
		}
	}
	
	// save distributed reward items, send reward info. and un-init MQuestGame_RewardItemObject array.
	for(vector<MQuestGame_RewardItemObject>::iterator i = vtDistribInfo.begin(); i != vtDistribInfo.end(); i++)
	{
		MQuestGame_RewardItemObject *pInfo = &(*i);
		
		MMatchObject *pCurrObj = pInfo->pObj;
		
		// send data array.
		vector<MTD_QuestItemNode> vtRewardQItem;
		vector<MTD_QuestZItemNode> vtRewardZItem;
		
		// save items.
		for(vector<MQuestGame_RewardItemNode>::iterator j = pInfo->pvtItem->begin(); j != pInfo->pvtItem->end(); j++)
		{
			MQuestGame_RewardItemNode *pItemNode = &(*j);
			
			if(IsQuestItemID(pItemNode->nItemID) == true)	// : quest item id condition - see above.
			{
				// add sacri item, setted to MAX_QUEST_ITEM_COUNT if max item count over.
				
				MMatchQuestItem *pQuestItem = pCurrObj->GetQuestItem(pItemNode->nItemID);
				if(pQuestItem == NULL)
				{
					// add.
					int nNewCount = pItemNode->nCount;
					if(nNewCount > MAX_QUEST_ITEM_COUNT) nNewCount = MAX_QUEST_ITEM_COUNT;
					
					MMatchQuestItem item = {pItemNode->nItemID, nNewCount};
					pCurrObj->m_QuestItemList.push_back(item);
				}
				else
				{
					// update.
					pQuestItem->nItemCount += pItemNode->nCount;
					if(pQuestItem->nItemCount > MAX_QUEST_ITEM_COUNT) pQuestItem->nItemCount = MAX_QUEST_ITEM_COUNT;
				}
				
				MTD_QuestItemNode qinode;
				qinode.nItemID = pItemNode->nItemID;
				qinode.nCount = pItemNode->nCount;
				vtRewardQItem.push_back(qinode);
			}
			else
			{
				// add char item.
				
				/*
				bool bResult = false;
				
				int nCIID;
				if(pItemNode->nRentHourPeriod == 0)
				{
					bResult = Db_InsertItem(pCurrObj->m_Char.nCID, pItemNode->nItemID, pItemNode->nCount, pCurrObj->m_Exp.nBP, &nCIID);
				}
				else
				{
					bResult = Db_InsertItem(pCurrObj->m_Char.nCID, pItemNode->nItemID, pItemNode->nCount, pItemNode->nRentHourPeriod, pCurrObj->m_Exp.nBP, &nCIID);
				}
				
				if(bResult == false) continue;
				
				pCurrObj->AddCharItem(MUID::Assign(), nCIID, pItemNode->nItemID, pItemNode->nCount, pItemNode->nRentHourPeriod * 60 * 60);
				*/
				
				AsyncDb_InsertItem(pCurrObj->GetUID(), pCurrObj->m_Char.nCID, pItemNode->nItemID, pItemNode->nCount, pItemNode->nRentHourPeriod, pCurrObj->m_Exp.nBP);
				
				MTD_QuestZItemNode zinode;
				zinode.nItemID = pItemNode->nItemID;
				zinode.nRentPeriodHour = pItemNode->nRentHourPeriod;
				zinode.nItemCnt = pItemNode->nCount;
				vtRewardZItem.push_back(zinode);
			}
		}
		
		// send info about rewards.
		if(CheckGameVersion(2012) == true)
		{
			MCmdWriter Cmd;
			
			Cmd.WriteMUID(pCurrObj->GetUID());
		
			Cmd.WriteInt((int)m_fCurrXP);
			Cmd.WriteInt((int)m_fCurrBP);
		
			Cmd.StartBlob(sizeof(MTD_QuestItemNode));
			for(vector<MTD_QuestItemNode>::iterator j = vtRewardQItem.begin(); j != vtRewardQItem.end(); j++)
			{
				Cmd.WriteData(&(*j), sizeof(MTD_QuestItemNode));
			}
			Cmd.EndBlob();
		
			Cmd.StartBlob(sizeof(MTD_QuestZItemNode));
			for(vector<MTD_QuestZItemNode>::iterator j = vtRewardZItem.begin(); j != vtRewardZItem.end(); j++)
			{
				Cmd.WriteData(&(*j), sizeof(MTD_QuestZItemNode));
			}
			Cmd.EndBlob();
		
			Cmd.Finalize(MC_MATCH_USER_QUEST_REWARD, MCFT_END);
			SendToStage(&Cmd, m_pStage->GetUID());
		}
		else
		{
			MCmdWriter Cmd;
		
			Cmd.WriteInt((int)m_fCurrXP);
			Cmd.WriteInt((int)m_fCurrBP);
		
			Cmd.StartBlob(sizeof(MTD_QuestItemNode));
			for(vector<MTD_QuestItemNode>::iterator j = vtRewardQItem.begin(); j != vtRewardQItem.end(); j++)
			{
				Cmd.WriteData(&(*j), sizeof(MTD_QuestItemNode));
			}
			Cmd.EndBlob();
		
			Cmd.StartBlob(sizeof(MTD_QuestZItemNode));
			for(vector<MTD_QuestZItemNode>::iterator j = vtRewardZItem.begin(); j != vtRewardZItem.end(); j++)
			{
				Cmd.WriteData(&(*j), sizeof(MTD_QuestZItemNode));
			}
			Cmd.EndBlob();
		
			Cmd.Finalize(MC_MATCH_USER_QUEST_REWARD, MCFT_END);
			SendToClient(&Cmd, pCurrObj->GetUID());
		}
		
		delete pInfo->pvtItem;
	}
	
	vtDistribInfo.clear();
	
	m_vtTookNPCReward.clear();
}

void MMatchGame_Quest::SurvivalFinish()
{
	if(m_pStage->GetGameType() != (int)MMGT_SURVIVAL) return;
	
	int nGainPoint = (int)((float)m_nRound * 1.2f);
	
	// send result.
	MCmdWriter CmdResult;
	CmdResult.WriteInt(m_nRound + 1);
	CmdResult.WriteInt(nGainPoint);
	CmdResult.Finalize(MC_QUEST_SURVIVAL_RESULT, MCFT_END);
	SendToBattle(&CmdResult, m_pStage->GetUID());
		
	// set & send player's private ranking.
	for(map<MUID, MMatchObject *>::iterator i = m_pStage->ObjBegin(); i != m_pStage->ObjEnd(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		
		if(pCurr->IsHide() == false)
		{
			pCurr->m_Survival.nPoint += nGainPoint;
		}
		
		MCmdWriter CmdPRank;
		CmdPRank.WriteUInt((unsigned int)pCurr->m_Survival.nRanking);
		CmdPRank.WriteUInt((unsigned int)pCurr->m_Survival.nPoint);
		CmdPRank.Finalize(MC_SURVIVAL_PRIVATE_RANKING, MCFT_END);
		SendToClient(&CmdPRank, pCurr->GetUID());
	}
	
	// send global ranking to stage.
	g_Quest.SendSurvivalRankingInfo(m_pStage->GetUID());
}

void MMatchGame_Quest::RevivalAll()
{
	for(map<MUID, MMatchObject *>::iterator i = m_pStage->ObjBegin(); i != m_pStage->ObjEnd(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		if(pCurr->CheckGameFlag(MMOGF_ENTERED) == false) continue;
		
		if(pCurr->IsHide() == false)
		{
			pCurr->m_GameInfo.bAlive = true;
		}
	}
}

// challenge quest.
MMatchGame_ChallengeQuest::MMatchGame_ChallengeQuest()
{
	m_pScenario = NULL;
	m_nRound = 0;
	m_nQuestStartedTime = 0;
	m_nQuestEndedTime = 0;
	m_bActorSpawned = false;
	m_nActorSpawnTime = 0;
	m_bRoundCleared = false;
	m_nNextRoundStartTime = 0;
	m_fCurrXP = 0.0f;
	m_fCurrBP = 0.0f;
}

MMatchGame_ChallengeQuest::~MMatchGame_ChallengeQuest()
{
	m_AliveActorList.clear();
}

void MMatchGame_ChallengeQuest::OnCreate()
{
	m_pScenario = g_ChallengeQuest.GetScenario(m_pStage->GetMapName());
	if(m_pScenario == NULL)
	{
		mlog("MMatchGame_ChallengeQuest::OnCreate() - not existing scenario.");
		Finish();
		return;
	}
	
	m_fCurrXP = m_pScenario->fXP;
	m_fCurrBP = m_pScenario->fBP;
}

void MMatchGame_ChallengeQuest::Update(unsigned long nTime)
{
	if(m_bFinished == true) return;
	
	MMatchStage *pStage = m_pStage;	// this stage.
	
	if(pStage->GetState() != (int)MMSS_RUN) return;
	
	// if(pStage->GetState() == (int)MMSS_RUN)
	{
		if(m_bGameStarted == false)
		{
			if(pStage->CheckGameStartable() == true)
			{
				m_nQuestStartedTime = nTime;
				
				SetRoundState((int)MMRS_COUNTDOWN);
				m_bGameStarted = true;
				
				RevivalAll();
			}
			else
			{
				return;
			}
		}
		else
		{
			if(m_nRoundState == (int)MMRS_COUNTDOWN)
			{
				if(m_nStartTime <= nTime)
				{
					SetRoundState((int)MMRS_PLAY);
					
					m_nActorSpawnTime = nTime + 2000;	// 2 sec.
					m_bActorSpawned = false;
				}
			}
		}
	}
	
	if(m_bRoundCleared == true)
	{
		if(m_nNextRoundStartTime <= nTime)
		{
			SectorStart();
		}
		return;
	}
	
	if(m_nRoundState == (int)MMRS_PLAY)
	{
		if(m_bActorSpawned == false)
		{
			if(m_nActorSpawnTime <= nTime)
			{
				SpawnPreparedActor();
				m_bActorSpawned = true;
			}
		}
		else
		{
			if(m_AliveActorList.size() == 0)
			{
				SectorFinish();
				return;
			}
		}
		
		if(CheckGameOver() == true)
		{
			QuestFailed();
			return;
		}
	}
	
	MMatchBaseGame::Update(nTime);
}

void MMatchGame_ChallengeQuest::SetRoundState(int nState)
{
	if(m_nRoundState == nState) return;
	
	switch(nState)
	{
		case (int)MMRS_COUNTDOWN	:
		{
			m_nStartTime = GetTime() + 2000;
			EraseAllWorldItem();
		}
		break;
	}
	
	SendRoundStateToBattle(m_pStage->GetUID(), m_nRound, nState, 0);
	
	m_nRoundState = nState;
}

void MMatchGame_ChallengeQuest::Finish()
{
	if(m_bFinished == true) return;
	
	SetRoundState((int)MMRS_FINISH);
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(m_pStage->GetUID());
	Cmd.WriteBool(m_pStage->GetRelayMapSetting()->IsUnFinish());
	Cmd.Finalize(MC_MATCH_STAGE_FINISH_GAME, MCFT_END);
	SendToStage(&Cmd, m_pStage->GetUID());
	
	m_pStage->AbortVoting();
	
	m_pStage->SetState((int)MMSS_STANDBY);
	m_bFinished = true;
}

MUID MMatchGame_ChallengeQuest::RandomActorOwner()
{
	vector<MUID> vtCandUID;
	
	for(map<MUID, MMatchObject *>::iterator i = m_pStage->ObjBegin(); i != m_pStage->ObjEnd(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		if(pCurr->CheckGameFlag(MMOGF_ENTERED) == false) continue;
		
		if(pCurr->IsHide() == false)
		{
			vtCandUID.push_back(pCurr->GetUID());
		}
	}
	
	if(vtCandUID.size() == 0) return MUID(0, 0);
	
	int nRand = (int)(RandNum() % (unsigned int)vtCandUID.size());
	return vtCandUID[nRand];
}

void MMatchGame_ChallengeQuest::ReSelectActorOwner()
{
	for(list<MAliveNPCInfo>::iterator i = m_AliveActorList.begin(); i != m_AliveActorList.end(); i++)
	{
		MAliveNPCInfo *pCurr = &(*i);
		
		MUID uidNewOwner = RandomActorOwner();
		pCurr->uidOwner = uidNewOwner;
		
		MCmdWriter Cmd;
		Cmd.WriteMUID(uidNewOwner);
		Cmd.WriteMUID(pCurr->uidNPC);
		Cmd.Finalize(MC_CHALLENGE_QUEST_ENTRUST_NPC_CONTROL, MCFT_END);
		SendToBattle(&Cmd, m_pStage->GetUID());
	}
}

bool MMatchGame_ChallengeQuest::SpawnActor(const char *pszActorName, int nPos, int nPosIndex)
{
	MMatchChallengeQuestNPCInfo *pActorInfo = g_ChallengeQuest.GetNPCInfo(pszActorName);
	if(pActorInfo == NULL) return false;
	
	MUID uidOwner = RandomActorOwner();
	MUID uidActor = MUID::Assign();
	
	MAliveNPCInfo info = {uidOwner, uidActor};
	m_AliveActorList.push_back(info);
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidOwner);
	Cmd.WriteMUID(uidActor);
	Cmd.WriteString(pszActorName);
	Cmd.WriteUChar((unsigned char)nPos);
	Cmd.WriteUChar((unsigned char)nPosIndex);
	Cmd.WriteUShort(pActorInfo->nHP);
	Cmd.WriteUShort(pActorInfo->nAP);
	Cmd.WriteFloat(1.0f);
	Cmd.Finalize(MC_CHALLENGE_QUEST_NPC_SPAWN, MCFT_END);
	SendToBattle(&Cmd, m_pStage->GetUID());
	
	return true;
}

bool MMatchGame_ChallengeQuest::SpawnActorByPlayer(const MUID &uidOwnerActor, const char *pszActorName, const FloatVector *pPos, const FloatVector *pDir)
{
	MMatchChallengeQuestNPCInfo *pActorInfo = g_ChallengeQuest.GetNPCInfo(pszActorName);
	if(pActorInfo == NULL) return false;
	
	MUID uidOwner = RandomActorOwner();
	MUID uidActor = MUID::Assign();
	
	MAliveNPCInfo info = {uidOwner, uidActor};
	m_AliveActorList.push_back(info);
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidOwnerActor);
	Cmd.WriteMUID(uidOwner);
	Cmd.WriteMUID(uidActor);
	Cmd.WriteString(pszActorName);
	Cmd.WriteVec(pPos->x, pPos->y, pPos->z);
	Cmd.WriteVec(pDir->x, pDir->y, pDir->z);
	Cmd.WriteUShort(pActorInfo->nHP);
	Cmd.WriteUShort(pActorInfo->nAP);
	Cmd.WriteFloat(1.0f);
	Cmd.Finalize(MC_CHALLENGE_QUEST_PLAYER_NPC_SPAWN, MCFT_END);
	SendToBattle(&Cmd, m_pStage->GetUID());
	
	return true;
}

void MMatchGame_ChallengeQuest::SpawnPreparedActor()
{
	for(vector<MMatchChallengeQuestSpawnActor>::iterator i = m_pScenario->vtSectors[m_nRound].vtActor.begin(); i != m_pScenario->vtSectors[m_nRound].vtActor.end(); i++)
	{
		MMatchChallengeQuestSpawnActor *pCurrActor = &(*i);
		
		int nTotalSpawn = 0;
		
		if(pCurrActor->bAdjustPlayerNum == true)
		{
			for(int j = 0; j < pCurrActor->nCount && j < m_pStage->GetPlayer(); j++)
			{
				nTotalSpawn++;
			}
		}
		else
		{
			nTotalSpawn = pCurrActor->nCount;
			
			for(int j = 1; j < m_pStage->GetPlayer(); j++)
			{
				nTotalSpawn += pCurrActor->nInc;
			}
		}
		
		for(int j = 0; j < nTotalSpawn; j++)
		{
			SpawnActor(pCurrActor->szActorName, pCurrActor->nPos, j);
		}
	}
}

bool MMatchGame_ChallengeQuest::IsValidActorOwner(const MUID &uidOwner, const MUID &uidNPC)
{
	for(list<MAliveNPCInfo>::iterator i = m_AliveActorList.begin(); i != m_AliveActorList.end(); i++)
	{
		MAliveNPCInfo *pCurr = &(*i);
		
		if(pCurr->uidOwner == uidOwner)
		{
			if(pCurr->uidNPC == uidNPC)
				return true;
		}
	}
	
	return false;
}

bool MMatchGame_ChallengeQuest::ActorDead(const MUID &uidKiller, const MUID &uidNPC)
{
	for(list<MAliveNPCInfo>::iterator i = m_AliveActorList.begin(); i != m_AliveActorList.end(); i++)
	{
		MAliveNPCInfo *pCurr = &(*i);
		
		if(pCurr->uidNPC == uidNPC)
		{
			MCmdWriter Cmd;
			Cmd.WriteMUID(uidKiller);
			Cmd.WriteMUID(uidNPC);
			Cmd.Finalize(MC_CHALLENGE_QUEST_NPC_DEAD, MCFT_END);
			SendToBattle(&Cmd, m_pStage->GetUID());
			
			m_AliveActorList.erase(i);
			return true;
		}
	}
	
	return false;
}

bool MMatchGame_ChallengeQuest::CheckGameOver()
{
	for(map<MUID, MMatchObject *>::iterator i = m_pStage->ObjBegin(); i != m_pStage->ObjEnd(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		if(pCurr->CheckGameFlag(MMOGF_ENTERED) == true && pCurr->m_GameInfo.bAlive == true) return false;
	}
	
	return true;
}

void MMatchGame_ChallengeQuest::SectorFinish()
{
	if(m_bRoundCleared == true) return;
	
	for(map<MUID, MMatchObject *>::iterator i = m_pStage->ObjBegin(); i != m_pStage->ObjEnd(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		if(pCurr->CheckGameFlag(MMOGF_ENTERED) == false) continue;
		
		if(pCurr->IsHide() == true) continue;
		
		int nNewTotalExp = CheckPlusOver(pCurr->m_Exp.nXP, pCurr->m_GameInfo.nExp + (int)m_fCurrXP);
		int nNewLevel = g_FormulaMgr.GetLevelFromExp(nNewTotalExp);
		int nExpPercent = g_FormulaMgr.GetExpPercent(nNewLevel, nNewTotalExp);
		
		MCmdWriter Cmd;
		Cmd.WriteUInt((unsigned int)nExpPercent);
		Cmd.WriteULong((unsigned long)m_fCurrXP);
		Cmd.WriteULong((unsigned long)m_fCurrBP);
		Cmd.Finalize(MC_CHALLENGE_QUEST_SECTOR_SUCCESS, MCFT_END);
		SendToClient(&Cmd, pCurr->GetUID());
		
		if(nNewLevel > pCurr->m_Exp.nLevel)
		{
			MCmdWriter Cmd2;
			Cmd2.WriteMUID(pCurr->GetUID());
			Cmd2.WriteInt(nNewLevel);
			Cmd2.Finalize(MC_MATCH_GAME_LEVEL_UP, MCFT_END);
			SendToBattle(&Cmd2, m_pStage->GetUID());
		}
		
		pCurr->m_Exp.nLevel = nNewLevel;
		pCurr->m_GameInfo.nExp += (int)m_fCurrXP;
		pCurr->m_Exp.nBP += (int)m_fCurrBP;
		
		pCurr->m_Account.nCash += g_nCashBonus[CASH_BONUS_CHALLENGE_QUEST];
	}
	
	if(m_nRound + 1 >= (int)m_pScenario->vtSectors.size())
	{
		QuestCompleted();
	}
	else
	{
		m_nNextRoundStartTime = GetTime() + 8000;
		m_bRoundCleared = true;
	}
}

void MMatchGame_ChallengeQuest::SectorStart()
{
	if(m_bRoundCleared == false) return;
	
	m_nRound++;
	
	SetRoundState((int)MMRS_PREPARE);
	
	RevivalAll();
	
	m_AliveActorList.clear();
	
	MCmdWriter Cmd;
	Cmd.Finalize(MC_CHALLENGE_QUEST_ADVANCE_SECTOR, MCFT_END);
	SendToBattle(&Cmd, m_pStage->GetUID());
	
	m_fCurrXP *= 1.5f;
	m_fCurrBP *= 1.5f;
	
	SetRoundState((int)MMRS_COUNTDOWN);
	
	m_bRoundCleared = false;
}

void MMatchGame_ChallengeQuest::QuestCompleted()
{
	m_nQuestEndedTime = GetTime();
	
	int nElapsedTime = (int)((m_nQuestEndedTime - m_nQuestStartedTime) / 1000);
	
	MCmdWriter Cmd;
	
	Cmd.WriteUInt((unsigned int)nElapsedTime);
	Cmd.Finalize(MC_CHALLENGE_QUEST_RESULT_TOTAL_TIME);
	
	Cmd.WriteInt(m_pScenario->nRewardItemID);	// gamble item && id >= 3000001 only.
	Cmd.Finalize(MC_CHALLENGE_QUEST_COMPLETED, MCFT_END);
	
	SendToBattle(&Cmd, m_pStage->GetUID());
	
	DistributeReward();
	SaveRecord();
	
	Finish();
}

void MMatchGame_ChallengeQuest::QuestFailed()
{
	m_nQuestEndedTime = GetTime();
	
	int nElapsedTime = (int)((m_nQuestEndedTime - m_nQuestStartedTime) / 1000);
	
	MCmdWriter Cmd;
	
	Cmd.WriteUInt((unsigned int)nElapsedTime);
	Cmd.Finalize(MC_CHALLENGE_QUEST_RESULT_TOTAL_TIME);
	
	Cmd.Finalize(MC_CHALLENGE_QUEST_FAILED, MCFT_END);
	
	SendToBattle(&Cmd, m_pStage->GetUID());
	
	Finish();
}

void MMatchGame_ChallengeQuest::DistributeReward()
{
	if(m_pScenario->nRewardItemID == 0) return;
	
	// distrib reward gamble item.
	for(map<MUID, MMatchObject *>::iterator i = m_pStage->ObjBegin(); i != m_pStage->ObjEnd(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		
		MMatchCharGambleItem *pGItem = pCurr->GetCharGambleItemByItemID(m_pScenario->nRewardItemID);
		if(pGItem == NULL)
		{
			/*
			int nGIID;
			if(Db_InsertGambleItem(pCurr->m_Char.nCID, m_pScenario->nRewardItemID, 1, pCurr->m_Exp.nBP, &nGIID) == false) continue;
			
			pCurr->AddCharGambleItem(MUID::Assign(), nGIID, m_pScenario->nRewardItemID, 1);
			*/
			AsyncDb_InsertGambleItem(pCurr->GetUID(), pCurr->m_Char.nCID, m_pScenario->nRewardItemID, 1, pCurr->m_Exp.nBP);
		}
		else
		{
			// if(Db_UpdateGambleItem(pCurr->m_Char.nCID, pGItem->nGIID, pGItem->nCount + 1, pCurr->m_Exp.nBP) == false) continue;
			AsyncDb_UpdateGambleItem(pCurr->GetUID(), pCurr->m_Char.nCID, pGItem->nGIID, pGItem->nCount + 1, pCurr->m_Exp.nBP);
			pGItem->nCount += 1;
		}
	}
}

void MMatchGame_ChallengeQuest::SaveRecord()
{
	#ifdef _RELEASE
	if(m_pStage->GetPlayer() < m_pScenario->nPlayer) return;
	#endif
	
	// save finish time record info.
	int nTime = (int)((m_nQuestEndedTime - m_nQuestStartedTime) / 1000);
	
	for(map<MUID, MMatchObject *>::iterator i = m_pStage->ObjBegin(); i != m_pStage->ObjEnd(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		
		// if(Db_InsertCQRecord(pCurr->m_Account.nAID, m_pScenario->nMapID, nTime) == false) continue;
		AsyncDb_InsertCQRecord(pCurr->GetUID(), pCurr->m_Account.nAID, m_pScenario->nMapID, nTime);
		pCurr->AddCQRecord(m_pScenario->nMapID, nTime);
		
		OnChallengeQuestBestRecord(pCurr->GetUID());
	}
}

void MMatchGame_ChallengeQuest::RevivalAll()
{
	for(map<MUID, MMatchObject *>::iterator i = m_pStage->ObjBegin(); i != m_pStage->ObjEnd(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		if(pCurr->CheckGameFlag(MMOGF_ENTERED) == false) continue;
		
		if(pCurr->IsHide() == false)
		{
			pCurr->m_GameInfo.bAlive = true;
		}
	}
}

// blitzkrieg.
MMatchGame_BlitzKrieg::MMatchGame_BlitzKrieg()
{
	m_bPreparingGame = false;
	m_nGameStartedTime = 0;
	m_nLastRoundStateArg = 0;
	m_nBlitzState = (int)MMBRS_PREPARE;
	m_nNextBonusHonorIncreaseTime = 0;
	m_uidRedRadar = m_uidBlueRadar = MUID(0, 0);
	m_nRedBarricadeCount = m_nBlueBarricadeCount = 0;
	m_nWinnerTeam = 0;
}

MMatchGame_BlitzKrieg::~MMatchGame_BlitzKrieg()
{
	m_vtPlayerStatus.clear();
	m_AliveActorList.clear();
	m_vtTreasure.clear();
}

void MMatchGame_BlitzKrieg::OnCreate()
{
	// TODO : init blitz info on here ..................................................
	
	// init player status.
	for(map<MUID, MMatchObject *>::iterator i = m_pStage->ObjBegin(); i != m_pStage->ObjEnd(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		
		MBlitzPlayerStatus status(pCurr->GetUID(), pCurr->m_GameInfo.nTeam);
		m_vtPlayerStatus.push_back(status);
	}
	
	// hard-coded treasure chests.
	#define ADD_TREASURE(actor, ind)										\
	{																		\
		MBlitzTreasureChest info = {actor, MUID(0, 0), false, 0, ind};		\
		m_vtTreasure.push_back(info);										\
	}
	
	ADD_TREASURE("tresure1", 0);
	ADD_TREASURE("tresure2", 1);
	ADD_TREASURE("tresure3", 2);
	ADD_TREASURE("tresure4", 3);
}

void MMatchGame_BlitzKrieg::Update(unsigned long nTime)
{
	if(m_bFinished == true) return;
	
	MMatchStage *pStage = m_pStage;	// this stage.
	
	if(pStage->GetState() != (int)MMSS_RUN) return;
	
	// if(pStage->GetState() == (int)MMSS_RUN)
	{
		if(m_bGameStarted == false)
		{
			if(pStage->CheckGameStartable() == true)
			{
				SetRoundState((int)MMRS_COUNTDOWN);
				m_bGameStarted = true;
			}
			else
			{
				return;
			}
		}
		else if(m_bPreparingGame == false)
		{
			if(m_nStartTime <= nTime)
			{
				SetRoundState((int)MMRS_PLAY);
				SpawnPreparedActor();
				
				#ifdef _RELEASE
				m_nStartTime = nTime + 30000;
				#endif
				m_bPreparingGame = true;
			}
		}
		else
		{
			if(m_nBlitzState == (int)MMBRS_PREPARE)
			{
				if(m_nStartTime <= nTime)
				{
					SetBlitzState((int)MMBRS_PLAY);
					m_nGameStartedTime = nTime;
				}
			}
		}
	}
	
	if(m_nBlitzState == (int)MMBRS_PLAY)
	{
		// gain bonus honor.
		if(m_nNextBonusHonorIncreaseTime <= nTime)
		{
			for(vector<MBlitzPlayerStatus>::iterator i = m_vtPlayerStatus.begin(); i != m_vtPlayerStatus.end(); i++)
			{
				(*i).AddHonorPoint(2);
			}
			
			// again after a sec.
			m_nNextBonusHonorIncreaseTime += 1000;	// prevent making time diffs : it's m_nNextBonusHonorIncreaseTime + 1000, not nTime + 1000.
		}
		
		bool bRedWin, bBlueWin;
		CheckTeamStatus(&bRedWin, &bBlueWin);
		
		if(bRedWin == true && bBlueWin == true)
		{
			// draw game.
			Finish(0);
			return;
		}
		else if(bRedWin == true)
		{
			// red win.
			Finish(MMT_RED);
			return;
		}
		else if(bBlueWin == true)
		{
			// blue win.
			Finish(MMT_BLUE);
			return;
		}
		
		UpdateTreasure(nTime);
	}
	
	MMatchBaseGame::Update(nTime);
}

void MMatchGame_BlitzKrieg::SetRoundState(int nState)
{
	if(m_nRoundState == nState) return;
	
	switch(nState)
	{
		case (int)MMRS_COUNTDOWN	:
		{
			m_nStartTime = GetTime() + 2000;
			EraseAllWorldItem();
		}
		break;
		
		case (int)MMRS_PLAY	:
		{
			SendAvailableClassInfo();
		}
		break;
	}
	
	SendRoundStateToBattle(m_pStage->GetUID(), 0, nState, m_nLastRoundStateArg);
	
	m_nRoundState = nState;
}

void MMatchGame_BlitzKrieg::Finish()
{
	if(m_bFinished == true) return;
	
	SetRoundState((int)MMRS_FINISH);
	
	// total elapsed time.
	unsigned long nTotalTime = GetElapsedTime();
	
	// xp & bp to gain.
	unsigned long nBaseXP, nBaseBP;
	g_FormulaMgr.GetKillExp(1, &nBaseXP, &nBaseBP);
	
	int nRewardXP = (int)((nTotalTime / 2400) * nBaseXP);
	int nRewardBP = (int)(nRewardXP / 50);
	
	#ifndef _BOUNTY
	nRewardBP = 0;
	#endif
	
	// find a MVP person.
	MUID uidWinKingmaker = MUID(0, 0), uidLoseKingmaker = MUID(0, 0);
	
	int nWinnerHighestPoint = 0, nLoserHighestPoint = 0;
	for(map<MUID, MMatchObject *>::iterator i = m_pStage->ObjBegin(); i != m_pStage->ObjEnd(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		if(pCurr->CheckGameFlag(MMOGF_ENTERED) == false) continue;
		
		if(pCurr->IsHide() == true) continue;
		
		MBlitzPlayerStatus *pPlayerStatus = GetPlayerStatus(pCurr->GetUID());
		if(pPlayerStatus == NULL) continue;
		
		if(pPlayerStatus->m_nTeamID == m_nWinnerTeam)
		{
			// winner.
			
			if((int)pPlayerStatus->m_fHonorPoint > nWinnerHighestPoint)
			{
				uidWinKingmaker = pCurr->GetUID();
				nWinnerHighestPoint = (int)pPlayerStatus->m_fHonorPoint;
			}
		}
		else
		{
			// loser.
			
			if((int)pPlayerStatus->m_fHonorPoint > nLoserHighestPoint)
			{
				uidLoseKingmaker = pCurr->GetUID();
				nLoserHighestPoint = (int)pPlayerStatus->m_fHonorPoint;
			}
		}
	}
	
	// send finish info.
	MCmdWriter Cmd;
	Cmd.WriteUInt((unsigned int)m_nWinnerTeam);	// victory team id.
	Cmd.WriteULong(nTotalTime);	// total battle time.
	Cmd.StartBlob(sizeof(MTD_BlitzPlayerResultInfoNode));
	for(map<MUID, MMatchObject *>::iterator i = m_pStage->ObjBegin(); i != m_pStage->ObjEnd(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		if(pCurr->CheckGameFlag(MMOGF_ENTERED) == false) continue;
		
		if(pCurr->IsHide() == true) continue;
		
		MBlitzPlayerStatus *pPlayerStatus = GetPlayerStatus(pCurr->GetUID());
		if(pPlayerStatus == NULL) continue;
		
		int nTempXP = nRewardXP, nTempBP = nRewardBP;
		
		int nRewardPoint = 5 + (pCurr->m_GameInfo.nKill / 2);
		int nRewardMedal = (int)pPlayerStatus->m_fHonorPoint / 25;
		
		if(pCurr->m_GameInfo.nTeam == m_nWinnerTeam)
		{
			pCurr->m_BlitzKrieg.nWin++;
		}
		else
		{
			pCurr->m_BlitzKrieg.nLose++;
			
			nTempXP /= 5;
			nTempBP /= 5;
			nRewardPoint = -3;
			nRewardMedal /= 4;
		}
		
		// MVP.
		if(pCurr->GetUID() == uidWinKingmaker)
		{
			float fKingmakerXP = (float)nTempXP * 1.15f;
			float fKingmakerBP = (float)nTempBP * 1.15f;
			float fKingmakerMedal = (float)nRewardMedal * 1.15f;
			
			nTempXP = (int)fKingmakerXP;
			nTempBP = (int)fKingmakerBP;
			nRewardMedal = (int)fKingmakerMedal;
		}
		else if(pCurr->GetUID() == uidLoseKingmaker)
		{
			float fKingmakerXP = (float)nTempXP * 1.45f;
			float fKingmakerBP = (float)nTempBP * 1.45f;
			float fKingmakerMedal = (float)nRewardMedal * 1.45f;
			
			nTempXP = (int)fKingmakerXP;
			nTempBP = (int)fKingmakerBP;
			nRewardMedal = (int)fKingmakerMedal;
		}
		
		int nNewTotalExp = CheckPlusOver(pCurr->m_Exp.nXP, pCurr->m_GameInfo.nExp + nTempXP);
		int nNewLevel = g_FormulaMgr.GetLevelFromExp(nNewTotalExp);
		int nExpPercent = g_FormulaMgr.GetExpPercent(nNewLevel, nNewTotalExp);
		
		if(nNewLevel > pCurr->m_Exp.nLevel)
		{
			MCmdWriter CmdLevelUp;
			CmdLevelUp.WriteMUID(pCurr->GetUID());
			CmdLevelUp.WriteInt(nNewLevel);
			CmdLevelUp.Finalize(MC_MATCH_GAME_LEVEL_UP, MCFT_END);
			SendToBattle(&CmdLevelUp, m_pStage->GetUID());
		}
		
		pCurr->m_Exp.nLevel = nNewLevel;
		pCurr->m_GameInfo.nExp += nTempXP;
		pCurr->m_Exp.nBP += nTempBP;
		
		pCurr->m_Account.nCash += g_nCashBonus[CASH_BONUS_BLITZKRIEG];
		
		pCurr->m_BlitzKrieg.nPoint = CheckPlusOver(pCurr->m_BlitzKrieg.nPoint, nRewardPoint);
		pCurr->m_BlitzKrieg.nMedal += nRewardMedal;
		
		MTD_BlitzPlayerResultInfoNode node;
		node.uidPlayer = pCurr->GetUID();
		node.nLevelPercent = nExpPercent;
		node.nExp = nTempXP;
		node.nBounty = nTempBP;
		node.nMedal = nRewardMedal;
		node.nHonorPoint = (int)pPlayerStatus->m_fHonorPoint;
		node.nBlitzPoint = pCurr->m_BlitzKrieg.nPoint;
		
		Cmd.WriteData(&node, sizeof(MTD_BlitzPlayerResultInfoNode));
	}
	Cmd.EndBlob();
	Cmd.Finalize(MC_BLITZ_MATCH_FINISH, MCFT_END);
	SendToStage(&Cmd, m_pStage->GetUID());
	
	m_pStage->SetState((int)MMSS_STANDBY);
	m_bFinished = true;
}

void MMatchGame_BlitzKrieg::Finish(int nWinnerTeam)
{
	// send win/lose.
	switch(nWinnerTeam)
	{
		case MMT_RED	: m_nLastRoundStateArg = (int)MMRSA_RED_WON;  break;
		case MMT_BLUE	: m_nLastRoundStateArg = (int)MMRSA_BLUE_WON; break;
		default			: m_nLastRoundStateArg = (int)MMRSA_DRAW; break;
	}
	
	m_nWinnerTeam = nWinnerTeam;
	
	Finish();
}

void MMatchGame_BlitzKrieg::CheckTeamStatus(bool *pOutRedWin, bool *pOutBlueWin)
{
	*pOutRedWin = *pOutBlueWin = false;
	
	int nRedCount = 0, nBlueCount = 0;
	
	for(map<MUID, MMatchObject *>::iterator i = m_pStage->ObjBegin(); i != m_pStage->ObjEnd(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		if(pCurr->CheckGameFlag(MMOGF_ENTERED) == false) continue;
		
		if(pCurr->IsHide() == true) continue;
		
		if(pCurr->m_GameInfo.nTeam == MMT_RED)
		{
			nRedCount++;
		}
		else if(pCurr->m_GameInfo.nTeam == MMT_BLUE)
		{
			nBlueCount++;
		}
	}
	
	if(nRedCount == 0 || m_uidRedRadar == MUID(0, 0)) *pOutBlueWin = true;
	if(nBlueCount == 0 || m_uidBlueRadar == MUID(0, 0)) *pOutRedWin = true;
}

void MMatchGame_BlitzKrieg::SendAvailableClassInfo()
{
	for(map<MUID, MMatchObject *>::iterator i = m_pStage->ObjBegin(); i != m_pStage->ObjEnd(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		if(pCurr->CheckGameFlag(MMOGF_ENTERED) == false) continue;
		
		MTD_BlitzAvailableClassInfo info;
		ZeroInit(&info, sizeof(MTD_BlitzAvailableClassInfo));
		
		for(int i = 0; i < MBLITZ_CLASS_MAX; i++)
		{
			if(g_nBlitzManualItemID[i] == 0 || pCurr->GetCharItemByItemID(g_nBlitzManualItemID[i]) != NULL)
			{
				info.bActive[i] = true;
			}
		}
		
		MCmdWriter Cmd;
		Cmd.StartBlob(sizeof(MTD_BlitzAvailableClassInfo));
		Cmd.WriteData(&info, sizeof(MTD_BlitzAvailableClassInfo));
		Cmd.EndBlob();
		Cmd.Finalize(MC_BLITZ_AVAILABLE_CLASS, MCFT_END);
		SendToClient(&Cmd, pCurr->GetUID());
	}
}

void MMatchGame_BlitzKrieg::UpdateTreasure(unsigned long nTime)
{
	// treasure chest.
	
	for(vector<MBlitzTreasureChest>::iterator i = m_vtTreasure.begin(); i != m_vtTreasure.end(); i++)
	{
		MBlitzTreasureChest *pCurr = &(*i);
		
		if(pCurr->bSpawned == false && pCurr->nNextSpawnTime <= nTime)
		{
			if(SpawnActor(pCurr->szActorName, MMT_ALL, pCurr->nIndex, BLITZACTOR_FLAG_TREASURE, &pCurr->uidActor) == true)
			{
				pCurr->bSpawned = true;
			}
			else
			{
				pCurr->uidActor = MUID(0, 0);
			}
		}
	}
}

void MMatchGame_BlitzKrieg::BrokeTreasure(const MUID &uidActor)
{
	#define BLITZ_TREASURE_SPAWN_TIME	120000	// 2 min.
	
	for(vector<MBlitzTreasureChest>::iterator i = m_vtTreasure.begin(); i != m_vtTreasure.end(); i++)
	{
		MBlitzTreasureChest *pCurr = &(*i);
		
		if(pCurr->uidActor == uidActor)
		{
			if(pCurr->bSpawned == false) return;
			
			pCurr->nNextSpawnTime = GetTime() + BLITZ_TREASURE_SPAWN_TIME;
			pCurr->bSpawned = false;
			
			return;
		}
	}
}

void MMatchGame_BlitzKrieg::SetBlitzState(int nState)
{
	if(m_nBlitzState == nState) return;
	
	switch(nState)
	{
		case (int)MMBRS_PLAY	:
		{
			// start increasing honor after a sec.
			m_nNextBonusHonorIncreaseTime = GetTime() + 1000;
		}
		break;
	}
	
	MCmdWriter Cmd;
	Cmd.WriteInt(nState);
	Cmd.Finalize(MC_BLITZ_ROUND_STATE, MCFT_END);
	SendToBattle(&Cmd, m_pStage->GetUID());
	
	m_nBlitzState = nState;
}

MBlitzPlayerStatus *MMatchGame_BlitzKrieg::GetPlayerStatus(const MUID &uidPlayer)
{
	for(vector<MBlitzPlayerStatus>::iterator i = m_vtPlayerStatus.begin(); i != m_vtPlayerStatus.end(); i++)
	{
		MBlitzPlayerStatus *pCurr = &(*i);
		if(pCurr->m_uidPlayer == uidPlayer) return pCurr;
	}
	
	return NULL;
}

MUID MMatchGame_BlitzKrieg::RandomActorOwner()
{
	vector<MUID> vtCandUID;
	
	for(map<MUID, MMatchObject *>::iterator i = m_pStage->ObjBegin(); i != m_pStage->ObjEnd(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		if(pCurr->CheckGameFlag(MMOGF_ENTERED) == false) continue;
		
		if(pCurr->IsHide() == false)
		{
			vtCandUID.push_back(pCurr->GetUID());
		}
	}
	
	if(vtCandUID.size() == 0) return MUID(0, 0);
	
	int nRand = (int)(RandNum() % (unsigned int)vtCandUID.size());
	return vtCandUID[nRand];
}

void MMatchGame_BlitzKrieg::ReSelectActorOwner()
{
	for(list<MBlitzAliveActorInfo>::iterator i = m_AliveActorList.begin(); i != m_AliveActorList.end(); i++)
	{
		MBlitzAliveActorInfo *pCurr = &(*i);
		
		MUID uidNewOwner = RandomActorOwner();
		pCurr->uidOwner = uidNewOwner;
		
		MCmdWriter Cmd;
		Cmd.WriteMUID(uidNewOwner);
		Cmd.WriteMUID(pCurr->uidActor);
		Cmd.Finalize(MC_CHALLENGE_QUEST_ENTRUST_NPC_CONTROL, MCFT_END);
		SendToBattle(&Cmd, m_pStage->GetUID());
	}
}

bool MMatchGame_BlitzKrieg::SpawnActor(const char *pszActorName, int nTeamID, int nPosIndex, unsigned long nFlags, MUID *pOutUID)
{
	MMatchChallengeQuestNPCInfo *pActorInfo = g_ChallengeQuest.GetNPCInfo(pszActorName);
	if(pActorInfo == NULL) return false;
	
	MUID uidOwner = RandomActorOwner();
	MUID uidActor = MUID::Assign();
	
	MBlitzAliveActorInfo info = {nTeamID, uidOwner, uidActor, GetActorPoint(pszActorName), nFlags};
	m_AliveActorList.push_back(info);
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidOwner);
	Cmd.WriteMUID(uidActor);
	Cmd.WriteString(pszActorName);
	Cmd.WriteUChar((unsigned char)nTeamID);
	Cmd.WriteUChar((unsigned char)nPosIndex);
	Cmd.WriteUShort(pActorInfo->nHP);
	Cmd.WriteUShort(pActorInfo->nAP);
	Cmd.WriteFloat(1.0f);
	Cmd.Finalize(MC_BLITZ_NPC_SPAWN, MCFT_END);
	SendToBattle(&Cmd, m_pStage->GetUID());
	
	if(pOutUID != NULL) *pOutUID = uidActor;
	
	return true;
}

bool MMatchGame_BlitzKrieg::SpawnActorByPlayer(const MUID &uidOwnerActor, const char *pszActorName, int nTeamID, const FloatVector *pPos, const FloatVector *pDir)
{
	// #define MAX_BLITZ_PLAYER_ACTOR_SPAWN	80
	// if(m_AliveActorList.size() >= MAX_BLITZ_PLAYER_ACTOR_SPAWN) return true;
		
	MMatchChallengeQuestNPCInfo *pActorInfo = g_ChallengeQuest.GetNPCInfo(pszActorName);
	if(pActorInfo == NULL) return false;
	
	MUID uidOwner = RandomActorOwner();
	MUID uidActor = MUID::Assign();
	
	MBlitzAliveActorInfo info = {nTeamID, uidOwner, uidActor, GetActorPoint(pszActorName), BLITZACTOR_FLAG_NONE};
	m_AliveActorList.push_back(info);
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidOwnerActor);
	Cmd.WriteMUID(uidOwner);
	Cmd.WriteMUID(uidActor);
	Cmd.WriteString(pszActorName);
	Cmd.WriteVec(pPos->x, pPos->y, pPos->z);
	Cmd.WriteVec(pDir->x, pDir->y, pDir->z);
	Cmd.WriteUShort(pActorInfo->nHP);
	Cmd.WriteUShort(pActorInfo->nAP);
	Cmd.WriteFloat(1.0f);
	Cmd.WriteUInt((unsigned int)GetSuitableRouteID(nTeamID));
	Cmd.Finalize(MC_BLITZ_PLAYER_NPC_SPAWN, MCFT_END);
	SendToBattle(&Cmd, m_pStage->GetUID());
	
	return true;
}

void MMatchGame_BlitzKrieg::SpawnPreparedActor()
{
	// TODO : hard-coded spawn infos. if you want to customize NPCs, this should be fixed.
	
	#define BLITZ_SPAWN_NPC_UID(actor, team, count, flag, outuid)	\
	{																\
		for(int i = 0; i < count; i++)								\
		{															\
			if(SpawnActor(actor, team, i, flag, outuid) == false)	\
			{														\
				if(outuid != NULL) *outuid = MUID(0, 0);			\
			}														\
		}															\
	}
	
	#define BLITZ_SPAWN_NPC(actor, team, count, flag)				\
	{																\
		for(int i = 0; i < count; i++)								\
			SpawnActor(actor, team, i, flag);						\
	}
	
	BLITZ_SPAWN_NPC_UID("radar_red", MMT_RED, 1, BLITZACTOR_FLAG_RADAR, &m_uidRedRadar);
	BLITZ_SPAWN_NPC_UID("radar_blue", MMT_BLUE, 1, BLITZACTOR_FLAG_RADAR, &m_uidBlueRadar);
	BLITZ_SPAWN_NPC("barricade_red", MMT_RED, 12, BLITZACTOR_FLAG_BARRICADE);
	BLITZ_SPAWN_NPC("barricade_blue", MMT_BLUE, 12, BLITZACTOR_FLAG_BARRICADE);
	BLITZ_SPAWN_NPC("guardian", MMT_RED, 1, BLITZACTOR_FLAG_NONE);
	BLITZ_SPAWN_NPC("guardian", MMT_BLUE, 1, BLITZACTOR_FLAG_NONE);
	
	m_nRedBarricadeCount = 12;
	m_nBlueBarricadeCount = 12;
}

bool MMatchGame_BlitzKrieg::IsValidActorOwner(const MUID &uidOwner, const MUID &uidNPC)
{
	for(list<MBlitzAliveActorInfo>::iterator i = m_AliveActorList.begin(); i != m_AliveActorList.end(); i++)
	{
		MBlitzAliveActorInfo *pCurr = &(*i);
		
		if(pCurr->uidOwner == uidOwner)
		{
			if(pCurr->uidActor == uidNPC)
				return true;
		}
	}
	
	return false;
}

bool MMatchGame_BlitzKrieg::ActorDead(const MUID &uidKiller, const MUID &uidNPC)
{
	for(list<MBlitzAliveActorInfo>::iterator i = m_AliveActorList.begin(); i != m_AliveActorList.end(); i++)
	{
		MBlitzAliveActorInfo *pCurr = &(*i);
		
		if(pCurr->uidActor == uidNPC)
		{
			// gain honor point.
			int nKillerPoint = pCurr->nPoint;
			int nTeamPoint = nKillerPoint / 2;
			
			// send kill info.
			MCmdWriter Cmd;
			Cmd.WriteMUID(uidKiller);
			Cmd.WriteMUID(uidNPC);
			Cmd.WriteUInt((unsigned int)nKillerPoint);	// honor to killer.
			Cmd.WriteUInt((unsigned int)nTeamPoint);	// honor to all team member.
			Cmd.Finalize(MC_BLITZ_NPC_DEAD, MCFT_END);
			SendToBattle(&Cmd, m_pStage->GetUID());
			
			// present honor to killer.
			MBlitzPlayerStatus *pKillerStatus = GetPlayerStatus(uidKiller);
			if(pKillerStatus != NULL)
			{
				pKillerStatus->AddHonorPoint(nKillerPoint);
			}

			// and to team members.
			int nKillerTeamID = MMT_ALL;
				
			if(pCurr->nTeamID == MMT_RED)
			{
				nKillerTeamID = MMT_BLUE;
			}
			else if(pCurr->nTeamID == MMT_BLUE)
			{
				nKillerTeamID = MMT_RED;
			}
			else
			{
				// unknown team ...?
			}
			
			for(vector<MBlitzPlayerStatus>::iterator j = m_vtPlayerStatus.begin(); j != m_vtPlayerStatus.end(); j++)
			{
				MBlitzPlayerStatus *pMemberStatus = &(*j);
				
				if(pMemberStatus->m_nTeamID == nKillerTeamID)
				{
					pMemberStatus->AddHonorPoint(nTeamPoint);
				}
			}
			
			// extra kill process depending on actor flag.
			if((pCurr->nFlags & BLITZACTOR_FLAG_RADAR) != 0)
			{
				if(pCurr->nTeamID == MMT_RED)
				{
					m_uidRedRadar = MUID(0, 0);
				}
				else if(pCurr->nTeamID == MMT_BLUE)
				{
					m_uidBlueRadar = MUID(0, 0);
				}
			}
			else if((pCurr->nFlags & BLITZACTOR_FLAG_BARRICADE) != 0)
			{
				BrokeBarricade(pCurr->nTeamID);
			}
			else if((pCurr->nFlags & BLITZACTOR_FLAG_TREASURE) != 0)
			{
				BrokeTreasure(uidNPC);
			}
			
			m_AliveActorList.erase(i);
			return true;
		}
	}
	
	return false;
}

int MMatchGame_BlitzKrieg::GetRandomRouteID()
{
	// hard-coded actor route list.
	
	int nRet = 0;
	
	switch(RandNum() % 8)
	{
		case 0  : nRet = 100; break;
		case 1  : nRet = 101; break;
		case 2  : nRet = 210; break;
		case 3  : nRet = 211; break;
		case 4  : nRet = 220; break;
		case 5  : nRet = 221; break;
		case 6  : nRet = 300; break;
		case 7  : nRet = 301; break;
	}
	
	return nRet;
}

int MMatchGame_BlitzKrieg::GetSuitableRouteID(int nTeamID)
{
	// hard-coded. 
	
	int nRet = GetRandomRouteID();	
	
	if(nTeamID == MMT_RED)
	{
		switch(RandNum() % 4)
		{
			case 0  : nRet = 100; break;
			case 1  : nRet = 210; break;
			case 2  : nRet = 220; break;
			case 3  : nRet = 300; break;
		}
	}
	else if(nTeamID == MMT_BLUE)
	{
		switch(RandNum() % 4)
		{
			case 0  : nRet = 101; break;
			case 1  : nRet = 211; break;
			case 2  : nRet = 221; break;
			case 3  : nRet = 301; break;
		}
	}
	
	return nRet;
}

int MMatchGame_BlitzKrieg::GetActorTeamID(const MUID &uidActor)
{
	for(list<MBlitzAliveActorInfo>::iterator i = m_AliveActorList.begin(); i != m_AliveActorList.end(); i++)
	{
		MBlitzAliveActorInfo *pCurr = &(*i);
		
		if(pCurr->uidActor == uidActor)
		{
			return pCurr->nTeamID;
		}
	}
	
	return MMT_ALL;
}

int MMatchGame_BlitzKrieg::GetActorPoint(const char *pszActor)
{
	// hard-coded actor points.
	
	if(MStricmp(pszActor, "barricade_red") == 0 || MStricmp(pszActor, "barricade_blue") == 0)
		return 30;
	else if(MStricmp(pszActor, "blitz_knifeman_red") == 0 || MStricmp(pszActor, "blitz_knifeman_blue") == 0)
		return 5;
	else if(MStricmp(pszActor, "blitz_throwman_red") == 0 || MStricmp(pszActor, "blitz_throwman_blue") == 0)
		return 10;
	else if(MStricmp(pszActor, "tresure1") == 0 || MStricmp(pszActor, "tresure2") == 0 || 
			MStricmp(pszActor, "tresure3") == 0 || MStricmp(pszActor, "tresure4") == 0)
		return 20;
	else if(MStricmp(pszActor, "blitz_zealot_red") == 0 || MStricmp(pszActor, "blitz_zealot_blue") == 0)
		return 40;
	else if(MStricmp(pszActor, "blitz_cleric_red") == 0 || MStricmp(pszActor, "blitz_cleric_blue") == 0)
		return 50;
	else if(MStricmp(pszActor, "blitz_knight_red") == 0 || MStricmp(pszActor, "blitz_knight_blue") == 0)
		return 60;
	else if(MStricmp(pszActor, "blitz_terminator_red") == 0 || MStricmp(pszActor, "blitz_terminator_blue") == 0)
		return 50;
		
	return 0;
}

void MMatchGame_BlitzKrieg::BrokeBarricade(int nTeamID)
{
	int *pBarricadeCount = NULL;
	
	if(nTeamID == MMT_RED) 
	{
		pBarricadeCount = &m_nRedBarricadeCount;
		nTeamID = MMT_BLUE;	// set team id to enemy one. (for reinforce actor.)
	}
	else if(nTeamID == MMT_BLUE) 
	{
		pBarricadeCount = &m_nBlueBarricadeCount;
		nTeamID = MMT_RED;
	}
	
	if(pBarricadeCount == NULL) return;
	
	(*pBarricadeCount)--;
	ReinforceActor(nTeamID, *pBarricadeCount);
}

void MMatchGame_BlitzKrieg::ReinforceActor(int nTeamID, int nBarricades)
{
	// hard-coded reinforce list.
	
	const char *pszState = NULL;
	
	switch(nBarricades)
	{
		case 0   : pszState = "summon_terminator"; break;
		case 3   : pszState = "summon_knight"; break;
		case 6   : pszState = "summon_cleric"; break;
		case 9   : pszState = "summon_zealot"; break;
	}
	
	if(pszState == NULL) return;
	
	// send info.
	MCmdWriter Cmd;
	Cmd.WriteUInt((unsigned int)nTeamID);
	Cmd.WriteString(pszState);
	Cmd.Finalize(MC_BLITZ_REINFORCE_STATE, MCFT_END);
	SendToBattle(&Cmd, m_pStage->GetUID());
}

// spy mode.
MMatchGame_Spy::MMatchGame_Spy()
{
	m_nRound = 0;
	m_nLastRoundStateArg = 0;
	m_bWaitingForNextRound = false;
	m_nNextRoundStartTime = 0;
	m_nStartPlayer = 0;
}

MMatchGame_Spy::~MMatchGame_Spy()
{
	m_vtSpys.clear();
	m_vtParticipants.clear();
}

void MMatchGame_Spy::OnCreate()
{
	MMatchBaseGame::OnCreate();
}

void MMatchGame_Spy::Update(unsigned long nTime)
{
	if(m_bFinished == true) return;
	
	MMatchStage *pStage = m_pStage;
	
	if(pStage->GetState() != (int)MMSS_RUN) return;
	
	// if(pStage->GetState() == (int)MMSS_RUN)
	{
		if(m_bGameStarted == false)
		{
			if(pStage->CheckGameStartable() == true)
			{
				if(CheckPlayersEnough() == true)
				{
					SetRoundState((int)MMRS_COUNTDOWN);
				}
				else
				{
					SetRoundState((int)MMRS_FREE);
				}
				m_bGameStarted = true;
			}
			else
			{
				return;
			}
		}
		else
		{
			if(m_nRoundState == (int)MMRS_COUNTDOWN)
			{
				if(m_nStartTime <= nTime)
				{
					SetRoundState((int)MMRS_PLAY);
					m_nEndTime = nTime + (unsigned long)pStage->GetTimeLimit();
				}
			}
		}
	}
	
	if(m_bWaitingForNextRound == true)
	{
		if(m_nNextRoundStartTime <= nTime)
		{
			if(CheckPlayersEnough() == true)
			{
				SetRoundState((int)MMRS_COUNTDOWN);
			}
			else
			{
				SetRoundState((int)MMRS_FREE);
			}
			m_bWaitingForNextRound = false;
		}
		return;
	}
	
	if(m_nRoundState == (int)MMRS_PLAY)
	{
		bool bSpyAlive, bTrackerAlive;
		CheckPlayerAlive(&bSpyAlive, &bTrackerAlive);
		
		if(bSpyAlive == false && bTrackerAlive == false)
		{
			RoundFinish(0);
		}
		else if(bSpyAlive == false)
		{
			RoundFinish(MMT_BLUE);
		}
		else if(bTrackerAlive == false)
		{
			RoundFinish(MMT_RED);
		}
		
		MMatchBaseGame::UpdateGameTimer(nTime);
	}
	else if(m_nRoundState == (int)MMRS_FREE)
	{
		if(CheckPlayersEnough() == true)
		{
			SetRoundState((int)MMRS_COUNTDOWN);
		}
	}
	
	MMatchBaseGame::Update(nTime);
}

void MMatchGame_Spy::SetRoundState(int nState)
{
	if(m_nRoundState == nState) return;
	
	switch(nState)
	{
		case (int)MMRS_COUNTDOWN	:
		{
			m_nStartTime = GetTime() + 10000;
			EraseAllWorldItem();
			
			RevivalAll();
			
			// register current players as participant.
			m_vtParticipants.clear();
			
			for(map<MUID, MMatchObject *>::iterator i = m_pStage->ObjBegin(); i != m_pStage->ObjEnd(); i++)
			{
				MMatchObject *pCurr = (*i).second;
				if(pCurr->CheckGameFlag(MMOGF_ENTERED) == false) continue;
				
				if(pCurr->m_GameInfo.bAlive == true)
				{
					m_vtParticipants.push_back(pCurr->GetUID());
				}
			}
			
			// init started players.
			m_nStartPlayer = (int)m_vtParticipants.size();
		}
		break;
		
		case (int)MMRS_PLAY	:
		{
			m_nNextAnnounceTime = 0;
			
			// select random spys.
			RandomSpy();
		}
		break;
	}
	
	if(nState == (int)MMRS_COUNTDOWN || nState == (int)MMRS_FREE)
	{
		SendRoundStateToBattle(m_pStage->GetUID(), m_nRound, (int)MMRS_PREPARE, m_nLastRoundStateArg);
	}
	
	SendRoundStateToBattle(m_pStage->GetUID(), m_nRound, nState, m_nLastRoundStateArg);
	
	m_nRoundState = nState;
}

void MMatchGame_Spy::Finish()
{
	if(m_bFinished == true) return;
	
	SetRoundState((int)MMRS_FINISH);
	
	m_pStage->AdvanceRelayMap();
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(m_pStage->GetUID());
	Cmd.WriteBool(m_pStage->GetRelayMapSetting()->IsUnFinish());
	Cmd.Finalize(MC_MATCH_STAGE_FINISH_GAME, MCFT_END);
	SendToStage(&Cmd, m_pStage->GetUID());
	
	m_pStage->AbortVoting();
	
	m_pStage->SetState((int)MMSS_STANDBY);
	m_bFinished = true;
}

void MMatchGame_Spy::TimeLimitReached()
{
	RoundFinish(MMT_RED);
}

bool MMatchGame_Spy::CheckPlayersEnough()
{
	// TODO : hard-coded.
	
	#ifdef _DEBUG
	return m_pStage->CheckInGamePlayerCount() >= 2 ? true : false ;
	#else
	return m_pStage->CheckInGamePlayerCount() >= 4 ? true : false ;
	#endif
}

bool MMatchGame_Spy::RandomSpy()
{
	// clear old data.
	m_vtSpys.clear();
	
	// make candidate.
	deque<MUID> SpyCands;
	
	for(map<MUID, MMatchObject *>::iterator i = m_pStage->ObjBegin(); i != m_pStage->ObjEnd(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		if(pCurr->CheckGameFlag(MMOGF_ENTERED) == false) continue;
		
		if(pCurr->m_GameInfo.bAlive == true)
		{
			SpyCands.push_back(pCurr->GetUID());
		}
	}
	
	if(SpyCands.size() == 0) return false;
	
	// shuffle.
	for(int i = 0; i < (int)SpyCands.size(); i++)
	{
		int nRand = (int)(RandNum() % (unsigned int)SpyCands.size());
		
		MUID uidTemp = SpyCands[i];
		SpyCands[i] = SpyCands[nRand];
		SpyCands[nRand] = uidTemp;
	}
	
	// make new spy.
	
	// TODO : this spy count is hard-coded.
	#ifdef _DEBUG
	int nSpyCount = (int)(SpyCands.size() / 2);
	#else
	int nSpyCount = (int)(SpyCands.size() / 4);
	#endif
	
	while(nSpyCount > 0 && SpyCands.size() != 0)
	{
		m_vtSpys.push_back(SpyCands.front());
		SpyCands.pop_front();
		
		nSpyCount--;
	}
	
	RefreshTeamIDs();
	
	SendGameInfo();
	
	return true;
}

void MMatchGame_Spy::RefreshTeamIDs()
{
	for(map<MUID, MMatchObject *>::iterator i = m_pStage->ObjBegin(); i != m_pStage->ObjEnd(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		if(pCurr->IsHide() == true) continue;
		
		if(IsSpy(pCurr->GetUID()) == true)
		{
			pCurr->m_GameInfo.nTeam = MMT_RED;
		}
		else
		{
			pCurr->m_GameInfo.nTeam = MMT_BLUE;
		}
	}
}

bool MMatchGame_Spy::IsSpy(const MUID &uidPlayer)
{
	for(vector<MUID>::iterator i = m_vtSpys.begin(); i != m_vtSpys.end(); i++)
	{
		if((*i) == uidPlayer) return true;
	}
	
	return false;
}

bool MMatchGame_Spy::IsParticipant(const MUID &uidPlayer)
{
	for(vector<MUID>::iterator i = m_vtParticipants.begin(); i != m_vtParticipants.end(); i++)
	{
		if((*i) == uidPlayer) return true;
	}
	
	return false;
}

void MMatchGame_Spy::JoinPlayer(const MUID &uidPlayer)
{
	MSpyPlayerStatus status(uidPlayer);
	m_vtPlayerStatus.push_back(status);
}

void MMatchGame_Spy::LeavePlayer(const MUID &uidPlayer)
{
	for(vector<MUID>::iterator i = m_vtSpys.begin(); i != m_vtSpys.end(); i++)
	{
		if((*i) == uidPlayer)
		{
			m_vtSpys.erase(i);
			break;
		}
	}
	
	for(vector<MUID>::iterator i = m_vtParticipants.begin(); i != m_vtParticipants.end(); i++)
	{
		if((*i) == uidPlayer)
		{
			m_vtParticipants.erase(i);
			break;
		}
	}
	
	for(vector<MSpyPlayerStatus>::iterator i = m_vtPlayerStatus.begin(); i != m_vtPlayerStatus.end(); i++)
	{
		if((*i).m_uidPlayer == uidPlayer)
		{
			m_vtPlayerStatus.erase(i);
			break;
		}
	}
}

void MMatchGame_Spy::CheckPlayerAlive(bool *pOutSpyAlive, bool *pOutTrackerAlive)
{
	(*pOutSpyAlive) = false;
	(*pOutTrackerAlive) = false;
	
	for(map<MUID, MMatchObject *>::iterator i = m_pStage->ObjBegin(); i != m_pStage->ObjEnd(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		if(pCurr->CheckGameFlag(MMOGF_ENTERED) == false) continue;
		
		if(pCurr->IsHide() == true) continue;
		
		if(pCurr->m_GameInfo.bAlive == true)
		{
			if(IsSpy(pCurr->GetUID()) == true)
			{
				(*pOutSpyAlive) = true;
			}
			else if(IsParticipant(pCurr->GetUID()) == true)
			{
				(*pOutTrackerAlive) = true;
			}
		}
	}
}

void MMatchGame_Spy::RoundFinish(int nWinnerTeam)
{
	switch(nWinnerTeam)
	{
		case MMT_RED	: m_nLastRoundStateArg = (int)MMRSA_RED_WON;	break;
		case MMT_BLUE	: m_nLastRoundStateArg = (int)MMRSA_BLUE_WON;	break;
		default			: m_nLastRoundStateArg = (int)MMRSA_DRAW;		break;
	}
	
	SetRoundState((int)MMRS_FINISH);
	
	TeamBonus(nWinnerTeam);
	
	m_nRound++;
	
	if(m_nRound >= m_pStage->GetRound())
	{
		Finish();
	}
	else
	{
		m_nNextRoundStartTime = GetTime() + 3000;
		m_bWaitingForNextRound = true;
	}
}

void MMatchGame_Spy::TeamBonus(int nWinnerTeam)
{
	unsigned long nTime = GetTime();
	
	unsigned long nBaseXP, nBaseBP;
	if(g_FormulaMgr.GetKillExp(1, &nBaseXP, &nBaseBP) == false) return;
	
	unsigned long nSpyXP = (nBaseXP * ((m_pStage->GetTimeLimit() - (m_nEndTime - nTime)) / 1000)), 
				  nSpyBP = (nBaseBP * ((m_pStage->GetTimeLimit() - (m_nEndTime - nTime)) / 1000));
	unsigned long nTrackerXP = (nBaseXP * ((m_nEndTime - nTime) / 1000)), 
				  nTrackerBP = (nBaseBP * ((m_nEndTime - nTime) / 1000));
				  
	// infinite time : don't gain xp/bp.
	// if(m_pStage->GetTimeLimit() == 0 || m_pStage->GetTimeLimit() == 99999)
	if(MMatchStage::IsUnLimitedTime(m_pStage->GetTimeLimit()) == true)
	{
		nSpyXP = nSpyBP = 0;
		nTrackerXP = nTrackerBP = 0;
	}
				  
	int nGainXP = 0, nGainBP = 0;
	
	if(nWinnerTeam == MMT_RED)
	{
		nGainXP = (int)nSpyXP;
		nGainBP = (int)nSpyBP;
	}
	else if(nWinnerTeam == MMT_BLUE)
	{
		nGainXP = (int)nTrackerXP;
		nGainBP = (int)nTrackerBP;
	}
				  
				  
	MCmdWriter Cmd;
	Cmd.StartBlob(sizeof(MTD_SpyRoundFinishInfo));
	
	for(map<MUID, MMatchObject *>::iterator i = m_pStage->ObjBegin(); i != m_pStage->ObjEnd(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		if(pCurr->CheckGameFlag(MMOGF_ENTERED) == false) continue;
		
		if(pCurr->IsHide() == true) continue;
		
		if(IsParticipant(pCurr->GetUID()) == false) continue;
		
		MSpyPlayerStatus *pPlayerStatus = GetPlayerStatus(pCurr->GetUID());
		if(pPlayerStatus == NULL) continue;
		
		int nTempXP = nGainXP, nTempBP = nGainBP;
		
		if(pCurr->m_GameInfo.nTeam == nWinnerTeam)
		{
			pPlayerStatus->m_nWin++;
		}
		else
		{
			nTempXP = 0;
			nTempBP = 0;
			
			pPlayerStatus->m_nLose++;
		}
		
		int nNewTotalExp = CheckPlusOver(pCurr->m_Exp.nXP, pCurr->m_GameInfo.nExp + nTempXP);
		int nNewLevel = g_FormulaMgr.GetLevelFromExp(nNewTotalExp);
		int nExpPercent = g_FormulaMgr.GetExpPercent(nNewLevel, nNewTotalExp);
		
		if(nNewLevel > pCurr->m_Exp.nLevel)
		{
			MCmdWriter Cmd2;
			Cmd2.WriteMUID(pCurr->GetUID());
			Cmd2.WriteInt(nNewLevel);
			Cmd2.Finalize(MC_MATCH_GAME_LEVEL_UP, MCFT_END);
			SendToBattle(&Cmd2, m_pStage->GetUID());
		}
		
		pCurr->m_Exp.nLevel = nNewLevel;
		pCurr->m_GameInfo.nExp += (int)nTempXP;
		pCurr->m_Exp.nBP += (int)nTempBP;
		
		pCurr->m_Account.nCash += g_nCashBonus[CASH_BONUS_SPY];
		
		MTD_SpyRoundFinishInfo info;
		info.uidPlayer = pCurr->GetUID();
		info.nXP = nTempXP;
		info.nPercent = nExpPercent;
		info.nBP = nTempBP;
		info.nPoint = pPlayerStatus->m_nAddPoint;	
		
		Cmd.WriteData(&info, sizeof(MTD_SpyRoundFinishInfo));
		
		pPlayerStatus->m_nPoint += pPlayerStatus->m_nAddPoint;
		pPlayerStatus->m_nAddPoint = 0;
	}
	
	Cmd.EndBlob();
	Cmd.Finalize(MC_SPY_GAME_RESULT, MCFT_END);
	
	SendToBattle(&Cmd, m_pStage->GetUID());
}

void MMatchGame_Spy::RevivalAll()
{
	for(map<MUID, MMatchObject *>::iterator i = m_pStage->ObjBegin(); i != m_pStage->ObjEnd(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		if(pCurr->CheckGameFlag(MMOGF_ENTERED) == false) continue;
		
		if(pCurr->IsHide() == false)
		{
			pCurr->m_GameInfo.bAlive = true;
		}
	}
}

MSpyPlayerStatus *MMatchGame_Spy::GetPlayerStatus(const MUID &uidPlayer)
{
	for(vector<MSpyPlayerStatus>::iterator i = m_vtPlayerStatus.begin(); i != m_vtPlayerStatus.end(); i++)
	{
		MSpyPlayerStatus *pCurr = &(*i);
		if(pCurr->m_uidPlayer == uidPlayer) return pCurr;
	}
	
	return NULL;
}

void MMatchGame_Spy::SendGameInfo()
{
	MMatchSpyTable *pSpyTable = g_SpySetting.GetSpyTable(m_nStartPlayer);
	if(pSpyTable == NULL) return;
	
	MCmdWriter Cmd;
	Cmd.StartBlob(sizeof(MUID));
	for(vector<MUID>::iterator i = m_vtSpys.begin(); i != m_vtSpys.end(); i++)
	{
		Cmd.WriteMUID(*i);
	}
	Cmd.EndBlob();
	Cmd.StartBlob(sizeof(MTD_SpyItemNode));
	for(vector<MMatchSpyItem>::iterator i = pSpyTable->vtSpyItem.begin(); i != pSpyTable->vtSpyItem.end(); i++)
	{
		MMatchSpyItem *pItem = &(*i);
		
		MTD_SpyItemNode node;
		node.nItemID = pItem->nItemID;
		node.nItemCount = pItem->nItemCount;
		
		Cmd.WriteData(&node, sizeof(MTD_SpyItemNode));
	}
	Cmd.EndBlob();
	Cmd.WriteUInt((unsigned int)pSpyTable->nHPAP);
	Cmd.StartBlob(sizeof(MTD_SpyItemNode));
	for(vector<MMatchSpyItem>::iterator i = g_SpySetting.TItemBegin(); i != g_SpySetting.TItemEnd(); i++)
	{
		MMatchSpyItem *pItem = &(*i);
		
		MTD_SpyItemNode node;
		node.nItemID = pItem->nItemID;
		node.nItemCount = pItem->nItemCount;
		
		Cmd.WriteData(&node, sizeof(MTD_SpyItemNode));
	}
	Cmd.EndBlob();
	Cmd.Finalize(MC_SPY_GAME_INFO, MCFT_END);
	SendToBattle(&Cmd, m_pStage->GetUID());
}

void MMatchGame_Spy::SendScoreInfo(const MUID &uidPlayer)
{
	MCmdWriter Cmd;
	Cmd.StartBlob(sizeof(MTD_SpyPlayerScoreInfo));
	for(vector<MSpyPlayerStatus>::iterator i = m_vtPlayerStatus.begin(); i != m_vtPlayerStatus.end(); i++)
	{
		MSpyPlayerStatus *pCurr = &(*i);
		
		MTD_SpyPlayerScoreInfo info;
		info.uidPlayer = pCurr->m_uidPlayer;
		info.nWin = pCurr->m_nWin;
		info.nLose = pCurr->m_nLose;
		info.nPoint = pCurr->m_nPoint;
		
		Cmd.WriteData(&info, sizeof(MTD_SpyPlayerScoreInfo));
	}
	Cmd.EndBlob();
	Cmd.Finalize(MC_SPY_GAME_SCORE, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
}
