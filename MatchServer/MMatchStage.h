#ifndef __MMATCHSTAGE_H__
#define __MMATCHSTAGE_H__

#include "MMatchMap.h"
// #include "MMatchGame.h"
class MMatchBaseGame;

#include "MMatchQuest.h"

// for RelayMap.
#define RELAYMAP_TYPE_STRAIGHT		0
#define RELAYMAP_TYPE_RANDOM		1

#define MAX_RELAYMAP_REPEAT_COUNT	4	// 4 is 5 times.

#define MAX_RELAYMAP_ELEMENT_COUNT	20

class MRelayMap
{
public:
	MRelayMap();
	void Initialize();
	
	// basic.
	void SetType(int nType);
	void SetRepeat(int nRepeat);
	
	int GetType() const	{ return m_nType; }
	int GetRepeat() const	{ return m_nRepeat; }
	
	// map id.
	int GetMapIDFromIndex(int nIndex) const;
	void GetMapID(int *pOut) const;
	
	void SetMapIDFromIndex(int nID, int nIndex);
	void SetMapID(const int *pIn);
	
	// etc.
	int GetMapCount() const;
	int GetCurrMapID() const	{ return m_nCurrMapID; }
	
	// for game.
	void Shuffle();
	int Next();	// returns next map id.
	
	void Empty();
	void Truncate();
	
	bool IsUnFinish() const	{ return m_bUnFinish; }
	void ForceFinish()	{ m_bUnFinish = false; }
	
private:
	int m_nType;	// straight or random.
	int m_nRepeat;
	
	int m_nMapID[MAX_RELAYMAP_ELEMENT_COUNT];
	int m_nCurrMapID;
	
	int m_nCurrIndex;
	int m_nCurrRepeat;
	
	bool m_bUnFinish;
};

// stage.
#define MAX_STAGE_NAME_LENGTH		64
#define MAX_STAGE_PASSWORD_LENGTH	(8 + 1)

// stage map.
#define MAX_STAGE_MAPNAME_LENGTH	64
#define STAGE_DEFAULT_MAPNAME	"Mansion"

// stage general.
enum MMatchGameType
{
	MMGT_DEATHMATCH_SOLO, 
	MMGT_DEATHMATCH_TEAM, 
	MMGT_GLADIATOR_SOLO, 
	MMGT_GLADIATOR_TEAM, 
	MMGT_ASSASSINATE, 
	MMGT_TRAINING, 
	MMGT_SURVIVAL, 
	MMGT_QUEST, 
	MMGT_BERSERKER, 
	MMGT_DEATHMATCH_TEAM_EXTREME, 
	MMGT_DUEL, 
	MMGT_DUEL_TOURNAMENT, 
	MMGT_CHALLENGE_QUEST, 
	MMGT_BLITZKRIEG, 
	MMGT_SPY, 
	MMGT_END
};

#define MAX_STAGE_PLAYER	16
#define MAX_STAGE_ROUND		100	// 100 round/kill.
// #define MAX_STAGE_TIME		60	// 60 min.
#define MAX_STAGE_TIME		3600000	// 60 min.

// stage state.
enum MMatchStageState
{
	MMSS_STANDBY, 
	MMSS_COUNTDOWN, 
	MMSS_RUN, 
	MMSS_CLOSE, 
	MMSS_END
};

// for quest.
struct MSacriItemSlot
{
	MUID uidOwner;
	int nItemID;
};

class MMatchStage
{
public:
	MMatchStage();
	~MMatchStage();
	
	void Create(const MUID &uidChannel, int nNumber, const char *pszName, const char *pszPassword);
	
	const MUID &GetChannelUID()	{ return m_uidChannel; }
	const MUID &GetUID()	{ return m_uidStage; }
	
	void Update(unsigned long nTime);
	
	int GetNumber()	{ return m_nID; }
	const char *GetName()	{ return m_szName; }
	
	void SetPassword(const char *pszPassword);
	bool IsPrivate()	{ return m_bPrivate; }
	
	int GetState()	{ return m_nState; }
	
	const char *GetMapName()	{ return m_szMapName; }
	int GetMapIndex()	{ return m_nMapIndex; }
	
