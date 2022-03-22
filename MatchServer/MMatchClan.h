#ifndef __MMATCHCLAN_H__
#define __MMATCHCLAN_H__

#include "MMatchObject.h"

#include "MMatchConstant.h"
#include "MMatchObject_Constant.h"

class MMatchClan
{
public:
	MMatchClan(int nCLID, const char *pszName, int nLevel, int nPoint, int nMasterCID, 
		int nWin, int nLose, int nDraw, int nTotalPoint, int nRanking, const char *pszEmblemURL, int nEmblemChecksum);
		
	int GetCLID()	{ return m_nCLID; }
	const char *GetName()	{ return m_szName; }
	int GetLevel()	{ return m_nLevel; }
	int GetPoint()	{ return m_nPoint; }
	int GetMasterCID()	{ return m_nMasterCID; }
	int GetWin()	{ return m_nWin; }
	int GetLose()	{ return m_nLose; }
	int GetDraw()	{ return m_nDraw; }
	int GetTotalPoint()	{ return m_nTotalPoint; }
	int GetRanking()	{ return m_nRanking; }
	const char *GetEmblemURL()	{ return m_szEmblemURL; }
	int GetEmblemChecksum()	{ return m_nEmblemChecksum; }
	
	void Join(MMatchObject *pObj);
	void Leave(MMatchObject *pObj);
	void AllLeave();
	
	map<MUID, MMatchObject *>::iterator Begin()	{ return m_Users.begin(); }
	map<MUID, MMatchObject *>::iterator End()	{ return m_Users.end(); }
	
	bool IsEmpty()	{ return m_Users.empty(); }
	
private:
	int m_nCLID;
	char m_szName[DB_CLANNAME_LEN];
	int m_nLevel;
	int m_nPoint;
	int m_nMasterCID;
	int m_nWin;
	int m_nLose;
	int m_nDraw;
	int m_nTotalPoint;
	int m_nRanking;
	char m_szEmblemURL[256];
	int m_nEmblemChecksum;
	
	// logged-in clan members.
	map<MUID, MMatchObject *> m_Users;
};

// manager class.
class MMatchClanManager
{
public:
	MMatchClanManager();
	~MMatchClanManager();
	
	MMatchClan *Get(int nCLID);
	MMatchClan *Get(const char *pszClanName);
	MMatchClan *Add(int nCLID, const char *pszName, int nLevel, int nPoint, int nMasterCID, 
		int nWin, int nLose, int nDraw, int nTotalPoint, int nRanking, const char *pszEmblemURL, int nEmblemChecksum);
	void ClearAll();
	void Update(unsigned long nTime);

private:
	unsigned long m_nNextUpdateTime;
	
	list<MMatchClan *> m_ClanList;
};

extern MMatchClanManager g_ClanMgr;

inline bool IsHaveClanRights(int nGrade, int nRequired)
{
	if(nGrade == MCG_NONE) return false;
	return nGrade <= nRequired ? true : false ;
}

#endif	// __MMATCHCLAN_H__