#ifndef __MMATCHGAME_H__
#define __MMATCHGAME_H__

#include "MMatchQuest.h"
#include "MMatchChallengeQuest.h"

// #include "MMatchStage.h"
class MMatchStage;

// dropped worlditem.
struct MDropWorldItem
{
	int nItemID;
	unsigned long nDropTime;
	
	float x;
	float y;
	float z;
};

struct MSpawnWorldItem
{
	unsigned short nUID;
	int nItemID;
	unsigned long nRemovedTime;	// 0 = static, otherwise time of dynamic.
	
	float x;
	float y;
	float z;
};

// round state.
enum MMatchRoundState
{
	MMRS_PREPARE, 
	MMRS_COUNTDOWN, 
	MMRS_PLAY, 
	MMRS_FINISH, 
	MMRS_EXIT, 
	MMRS_FREE, 
	MMRS_FAILED, 
	MMRS_PRE_COUNTDOWN, 
	MMRS_END
};

// team result.
enum MMatchRoundStateArgument
{
	MMRSA_DRAW, 
	MMRSA_RED_WON, 
	MMRSA_BLUE_WON, 
	MMRSA_RED_ALL_OUT, 
	MMRSA_BLUE_ALL_OUT, 
	MMRSA_END
};

// global func.
bool IsIndividualGame(int nGameType);
bool IsTeamGame(int nGameType);
bool IsQuestGame(int nGameType);
bool IsQuestDerived(int nGameType);
bool IsDuelGame(int nGameType);
bool IsEndableGameByLeave(int nGameType);

void SendRoundStateToClient(const MUID &uidStage, int nRound, int nState, int nArg, const MUID &uidPlayer);

#include "MMatchMap.h"

// base game class.
class MMatchBaseGame
{
public:
	MMatchBaseGame();
	virtual ~MMatchBaseGame();
	
	virtual void Create(MMatchStage *pStage);
	virtual void OnCreate();
	
	virtual void Update(unsigned long nTime);
	
	virtual void SetRoundState(int nState);
	
	virtual void Finish();
	
	void UpdateGameTimer(unsigned long nTime);
	virtual void TimeLimitReached();
	
	bool IsPlaying()	{ return m_nRoundState == (int)MMRS_PLAY ? true : false ; }
	int GetRoundState()	{ return m_nRoundState; }
	
	// world item.
	void DropWorldItem(int nItemID, float x, float y, float z, unsigned long nDropDelayTime);
	void UpdateWorldItem(unsigned long nTime);
	unsigned short AddWorldItem(int nItemID, float x, float y, float z, int nSubType);
	bool TakeWorldItem(const MUID &uidPlayer, unsigned short nUID, int *pOutTookItemID = NULL);
	void RemoveWorldItem(unsigned short nUID);
	void EraseAllWorldItem();
	
	list<MSpawnWorldItem *>::iterator WorldItemBegin()	{ return m_SpawnWorldItemList.begin(); }
	list<MSpawnWorldItem *>::iterator WorldItemEnd()	{ return m_SpawnWorldItemList.end(); }
	
	// game finish.
	bool IsFinished()	{ return m_bFinished; }
	
protected:
	unsigned short AssignWorldItemUID();
	
protected:
	// this game stage.
	MMatchStage *m_pStage;
	
	bool m_bGameStarted;
	
	// round state.
	int m_nRoundState;
	unsigned long m_nStartTime;
	
	// game timer.
	unsigned long m_nEndTime;
	unsigned long m_nNextAnnounceTime;
	
	// worlditem related.
	list<MDropWorldItem *> m_DropWorldItemList;
	list<MSpawnWorldItem *> m_SpawnWorldItemList;
	unsigned short m_nWorldItemUID;
	
	list<MMatchMapWorldItemSpawnNode> m_MapWorldItemList;
	void ResetAllMapWorldItemSpawnTime(unsigned long nTime);
	bool ResetMapWorldItemSpawnTime(unsigned short nUID);
	
