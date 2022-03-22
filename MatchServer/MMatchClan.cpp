#include "pch.h"
#include "MMatchClan.h"

#include "MMatchObject.h"

MMatchClanManager g_ClanMgr;

MMatchClan::MMatchClan(int nCLID, const char *pszName, int nLevel, int nPoint, int nMasterCID, 
	int nWin, int nLose, int nDraw, int nTotalPoint, int nRanking, const char *pszEmblemURL, int nEmblemChecksum)
{
	m_nCLID = nCLID;
	strcpy(m_szName, pszName);
	m_nLevel = nLevel;
	m_nPoint = nPoint;
	m_nMasterCID = nMasterCID;
	m_nWin = nWin;
	m_nLose = nLose;
	m_nDraw = nDraw;
	m_nTotalPoint = nTotalPoint;
	m_nRanking = nRanking;
	strcpy(m_szEmblemURL, pszEmblemURL);
	m_nEmblemChecksum = nEmblemChecksum;
}

void MMatchClan::Join(MMatchObject *pObj)
{
	if(m_Users.find(pObj->GetUID()) != m_Users.end()) return;
	m_Users.insert(pair<MUID, MMatchObject *>(pObj->GetUID(), pObj));
}

void MMatchClan::Leave(MMatchObject *pObj)
{
	m_Users.erase(pObj->GetUID());
}

void MMatchClan::AllLeave()
{
	m_Users.clear();
}

// manager class.
MMatchClanManager::MMatchClanManager()
{
	m_nNextUpdateTime = 0;
}

MMatchClanManager::~MMatchClanManager()
{
	ClearAll();
}

MMatchClan *MMatchClanManager::Get(int nCLID)
{
	if(nCLID == 0) return NULL;
	
	for(list<MMatchClan *>::iterator i = m_ClanList.begin(); i != m_ClanList.end(); i++)
	{
		MMatchClan *pCurr = (*i);
		if(pCurr->GetCLID() == nCLID) return pCurr;
	}
	return NULL;
}

MMatchClan *MMatchClanManager::Get(const char *pszClanName)
{
	if(pszClanName == NULL) return NULL;
	
	for(list<MMatchClan *>::iterator i = m_ClanList.begin(); i != m_ClanList.end(); i++)
	{
		MMatchClan *pCurr = (*i);
		if(MStricmp(pCurr->GetName(), pszClanName) == 0) return pCurr;
	}
	return NULL;
}

MMatchClan *MMatchClanManager::Add(int nCLID, const char *pszName, int nLevel, int nPoint, int nMasterCID, 
	int nWin, int nLose, int nDraw, int nTotalPoint, int nRanking, const char *pszEmblemURL, int nEmblemChecksum)
{
	if(nCLID == 0) return NULL;
	
	MMatchClan *pClan = Get(nCLID);
	if(pClan != NULL) return pClan;
	
	MMatchClan *pNew = new MMatchClan(nCLID, pszName, nLevel, nPoint, nMasterCID, nWin, nLose, nDraw, nTotalPoint, 
									  nRanking, pszEmblemURL, nEmblemChecksum);
	m_ClanList.push_back(pNew);
	return pNew;
}

void MMatchClanManager::ClearAll()
{
	for(list<MMatchClan *>::iterator i = m_ClanList.begin(); i != m_ClanList.end(); i++)
	{
		delete (*i);
	}
	m_ClanList.clear();
}

void MMatchClanManager::Update(unsigned long nTime)
{
	if(m_nNextUpdateTime > nTime)
	{
		return;
	}
	
	list<MMatchClan *>::iterator it = m_ClanList.begin();
	
	while(it != m_ClanList.end())
	{
		MMatchClan *pCurr = (*it);
		
		if(pCurr->IsEmpty() == false)
		{
			it++;
			continue;
		}
		
		delete pCurr;
		it = m_ClanList.erase(it);
	}
	
	m_nNextUpdateTime = nTime + 3600000;	// 1 hour.
}