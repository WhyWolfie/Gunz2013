#include "pch.h"
#include "MMatchStage.h"

#include "MMatchObject.h"
#include "MMatchMap.h"
#include "MMatchGame.h"

#include "MMatchServer_OnCommand.h"

MMatchStageManager g_StageMgr;

// stage manager.
MMatchStageManager::MMatchStageManager()
{
}

MMatchStageManager::~MMatchStageManager()
{
	for(list<MMatchStage *>::iterator i = m_Stages.begin(); i != m_Stages.end();)
	{
		delete (*i);
	}
	m_Stages.clear();
}

MMatchStage *MMatchStageManager::Create(int nStageNum)
{
	MMatchStage *pNew = new MMatchStage;
	
	// insert to correct position (based on stage number).
	bool bInserted = false;
	
	for(list<MMatchStage *>::iterator i = m_Stages.begin(); i != m_Stages.end(); i++)
	{
		MMatchStage *pCurr = (*i);
		
		if(pCurr->GetNumber() > nStageNum)
		{
			m_Stages.insert(i, pNew);
			bInserted = true;
			break;
		}
	}
	
	// not inserted. = there is no existing greater stage number than it. add to back (most greater).
	if(bInserted == false)
		m_Stages.push_back(pNew);
	
	return pNew;
}

MMatchStage *MMatchStageManager::Get(const MUID &uidStage)
{
	for(list<MMatchStage *>::iterator i = m_Stages.begin(); i != m_Stages.end(); i++)
	{
		MMatchStage *pCurr = (*i);
		if(pCurr->GetUID() == uidStage) return pCurr;
	}
	return NULL;
}

bool MMatchStageManager::Remove(const MUID &uidStage)
{
	for(list<MMatchStage *>::iterator i = m_Stages.begin(); i != m_Stages.end(); i++)
	{
		MMatchStage *pCurr = (*i);
		
		if(pCurr->GetUID() == uidStage)
		{
			delete pCurr;
			m_Stages.erase(i);
			break;
		}
	}
	return true;
}

bool MMatchStageManager::Remove(MMatchStage *pStage)
{
	return Remove(pStage->GetUID());
}

void MMatchStageManager::RetrieveList(const MUID &uidChannel, int nPage, MStageList *pOut)
{
	int nTotal = 0;
	int nPrevStages = 0, nNextStages = 0;
	
	unsigned int nChecksum = 0;
	
	for(list<MMatchStage *>::iterator i = m_Stages.begin(); i != m_Stages.end(); i++)
	{
		MMatchStage *pCurr = (*i);
		
		if(pCurr->GetChannelUID() == uidChannel)
		{
			if(nPage <= 0)
			{
				if(nTotal < STAGELIST_COUNT)
				{
					pOut->pStage[nTotal] = pCurr;
					nChecksum += pCurr->MakeChecksum();
					
					nTotal++;
				}
				else
					nNextStages++;
			}
			else 
			{
				nPage--;
				nPrevStages++;
			}
		}
	}
	
	pOut->nCount = nTotal;
	pOut->nPrevStageCount = nPrevStages;
	pOut->nNextStageCount = nNextStages;
	pOut->nChecksum = nChecksum;
}

// relay maps.
MRelayMap::MRelayMap()
{
	Initialize();
}

void MRelayMap::Initialize()
{
	m_nType = 0;
	m_nRepeat = 2;	// 3 times.
	
	Empty();
	m_nCurrMapID = 0;
	
	m_nCurrIndex = 0;
	m_nCurrRepeat = 0;
	
	m_bUnFinish = false;
}

void MRelayMap::SetType(int nType)
{
	if(nType != RELAYMAP_TYPE_STRAIGHT && nType != RELAYMAP_TYPE_RANDOM) return;
	m_nType = nType;
}

void MRelayMap::SetRepeat(int nRepeat)
{
	if(nRepeat < 0 || nRepeat > MAX_RELAYMAP_REPEAT_COUNT) return;
	m_nRepeat = nRepeat;
}

int MRelayMap::GetMapIDFromIndex(int nIndex) const
{
	if(nIndex < 0 || nIndex >= MAX_RELAYMAP_ELEMENT_COUNT) return -1;
	return m_nMapID[nIndex];
}