	// for finish.
	bool m_bFinished;
};

class MMatchGame_BaseDeathmatch : public MMatchBaseGame
{
public:
	MMatchGame_BaseDeathmatch();
	virtual ~MMatchGame_BaseDeathmatch();
	
	virtual void Create(MMatchStage *pStage);
	virtual void Update(unsigned long nTime);
	
	virtual void SetRoundState(int nState);
	
	virtual void Finish();
	
	virtual void TimeLimitReached();
};

// following gametype doesn't have any special deathmatch rule. : just typedefs.
typedef MMatchGame_BaseDeathmatch MMatchGame_Deathmatch;
typedef MMatchGame_BaseDeathmatch MMatchGame_Gladiator;
typedef MMatchGame_BaseDeathmatch MMatchGame_Training;

// base-deathmatch class derived gametype.
class MMatchGame_Berserker : public MMatchGame_BaseDeathmatch
{
public:
	MMatchGame_Berserker();
	
	void AssignBerserker(const MUID &uidPlayer);
	const MUID &GetBerserkerUID()	{ return m_uidBerserker; }
	
private:
	MUID m_uidBerserker;
};

// team deathmatch.
class MMatchGame_BaseTeamDeathmatch : public MMatchBaseGame
{
public:
	MMatchGame_BaseTeamDeathmatch();
	virtual ~MMatchGame_BaseTeamDeathmatch();
	
	virtual void Create(MMatchStage *pStage);
	virtual void Update(unsigned long nTime);
	
	virtual void SetRoundState(int nState);
	
	// game finish.
	virtual void Finish();
	
	virtual void TimeLimitReached();
	void RoundFinish(int nWinnerTeam);
	
	bool CheckTeamMemberAlive(int nTeam);
	bool CheckRoundStartable();
	
	int GetCurrRound()	{ return m_nRound; }
	int GetLastRoundStateArg()	{ return m_nLastRoundStateArg; }
	
	void GetTeamScore(int *pOutRed, int *pOutBlue);
	
	void RevivalAll();
	void MemberBalancing();
	
protected:
	int m_nRound;
	int m_nLastRoundStateArg;
	
	int m_nRedWins;
	int m_nBlueWins;
	
	bool m_bWaitingForNextRound;
	unsigned long m_nNextRoundStartTime;
};

typedef MMatchGame_BaseTeamDeathmatch MMatchGame_TeamDeathmatch;
typedef MMatchGame_BaseTeamDeathmatch MMatchGame_TeamGladiator;

// assassinate.
class MMatchGame_Assassinate : public MMatchGame_BaseTeamDeathmatch
{
public:
	MMatchGame_Assassinate();
	
	virtual void Update(unsigned long nTime);
	
	virtual void SetRoundState(int nState);
	
	void SetRandomTeamCommander();
	
	bool CheckRedTeamCommanderAlive();
	bool CheckBlueTeamCommanderAlive();
	
	void SetRedTeamCommanderUID(const MUID &uidCommander);
	void SetBlueTeamCommanderUID(const MUID &uidCommander);
	
	const MUID &GetRedTeamCommanderUID()	{ return m_uidRedCommander; }
	const MUID &GetBlueTeamCommanderUID()	{ return m_uidBlueCommander; }
	
	void SendCommanderInfo();
	
protected:
	MUID m_uidRedCommander;
	MUID m_uidBlueCommander;
};

// team dm ex.
class MMatchGame_TeamDeathmatchExtreme : public MMatchBaseGame
{
public:
	MMatchGame_TeamDeathmatchExtreme();
	virtual ~MMatchGame_TeamDeathmatchExtreme();
	
	virtual void Update(unsigned long nTime);
	
	virtual void SetRoundState(int nState);
	
	virtual void Finish();
	
