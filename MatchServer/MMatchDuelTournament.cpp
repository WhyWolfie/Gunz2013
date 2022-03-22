#include "pch.h"
#include <time.h>

#include "MMatchDuelTournament.h"

#include "MMatchObject.h"
#include "MMatchStage.h"

#include "MMatchMap.h"
#include "MMatchGame.h"

#include "MMatchDBMgr.h"
#include "MAsyncDBProcess.h"

// from channel.
void SendChannelPlayerList(const MUID &uidChannel);

// from stage.
void SendObjectCache(int nCacheType, MMatchStage *pSrcStage, const MUID &uidDestPlayer = MUID(0, 0));
void CreateStageSettingCommand(MMatchStage *pStage, MCommandWriter *pOut);

void ReserveStageAgent(const MUID &uidStage);

// DT queue ------------------------------------------------------------.
MMatchDuelTournamentQueueManager g_DTQMgr;

MMatchDuelTournamentQueueManager::MMatchDuelTournamentQueueManager()
{
	m_nNextProcessTime = 0;
}

MMatchDuelTournamentQueueManager::~MMatchDuelTournamentQueueManager()
{
	Destroy();
}

bool MMatchDuelTournamentQueueManager::CancelChallenge(MMatchObject *pObj)
{
	for(list<MMatchObject *>::iterator i = m_ChallengerQueue.begin(); i != m_ChallengerQueue.end(); i++)
	{
		MMatchObject *pCurr = (*i);
		
		if(pCurr->GetUID() == pObj->GetUID())
		{
			m_ChallengerQueue.erase(i);
			return true;
		}
	}
	
	return false;
}

bool MMatchDuelTournamentQueueManager::ReserveChallenge(MMatchObject *pObj)
{
	CancelChallenge(pObj);
	
	m_ChallengerQueue.push_back(pObj);
	return true;
}

void MMatchDuelTournamentQueueManager::Destroy()
{
	m_ChallengerQueue.clear();
}

