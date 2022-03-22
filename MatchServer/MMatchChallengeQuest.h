#ifndef __MMATCHCHALLENGEQUEST_H__
#define __MMATCHCHALLENGEQUEST_H__

struct MMatchChallengeQuestNPCInfo
{
	char szActorName[256];
	unsigned short nHP, nAP;
};


struct MMatchChallengeQuestSpawnActor
{
	char szActorName[256];
	int nCount, nInc;
	int nPos;
	bool bAdjustPlayerNum;
};

struct MMatchChallengeQuestSector
{
	vector<MMatchChallengeQuestSpawnActor> vtActor;
};

struct MMatchChallengeQuestScenario
{
	int nMapID;
	char szMapName[64];
	int nRewardItemID;
	int nPlayer;
	
	float fXP;
	float fBP;
	
	vector<MMatchChallengeQuestSector> vtSectors;
};


class MMatchChallengeQuest
{
public:
	MMatchChallengeQuest();
	~MMatchChallengeQuest();
	
	// NPC.
	bool LoadNPCInfo();
	MMatchChallengeQuestNPCInfo *GetNPCInfo(const char *pszActor);
	void ClearNPCInfoList();
	
	// Scenario.
	bool LoadScenario();
	MMatchChallengeQuestScenario *GetScenario(const char *pszMapName);
	void ClearScenario();

private:
	vector<MMatchChallengeQuestNPCInfo *> m_vtNPCInfo;
	vector<MMatchChallengeQuestScenario *> m_vtScenario;
};

extern MMatchChallengeQuest g_ChallengeQuest;

#endif