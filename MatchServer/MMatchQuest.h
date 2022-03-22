#ifndef __MMATCHQUEST_H__
#define __MMATCHQUEST_H__

// #include "MCommandBlob.h"
struct MTD_NPCINFO;
struct MTD_SurvivalRanking;

// #include "MMatchStage.h"

// #include "MMatchDBMgr.h"
struct DbData_SurvivalRankingInfo;

// NPC, MQuestNPCAttackType.
#define MQUEST_NPC_ATTACK_TYPE_NONE		0
#define MQUEST_NPC_ATTACK_TYPE_MELEE	1
#define MQUEST_NPC_ATTACK_TYPE_RANGE	2
#define MQUEST_NPC_ATTACK_TYPE_MAGIC	4

#define SACRIITEM_SLOT_COUNT	2

// for Game.
struct MMatchGame_QuestRound
{
	bool bInfinite;
	deque<int>	NPCIDs;	// num of npc_count will be inserted.
	deque<int>	BossNPCIDs;	// contains only boss npc id.
};

// -------------------------------------------------- .

// quest worlditem & reward system.
struct MQuestNPCDropItemNode
{
	int nItemID;
	bool bZItem;
	int nRentHourPeriod;
};

class MQuestNPCDropItem
{
public:
	MQuestNPCDropItem(int nNPCID);
	
	void Add(int nItemID, bool bZItem = false, int nRentHourPeriod = 0);
	
	int GetNPCID()	{ return m_nNPCID; }
	MQuestNPCDropItemNode *GetRandomItem();
	
private:
	int m_nNPCID;
	vector<MQuestNPCDropItemNode> m_vtNode;
};

// for XMLs.
struct MQuestRoundNPCNode
{
	int nNPCID;
	float fRate;
	bool bBoss;
};

struct MQuestRound
{
	int nBaseNPCID;
	bool bInfinite;
	list<MQuestRoundNPCNode> NPCList;
};

struct MQuestScenario
{
	int nID;
	int nQL;	// Quest Level.
	// char szMapName[MAX_STAGE_MAPNAME_LENGTH];
	char szMapName[32];
	int nExp;
	int nBounty;
	int nRound;
	int nRepeat;
	int nNPCCount;
	
	int nQuestItemID[SACRIITEM_SLOT_COUNT];
	
	bool bRandomSector;
	vector<int> vtSector;
	
	vector<MQuestRound> vtRound;
};


#define MXML_QUEST_NPC_FILENAME				"npc.xml"
#define MXML_QUEST_SCENARIO_FILENAME		"scenario_quest.xml"
#define MXML_SURVIVAL_SCENARIO_FILENAME		"scenario_survival.xml"
#define MXML_QUEST_NPC_DROPITEM_FILENAME	"quest_dropitem.xml"

class MMatchQuest
{
public:
	MMatchQuest();
	~MMatchQuest();
	
	bool LoadNPCInfo();
	bool LoadScenario(const char *pszFileName);
	bool LoadNPCDropItem();

	MQuestScenario *GetScenario(int nID);
	int GetScenarioIDByItemID(const int *id /* int id[SACRIITEM_SLOT_COUNT] */, const char *pszMapName);
	bool InitializeGameRound(int nScenarioID, vector<MMatchGame_QuestRound> *pOut);
	bool BuildNPCInfoListCommand(int nScenarioID, int nGameType, MCmdWriter *pOut);
	bool BuildGameInfoCommand(int nScenarioID, int nGameType, MCmdWriter *pOut);
	int GetRandomNPC(int nScenarioID, int nRound);
	int GetSurvivalScenarioID(const char *pszMapName);
	
	MQuestNPCDropItemNode *GetRandomNPCItem(int nNPCID);
	
	void Destroy();
	
	// survival rank.
	void Run(unsigned long nTime);
	
	bool FetchSurvivalRanking();
	void OnFetchSurvivalRanking(vector<DbData_SurvivalRankingInfo> &vtRankingInfo);
	
	bool UpdateSurvivalRanking();
	
	void SendSurvivalRankingInfo(const MUID &uidStage);
	
	// hard-coded quest map info.
	void GetSuitableSectorInfo(int to, int *from, int *index);	// in, out, out.
	
private:
	vector<MTD_NPCINFO> m_vtNPCInfo;
	vector<MQuestScenario *> m_vtScenario;
	
	vector<MQuestNPCDropItem> m_vtNPCDropItem;
	
	// survival rank.
	vector<MTD_SurvivalRanking> m_vtSurvivalRanking;
	unsigned long m_nNextSurvivalRankingUpdateTime;
};

extern MMatchQuest g_Quest;

inline bool IsQuestItemID(int nID)
{
	if(nID >= 200001 && nID <= 200044) return true;
	return false;
}

#endif