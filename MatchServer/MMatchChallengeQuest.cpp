#include "pch.h"
#include "MMatchChallengeQuest.h"

MMatchChallengeQuest g_ChallengeQuest;

MMatchChallengeQuest::MMatchChallengeQuest()
{
}

MMatchChallengeQuest::~MMatchChallengeQuest()
{
	ClearScenario();
	ClearNPCInfoList();
}

bool MMatchChallengeQuest::LoadNPCInfo()
{
	XMLDocument doc;
	if(doc.LoadFile("npc2.xml") != XML_SUCCESS)
	{
		mlog("Read NPC info for challenge quest failed.");
		return false;
	}
	
	mlog("---> Start challenge quest NPC load.");
	
	XMLHandle han(&doc);
	XMLElement *elem;
	
	elem = han.FirstChildElement("XML").FirstChildElement("ACTOR").ToElement();
	
	while(elem != NULL)
	{
		if(elem->Attribute("name") == NULL)
		{
			elem = elem->NextSiblingElement();
			continue;
		}
		
		MMatchChallengeQuestNPCInfo *pNew = new MMatchChallengeQuestNPCInfo;
		strcpy(pNew->szActorName, elem->Attribute("name"));
		pNew->nHP = (unsigned short)elem->UnsignedAttribute("max_hp");
		pNew->nAP = (unsigned short)elem->UnsignedAttribute("max_ap");
		
		m_vtNPCInfo.push_back(pNew);
		
		elem = elem->NextSiblingElement();
	}
	
	mlog("---> End challenge quest NPC load.");
	
	return true;
}

MMatchChallengeQuestNPCInfo *MMatchChallengeQuest::GetNPCInfo(const char *pszActor)
{
	for(vector<MMatchChallengeQuestNPCInfo *>::iterator i = m_vtNPCInfo.begin(); i != m_vtNPCInfo.end(); i++)
	{
		MMatchChallengeQuestNPCInfo *pCurr = (*i);
		if(MStricmp(pCurr->szActorName, pszActor) == 0) return pCurr;
	}
	return NULL;
}

void MMatchChallengeQuest::ClearNPCInfoList()
{
	for(vector<MMatchChallengeQuestNPCInfo *>::iterator i = m_vtNPCInfo.begin(); i != m_vtNPCInfo.end(); i++)
	{
		delete (*i);
	}
	m_vtNPCInfo.clear();
}

// ---------------------------------------------------------------------------.
bool MMatchChallengeQuest::LoadScenario()
{
	XMLDocument doc;
	if(doc.LoadFile("scenario_challengequest.xml") != XML_SUCCESS)
	{
		mlog("Read NPC info for challenge quest failed.");
		return false;
	}
	
	mlog("---> Start challenge quest scenario load.");
	
	XMLHandle han(&doc);
	XMLElement *elem;
	
	float fExpRate, fBountyRate;
	
	elem = han.FirstChildElement("XML").FirstChildElement("EXPRATE").ToElement();
	if(elem != NULL)
	{
		fExpRate = elem->FloatAttribute("value");
		fBountyRate = elem->FloatAttribute("bp_value");
	}
	else
	{
		mlog("No valid exp rate setting.");
		return false;
	}
	
	elem = han.FirstChildElement("XML").FirstChildElement("SCENARIO").ToElement();
	while(elem != NULL)
	{
		MMatchChallengeQuestScenario *pNew = new MMatchChallengeQuestScenario;
		
		pNew->nMapID = elem->IntAttribute("map_id");
		strcpy(pNew->szMapName, elem->Attribute("map_name") == NULL ? "" : elem->Attribute("map_name"));
		pNew->nRewardItemID = elem->IntAttribute("reward_itemid");
		pNew->nPlayer = elem->IntAttribute("player");
		
		XMLHandle han2(elem);
		XMLElement *elem2 = han2.FirstChildElement("EXP").ToElement();
		if(elem2 != NULL)
		{
			pNew->fXP = elem2->FloatAttribute("value") * fExpRate;
			#ifdef _BOUNTY
			pNew->fBP = elem2->FloatAttribute("bp_value") * fBountyRate;
			#else
			pNew->fBP = 0.0f;
			#endif
		}
		
		XMLHandle han3(elem);
		XMLElement *elem3 = han3.FirstChildElement("SECTORLIST").FirstChildElement("SECTOR").ToElement();
		while(elem3 != NULL)
		{
			MMatchChallengeQuestSector sector;
			
			XMLHandle han4(elem3);
			XMLElement *elem4 = han4.FirstChildElement("ACTOR").ToElement();
			while(elem4 != NULL)
			{
				if(elem4->Attribute("name") == NULL)
				{
					elem4 = elem4->NextSiblingElement();
					continue;
				}
				
				MMatchChallengeQuestSpawnActor actor;
				strcpy(actor.szActorName, elem4->Attribute("name"));
				actor.nCount = elem4->IntAttribute("num");
				actor.nInc = elem4->IntAttribute("inc");
				actor.nPos = elem4->IntAttribute("pos");
				actor.bAdjustPlayerNum = elem4->BoolAttribute("adjustplayernum");
				
				sector.vtActor.push_back(actor);
				elem4 = elem4->NextSiblingElement();
			}
			
			pNew->vtSectors.push_back(sector);
			elem3 = elem3->NextSiblingElement();
		}
		
		m_vtScenario.push_back(pNew);
		elem = elem->NextSiblingElement();
	}
	
	mlog("---> End challenge quest scenario load.");
	
	return true;
}

MMatchChallengeQuestScenario *MMatchChallengeQuest::GetScenario(const char *pszMapName)
{
	for(vector<MMatchChallengeQuestScenario *>::iterator i = m_vtScenario.begin(); i != m_vtScenario.end(); i++)
	{
		MMatchChallengeQuestScenario *pCurr = (*i);
		if(MStricmp(pCurr->szMapName, pszMapName) == 0) return pCurr;
	}
	return NULL;
}

void MMatchChallengeQuest::ClearScenario()
{
	for(vector<MMatchChallengeQuestScenario *>::iterator i = m_vtScenario.begin(); i != m_vtScenario.end(); i++)
	{
		delete (*i);
	}
	m_vtScenario.clear();
}