#include "pch.h"
#include "MMatchSpy.h"

MMatchSpyTable *MMatchSpySetting::GetSpyTable(int nPlayerCount)
{
	for(vector<MMatchSpyTable>::iterator i = m_vtSpyTable.begin(); i != m_vtSpyTable.end(); i++)
	{
		MMatchSpyTable *pCurr = &(*i);
		if(pCurr->nTotalCount == nPlayerCount) return pCurr;
	}
	
	return NULL;
}


MMatchSpyMap *MMatchSpyMapSetting::GetMap(int nID)
{
	for(vector<MMatchSpyMap>::iterator i = m_vtMap.begin(); i != m_vtMap.end(); i++)
	{
		MMatchSpyMap *pCurr = &(*i);
		if(pCurr->nID == nID) return pCurr;
	}
	
	return NULL;
}

MMatchSpyMap *MMatchSpyMapSetting::GetMap(const char *pszName)
{
	for(vector<MMatchSpyMap>::iterator i = m_vtMap.begin(); i != m_vtMap.end(); i++)
	{
		MMatchSpyMap *pCurr = &(*i);
		if(MStricmp(pCurr->szName, pszName) == 0) return pCurr;
	}
	
	return NULL;
}


MMatchSpySetting g_SpySetting;

bool LoadSpyModeXML()
{
	XMLDocument doc;
	if(doc.LoadFile("spymode.xml") != XML_SUCCESS) 
	{
		mlog("Failed to load spy mode setting.");
		return false;
	}
	
	mlog("Spy mode setting loading...");
	
	XMLHandle handle(&doc);
	
	// BASE.
	XMLElement *base = handle.FirstChildElement("XML").FirstChildElement("BASE").ToElement();
	if(base == NULL) return false;
	
	g_SpySetting.m_nMinPlayer = base->IntAttribute("minPlayer");
	if(g_SpySetting.m_nMinPlayer <= 0) return false;
	
	// SPY_ITEM_DESC.
	XMLElement *itemdesc = handle.FirstChildElement("XML").FirstChildElement("SPY_ITEM_DESC").ToElement();
	
	map<int, string> SpyItems;	// id, desc.
	
	while(itemdesc != NULL)
	{
		int nItemID;
		char szItemDesc[64];
		
		// nItemID = itemdesc->IntAttribute("ID");
		// strcpy(szItemDesc, itemdesc->Attribute("Desc"));
		
		if(itemdesc->QueryIntAttribute("ID", &nItemID) != XML_NO_ERROR) break;
		
		if(itemdesc->Attribute("Desc") == NULL) break;
		strcpy(szItemDesc, itemdesc->Attribute("Desc"));
		
		SpyItems.insert(pair<int, string>(nItemID, (string)szItemDesc));
		
		itemdesc = itemdesc->NextSiblingElement();
	}
	
	// SPY_TABLE.
	XMLElement *spytable = handle.FirstChildElement("XML").FirstChildElement("SPY_TABLE").ToElement();
	
	while(spytable != NULL)
	{
		MMatchSpyTable table;
		
		table.nTotalCount = spytable->IntAttribute("TotalCount");
		table.nSpyCount = spytable->IntAttribute("SpyCount");
		table.nHPAP = spytable->IntAttribute("HPAP");
		
		for(map<int, string>::iterator i = SpyItems.begin(); i != SpyItems.end(); i++)
		{
			int nItemCount;
			
			if(spytable->QueryIntAttribute((*i).second.c_str(), &nItemCount) == XML_SUCCESS)
			{
				MMatchSpyItem item = {(*i).first, nItemCount};
				table.vtSpyItem.push_back(item);
			}
		}
		
		g_SpySetting.m_vtSpyTable.push_back(table);
		
		spytable = spytable->NextSiblingElement();
	}
	
	// TRACER_TABLE.
	XMLElement *trackertable = handle.FirstChildElement("XML").FirstChildElement("TRACER_TABLE").ToElement();
	
	if(trackertable != NULL)
	{
		for(map<int, string>::iterator i = SpyItems.begin(); i != SpyItems.end(); i++)
		{
			int nItemCount;
			
			if(trackertable->QueryIntAttribute((*i).second.c_str(), &nItemCount) == XML_SUCCESS)
			{
				MMatchSpyItem item = {(*i).first, nItemCount};
				g_SpySetting.m_vtTrackerItem.push_back(item);
			}
		}
	}
	
	mlog("Spy mode info loaded.");
	
	return true;
}

MMatchSpyMapSetting g_SpyMapSetting;

bool LoadSpyMapXML()
{
	XMLDocument doc;
	if(doc.LoadFile("spymaplist.xml") != XML_SUCCESS) 
	{
		mlog("Fail to load spy map descriptor.");
		return false;
	}
	
	mlog("Loading spy mode maps...");
	
	XMLHandle handle(&doc);
	
	// SPY_MAP.
	XMLElement *spymap = handle.FirstChildElement("XML").FirstChildElement("SPY_MAP").ToElement();
	
	while(spymap != NULL)
	{
		MMatchSpyMap map;
		
		map.nID = spymap->IntAttribute("id");
		strcpy(map.szName, spymap->Attribute("name") == NULL ? "" : spymap->Attribute("name"));
		map.nMinPlayer = spymap->IntAttribute("minPlayers");
		map.nMaxPlayer = spymap->IntAttribute("maxPlayers");
		map.nLimitTime = spymap->IntAttribute("limitTime");
		map.nSpyOpenTime = spymap->IntAttribute("spyOpenTime");
		
		g_SpyMapSetting.m_vtMap.push_back(map);
		
		spymap = spymap->NextSiblingElement();
	}
	
	mlog("Spy map list loaded.");
	
	return true;
}