	void SetMap(const char *pszMapName);
	void SetMap(int nMapIndex);
	void SetMap(MMatchMap *pMap);
	
	int GetGameType()	{ return m_nGameType; }
	int GetMaxPlayer()	{ return m_nMaxPlayer; }
	int GetRound()	{ return m_nRound; }
	int GetTimeLimit()	{ return m_nTime; }
	bool IsForcedEntryEnabled()	{ return m_bForcedEntry; }
	bool IsTeamBalancingEnabled()	{ return m_bTeamBalance; }
	bool IsFriendlyFireEnabled()	{ return m_bTeamFriendKill; }
	
	bool SetGameType(int n);
	bool SetMaxPlayer(int n);
	bool SetRound(int n);
	bool SetTimeLimit(int n);
	void SetForcedEntry(bool b)	{ m_bForcedEntry = b; }
	void SetTeamBalance(bool b)	{ m_bTeamBalance = b; }
	void SetTeamFriendlyKill(bool b)	{ m_bTeamFriendKill = b; }
	
	void SetMaster(const MUID &uidMaster, int nLevel);
	void SetLevelLimit(int n)	{ m_nLimitLevel = n; }
	
	const MUID &GetMasterUID()	{ return m_uidMaster; }
	bool IsMaster(const MUID &uidPlayer)	{ return m_uidMaster == uidPlayer ? true : false ; }
	
	void RandomMaster();
	
	int GetMasterLevel()	{ return m_nMasterLevel; }
	int GetLimitLevel()	{ return m_nLimitLevel; }
	
	MRelayMap *GetRelayMapSetting()	{ return &m_RelayMap; }
	bool IsRelayMapStage()	{ return m_bRelayMap; }
	void EnableRelayMap(bool b = true)	{ m_bRelayMap = b; }
	
	void AdvanceRelayMap();
	int GetCurrRelayMapID()	{ return m_nNextRelayMapID; }
	
	bool Enter(MMatchObject *pObj);
	bool Leave(MMatchObject *pObj);
	
	// int GetPlayer()	{ return (int)m_Users.size(); }
	int GetPlayer();	// without hider.
	int GetRealPlayer()	{ return (int)m_Users.size(); }	// with hider.
	
	map<MUID, MMatchObject *>::iterator ObjBegin()	{ return m_Users.begin(); }
	map<MUID, MMatchObject *>::iterator ObjEnd()	{ return m_Users.end(); }
	bool ObjExists(const MUID &uidPlayer)	{ return m_Users.find(uidPlayer) != m_Users.end() ? true : false ; }
	
	bool IsPasswordMatch(const char *pszPassword);//	{ return strcmp(m_szPassword, pszPassword) == 0 ? true : false ; }
	bool IsForceJoinable()	{ return (m_bForcedEntry == true || m_nState == (int)MMSS_STANDBY) ? true : false ; }
	bool IsMaxPlayersReached()	{ return (int)m_Users.size() >= m_nMaxPlayer ? true : false ; }
	bool CheckLevelLimit(int nLevel);
	
	int GetSuitableTeam();
	
	void SetState(int nState);
	bool IsGameLaunched()	{ return (m_nState == (int)MMSS_COUNTDOWN || m_nState == (int)MMSS_RUN) ? true : false ; }
	// bool IsRunning()	{ return m_nState == (int)MMSS_RUN ? true : false ; }
	
	MMatchBaseGame *GetGame()	{ return m_pGame; }
	bool CheckGameInfoExists()	{ return m_pGame != NULL ? true : false ; }
	bool CreateGame();
	void DestroyGame();
	
	bool CheckGameStartable();
	int CheckInGamePlayerCount();
	
	// added for staff rights.
	const char *GetPassword()	{ return m_szPassword; }
	
	// vote kick.
	bool LaunchVote(const MUID &uidProposer, const MUID &uidTarget);
	bool AbortVoting(bool bPassed = false);
	
	bool Vote(const MUID &uidVoter, bool bAgreed);
	bool IsValidVoter(const MUID &uidVoter);
	
	bool IsVoting()	{ return m_bVoting; }
	
	bool IsBannedAID(int nAID);
	void BanAID(int nAID);
	