	virtual void TimeLimitReached();
	void PreFinish(int nWinnerTeam);
	
	bool IsRedWin();
	bool IsBlueWin();
	
	void RedDead()	{ m_nBlueKills++; }
	void BlueDead()	{ m_nRedKills++; }
	
	void GetTeamKills(int *pOutRed, int *pOutBlue);
	
protected:
	int m_nLastRoundStateArg;
	
	int m_nRedKills;
	int m_nBlueKills;
	
	bool m_bWaitingForFinish;
	unsigned long m_nFinishTime;
};

// duel.
class MMatchGame_DuelMatch : public MMatchBaseGame
{
public:
	MMatchGame_DuelMatch();
	virtual ~MMatchGame_DuelMatch();
	
	virtual void Update(unsigned long nTime);
	
	virtual void SetRoundState(int nState);
	
	virtual void Finish();
	
	virtual void TimeLimitReached();
	
	void AddPlayer(const MUID &uidPlayer);
	bool RemovePlayer(const MUID &uidPlayer);
	void ReQueuePlayer(const MUID &uidPlayer);
	
	bool StartMatch();
	bool EndMatch(const MUID &uidWinner = MUID(0, 0));
	
	bool CheckMatchStartable();
	
	void CheckFighterAlive(bool *pOutPlayer1, bool *pOutPlayer2);
	void SendDuelQueueInfo(bool bRoundEnd = false);
	
	bool CheckGameFinishable();
	
	void BroadcastWinningState();
	void BroadcastWinningBreak();
	
protected:
	list<MUID> m_PlayerQueueList;
	
	MUID m_uidLastWinner;
	
	MUID m_uidFighter[2];	// 2 players.
	int m_nWinningStreak;
	
	bool m_bWaitingForNextRound;
	unsigned long m_nNextRoundStartTime;
};

// duel tournament.
#define MDUELTOURNAMENT_TYPE_FINAL			0
#define MDUELTOURNAMENT_TYPE_SEMIFINAL		1
#define MDUELTOURNAMENT_TYPE_QUARTERFINAL	2
#define MDUELTOURNAMENT_TYPE_COUNT			3

#define MDUELTOURNAMENT_ROUNDSTATE_FINAL			0
#define MDUELTOURNAMENT_ROUNDSTATE_SEMIFINAL		1
#define MDUELTOURNAMENT_ROUNDSTATE_QUARTERFINAL		2
#define MDUELTOURNAMENT_ROUNDSTATE_COUNT			3

struct MDuelTournamentPlayerHealthStatus
{
	bool bValid;
	int nDamagedPoint;
	int nHP, nAP;
};

struct MDuelTournamentPlayerInfo
{
	int nWin;
	MDuelTournamentPlayerHealthStatus status;
};

class MMatchGame_DuelTournament : public MMatchBaseGame
{
public:
	MMatchGame_DuelTournament();
	virtual ~MMatchGame_DuelTournament();
	
	virtual void Update(unsigned long nTime);
	virtual void SetRoundState(int nState);
	virtual void Finish();
	virtual void TimeLimitReached();
	
	void IncPlayerIndex();
	
	void SetQuarterFinalPlayerUID(const MUID &uidPlayer, int nIndex);
	void SetSemiFinalPlayerUID(const MUID &uidPlayer, int nIndex);
	void SetFinalPlayerUID(const MUID &uidPlayer, int nIndex);
	
	int GetMatchType();
	int GetRoundCount();
	
	void SendGameInfo(bool bRoundEnd = false);
	void SendNextMatchInfo();
	void SendRoundResultInfo(const MUID &uidWinner, const MUID &uidLoser);
	void SendMatchResultInfo(const MUID &uidWinner, int nGainTP, const MUID &uidLoser, int nLoseTP);
	
	void StartPreCountdown();
	void StartCountdown();
	
	void StartMatch();
	void EndMatch(const MUID &uidWinner = MUID(0, 0));
	