void MMatchDuelTournamentQueueManager::ProcessChallengers(unsigned long nTime)
{
	#ifdef _DEBUG
		#define DUELTOURNAMENT_CHALLENGER_QUEUE_PROCESS_INTERVAL	1000	// 1 sec.
	#else
		#define DUELTOURNAMENT_CHALLENGER_QUEUE_PROCESS_INTERVAL	20000	// 20 sec.
	#endif
	
	if(m_nNextProcessTime > nTime) return;
	
	deque<MUID> PlayerChannelUIDs;
	
	while(m_ChallengerQueue.size() >= DUELTOURNAMENT_PLAYER_COUNT)
	{
		// random duel map.
		MMatchMap *pMap = g_MapMgr.GetRandomDuelMap();
		if(pMap == NULL) 
		{
			mlog("Duel Tournament launch error : A randomly duel map select failed! Please set-up correct map info manually...");
			break;
		}
		
		MMatchStage *pNewStage = g_StageMgr.Create(0);
		pNewStage->Create(MUID(0, 0), 0, "DuelTournament_Stage", "pwd");
		
		// set stage game info.
		pNewStage->SetMap(pMap);
		pNewStage->SetGameType((int)MMGT_DUEL_TOURNAMENT);
		pNewStage->SetMaxPlayer(DUELTOURNAMENT_PLAYER_COUNT);
		pNewStage->SetRound(7);	// 4 match + 2 match + 1 match = 7.
		// pNewStage->SetTimeLimit(1);	// 1 min (60 sec).
		pNewStage->SetTimeLimit(60000);	// 1 min (60 sec).
		pNewStage->SetForcedEntry(false);
		
		vector<MTD_DTPlayerInfo> DTPlayerInfoList;
		
		list<MMatchObject *>::iterator it = m_ChallengerQueue.begin();
		
		for(int i = 0; i < DUELTOURNAMENT_PLAYER_COUNT; i++)
		{
			MMatchObject *pCurr = (*it);
			
			// set object info.
			pCurr->m_GameInfo.nTeam = MMT_ALL;
			
			pCurr->m_uidStage = pNewStage->GetUID();
			pNewStage->Enter(pCurr);
			
			pCurr->m_nGameFlag = MMOGF_INGAME | MMOGF_LAUNCHED;
			pCurr->m_nStageState = MOSS_WAIT;
			
			pCurr->m_nPlace = MMP_STAGE;
			
			pCurr->ReserveStageKick(60000);	// 1 min.
			
			// dt player info for prepare match command.
			MTD_DTPlayerInfo info;
			ZeroInit(&info, sizeof(MTD_DTPlayerInfo));
			
			strcpy(info.szName, pCurr->m_Char.szName);
			info.uidPlayer = pCurr->GetUID();
			info.nTP = pCurr->m_DuelTournament.nTP;
			
			DTPlayerInfoList.push_back(info);
			
			if(find(PlayerChannelUIDs.begin(), PlayerChannelUIDs.end(), pCurr->m_uidChannel) == PlayerChannelUIDs.end())
			{
				PlayerChannelUIDs.push_back(pCurr->m_uidChannel);
			}
			
			// erase from global wait queue.
			it = m_ChallengerQueue.erase(it);
		}
		
		// 1, prepare match.
		MCmdWriter CmdPrepareMatch;
		CmdPrepareMatch.WriteMUID(pNewStage->GetUID());
		CmdPrepareMatch.WriteInt(2); // quarter final.
		CmdPrepareMatch.StartBlob(sizeof(MTD_DTPlayerInfo));
		for(vector<MTD_DTPlayerInfo>::iterator i = DTPlayerInfoList.begin(); i != DTPlayerInfoList.end(); i++)
		{
			CmdPrepareMatch.WriteData(&(*i), sizeof(MTD_DTPlayerInfo));
		}
		CmdPrepareMatch.EndBlob();
		CmdPrepareMatch.Finalize(MC_MATCH_DUELTOURNAMENT_PREPARE_MATCH, MCFT_END);
		SendToStage(&CmdPrepareMatch, pNewStage->GetUID());
		
		// 2, stage setting.
		MCmdWriter CmdStageSetting;
		CreateStageSettingCommand(pNewStage, &CmdStageSetting);
		SendToStage(&CmdStageSetting, pNewStage->GetUID());
		
		// 3. reserve agent.
		ReserveStageAgent(pNewStage->GetUID());
		
		// 4, game start command.
		MCmdWriter CmdStartGame;
		if(CheckGameVersion(2013) == true)
		{
			CmdStartGame.WriteInt(0);
			CmdStartGame.Finalize(MC_MATCH_STAGE_START);
		}
		else
		{
			CmdStartGame.WriteMUID(MUID(0, 0));
			CmdStartGame.WriteMUID(pNewStage->GetUID());
			CmdStartGame.WriteInt(0);
			CmdStartGame.Finalize(MC_MATCH_STAGE_START);
		}
		CmdStartGame.Finalize();
		SendToStage(&CmdStartGame, pNewStage->GetUID());
		
		// 5, obj cache.
		SendObjectCache((int)MMOCT_UPDATE, pNewStage);
		
		// 6, launch match.
		MCmdWriter CmdLaunchMatch;
		CmdLaunchMatch.WriteMUID(pNewStage->GetUID());
		CmdLaunchMatch.WriteString(pMap->szName);
		CmdLaunchMatch.Finalize(MC_MATCH_DUELTOURNAMENT_LAUNCH_MATCH, MCFT_END);
		SendToStage(&CmdLaunchMatch, pNewStage->GetUID());
		
		// 7, run game.
		pNewStage->SetState((int)MMSS_RUN);
		
		if(pNewStage->CreateGame() == true)
		{
			MMatchGame_DuelTournament *pGame = (MMatchGame_DuelTournament *)pNewStage->GetGame();
			
			int nIndex = 0;
			for(vector<MTD_DTPlayerInfo>::iterator i = DTPlayerInfoList.begin(); i != DTPlayerInfoList.end(); i++)
			{
				pGame->SetQuarterFinalPlayerUID((*i).uidPlayer, nIndex);
				nIndex++;
			}
		}
		else
		{
			// error launching dt.
			mlog("Duel Tournament launch error : some unknown error happened when launching a game! Aborting...");
			
			pNewStage->DestroyGame();
			g_StageMgr.Remove(pNewStage);
			break;
		}
	}
	
	// update channel player list if needed.
	while(PlayerChannelUIDs.size() != 0)
	{
		SendChannelPlayerList(PlayerChannelUIDs.front());
		PlayerChannelUIDs.pop_front();
	}
	
	m_nNextProcessTime = nTime + DUELTOURNAMENT_CHALLENGER_QUEUE_PROCESS_INTERVAL;
}

