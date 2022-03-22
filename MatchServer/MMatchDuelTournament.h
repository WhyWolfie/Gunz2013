#ifndef __MMATCHDUELTOURNAMENT_H__
#define __MMATCHDUELTOURNAMENT_H__

#include "MCommandBlob.h"

// #include "MMatchDBMgr.h"
struct DbData_DTRankingInfo;

#ifdef _DEBUG
	#define DUELTOURNAMENT_PLAYER_COUNT		2
#else
	#define DUELTOURNAMENT_PLAYER_COUNT		8
#endif

// queue management.
class MMatchDuelTournamentQueueManager
{
public:
	MMatchDuelTournamentQueueManager();
	~MMatchDuelTournamentQueueManager();
	
	bool CancelChallenge(MMatchObject *pObj);
	bool ReserveChallenge(MMatchObject *pObj);
	
	void Destroy();
	
	void ProcessChallengers(unsigned long nTime);

private:
	unsigned long m_nNextProcessTime;
	
	list<MMatchObject *> m_ChallengerQueue;
};

extern MMatchDuelTournamentQueueManager g_DTQMgr;

// ranking updater.
class MMatchDuelTournamentRankingManager
{
public:
	MMatchDuelTournamentRankingManager();
	~MMatchDuelTournamentRankingManager();
	
	bool LoadTime();
	bool SaveTime();
	
	void Run();
	
	void UpdateDailyRanking(time_t nTime);
	void UpdateWeeklyRanking(time_t nTime);
	
	bool FetchRanking();
	void OnFetchRanking(vector<DbData_DTRankingInfo> &vtRankingInfo);	// called from async db.
	
	void SendRankingInfo(const MUID &uidPlayer);
	
	void ResetDTAllPlayerInfo();
	
private:
	time_t m_nNextDailyRankingUpdateTime;
	time_t m_nNextWeeklyRankingUpdateTime;
	
	vector<MTD_DTRankingInfo> m_vtRankingInfo;
};

extern MMatchDuelTournamentRankingManager g_DTRankingMgr;;

#endif