	void MatchFinish(const MUID &uidWinner, const MUID &uidLoser);
	bool RoundFinish();
	
	void SpawnFighter();
	void CheckFighterAlive(bool *pOutPlayer1, bool *pOutPlayer2);
	bool IsValidFighter(const MUID &uidFighter);
	
	void Advance(const MUID &uidWinner);
	bool IsMatchingPlayer(const MUID &uidPlayer);
	
	void SetPlayerStatus(const MUID &uidPlayer, int nDamaged, int nRemainHP, int nRemainAP);
	void ResetPlayerInfo();
	void InvalidatePlayerStatus();
	
	void DetermineWinner();
	
	void ChangeToInfiniteTime();
	
private:
	int m_nMatchOrder;	// 0~ : quarter, 4~ : semi, 6~ : final.
	int m_nPlayerIndex;
	
	int m_nRound;
	
	MUID m_uidFighter[2];
	MUID m_uidLastWinner;
	
	MUID m_uidQFinalPlayer[8];	// quarter final (1v1 x 4 = 8).
	MUID m_uidSFinalPlayer[4];	// semi final (1v1 x 2 = 4).
	MUID m_uidFinalPlayer[2];	// final match (1v1 x 1 = 2).
	
	bool m_bTimeout;
	bool m_bDrawGame;
	bool m_bMatchFinish;	// true on round done, false on round remaining.
	
	unsigned long m_nPreCountdownEndTime;	// end time of show match char info.
	unsigned long m_nPreCountdownStartTime;	// end time of show result info.
	bool m_bWaitingForNextCountdown;	// is waiting for next pre-countdown (m_nPreCountdownStartTime).
	
	unsigned long m_nRoundFinishTime;	// round finish time when EndMatch() called.
	bool m_bWaitingForRoundFinish;		// is waiting for finish round (m_nRoundFinishTime).
	
	MDuelTournamentPlayerInfo m_PlayerInfo[2];	// used on time out.
};

// quest.
struct MAliveNPCInfo
{
	MUID uidOwner;
	MUID uidNPC;
};
	
struct MQuestPlayerStatus
{
	MUID uidPlayer;
	bool bNextSectorReady;
};

// npc worlditem.
struct MQuestNPC_ScheduledItem
{
	MUID uidNPC;
	unsigned short nWorldItemUID;
	MQuestNPCDropItemNode *pNode;
};

// the following two structs used when distributing items.
struct MQuestGame_RewardItemNode
{
	int nItemID;
	int nCount;
	int nRentHourPeriod;
};

struct MQuestGame_RewardItemObject
{
	MMatchObject *pObj;
	vector<MQuestGame_RewardItemNode> *pvtItem;
};

class MMatchGame_Quest : public MMatchBaseGame
{
public:
	MMatchGame_Quest();
	virtual ~MMatchGame_Quest();
	
	virtual void OnCreate();
	virtual void Update(unsigned long nTime);
	virtual void SetRoundState(int nState);
	virtual void Finish();

	enum MQuestCombatState
	{
		MQCS_NONE, 
		MQCS_PREPARE, 
		MQCS_PLAY, 
		MQCS_COMPLETED
	};
	void SetCombatState(int nState);
	
	MUID RandomNPCOwner();
	
	void SpawnNPC();
	void SpawnBoss();
	void UnLimitedSpawnNPC();
	
	void ReSelectNPCOwner();
	
	bool IsValidNPCOwner(const MUID &uidOwner, const MUID &uidNPC);
	bool NPCDead(const MUID &uidKiller, const MUID &uidNPC);
	
	MQuestPlayerStatus *GetPlayerStatus(const MUID &uidPlayer);
	
	bool CheckGameOver();
	bool CheckSectorProcessable();
	
	int GetCurrRound()	{ return m_nRound; }
	void SectorPlayerReady(const MUID &uidPlayer);
	
