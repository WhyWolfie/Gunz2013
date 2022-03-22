#include "pch.h"
#include "MMatchMap.h"
#include "MMatchStage.h"

MMatchMapManager g_MapMgr;
MMatchMapWorldItemManager g_MapWorldItemMgr;

int GetWorldItemIDFromName(const char *pszName)
{
	if(MStricmp(pszName, "hp01") == 0)
		return 1;
	else if(MStricmp(pszName, "hp02") == 0)
		return 2;
	else if(MStricmp(pszName, "hp03") == 0)
		return 3;
	else if(MStricmp(pszName, "ap01") == 0)
		return 4;
	else if(MStricmp(pszName, "ap02") == 0)
		return 5;
	else if(MStricmp(pszName, "ap03") == 0)
		return 6;
	else if(MStricmp(pszName, "bullet01") == 0)
		return 7;
	else if(MStricmp(pszName, "bullet02") == 0)
		return 8;
		
	return 0;
}

// ------------------------------.

MMatchMapManager::MMatchMapManager()
{
	for(int i = 0; i < MMMST_END; i++)
	{
		ZeroInit(m_szDefaultMapName[i], sizeof(m_szDefaultMapName[i]));
	}
	m_nRelayMapID = 22;
}

MMatchMapManager::~MMatchMapManager()
{
	ClearAll();
}

bool MMatchMapManager::Load()
{
	XMLDocument doc;
	doc.LoadFile(MXML_FILENAME_MAPID);
	
	if(doc.Error() == true) 
	{
		mlog("Error while loading map informations...");
		return false;
	}
	
	mlog("Loading map information from %s...", MXML_FILENAME_MAPID);
	
	XMLHandle handle(&doc);
	handle = handle.FirstChildElement("XML");
	
	XMLElement *element;
	
	element = handle.FirstChildElement("RELAYMAP_INDEX").ToElement();
	if(element == NULL || element->GetText() == NULL) return false;
	SetRelayMapID(ToInt(element->GetText()));
	
	element = handle.FirstChildElement("GENERAL").FirstChildElement("MAP").ToElement();
	while(element != NULL)
	{
		if(element->Attribute("name") == NULL)
		{
			element = element->NextSiblingElement();
			continue;
		}
		
		Add(element->IntAttribute("id"), element->Attribute("name"), MMMST_GENERAL);
		if(element->BoolAttribute("default") == true) SetDefaultMapName(element->Attribute("name"), MMMST_GENERAL);
		element = element->NextSiblingElement();
	}
	
	element = handle.FirstChildElement("DUEL").FirstChildElement("MAP").ToElement();
	while(element != NULL)
	{
		if(element->Attribute("name") == NULL)
		{
			element = element->NextSiblingElement();
			continue;
		}
		
		Add(element->IntAttribute("id"), element->Attribute("name"), MMMST_DUEL);
		if(element->BoolAttribute("default") == true) SetDefaultMapName(element->Attribute("name"), MMMST_DUEL);
		element = element->NextSiblingElement();
	}
	
	element = handle.FirstChildElement("QUEST").FirstChildElement("MAP").ToElement();
	while(element != NULL)
	{
		if(element->Attribute("name") == NULL)
		{
			element = element->NextSiblingElement();
			continue;
		}
		
		Add(element->IntAttribute("id"), element->Attribute("name"), MMMST_QUEST);
		if(element->BoolAttribute("default") == true) SetDefaultMapName(element->Attribute("name"), MMMST_QUEST);
		element = element->NextSiblingElement();
	}
	
	element = handle.FirstChildElement("CHALLENGEQUEST").FirstChildElement("MAP").ToElement();
	while(element != NULL)
	{
		if(element->Attribute("name") == NULL)
		{
			element = element->NextSiblingElement();
			continue;
		}
		
		Add(element->IntAttribute("id"), element->Attribute("name"), MMMST_CHALLENGE_QUEST);
		if(element->BoolAttribute("default") == true) SetDefaultMapName(element->Attribute("name"), MMMST_CHALLENGE_QUEST);
		element = element->NextSiblingElement();
	}
	
	mlog("Map infos are loaded.");
	
	return true;
}