// ranking ------------------------------------------------------------.
#define DUELTOURNAMENT_RANKING_UPDATER_TIME_FILE	"./dtranking.conf"

MMatchDuelTournamentRankingManager g_DTRankingMgr;

MMatchDuelTournamentRankingManager::MMatchDuelTournamentRankingManager()
{
	m_nNextDailyRankingUpdateTime = 0;
	m_nNextWeeklyRankingUpdateTime = 0;
}

MMatchDuelTournamentRankingManager::~MMatchDuelTournamentRankingManager()
{
	m_vtRankingInfo.clear();
}

bool MMatchDuelTournamentRankingManager::LoadTime()
{
	FILE *pFile = fopen(DUELTOURNAMENT_RANKING_UPDATER_TIME_FILE, "r+");
	if(pFile == NULL)
	{
		mlog("ERROR : Duel Tournament ranking time file Loading failed. : "
			 "Please create an empty file called \"%s\" and give writable/readable permissions to it.", 
			 DUELTOURNAMENT_RANKING_UPDATER_TIME_FILE);
		return false;
	}
	
	time_t nDailyTime = 0, nWeeklyTime = 0;
	if(fscanf(pFile, "%ld %ld", &nDailyTime, &nWeeklyTime) != 2)
	{
		fputs("0 0", pFile);
		
		// fclose(pFile);
		// return false;
	}
	
	m_nNextDailyRankingUpdateTime = nDailyTime;
	m_nNextWeeklyRankingUpdateTime= nWeeklyTime;
	
	fclose(pFile);
	return true;
}

bool MMatchDuelTournamentRankingManager::SaveTime()
{
	FILE *pFile = fopen(DUELTOURNAMENT_RANKING_UPDATER_TIME_FILE, "w");
	if(pFile == NULL) 
	{
		mlog("Fail to open %s file for write. The Duel Tournament ranking time couldn't be saved.", DUELTOURNAMENT_RANKING_UPDATER_TIME_FILE);
		return false;
	}
	
	if(fprintf(pFile, "%ld %ld", m_nNextDailyRankingUpdateTime, m_nNextWeeklyRankingUpdateTime) < 0)
	{
		fclose(pFile);
		return false;
	}
	
	fclose(pFile);
	return true;
}

void MMatchDuelTournamentRankingManager::Run()
{
	time_t nNowTime = time(NULL);
	
	if(m_nNextWeeklyRankingUpdateTime <= nNowTime)
	{
		UpdateWeeklyRanking(nNowTime);
	}
	else if(m_nNextDailyRankingUpdateTime <= nNowTime)
	{
		UpdateDailyRanking(nNowTime);
	}
}

void MMatchDuelTournamentRankingManager::UpdateDailyRanking(time_t nTime)
{
	// Db_UpdateDTDailyRanking();
	AsyncDb_UpdateDTDailyRanking();
	m_nNextDailyRankingUpdateTime = nTime + 86400;	// a day.
	
	SaveTime();
	
	printf("Updated Duel Tournament daily ranking.\n");
	mlog("Daily DT ranking has updated. Next daily update will be %ld.", m_nNextDailyRankingUpdateTime);
	
	FetchRanking();
}