void MRelayMap::GetMapID(int *pOut) const
{
	for(int i = 0; i < MAX_RELAYMAP_ELEMENT_COUNT; i++)
	{
		pOut[i] = m_nMapID[i];
	}
}

void MRelayMap::SetMapIDFromIndex(int nID, int nIndex)
{
	if(nIndex < 0 || nIndex >= MAX_RELAYMAP_ELEMENT_COUNT) return;
	m_nMapID[nIndex] = nID;
}

void MRelayMap::SetMapID(const int *pIn)
{
	for(int i = 0; i < MAX_RELAYMAP_ELEMENT_COUNT; i++)
	{
		m_nMapID[i] = pIn[i];
	}
}

int MRelayMap::GetMapCount() const
{
	int nCount = 0;
	
	for(int i = 0; i < MAX_RELAYMAP_ELEMENT_COUNT; i++)
	{
		if(m_nMapID[i] == -1) continue; // break;
		nCount++;
	}
	
	return nCount;
}

void MRelayMap::Shuffle()
{
	if(m_nType == RELAYMAP_TYPE_STRAIGHT) return;
	
	// http://d.hatena.ne.jp/cubicdaiya/20070831/1188574485
	for(int i = 0; i < MAX_RELAYMAP_ELEMENT_COUNT; i++)
	{
		int rnd = (int)(RandNum() % MAX_RELAYMAP_ELEMENT_COUNT);
		
		int temp = m_nMapID[i];
		m_nMapID[i] = m_nMapID[rnd];
		m_nMapID[rnd] = temp;
	}
	
	Truncate();
}

int MRelayMap::Next()
{
	int nCount = GetMapCount();
	
	if(m_nCurrIndex >= nCount)
	{
		m_nCurrIndex = 0;
		m_nCurrRepeat++;
		
		Shuffle();
		
		if(m_nCurrRepeat > m_nRepeat)
		{
			m_nCurrRepeat = 0;
			m_bUnFinish = false;
			return -1;
		}
	}
	
	int nRet = m_nMapID[m_nCurrIndex];
	m_nCurrIndex++;
	
	m_bUnFinish = true;
	
	return nRet;
}

void MRelayMap::Empty()
{
	m_nMapID[0] = 0;	// default 'mansion' map.
	
	for(int i = 1; i < MAX_RELAYMAP_ELEMENT_COUNT; i++)
	{
		m_nMapID[i] = -1;
	}
}

void MRelayMap::Truncate()
{
	int nCount = GetMapCount();
	
	int i = 0;
	while(i < MAX_RELAYMAP_ELEMENT_COUNT)
	{
		if(nCount > 0)
		{
			if(m_nMapID[i] != -1)
			{
				nCount--;
				i++;
			}
			else
			{
				for(int j = i; j < MAX_RELAYMAP_ELEMENT_COUNT - 1; j++)
				{
					m_nMapID[j] = m_nMapID[j + 1];
				}
			}
		}
		else
		{
			m_nMapID[i] = -1;
			i++;
		}
	}
}

// ------------------------------------------------------------------.

MMatchStage::MMatchStage()
{
	m_uidChannel = MUID(0, 0);
	m_uidStage = MUID::Assign();
	
	m_nID = 0;
	m_szName[0] = '\0';
	
	m_szPassword[0] = '\0';
	m_bPrivate = false;
	
	m_nState = (int)MMSS_STANDBY;
	
	m_szMapName[0] = '\0';
	strcpy(m_szMapName, STAGE_DEFAULT_MAPNAME);
	m_nMapIndex = 0;
	
	m_nGameType = (int)MMGT_DEATHMATCH_SOLO;
	m_nMaxPlayer = 8;
	m_nRound = 50;
	m_nTime = 1800000;
	
	m_bForcedEntry = true;
	m_bTeamBalance = true;
	m_bTeamFriendKill = false;
	m_bTeamWinPoint = false;
	
	m_uidMaster = MUID(0, 0);
	m_nMasterLevel = 0;
	m_nLimitLevel = 0;
	
	m_bRelayMap = false;
	m_bStartRelayMap = false;
	
	m_nNextRelayMapID = -1;
	
	m_pGame = NULL;
	
	// vote kick.
	m_bVoting = false;
	m_nVoteAbortTime = 0;
	
	m_uidVoteTarget = MUID(0, 0);
	m_nVoteTargetAID = 0;
	m_szVoteTargetName[0] = '\0';
	
	m_nRequiredVotes = 0;
	
	m_nVoteAgreedCount = 0;
	m_nVoteOpposedCount = 0;
	// end of vote kick.
	
	// m_nQuestScenarioID = 0;
	ResetQuestInfo();
}