	void VoteUpdate(unsigned long nTime);
	
	void NotifyVote2011();
	void NotifyVote2012(const MUID &uidVoter, bool bLaunched);
	// end of vote kick.
	
	// quest.
	int GetQuestScenarioID()	{ return m_nQuestScenarioID; }
	void SetQuestScenarioID(int nID)	{ m_nQuestScenarioID = nID; }
	void RefreshScenarioID();
	
	MSacriItemSlot *GetQuestSlotItem(int nSlotIndex);
	void SetQuestSlotItem(const MUID &uidOwner, int nItemID, int nSlotIndex);
	
	void ResetQuestInfo();
	
	// spy.
	void ActivateSpyMap(int nMapID);	// unban.
	bool DeActivateSpyMap(int nMapID);	// ban.
	
	bool IsActiveSpyMap(int nMapID);
	
	void ActivateAllSpyMap();
	
	void SendSpyMapInfo();
	
	// extra.
	static bool IsUnLimitedTime(int nTime);
	
	
	unsigned int MakeChecksum();
	
private:
	MUID m_uidChannel;	// located channel.
	MUID m_uidStage;	// this stage uid.
	
	int m_nID;	// stage number.
	char m_szName[MAX_STAGE_NAME_LENGTH];	// stage name.
	
	char m_szPassword[MAX_STAGE_PASSWORD_LENGTH];
	bool m_bPrivate;	// password yes or no.
	
	int m_nState;	// standby = 0, countdown = 1, run = 2, close = 3.
	
	char m_szMapName[MAX_STAGE_MAPNAME_LENGTH];
	int m_nMapIndex;
	
	int m_nGameType;
	int m_nMaxPlayer;
	int m_nRound;
	int m_nTime;
	
	bool m_bForcedEntry;
	bool m_bTeamBalance;
	bool m_bTeamFriendKill;
	bool m_bTeamWinPoint;	// always true : just compatibility.
	
	MUID m_uidMaster;
	int m_nMasterLevel;
	int m_nLimitLevel;
	
	MRelayMap m_RelayMap;
	bool m_bRelayMap;
	bool m_bStartRelayMap;	// just for client compatibility.
	
	int m_nNextRelayMapID;
	
	// several game mode's info.
	MMatchBaseGame *m_pGame;
	
	// vote kick.
	bool m_bVoting;
	unsigned long m_nVoteAbortTime;
	
	MUID m_uidVoteTarget;
	int m_nVoteTargetAID;
	char m_szVoteTargetName[CHARNAME_LEN];
	
	vector<MUID> m_vtVoters;
	
	int m_nRequiredVotes;
	
	int m_nVoteAgreedCount;
	int m_nVoteOpposedCount;
	
	vector<int> m_vtKickedAID;
	// end of vote kick.
	
	// quest.
	int m_nQuestScenarioID;
	MSacriItemSlot m_QuestItemSlot[SACRIITEM_SLOT_COUNT];
	
	// spy.
	list<int> m_SpyBanMapList;
	
	
	map<MUID, MMatchObject *> m_Users;
};

#define STAGELIST_COUNT	8
struct MStageList
{
	int nCount;
	MMatchStage *pStage[STAGELIST_COUNT];
	
	int nPrevStageCount;
	int nNextStageCount;
	
	unsigned int nChecksum;
};


class MMatchStageManager
{
public:
	MMatchStageManager();
	~MMatchStageManager();
	
	MMatchStage *Create(int nStageNum);
	MMatchStage *Get(const MUID &uidStage);
	bool Remove(const MUID &uidStage);
	bool Remove(MMatchStage *pStage);
	void RetrieveList(const MUID &uidChannel, int nPage, MStageList *pOut);
	
	list<MMatchStage *>::iterator Begin()	{ return m_Stages.begin(); }
	list<MMatchStage *>::iterator End()		{ return m_Stages.end(); }
	
private:
	list<MMatchStage *> m_Stages;
};

extern MMatchStageManager g_StageMgr;

enum MMatchObjectCacheType
{
	MMOCT_ADD, 
	MMOCT_REMOVE, 
	MMOCT_UPDATE, 
	MMOCT_REPLACE, 
	MMOCT_END
};

#endif