void MMatchMapManager::Add(int nID, const char *pszName, int nMapsetType)
{
	if(nMapsetType < 0 || nMapsetType >= MMMST_END) return;
	
	MMatchMap *pNew = new MMatchMap;
	pNew->nIndex = nID;
	strcpy(pNew->szName, pszName);
	m_vtMaps[nMapsetType].push_back(pNew);
	
	if(nMapsetType == MMMST_GENERAL)
	{
		g_MapWorldItemMgr.Load(pszName, nID);
	}
}

void MMatchMapManager::SetDefaultMapName(const char *pszName, int nMapsetType)
{
	if(nMapsetType < 0 || nMapsetType >= MMMST_END) return;
	strcpy(m_szDefaultMapName[nMapsetType], pszName);
}

const char *MMatchMapManager::GetDefaultMapName(int nMapsetType)
{
	if(nMapsetType < 0 || nMapsetType >= MMMST_END) return NULL;
	return m_szDefaultMapName[nMapsetType];
}

int MMatchMapManager::GetMapsetFromGameType(int nGameType)
{
	switch(nGameType)
	{
		case (int)MMGT_SURVIVAL			:
		case (int)MMGT_QUEST			:
			return MMMST_QUEST;
			
		case (int)MMGT_CHALLENGE_QUEST	:
			return MMMST_CHALLENGE_QUEST;
	}
	
	return MMMST_GENERAL;
}

MMatchMap *MMatchMapManager::GetMapFromName(const char *pszName, int nMapsetType)
{
	if(nMapsetType < 0 || nMapsetType >= MMMST_END) return NULL;
	
	for(vector<MMatchMap *>::iterator it = m_vtMaps[nMapsetType].begin(); it != m_vtMaps[nMapsetType].end(); it++)
	{
		MMatchMap *pCurr = (*it);
		if(MStricmp(pCurr->szName, pszName) == 0) return pCurr;
	}
	
	return NULL;
}

MMatchMap *MMatchMapManager::GetMapFromIndex(int nIndex, int nMapsetType)
{
	if(nMapsetType < 0 || nMapsetType >= MMMST_END) return NULL;
	
	for(vector<MMatchMap *>::iterator it = m_vtMaps[nMapsetType].begin(); it != m_vtMaps[nMapsetType].end(); it++)
	{
		MMatchMap *pCurr = (*it);
		if(pCurr->nIndex == nIndex) return pCurr;
	}
	
	return NULL;
}

MMatchMap *MMatchMapManager::GetRandomDuelMap()
{
	if(m_vtMaps[MMMST_DUEL].size() == 0) return NULL;	// anti zero division.
	
	int nRand = (int)(RandNum() % (unsigned int)m_vtMaps[MMMST_DUEL].size());
	return m_vtMaps[MMMST_DUEL][nRand];
}

bool MMatchMapManager::IsQuestMap(const char *pszName)
{
	for(vector<MMatchMap *>::iterator it = m_vtMaps[MMMST_QUEST].begin(); it != m_vtMaps[MMMST_QUEST].end(); it++)
	{
		if(MStricmp((*it)->szName, pszName) == 0) return true;
	}
	return false;
}

void MMatchMapManager::SetRelayMapID(int nID)
{
	m_nRelayMapID = nID;
}

int MMatchMapManager::GetRelayMapID()
{
	return m_nRelayMapID;
}

void MMatchMapManager::ClearAll()
{
	for(int i = 0; i < MMMST_END; i++)
	{
		for(vector<MMatchMap *>::iterator it = m_vtMaps[i].begin(); it != m_vtMaps[i].end(); it++)
		{
			delete (*it);
		}
		
		m_vtMaps[i].clear();
	}
}

// ------------------------------.

MMatchMapWorldItemManager::MMatchMapWorldItemManager()
{
}

MMatchMapWorldItemManager::~MMatchMapWorldItemManager()
{
	Clear();
}