void MMatchDuelTournamentRankingManager::UpdateWeeklyRanking(time_t nTime)
{
	ResetDTAllPlayerInfo();
	
	// Db_UpdateDTWeeklyRanking();
	AsyncDb_UpdateDTWeeklyRanking();
	
	m_nNextDailyRankingUpdateTime = nTime + 86400;	// a day.
	m_nNextWeeklyRankingUpdateTime = nTime + 604800;	// a week.
	
	SaveTime();
	
	printf("Updated Duel Tournament weekly ranking.\n");
	mlog("Weekly DT ranking has updated. Next weekly update will be %ld.", m_nNextWeeklyRankingUpdateTime);
	
	FetchRanking();
}

bool MMatchDuelTournamentRankingManager::FetchRanking()
{
	/*
	vector<DbData_DTRankingInfo> vtRankingInfo;
	if(Db_FetchDTTopRanking(&vtRankingInfo) == false) return false;
	
	m_vtRankingInfo.clear();
	
	// convert db type to mtd type.
	for(vector<DbData_DTRankingInfo>::iterator i = vtRankingInfo.begin(); i != vtRankingInfo.end(); i++)
	{
		DbData_DTRankingInfo *pCurr = &(*i);
		
		MTD_DTRankingInfo info;
		ZeroInit(&info, sizeof(MTD_DTRankingInfo));
		
		strcpy(info.szCharName, pCurr->szCharName);
		info.nTP = pCurr->nTP;
		info.nWins = pCurr->nWin;
		info.nLoses = pCurr->nLose;
		info.nRanking = pCurr->nRanking;
		info.nRankingIncrease = pCurr->nRankingDiff;
		info.nFinalWins = pCurr->nFinalWin;
		info.nGrade = pCurr->nClass;
		
		m_vtRankingInfo.push_back(info);
	}
	
	printf("Duel Tournament top ranking is taken.\n");
	mlog("Success to Refresh Match Server's DT top ranking.");
	*/
	
	AsyncDb_FetchDTTopRanking();
	
	return true;
}

void MMatchDuelTournamentRankingManager::OnFetchRanking(vector<DbData_DTRankingInfo> &vtRankingInfo)
{
	m_vtRankingInfo.clear();
	
	// convert db type to mtd type.
	for(vector<DbData_DTRankingInfo>::iterator i = vtRankingInfo.begin(); i != vtRankingInfo.end(); i++)
	{
		DbData_DTRankingInfo *pCurr = &(*i);
		
		MTD_DTRankingInfo info;
		ZeroInit(&info, sizeof(MTD_DTRankingInfo));
		
		strcpy(info.szCharName, pCurr->szCharName);
		info.nTP = pCurr->nTP;
		info.nWins = pCurr->nWin;
		info.nLoses = pCurr->nLose;
		info.nRanking = pCurr->nRanking;
		info.nRankingIncrease = pCurr->nRankingDiff;
		info.nFinalWins = pCurr->nFinalWin;
		info.nGrade = pCurr->nClass;
		
		m_vtRankingInfo.push_back(info);
	}
	
	printf("Duel Tournament top ranking is taken.\n");
	mlog("Success to Refresh Match Server's DT top ranking.");
}

void MMatchDuelTournamentRankingManager::SendRankingInfo(const MUID &uidPlayer)
{
	MCmdWriter Cmd;
	
	Cmd.StartBlob(sizeof(MTD_DTRankingInfo));
	for(vector<MTD_DTRankingInfo>::iterator i = m_vtRankingInfo.begin(); i != m_vtRankingInfo.end(); i++)
	{
		Cmd.WriteData(&(*i), sizeof(MTD_DTRankingInfo));
	}
	Cmd.EndBlob();
	
	Cmd.Finalize(MC_MATCH_DUELTOURNAMENT_RESPONSE_SIDERANKING_INFO, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
}

void MMatchDuelTournamentRankingManager::ResetDTAllPlayerInfo()
{
	for(list<MMatchObject *>::iterator i = g_ObjectMgr.Begin(); i != g_ObjectMgr.End(); i++)
	{
		MMatchObject *pCurr = (*i);
		if(pCurr->m_bCharInfoExist == false) continue;
		
		pCurr->ResetDTScore();
	}
	
	printf("DT info of all players has been reset.\n");
	mlog("All of in-game DT player info is resetted.");
}