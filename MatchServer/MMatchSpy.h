#ifndef __MMATCHSPY_H__
#define __MMATCHSPY_H__

struct MMatchSpyItem
{
	int nItemID;
	int nItemCount;
};

struct MMatchSpyTable
{
	int nTotalCount;
	int nSpyCount;
	int nHPAP;
	vector<MMatchSpyItem> vtSpyItem;
};

class MMatchSpySetting
{
public:
	int GetMinPlayer()	{ return m_nMinPlayer; }
	
	MMatchSpyTable *GetSpyTable(int nPlayerCount);
	
	vector<MMatchSpyItem>::iterator TItemBegin()	{ return m_vtTrackerItem.begin(); }
	vector<MMatchSpyItem>::iterator TItemEnd()		{ return m_vtTrackerItem.end(); }
	
public:
	int m_nMinPlayer;
	
	vector<MMatchSpyTable> m_vtSpyTable;
	vector<MMatchSpyItem> m_vtTrackerItem;	// tracker seems doesn't have two or more tables.
};

extern MMatchSpySetting g_SpySetting;


struct MMatchSpyMap
{
	int nID;
	char szName[64];
	int nMinPlayer;
	int nMaxPlayer;
	int nLimitTime;
	int nSpyOpenTime;
};

class MMatchSpyMapSetting
{
public:
	MMatchSpyMap *GetMap(int nID);
	MMatchSpyMap *GetMap(const char *pszName);
	
public:
	vector<MMatchSpyMap> m_vtMap;
};

extern MMatchSpyMapSetting g_SpyMapSetting;


bool LoadSpyModeXML();
bool LoadSpyMapXML();

#endif