	void ExpBonus();
	
	void StartSector();
	void RoundRefresh();
	
	void QuestCompleted();
	void QuestFailed();
	
	int GetSectorIndex()	{ return m_nRoundInfoIndex; }
	int GetRepeatIndex()	{ return m_nRepeated; }
	
	// public class function for world items.
	void ScheduleNPCDropItem(const MUID &uidNPC, int nNPCID);
	int GetNPCDropWorldItemID(const MUID &uidNPC);
	void SetNPCDropWorldItemUID(const MUID &uidNPC, unsigned short nUID);
	int TakeNPCDropItem(unsigned short nWorldItemUID);
	
	void DistributeReward();
	void SurvivalFinish();
	
	void RevivalAll();
	
protected:
	MQuestScenario *m_pScenario;
	
	unsigned long m_nRoundStartTime;
	unsigned long m_nPortalEndTime;
	
	unsigned long m_nNextNPCSpawnTime;
	
	int m_nRound, m_nRoundInfoIndex;
	vector<MMatchGame_QuestRound> m_vtRoundInfo;
	
	int m_nCombatState;
	
	list<MAliveNPCInfo> m_AliveNPCList;
	list<MAliveNPCInfo> m_AliveBossList;
	
	bool m_bBossRound;
	
	vector<MQuestPlayerStatus> m_vtPlayerStatus;
	
	// XP, BP increases when a round clear.
	float m_fCurrXP;
	float m_fCurrBP;
	
	// repeat bonus increases when round repeat.
	float m_fRepeatBonus;
	
	bool m_bQuestDone;
	unsigned long m_nFinishTime;
	
	int m_nRepeated;
	
	// world items.
	vector<MQuestNPC_ScheduledItem> m_vtNPCScheduledItem;
	vector<MQuestNPCDropItemNode *> m_vtTookNPCReward;
};

// challenge quest.
class MMatchGame_ChallengeQuest : public MMatchBaseGame
{
public:
	MMatchGame_ChallengeQuest();
	virtual ~MMatchGame_ChallengeQuest();
	
	virtual void OnCreate();
	virtual void Update(unsigned long nTime);
	virtual void SetRoundState(int nState);
	virtual void Finish();
	
	MUID RandomActorOwner();
	void ReSelectActorOwner();
	
	bool SpawnActor(const char *pszActorName, int nPos, int nPosIndex);
	bool SpawnActorByPlayer(const MUID &uidOwnerActor, const char *pszActorName, const FloatVector *pPos, const FloatVector *pDir);
	
	void SpawnPreparedActor();
	
	bool IsValidActorOwner(const MUID &uidOwner, const MUID &uidNPC);
	bool ActorDead(const MUID &uidKiller, const MUID &uidNPC);
	
	bool CheckGameOver();
	
	void SectorFinish();
	void SectorStart();
	
	void QuestCompleted();
	void QuestFailed();
	
	void DistributeReward();
	void SaveRecord();
	
	void RevivalAll();
	
protected:
	MMatchChallengeQuestScenario *m_pScenario;
	
	int m_nRound;
	
	unsigned long m_nQuestStartedTime;
	unsigned long m_nQuestEndedTime;
	
	bool m_bActorSpawned;
	unsigned m_nActorSpawnTime;
	
	list<MAliveNPCInfo> m_AliveActorList;
	
	bool m_bRoundCleared;
	unsigned long m_nNextRoundStartTime;
	
	float m_fCurrXP;
	float m_fCurrBP;
};

// blitzkrieg.
struct MBlitzAliveActorInfo
{
	#define BLITZACTOR_FLAG_NONE		0
	#define BLITZACTOR_FLAG_RADAR		1
	#define BLITZACTOR_FLAG_BARRICADE	2
	#define BLITZACTOR_FLAG_TREASURE	4
	
	int nTeamID;
	