MMatchStage::~MMatchStage()
{
	DestroyGame();
}

void MMatchStage::Create(const MUID &uidChannel, int nNumber, const char *pszName, const char *pszPassword)
{
	m_uidChannel = uidChannel;
	m_nID = nNumber;
	strcpy(m_szName, pszName);
	SetPassword(pszPassword);
}

void MMatchStage::Update(unsigned long nTime)
{
	if(m_pGame != NULL)
	{
		m_pGame->Update(nTime);
	}
	
	VoteUpdate(nTime);
}

void MMatchStage::SetPassword(const char *pszPassword)
{
	strcpy(m_szPassword, pszPassword);
	if(m_szPassword[0] != '\0') m_bPrivate = true;
	else m_bPrivate = false;
}

void MMatchStage::SetMap(const char *pszMapName)
{
	int nMapsetType = g_MapMgr.GetMapsetFromGameType(m_nGameType);
	
	MMatchMap *pMap = g_MapMgr.GetMapFromName(pszMapName, nMapsetType);
	if(pMap == NULL) return;
	
	SetMap(pMap);
}

void MMatchStage::SetMap(int nMapIndex)
{
	int nMapsetType = g_MapMgr.GetMapsetFromGameType(m_nGameType);
	
	MMatchMap *pMap = g_MapMgr.GetMapFromIndex(nMapIndex, nMapsetType);
	if(pMap == NULL) return;
	
	SetMap(pMap);
}

void MMatchStage::SetMap(MMatchMap *pMap)
{
	m_nMapIndex = pMap->nIndex;
	strcpy(m_szMapName, pMap->szName);
	
	m_bRelayMap = m_nMapIndex == g_MapMgr.GetRelayMapID() ? true : false ;
}

bool MMatchStage::SetGameType(int n)
{
	if(n < 0 || n >= (int)MMGT_END) return false;
	m_nGameType = n; return true;
}

bool MMatchStage::SetMaxPlayer(int n)
{
	if(n <= 0 || n > MAX_STAGE_PLAYER) return false;
	m_nMaxPlayer = n; return true;
}

bool MMatchStage::SetRound(int n)
{
	if(n <= 0 || n > MAX_STAGE_ROUND) return false;
	m_nRound = n; return true;
}

bool MMatchStage::SetTimeLimit(int n)
{
	// time 0, 99999 = infinite.
	// if((n < 0 || n > MAX_STAGE_TIME) && n != 99999) return false;
	if((n <= 0 || n > MAX_STAGE_TIME) && IsUnLimitedTime(n) == false) return false;
	m_nTime = n; return true;
}

void MMatchStage::SetMaster(const MUID &uidMaster, int nLevel)
{
	m_uidMaster = uidMaster;
	m_nMasterLevel = nLevel;
}

void MMatchStage::RandomMaster()
{
	/*
	if(m_Users.size() == 0) return;	// anti zero division.
	
	int nRand = (int)(RandNum() % (unsigned int)m_Users.size());
	
	int i = 0;
	for(map<MUID, MMatchObject *>::iterator it = m_Users.begin(); it != m_Users.end(); it++)
	{
		if(i != nRand)
		{
			i++;
			continue;
		}
		
		MMatchObject *pCurr = (*it).second;
		m_uidMaster = pCurr->GetUID();
		m_nMasterLevel = pCurr->m_Exp.nLevel;
		
		break;
	}
	*/
	
	// search for master candidates.
	vector<MMatchObject *> vtCandMaster, vtHideObj;
	
	for(map<MUID, MMatchObject *>::iterator it = m_Users.begin(); it != m_Users.end(); it++)
	{
		MMatchObject *pCurr = (*it).second;
		
		if(pCurr->IsHide() == true)
		{
			vtHideObj.push_back(pCurr);
		}
		else
		{
			vtCandMaster.push_back(pCurr);
		}
	}
	
	// try to give master to player.
	if(vtCandMaster.size() != 0)
	{
		int nRand = (int)(RandNum() % (unsigned int)vtCandMaster.size());
		
		MMatchObject *pObj = vtCandMaster[nRand];
		m_uidMaster = pObj->GetUID();
		m_nMasterLevel = pObj->m_Exp.nLevel;
	}
	// try to give master to hide admin.
	else if(vtHideObj.size() != 0)
	{
		int nRand = (int)(RandNum() % (unsigned int)vtHideObj.size());
		
		MMatchObject *pObj = vtHideObj[nRand];
		m_uidMaster = pObj->GetUID();
		m_nMasterLevel = pObj->m_Exp.nLevel;
	}
	// none of found.
	else
	{
		m_uidMaster = MUID(0, 0);
		m_nMasterLevel = 0;
	}
}

