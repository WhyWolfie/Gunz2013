#ifndef __MMATCHMAP_H__
#define __MMATCHMAP_H__

#define MXML_FILENAME_MAPID	"mapid.xml"

// mmatchmap mapset type for array index & getmap() function.
#define MMMST_GENERAL			0
#define MMMST_DUEL				1
#define MMMST_QUEST				2
#define MMMST_CHALLENGE_QUEST	3
#define MMMST_END				4

struct MMatchMap
{
	int nIndex;
	char szName[64];
};

struct MMatchMapWorldItemNode
{
	int nWorldItemID;
	int nTimeMSec;
	
	float x, y, z;
};

struct MMatchMapWorldItemSpawnNode : public MMatchMapWorldItemNode
{
	bool bSpawned;
	unsigned long nNextTime;
	unsigned short nUID;
};

// for gametype id.
#define MMMWIGTID_SOLO	0
#define MMMWIGTID_TEAM	1

struct MMatchMapWorldItem
{
	int nMapID;
	
	int nGameTypeID;
	vector<MMatchMapWorldItemNode> vtNode;
};


class MMatchMapManager
{
public:
	MMatchMapManager();
	~MMatchMapManager();
	
	bool Load();
	void Add(int nID, const char *pszName, int nMapsetType);
	
	void SetDefaultMapName(const char *pszName, int nMapsetType);
	const char *GetDefaultMapName(int nMapsetType);
	
	int GetMapsetFromGameType(int nGameType);
	
	MMatchMap *GetMapFromName(const char *pszName, int nMapsetType);
	MMatchMap *GetMapFromIndex(int nIndex, int nMapsetType);
	
	MMatchMap *GetRandomDuelMap();
	
	bool IsQuestMap(const char *pszName);
	
	void SetRelayMapID(int nID);
	int GetRelayMapID();
	
	void ClearAll();
	
private:
	vector<MMatchMap *> m_vtMaps[MMMST_END];
	char m_szDefaultMapName[MMMST_END][64];
	
	int m_nRelayMapID;
};

#include "MMatchGame.h"

class MMatchMapWorldItemManager
{
public:
	MMatchMapWorldItemManager();
	~MMatchMapWorldItemManager();
	
	bool Load(const char *pszMapName, int nMapIndex);
	bool Get(int nMapIndex, int nGameTypeID, vector<MMatchMapWorldItemNode> *pOut);
	bool Get(int nMapIndex, int nGameTypeID, list<MMatchMapWorldItemSpawnNode> *pOut);
	void Clear();
	
private:
	vector<MMatchMapWorldItem *> m_vtWorldItem;
};

extern MMatchMapManager g_MapMgr;
extern MMatchMapWorldItemManager g_MapWorldItemMgr;

#endif