	MUID uidOwner;
	MUID uidActor;
	
	int nPoint;
	
	unsigned long nFlags;
};

struct MBlitzTreasureChest
{
	char szActorName[256];
	
	MUID uidActor;
	
	bool bSpawned;
	unsigned long nNextSpawnTime;
	
	int nIndex;
};

enum MMatchBlitzRoundState
{
	MMBRS_PREPARE, 
	MMBRS_PLAY
};

#define MBLITZ_CLASS_HUNTER			0
#define MBLITZ_CLASS_SLAUGHTERER	1
#define MBLITZ_CLASS_TRICKSTER		2
#define MBLITZ_CLASS_GLADIATOR		3
#define MBLITZ_CLASS_DUELIST		4
#define MBLITZ_CLASS_INCINERATOR	5
#define MBLITZ_CLASS_COMBATOFFICER	6
#define MBLITZ_CLASS_ASSASSIN		7
#define MBLITZ_CLASS_WRECKER		8
#define MBLITZ_CLASS_MAX			9

// hard-coded, required manual items to use those classes. (id 0 to unlimited use.)
static const int g_nBlitzManualItemID[MBLITZ_CLASS_MAX] = 
	{0, 0, 0, 900000, 900001, 900002, 900003, 900004, 900005};

#define MBLITZ_SKILLTYPE_WEAPON		0
#define MBLITZ_SKILLTYPE_BULLET		1
#define MBLITZ_SKILLTYPE_ARMOR		2
#define MBLITZ_SKILLTYPE_BOMB		3
#define MBLITZ_SKILLTYPE_MAGAZINE	4
#define MBLITZ_SKILLTYPE_MEDIC		5
#define MBLITZ_SKILLTYPE_MAX		6

#define MBLITZ_SKILLGRADE_MAX		4

class MBlitzPlayerStatus
{
public:
	MBlitzPlayerStatus(const MUID &uidPlayer, int nTeamID)
	{
		m_uidPlayer = uidPlayer;
		m_nTeamID = nTeamID;
		m_fHonorPoint = 0.0f;
		m_nClass = MBLITZ_CLASS_HUNTER;
		ZeroInit(m_nSkillGrade, sizeof(m_nSkillGrade));
	}
	~MBlitzPlayerStatus()
	{
	}
	
	void AddHonorPoint(int nInc)
	{
		if(m_nClass == MBLITZ_CLASS_HUNTER)
			m_fHonorPoint += (float)nInc * 1.20;	// 20 percent up.
		else
			m_fHonorPoint += (float)nInc;
	}
	bool UseHonorPoint(int nDec)
	{
		if((int)m_fHonorPoint < nDec) return false;
		m_fHonorPoint -= (float)nDec; return true;
	}
	
public:
	MUID m_uidPlayer;
	int m_nTeamID;
	float m_fHonorPoint;
	int m_nClass;
	int m_nSkillGrade[MBLITZ_SKILLTYPE_MAX];
};

class MMatchGame_BlitzKrieg : public MMatchBaseGame
{
public:
	MMatchGame_BlitzKrieg();
	virtual ~MMatchGame_BlitzKrieg();
	
	virtual void OnCreate();
	virtual void Update(unsigned long nTime);
	virtual void SetRoundState(int nState);
	virtual void Finish();
	
	int GetBlitzRoundState()	{ return m_nBlitzState; }
	void Finish(int nWinnerTeam);
	
	void CheckTeamStatus(bool *pOutRedWin, bool *pOutBlueWin);
	
	void SendAvailableClassInfo();
	
	void UpdateTreasure(unsigned long nTime);
	void BrokeTreasure(const MUID &uidActor);
	
	unsigned long GetElapsedTime()	{ return GetTime() - m_nGameStartedTime; }
	
	void SetBlitzState(int nState);
	
	MBlitzPlayerStatus *GetPlayerStatus(const MUID &uidPlayer);
	