void MMatchStage::AdvanceRelayMap()
{
	if(m_bRelayMap == false) return;
	m_nNextRelayMapID = m_RelayMap.Next();
}

bool MMatchStage::Enter(MMatchObject *pObj)
{
	if(m_Users.find(pObj->GetUID()) != m_Users.end()) return false;	// already exists.
	
	m_Users.insert(pair<MUID, MMatchObject *>(pObj->GetUID(), pObj));
	return true;
}

bool MMatchStage::Leave(MMatchObject *pObj)
{
	m_Users.erase(pObj->GetUID());
	return true;
}

int MMatchStage::GetPlayer()
{
	int nResult = 0;
	
	for(map<MUID, MMatchObject *>::iterator i = m_Users.begin(); i != m_Users.end(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		
		if(pCurr->IsAdmin() == false)
		{
			nResult++;
		}
	}
	
	return nResult;
}

bool MMatchStage::IsPasswordMatch(const char *pszPassword)
{
	if(pszPassword == NULL) return false;
	return strcmp(m_szPassword, pszPassword) == 0 ? true : false ;
}

bool MMatchStage::CheckLevelLimit(int nLevel)
{
	if(m_nLimitLevel == 0) return true;
	return (m_nMasterLevel - m_nLimitLevel <= nLevel && m_nMasterLevel + m_nLimitLevel >= nLevel) ? true : false ;
}

int MMatchStage::GetSuitableTeam()
{
	int nRed = 0, nBlue = 0;
	
	for(map<MUID, MMatchObject *>::iterator i = m_Users.begin(); i != m_Users.end(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		
		if(pCurr->m_GameInfo.nTeam == MMT_RED)
		{
			nRed++;
		}
		else if(pCurr->m_GameInfo.nTeam == MMT_BLUE)
		{
			nBlue++;
		}
	}
	
	int nSuitableTeam = MMT_RED;
	
	if(nRed > nBlue)
	{
		nSuitableTeam = MMT_BLUE;
	}
	
	return nSuitableTeam;
}

void MMatchStage::SetState(int nState)
{
	if(nState < 0 || nState >= (int)MMSS_END) return;
	m_nState = nState;
}

bool MMatchStage::CreateGame()
{
	DestroyGame();
	
	MMatchBaseGame *pGame = NULL;
	
	switch(m_nGameType)
	{
		case (int)MMGT_DEATHMATCH_SOLO	:
			pGame = new MMatchGame_Deathmatch;
		break;
		
		case (int)MMGT_GLADIATOR_SOLO	:
			pGame = new MMatchGame_Gladiator;
		break;
		
		case (int)MMGT_TRAINING	:
			pGame = new MMatchGame_Training;
		break;
		
		case (int)MMGT_BERSERKER	:
			pGame = new MMatchGame_Berserker;
		break;
		
		case (int)MMGT_DEATHMATCH_TEAM	:
			pGame = new MMatchGame_TeamDeathmatch;
		break;
		
		case (int)MMGT_GLADIATOR_TEAM	:
			pGame = new MMatchGame_TeamGladiator;
		break;
		
		case (int)MMGT_ASSASSINATE	:
			pGame = new MMatchGame_Assassinate;
		break;
		
		case (int)MMGT_DEATHMATCH_TEAM_EXTREME	:
			pGame = new MMatchGame_TeamDeathmatchExtreme;
		break;
		
		case (int)MMGT_DUEL	:
			pGame = new MMatchGame_DuelMatch;
		break;
		
		case (int)MMGT_DUEL_TOURNAMENT	:
			pGame = new MMatchGame_DuelTournament;
		break;
		
		case (int)MMGT_SURVIVAL	:
		case (int)MMGT_QUEST	:
			pGame = new MMatchGame_Quest;
		break;
		
		case (int)MMGT_CHALLENGE_QUEST	:
			pGame = new MMatchGame_ChallengeQuest;
		break;
		
		case (int)MMGT_BLITZKRIEG	:
			pGame = new MMatchGame_BlitzKrieg;
		break;
		
		case (int)MMGT_SPY	:
			pGame = new MMatchGame_Spy;
		break;
	}
	
	if(pGame == NULL) return false;
	
	pGame->Create(this);
	m_pGame = pGame;
	
	return true;
}

void MMatchStage::DestroyGame()
{
	if(m_pGame != NULL)
	{
		delete m_pGame;
		m_pGame = NULL;
	}
}

bool MMatchStage::CheckGameStartable()
{
	for(map<MUID, MMatchObject *>::iterator i = m_Users.begin(); i != m_Users.end(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		
		if(pCurr->CheckGameFlag(MMOGF_LAUNCHED) == true)
			if(pCurr->CheckGameFlag(MMOGF_ENTERED) == false)
				return false;
	}
	
	return true;
}

int MMatchStage::CheckInGamePlayerCount()
{
	int nCount = 0;
	
	for(map<MUID, MMatchObject *>::iterator i = m_Users.begin(); i != m_Users.end(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		
		if(pCurr->CheckGameFlag(MMOGF_ENTERED) == true)
			nCount++;
	}
	
	return nCount;
}

bool MMatchStage::LaunchVote(const MUID &uidProposer, const MUID &uidTarget)
{
	if(IsGameLaunched() == false) return false;
	
	if(m_bVoting == true) return false;
	
	// TODO : prevent voting in ladder.
	if(m_nGameType == (int)MMGT_DUEL_TOURNAMENT || m_nGameType == (int)MMGT_BLITZKRIEG) return false;
	
	// check target object validation.
	MMatchObject *pTargetObj = g_ObjectMgr.Get(uidTarget);
	if(pTargetObj == NULL) return false;
	
	if(pTargetObj->m_bCharInfoExist == false) return false;
	if(pTargetObj->m_uidStage != m_uidStage) return false;
	if(pTargetObj->CheckGameFlag(MMOGF_ENTERED) == false) return false;
	
	// give vote rights to in-game players.
	m_vtVoters.clear();
	
	for(map<MUID, MMatchObject *>::iterator i = m_Users.begin(); i != m_Users.end(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		if(pCurr->CheckGameFlag(MMOGF_ENTERED) == false) continue;
		
		pCurr->m_bVoted = false;
		m_vtVoters.push_back(pCurr->GetUID());
	}
	
	// error, there is no need to use vote kick (too less player).
	if(m_vtVoters.size() < 2) return false;
	
	// set vote info.
	m_uidVoteTarget = uidTarget;
	m_nVoteTargetAID = pTargetObj->m_Account.nAID;
	strcpy(m_szVoteTargetName, pTargetObj->m_Char.szName);
	
	m_nVoteAgreedCount = m_nVoteOpposedCount = 0;
	m_nRequiredVotes = (int)(m_vtVoters.size() / 2);
	
	m_nVoteAbortTime = GetTime() + 15000;	// 15 sec.
	m_bVoting = true;
	
	// notify vote to players.
	/*
	MCmdWriter Cmd;
	Cmd.WriteString("kick");	// discuss string : always "kick".
	Cmd.WriteString(pTargetObj->m_Char.szName);
	Cmd.Finalize(MC_MATCH_NOTIFY_CALLVOTE, MCFT_END);
	SendToBattle(&Cmd, m_uidStage);
	*/
	
	if(CheckGameVersion(2012) == true)
	{
		NotifyVote2012(uidProposer, true);
	}
	else
	{
		NotifyVote2011();
	}
	
	return true;
}

bool MMatchStage::AbortVoting(bool bPassed)
{
	if(m_bVoting == false) return false;
	
	if(bPassed == true)
	{
		if(CheckGameVersion(2012) == true)
		{
			MCmdWriter Cmd;
			Cmd.WriteString("kick");	
			Cmd.WriteInt(1);	
			Cmd.WriteString(m_szVoteTargetName);
			Cmd.Finalize(MC_MATCH_NOTIFY_VOTERESULT, MCFT_END);
			SendToStage(&Cmd, m_uidStage);
		}
		else
		{
			MCmdWriter Cmd;
			Cmd.WriteString("kick");	// discuss string : always "kick".
			Cmd.WriteInt(1);	// passed.
			Cmd.Finalize(MC_MATCH_NOTIFY_VOTERESULT, MCFT_END);
			SendToStage(&Cmd, m_uidStage);
		}
		
		BanAID(m_nVoteTargetAID);
		
		MMatchObject *pTargetObj = g_ObjectMgr.Get(m_uidVoteTarget);
		if(pTargetObj != NULL)
		{
			if(pTargetObj->m_bCharInfoExist == true && 
				pTargetObj->m_uidStage == m_uidStage)
			{
				if(pTargetObj->m_nPlace == MMP_BATTLE)
				{
					OnStageLeaveBattle(m_uidVoteTarget, false);
				}
				
				OnStageLeave(m_uidVoteTarget);
				
				MCmdWriter Cmd2;
				Cmd2.Finalize(MC_MATCH_VOTE_STOP, MCFT_END);
				SendToStage(&Cmd2, m_uidStage);
			}
		}
	}
	else
	{
		if(CheckGameVersion(2012) == true)
		{
			MCmdWriter Cmd;
			Cmd.WriteString("kick");	
			Cmd.WriteInt(0);	
			Cmd.WriteString(m_szVoteTargetName);
			Cmd.Finalize(MC_MATCH_NOTIFY_VOTERESULT, MCFT_END);
			SendToStage(&Cmd, m_uidStage);
		}
		else
		{
			MCmdWriter Cmd;
			Cmd.WriteString("kick");	// discuss string : always "kick".
			Cmd.WriteInt(0);	// rejected.
			Cmd.Finalize(MC_MATCH_NOTIFY_VOTERESULT, MCFT_END);
			SendToStage(&Cmd, m_uidStage);
		}
	}
	
	m_bVoting = false;
	
	return true;
}

bool MMatchStage::Vote(const MUID &uidVoter, bool bAgreed)
{
	if(m_bVoting == false) return false;
	
	if(bAgreed == true)
	{
		m_nVoteAgreedCount++;
		
		if(CheckGameVersion(2012) == true)
		{
			NotifyVote2012(uidVoter, false);
		}
		
		if(m_nVoteAgreedCount > m_nRequiredVotes)
		{
			AbortVoting(true);
		}
	}
	else
	{
		m_nVoteOpposedCount++;
		
		if(m_nVoteOpposedCount > m_nRequiredVotes)
		{
			AbortVoting(false);
		}
	}
	
	return true;
}

bool MMatchStage::IsValidVoter(const MUID &uidVoter)
{
	for(vector<MUID>::iterator i = m_vtVoters.begin(); i != m_vtVoters.end(); i++)
	{
		if((*i) == uidVoter) return true;
	}
	
	return false;
}

bool MMatchStage::IsBannedAID(int nAID)
{
	for(vector<int>::iterator i = m_vtKickedAID.begin(); i != m_vtKickedAID.end(); i++)
	{
		if((*i) == nAID) return true;
	}
	
	return false;
}

void MMatchStage::BanAID(int nAID)
{
	if(IsBannedAID(nAID) == true) return;	// already banned, do not push duplicated value.
	m_vtKickedAID.push_back(nAID);
}

void MMatchStage::VoteUpdate(unsigned long nTime)
{
	if(m_bVoting == true)
	{
		if(m_nVoteAbortTime <= nTime)
		{
			AbortVoting();
		}
	}
}

void MMatchStage::NotifyVote2011()
{
	MCmdWriter Cmd;
	Cmd.WriteString("kick");	// discuss string : always "kick".
	Cmd.WriteString(m_szVoteTargetName);
	Cmd.Finalize(MC_MATCH_NOTIFY_CALLVOTE, MCFT_END);
	SendToBattle(&Cmd, m_uidStage);
}

void MMatchStage::NotifyVote2012(const MUID &uidVoter, bool bLaunched)
{
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidVoter);
	Cmd.WriteString("kick");
	Cmd.WriteString(m_szVoteTargetName);
	Cmd.WriteInt(m_nVoteAgreedCount);
	Cmd.WriteInt((int)m_vtVoters.size());	// total voters.
	Cmd.WriteBool(bLaunched);
	Cmd.Finalize(MC_MATCH_NOTIFY_CALLVOTE, MCFT_END);
	SendToBattle(&Cmd, m_uidStage);
}

void MMatchStage::RefreshScenarioID()
{
	int nSlotItemIDs[SACRIITEM_SLOT_COUNT];
	
	for(int i = 0; i < SACRIITEM_SLOT_COUNT; i++)
		nSlotItemIDs[i] = m_QuestItemSlot[i].nItemID;
		
	m_nQuestScenarioID = g_Quest.GetScenarioIDByItemID(nSlotItemIDs, m_szMapName);
}

MSacriItemSlot *MMatchStage::GetQuestSlotItem(int nSlotIndex)
{
	if(nSlotIndex < 0 || nSlotIndex >= SACRIITEM_SLOT_COUNT) return NULL;
	return &m_QuestItemSlot[nSlotIndex];
}

void MMatchStage::SetQuestSlotItem(const MUID &uidOwner, int nItemID, int nSlotIndex)
{
	// check index validate.
	if(nSlotIndex < 0 || nSlotIndex >= SACRIITEM_SLOT_COUNT) return;
	
	// set slot.
	m_QuestItemSlot[nSlotIndex].uidOwner = uidOwner;
	m_QuestItemSlot[nSlotIndex].nItemID = nItemID;
	
	// set scenario.
	RefreshScenarioID();
}

void MMatchStage::ResetQuestInfo()
{
	ZeroInit(m_QuestItemSlot, sizeof(MSacriItemSlot) * SACRIITEM_SLOT_COUNT);
	RefreshScenarioID();
}

void MMatchStage::ActivateSpyMap(int nMapID)
{
	for(list<int>::iterator i = m_SpyBanMapList.begin(); i != m_SpyBanMapList.end(); i++)
	{
		if((*i) == nMapID)
		{
			m_SpyBanMapList.erase(i);
			break;
		}
	}
}

bool MMatchStage::DeActivateSpyMap(int nMapID)
{
	#define MAX_SPY_STAGE_MAP_BAN_COUNT	30
	if(m_SpyBanMapList.size() >= MAX_SPY_STAGE_MAP_BAN_COUNT) return false;
	
	if(IsActiveSpyMap(nMapID) == false) return false;
	m_SpyBanMapList.push_back(nMapID); return true;
}

bool MMatchStage::IsActiveSpyMap(int nMapID)
{
	for(list<int>::iterator i = m_SpyBanMapList.begin(); i != m_SpyBanMapList.end(); i++)
	{
		if((*i) == nMapID) return false;
	}
	
	return true;
}

void MMatchStage::ActivateAllSpyMap()
{
	m_SpyBanMapList.clear();
}

void MMatchStage::SendSpyMapInfo()
{
	MCmdWriter Cmd;
	Cmd.StartBlob(sizeof(int));
	for(list<int>::iterator i = m_SpyBanMapList.begin(); i != m_SpyBanMapList.end(); i++)
	{
		Cmd.WriteInt(*i);
	}
	Cmd.EndBlob();
	Cmd.Finalize(MC_SPY_STAGE_BANNED_MAP_LIST, MCFT_END);
	SendToStage(&Cmd, m_uidStage, true);
}

bool MMatchStage::IsUnLimitedTime(int nTime)
{
	return (nTime == 0 || nTime == 99999) ? true : false ;
}

unsigned int MMatchStage::MakeChecksum()
{
	// for stage list.
	
	unsigned int nRet = 0;
	
	nRet += (unsigned int)(m_uidStage.ulHighID + m_uidStage.ulLowID);
	nRet += (unsigned int)(m_bPrivate == true ? 1 : 0);
	nRet += (unsigned int)m_nState;
	nRet += (unsigned int)m_nMapIndex;
	nRet += (unsigned int)m_nGameType;
	nRet += (unsigned int)m_nMaxPlayer;
	nRet += (unsigned int)(m_bForcedEntry == true ? 1 : 0);
	nRet += (unsigned int)m_nMasterLevel;
	nRet += (unsigned int)m_nLimitLevel;
	
	return nRet;
}