bool MMatchMapWorldItemManager::Load(const char *pszMapName, int nMapIndex)
{
	char szFileName[256];
	sprintf(szFileName, ".\\Maps\\%s\\spawn.xml", pszMapName);
	
	XMLDocument doc;
	doc.LoadFile(szFileName);
	
	if(doc.Error() == true) return false;
	
	XMLHandle handle(&doc);
	XMLElement *element = handle.FirstChildElement("XML").FirstChildElement("GAMETYPE").ToElement();
	
	while(element != NULL)
	{
		if(element->Attribute("id") == NULL)
		{
			element = element->NextSiblingElement();
			continue;
		}
		
		int nGameTypeID;
		
		if(MStricmp(element->Attribute("id"), "solo") == 0)
		{
			nGameTypeID = MMMWIGTID_SOLO;
		}
		else if(MStricmp(element->Attribute("id"), "team") == 0)
		{
			nGameTypeID = MMMWIGTID_TEAM;
		}
		else 
		{
			element = element->NextSiblingElement();
			continue;
		}
		
		vector<MMatchMapWorldItemNode> nodelist;
		
		XMLElement *subelement = element->FirstChildElement("SPAWN");
		while(subelement != NULL)
		{
			MMatchMapWorldItemNode node;
			
			node.nWorldItemID = GetWorldItemIDFromName(subelement->Attribute("item") == NULL ? "" : subelement->Attribute("item"));
			node.nTimeMSec = subelement->IntAttribute("timesec");
			
			XMLElement *poselement = subelement->FirstChildElement("POSITION");
			if(poselement == NULL || poselement->GetText() == NULL) 
			{
				subelement = subelement->NextSiblingElement();
				continue;
			}
			
			float x, y, z;
			if(sscanf(poselement->GetText(), "%f %f %f", &x, &y, &z) == 3)
			{
				node.x = x;
				node.y = y;
				node.z = z;
			}
			else
			{
				node.x = 0.0f;
				node.y = 0.0f;
				node.z = 0.0f;
			}
			
			nodelist.push_back(node);
			
			subelement = subelement->NextSiblingElement();
		}
		
		MMatchMapWorldItem *pNewItem = new MMatchMapWorldItem;
		pNewItem->nMapID = nMapIndex;
		pNewItem->nGameTypeID = nGameTypeID;
		pNewItem->vtNode = nodelist;
		m_vtWorldItem.push_back(pNewItem);
		
		element = element->NextSiblingElement();
	}
	
	return true;
}

bool MMatchMapWorldItemManager::Get(int nMapIndex, int nGameTypeID, vector<MMatchMapWorldItemNode> *pOut)
{
	for(vector<MMatchMapWorldItem *>::iterator it = m_vtWorldItem.begin(); it != m_vtWorldItem.end(); it++)
	{
		MMatchMapWorldItem *pItem = (*it);
		
		if(pItem->nMapID == nMapIndex)
		{
			if(pItem->nGameTypeID == nGameTypeID)
			{
				*pOut = pItem->vtNode;
				return true;
			}
		}
	}
	
	return false;
}

bool MMatchMapWorldItemManager::Get(int nMapIndex, int nGameTypeID, list<MMatchMapWorldItemSpawnNode> *pOut)
{
	pOut->clear();
	
	for(vector<MMatchMapWorldItem *>::iterator it = m_vtWorldItem.begin(); it != m_vtWorldItem.end(); it++)
	{
		MMatchMapWorldItem *pItem = (*it);
		
		if(pItem->nMapID == nMapIndex)
		{
			if(pItem->nGameTypeID == nGameTypeID)
			{
				for(vector<MMatchMapWorldItemNode>::iterator i = pItem->vtNode.begin(); i != pItem->vtNode.end(); i++)
				{
					MMatchMapWorldItemNode *pNode = &(*i);
					
					MMatchMapWorldItemSpawnNode snode;
					snode.nWorldItemID = pNode->nWorldItemID;
					snode.nTimeMSec = pNode->nTimeMSec;
					snode.x = pNode->x;
					snode.y = pNode->y;
					snode.z = pNode->z;
					snode.bSpawned = false;
					snode.nNextTime = 0;
					snode.nUID = 0;
					
					pOut->push_back(snode);
				}
				return true;
			}
		}
	}
	
	return false;
}

void MMatchMapWorldItemManager::Clear()
{
	for(vector<MMatchMapWorldItem *>::iterator it = m_vtWorldItem.begin(); it != m_vtWorldItem.end(); it++)
	{
		delete (*it);
	}
	m_vtWorldItem.clear();
}