	MUID RandomActorOwner();
	void ReSelectActorOwner();
	
	bool SpawnActor(const char *pszActorName, int nTeamID, int nPosIndex, unsigned long nFlags, MUID *pOutUID = NULL);
	bool SpawnActorByPlayer(const MUID &uidOwnerActor, const char *pszActorName, int nTeamID, const FloatVector *pPos, const FloatVector *pDir);
	
	void SpawnPreparedActor();
	
	bool IsValidActorOwner(const MUID &uidOwner, const MUID &uidNPC);
	bool ActorDead(const MUID &uidKiller, const MUID &uidNPC);
	
	int GetRandomRouteID();
	int GetSuitableRouteID(int nTeamID);
	
	int GetActorTeamID(const MUID &uidActor);
	int GetActorPoint(const char *pszActor);
	
	void BrokeBarricade(int nTeamID);
	void ReinforceActor(int nTeamID, int nBarricades);
	
protected:
	bool m_bPreparingGame;
	unsigned long m_nGameStartedTime;
	
	int m_nLastRoundStateArg;
	
	int m_nBlitzState;
	
	vector<MBlitzPlayerStatus> m_vtPlayerStatus;
	unsigned long m_nNextBonusHonorIncreaseTime;
	
	MUID m_uidRedRadar;
	MUID m_uidBlueRadar;
	
	int m_nRedBarricadeCount;
	int m_nBlueBarricadeCount;
	
	list<MBlitzAliveActorInfo> m_AliveActorList;
	vector<MBlitzTreasureChest> m_vtTreasure;
	
	int m_nWinnerTeam;
};

// spy mode.
class MSpyPlayerStatus
{
public:
	MSpyPlayerStatus(const MUID &uidPlayer)
	{
		m_uidPlayer = uidPlayer;
		m_nWin = 0;
		m_nLose = 0;
		m_nPoint = 0;
		m_nAddPoint = 0;
	}
	
public:
	MUID m_uidPlayer;
	int m_nWin;
	int m_nLose;
	int m_nPoint;
	int m_nAddPoint;
};

class MMatchGame_Spy : public MMatchBaseGame
{
public:
	MMatchGame_Spy();
	virtual ~MMatchGame_Spy();
	
	virtual void OnCreate();
	virtual void Update(unsigned long nTime);
	virtual void SetRoundState(int nState);
	virtual void Finish();
	virtual void TimeLimitReached();
	
	int GetCurrRound()	{ return m_nRound; }
	int GetLastRoundStateArg()	{ return m_nLastRoundStateArg; }
	
	bool CheckPlayersEnough();
	
	bool RandomSpy();
	void RefreshTeamIDs();
	
	bool IsSpy(const MUID &uidPlayer);
	bool IsParticipant(const MUID &uidPlayer);
	
	void JoinPlayer(const MUID &uidPlayer);
	void LeavePlayer(const MUID &uidPlayer);
	
	void CheckPlayerAlive(bool *pOutSpyAlive, bool *pOutTrackerAlive);
	
	// nWinnerTeam : MMT_RED for spy, MMT_BLUE for tracker. and 0 for draw.
	void RoundFinish(int nWinnerTeam = 0);
	void TeamBonus(int nWinnerTeam);
	
	void RevivalAll();
	
	MSpyPlayerStatus *GetPlayerStatus(const MUID &uidPlayer);
	
	void SendGameInfo();
	void SendScoreInfo(const MUID &uidPlayer);
	
protected:
	int m_nRound;
	int m_nLastRoundStateArg;
	
	bool m_bWaitingForNextRound;
	unsigned long m_nNextRoundStartTime;
	
	int m_nStartPlayer;	// total player count when game started.
	
	vector<MUID> m_vtSpys;
	vector<MUID> m_vtParticipants;
	
	vector<MSpyPlayerStatus> m_vtPlayerStatus;
};

#endif