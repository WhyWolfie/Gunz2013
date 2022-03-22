#include "pch.h"
#include "MMatchQuest.h"

#include "MMatchDBMgr.h"
#include "MAsyncDBProcess.h"

#include "MCommandBlob.h"

MMatchQuest g_Quest;

// ----------------------------------- .

MQuestNPCDropItem::MQuestNPCDropItem(int nNPCID)
{
	m_nNPCID = nNPCID;
}

void MQuestNPCDropItem::Add(int nItemID, bool bZItem, int nRentHourPeriod)
{
	MQuestNPCDropItemNode node = {nItemID, bZItem, nRentHourPeriod};
	m_vtNode.push_back(node);
}

MQuestNPCDropItemNode *MQuestNPCDropItem::GetRandomItem()
{
	if(m_vtNode.size() == 0) return NULL;
	
	int nRand = (int)(RandNum() % (unsigned int)m_vtNode.size());
	return &m_vtNode[nRand];
}

// ----------------------------------- .

MMatchQuest::MMatchQuest()
{
	m_nNextSurvivalRankingUpdateTime = 0;
}

MMatchQuest::~MMatchQuest()
{
	Destroy();
}

bool MMatchQuest::LoadNPCInfo()
{
	XMLDocument doc;
	
	if(doc.LoadFile(MXML_QUEST_NPC_FILENAME) != XML_SUCCESS)
	{
		mlog("%s loading failure.", MXML_QUEST_NPC_FILENAME);
		return false;
	}
	
	mlog("----> Loading %s ...", MXML_QUEST_NPC_FILENAME);
	
	XMLElement *element;
	XMLHandle handle(&doc);
	
	element = handle.FirstChildElement("XML").FirstChildElement("NPC").ToElement();
	while(element != NULL)
	{
		MTD_NPCINFO info;
		ZeroInit(&info, sizeof(MTD_NPCINFO));
		
		info.nNPCTID = (unsigned char)element->IntAttribute("id");
		info.nMaxHP = (unsigned short)element->IntAttribute("max_hp");
		info.nMaxAP = (unsigned short)element->IntAttribute("max_ap");
		info.nInt = (unsigned char)element->IntAttribute("int");
		info.nAgility = (unsigned char)element->IntAttribute("agility");
		info.fAngle = element->FloatAttribute("view_angle") * (3.141592f / 180.0f);
		
		info.fDyingTime = element->FloatAttribute("dyingtime");
		if(info.fDyingTime == 0.0f)
		{
			info.fDyingTime = 5.0f;
		}
		
		XMLElement *collision = element->FirstChildElement("COLLISION");
		if(collision != NULL)
		{
			info.fCollisonRadius = collision->FloatAttribute("radius");
			info.fCollisonHight = collision->FloatAttribute("height");
		}
		
		XMLElement *attack = element->FirstChildElement("ATTACK");
		if(attack != NULL)
		{
			const char *pszTypeAttr = attack->Attribute("type") == NULL ? "" : attack->Attribute("type");
			
			// NOTE : I don't have zskill.xml loaded - enable magic flag to all.
			if(MStricmp(pszTypeAttr, "melee") == 0)			info.nAttackType = MQUEST_NPC_ATTACK_TYPE_MELEE | MQUEST_NPC_ATTACK_TYPE_MAGIC;
			else if(MStricmp(pszTypeAttr, "range") == 0)	info.nAttackType = MQUEST_NPC_ATTACK_TYPE_RANGE | MQUEST_NPC_ATTACK_TYPE_MAGIC;
			else if(MStricmp(pszTypeAttr, "magic") == 0)	info.nAttackType = MQUEST_NPC_ATTACK_TYPE_MAGIC;
			else										info.nAttackType = MQUEST_NPC_ATTACK_TYPE_NONE | MQUEST_NPC_ATTACK_TYPE_MAGIC;
			
			info.fAttackRange = attack->FloatAttribute("range");
			info.nWeaponItemID = (unsigned long)attack->UnsignedAttribute("weaponitem_id");
		}
		
		XMLElement *speed = element->FirstChildElement("SPEED");
		if(speed != NULL)
		{
			info.fDefaultSpeed = speed->FloatAttribute("default");
		}
		
		m_vtNPCInfo.push_back(info);
		
		element = element->NextSiblingElement();
	}
	
	mlog("----> Loading done.");
	
	return true;
}

bool MMatchQuest::LoadScenario(const char *pszFileName)
{
	XMLDocument doc;
	
	if(doc.LoadFile(pszFileName) != XML_SUCCESS)
	{
		mlog("Fail to open %s .", pszFileName);
		return false;
	}
	
	mlog("----> Loading %s ...", pszFileName);
	
	XMLElement *element;
	XMLHandle handle(&doc);
	
	// load Exp, BP rate that need to be increased.
	
	float fExpRate, fBountyRate;
	
	element = handle.FirstChildElement("XML").FirstChildElement("EXP").ToElement();
	if(element != NULL)
	{
		if(element->QueryFloatAttribute("rate", &fExpRate) != XML_SUCCESS)
		{
			mlog("Invalid EXP rate setting.");
			return false;
		}
		
		if(element->QueryFloatAttribute("bp_rate", &fBountyRate) != XML_SUCCESS)
		{
			mlog("Invalid BP rate setting.");
			return false;
		}
	}
	else
	{
		mlog("All of EXP rate setting is invalid.");
		return false;
	}
	
	// Load main scenario.
	
	element = handle.FirstChildElement("XML").FirstChildElement("SCENARIO").ToElement();
	while(element != NULL)
	{
		MQuestScenario *pNew = new MQuestScenario;
		ZeroInit(pNew, sizeof(MQuestScenario));
		
		pNew->nID = element->IntAttribute("id");
		pNew->nQL = element->IntAttribute("lv");
		strcpy(pNew->szMapName, element->Attribute("map") == NULL ? "" : element->Attribute("map"));
		pNew->nExp = (int)((float)element->IntAttribute("xp") * fExpRate);
		#ifdef _BOUNTY
		pNew->nBounty = (int)((float)element->IntAttribute("bp") * fBountyRate);
		#else
		pNew->nBounty = 0;
		#endif
		pNew->nRound = element->IntAttribute("round");
		if(element->QueryIntAttribute("round_repeat", &pNew->nRepeat) != XML_SUCCESS)
		{
			pNew->nRepeat = 1;
		}
		pNew->nNPCCount = element->IntAttribute("npc_count");
		
		XMLHandle handle2(element);
		
		XMLElement *sitem = handle2.FirstChildElement("SACRIFICE_ITEM").FirstChildElement("ITEM").ToElement();
		for(int i = 0; sitem != NULL; i++)
		{
			pNew->nQuestItemID[i] = sitem->IntAttribute("id");
			sitem = sitem->NextSiblingElement();
		}
		
		XMLElement *map = handle2.FirstChildElement("MAP").ToElement();
		if(map != NULL)
		{
			pNew->bRandomSector = map->BoolAttribute("random");
			
			XMLElement *sector = map->FirstChildElement("SECTOR");
			while(sector != NULL)
			{
				pNew->vtSector.push_back(sector->IntAttribute("id"));
				sector = sector->NextSiblingElement();
			}
		}
		
		XMLElement *npcset = handle2.FirstChildElement("NPCSET").FirstChildElement("ROUND").ToElement();
		while(npcset != NULL)
		{
			MQuestRound round;
			round.nBaseNPCID = npcset->IntAttribute("base_npcid");
			round.bInfinite = npcset->BoolAttribute("infinite");
			
			XMLElement *npc = npcset->FirstChildElement("NPC");
			while(npc != NULL)
			{
				MQuestRoundNPCNode node;
				node.nNPCID = npc->IntAttribute("id");
				node.fRate = npc->FloatAttribute("rate");
				node.bBoss = npc->BoolAttribute("boss");
				
				round.NPCList.push_back(node);
				npc = npc->NextSiblingElement();
			}
			
			pNew->vtRound.push_back(round);
			npcset = npcset->NextSiblingElement();
		}
		
		m_vtScenario.push_back(pNew);
		
		element = element->NextSiblingElement();
	}
	
	mlog("----> Load successfully.");
	
	return true;
}

bool MMatchQuest::LoadNPCDropItem()
{
	XMLDocument doc;
	
	if(doc.LoadFile(MXML_QUEST_NPC_DROPITEM_FILENAME) != XML_SUCCESS)
	{
		mlog("%s load failure.", MXML_QUEST_NPC_DROPITEM_FILENAME);
		return false;
	}
	
	mlog("----> Quest NPC Dropitem list loading...");
	
	XMLElement *element;
	XMLHandle handle(&doc);
	
	element = handle.FirstChildElement("XML").FirstChildElement("DROPSET").ToElement();
	while(element != NULL)
	{
		MQuestNPCDropItem item(element->IntAttribute("npc_id"));
		
		XMLElement *element2 = element->FirstChildElement("ITEM");
		while(element2 != NULL)
		{
			item.Add(element2->IntAttribute("id"), element2->BoolAttribute("zitem"), element2->IntAttribute("rent_period_hour"));
			element2 = element2->NextSiblingElement();
		}
		
		m_vtNPCDropItem.push_back(item);
		element = element->NextSiblingElement();
	}
	
	mlog("----> Done.");
	
	return true;
}

MQuestScenario *MMatchQuest::GetScenario(int nID)
{
	for(vector<MQuestScenario *>::iterator i = m_vtScenario.begin(); i != m_vtScenario.end(); i++)
	{
		MQuestScenario *pScenario = (*i);
		if(pScenario->nID == nID) return pScenario;
	}
	return NULL;
}

int MMatchQuest::GetScenarioIDByItemID(const int *id /* int id[SACRIITEM_SLOT_COUNT] */, const char *pszMapName)
{
	// scenario search.
	for(vector<MQuestScenario *>::iterator i = m_vtScenario.begin(); i != m_vtScenario.end(); i++)
	{
		MQuestScenario *pScenario = (*i);
		if(MStricmp(pScenario->szMapName, pszMapName) != 0) continue;
		
		// copy itemid.
		int nItemID[SACRIITEM_SLOT_COUNT];
		for(int i = 0; i < SACRIITEM_SLOT_COUNT; i++)
		{
			nItemID[i] = id[i];
		}
		
		bool bError = false;	// will be true if no scenario.
		
		// check itemid match.
		for(int j = 0; j < SACRIITEM_SLOT_COUNT; j++)	// scenario pre-defined slot.
		{
			bool bMatch = false;
			
			for(int k = 0; k < SACRIITEM_SLOT_COUNT; k++)	// passed slot.
			{
				if(pScenario->nQuestItemID[j] == nItemID[k])
				{
					nItemID[k] = -1;	// set to invalid id = do not check this item for twice or more time.
					
					bMatch = true;
					break;
				}
			}
			
			// mismatch with this scenario itemid.
			if(bMatch == false)
			{
				bError = true;
				break;
			}
		}
		
		// this itemid is for this scenario.
		if(bError == false) return pScenario->nID;
	}
	
	return 0;
}

bool MMatchQuest::InitializeGameRound(int nScenarioID, vector<MMatchGame_QuestRound> *pOut)
{
	pOut->clear();
	
	MQuestScenario *pScenario = GetScenario(nScenarioID);
	if(pScenario == NULL) return false;
	
	for(vector<MQuestRound>::iterator i = pScenario->vtRound.begin(); i != pScenario->vtRound.end(); i++)
	{
		MQuestRound *pCurrRound = &(*i);
		
		MMatchGame_QuestRound info;
		info.bInfinite = pCurrRound->bInfinite;
		
		for(list<MQuestRoundNPCNode>::iterator j = pCurrRound->NPCList.begin(); j != pCurrRound->NPCList.end(); j++)
		{
			MQuestRoundNPCNode *pNPCNode = &(*j);
			
			if(pNPCNode->bBoss == true)
			{
				info.BossNPCIDs.push_back(pNPCNode->nNPCID);
			}
			else
			{
				int nInsertCount = (int)((float)pScenario->nNPCCount * pNPCNode->fRate);
				
				if(nInsertCount == 0)
				{
					bool bRand = RandNum(0, 1) != 0 ? true : false ;
					
					if(bRand == true)
					{
						info.NPCIDs.push_back(pNPCNode->nNPCID);
					}
				}
				else
				{
					while(nInsertCount > 0)
					{
						info.NPCIDs.push_back(pNPCNode->nNPCID);
						nInsertCount--;
					}
				}
			}
		}
		
		while((int)info.NPCIDs.size() < pScenario->nNPCCount)
		{
			info.NPCIDs.push_back(pCurrRound->nBaseNPCID);
		}
		
		// shuffle npc.
		for(int j = 0; j < (int)info.NPCIDs.size(); j++)
		{
			int nRand = (int)(RandNum() % (unsigned int)info.NPCIDs.size());
			
			int nTemp = info.NPCIDs[j];
			info.NPCIDs[j] = info.NPCIDs[nRand];
			info.NPCIDs[nRand] = nTemp;
		}
		
		pOut->push_back(info);
	}
	
	return true;
}

bool MMatchQuest::BuildNPCInfoListCommand(int nScenarioID, int nGameType, MCmdWriter *pOut)
{
	MQuestScenario *pScenario = GetScenario(nScenarioID);
	if(pScenario == NULL) return false;
	
	vector<int> NPCIDs;	// npc ids without duplicate.
	
	for(vector<MQuestRound>::iterator i = pScenario->vtRound.begin(); i != pScenario->vtRound.end(); i++)
	{
		MQuestRound *pCurrRound = &(*i);
		
		if(find(NPCIDs.begin(), NPCIDs.end(), pCurrRound->nBaseNPCID) == NPCIDs.end())
			NPCIDs.push_back(pCurrRound->nBaseNPCID);
		
		for(list<MQuestRoundNPCNode>::iterator j = pCurrRound->NPCList.begin(); j != pCurrRound->NPCList.end(); j++)
		{
			MQuestRoundNPCNode *pNPCNode = &(*j);
			
			if(find(NPCIDs.begin(), NPCIDs.end(), pNPCNode->nNPCID) == NPCIDs.end())
				NPCIDs.push_back(pNPCNode->nNPCID);
		}
	}
	
	pOut->StartBlob(sizeof(MTD_NPCINFO));
	
	for(vector<MTD_NPCINFO>::iterator i = m_vtNPCInfo.begin(); i != m_vtNPCInfo.end(); i++)
	{
		MTD_NPCINFO *pInfo = &(*i);
		
		if(find(NPCIDs.begin(), NPCIDs.end(), (int)pInfo->nNPCTID) != NPCIDs.end())
		{
			pOut->WriteData(pInfo, sizeof(MTD_NPCINFO));
		}
	}
	
	pOut->EndBlob();
	
	pOut->WriteInt(nGameType);
	pOut->Finalize(MC_QUEST_NPCLIST, MCFT_END);
	
	return true;
}

bool MMatchQuest::BuildGameInfoCommand(int nScenarioID, int nGameType, MCmdWriter *pOut)
{
	MQuestScenario *pScenario = GetScenario(nScenarioID);
	if(pScenario == NULL) return false;
	
	vector<int> NPCIDs;	// npc ids without duplicate.
	
	for(vector<MQuestRound>::iterator i = pScenario->vtRound.begin(); i != pScenario->vtRound.end(); i++)
	{
		MQuestRound *pCurrRound = &(*i);

		if(find(NPCIDs.begin(), NPCIDs.end(), pCurrRound->nBaseNPCID) == NPCIDs.end())
			NPCIDs.push_back(pCurrRound->nBaseNPCID);
		
		for(list<MQuestRoundNPCNode>::iterator j = pCurrRound->NPCList.begin(); j != pCurrRound->NPCList.end(); j++)
		{
			MQuestRoundNPCNode *pNPCNode = &(*j);
			
			if(find(NPCIDs.begin(), NPCIDs.end(), pNPCNode->nNPCID) == NPCIDs.end())
				NPCIDs.push_back(pNPCNode->nNPCID);
		}
	}
	
	MTD_QuestGameInfo info;
	ZeroInit(&info, sizeof(MTD_QuestGameInfo));
	
	info.nQL = (unsigned short)pScenario->nQL;
	info.fNPC_TC = (0.3f * (float)pScenario->nQL) + 1.0f;
	info.nNPCCount = (unsigned short)pScenario->nNPCCount;
	
	if(nGameType == (int)MMGT_SURVIVAL)
	{
		// survival : fix remain NPC count display, increase 1 (= for boss).
		// by default, the survival mode NPC is 30. but for this, it will be 31. seems weird but ok...
		info.nNPCCount++;
	}
	
	info.nNPCInfoCount = (unsigned char)NPCIDs.size();
	
	int nNPCIndex = 0;
	for(vector<int>::iterator i = NPCIDs.begin(); i != NPCIDs.end(); i++)
	{
		info.nNPCInfo[nNPCIndex] = (unsigned char)(*i);
		nNPCIndex++;
	}
	
	/*
	info.nMapSectorCount = (unsigned short)pScenario->nRound;
	for(int i = 0; i < pScenario->nRound; i++)
	{
		if(pScenario->bRandomSector == false)
		{
			info.nMapSectorID[i] = (unsigned short)pScenario->vtSector[i];
		}
		else
		{
			int nRand = (int)(RandNum() % (unsigned int)pScenario->vtSector.size());
			info.nMapSectorID[i] = (unsigned short)pScenario->vtSector[nRand];
		}
	}
	*/
	
	info.nMapSectorCount = (unsigned short)pScenario->nRound;
	if(nGameType == (int)MMGT_QUEST)
	{
		int nLastSectorID = pScenario->bRandomSector == true ? 
			pScenario->vtSector[RandNum() % (unsigned int)pScenario->vtSector.size()] : 
			pScenario->vtSector.back();
		int nLastBacklinkIndex = 0;
		
		for(int i = pScenario->nRound - 1; i >= 0; i--)
		{
			info.nMapSectorID[i] = (unsigned short)nLastSectorID;
			info.nMapSectorLinkIndex[i] = (char)nLastBacklinkIndex;
		
			GetSuitableSectorInfo(nLastSectorID, &nLastSectorID, &nLastBacklinkIndex);
		}
	}
	else
	{
		for(int i = 0; i < pScenario->nRound; i++)
		{
			if(pScenario->bRandomSector == false)
			{
				info.nMapSectorID[i] = (unsigned short)pScenario->vtSector[i];
			}
			else
			{
				int nRand = (int)(RandNum() % (unsigned int)pScenario->vtSector.size());
				info.nMapSectorID[i] = (unsigned short)pScenario->vtSector[nRand];
			}
		}
	}
	
	info.nRepeat = pScenario->nRepeat;
	info.eGameType = nGameType;
	
	pOut->StartBlob(sizeof(MTD_QuestGameInfo));
	pOut->WriteData(&info, sizeof(MTD_QuestGameInfo));
	pOut->EndBlob();
	pOut->Finalize(MC_QUEST_GAME_INFO, MCFT_END);
	
	return true;
}

int MMatchQuest::GetRandomNPC(int nScenarioID, int nRound)	// nRound ; 1R, 2R, 3R will be 0, 1, 2 ...
{
	MQuestScenario *pScenario = GetScenario(nScenarioID);
	if(pScenario == NULL) return 0;
	
	float fRandRate = RandNum(0.01f, 1.0f);
	
	float fCurr = 0.0f;
	for(list<MQuestRoundNPCNode>::iterator i = pScenario->vtRound[nRound].NPCList.begin(); i != pScenario->vtRound[nRound].NPCList.end(); i++)
	{
		MQuestRoundNPCNode *pNPCNode = &(*i);
		
		fCurr += pNPCNode->fRate;
		if(fCurr >= fRandRate) return pNPCNode->nNPCID;
	}
	
	return pScenario->vtRound[nRound].nBaseNPCID;
}

int MMatchQuest::GetSurvivalScenarioID(const char *pszMapName)
{
	// hard-coded ids.
	
	if(MStricmp(pszMapName, "Mansion") == 0)		return 20001;
	else if(MStricmp(pszMapName, "Prison") == 0)	return 20002;
	else if(MStricmp(pszMapName, "Dungeon") == 0)	return 20003;
	
	return 0;
}

MQuestNPCDropItemNode *MMatchQuest::GetRandomNPCItem(int nNPCID)
{
	for(vector<MQuestNPCDropItem>::iterator i = m_vtNPCDropItem.begin(); i != m_vtNPCDropItem.end(); i++)
	{
		MQuestNPCDropItem *pCurr = &(*i);
		if(pCurr->GetNPCID() == nNPCID) return pCurr->GetRandomItem();
	}
	return NULL;
}

void MMatchQuest::Destroy()
{
	for(vector<MQuestScenario *>::iterator i = m_vtScenario.begin(); i != m_vtScenario.end(); i++)
	{
		delete (*i);
	}
	m_vtScenario.clear();
}

void MMatchQuest::Run(unsigned long nTime)
{
	if(m_nNextSurvivalRankingUpdateTime > nTime) return;
	
	UpdateSurvivalRanking();
	m_nNextSurvivalRankingUpdateTime = nTime + 86400000;	// 1 day.
}

bool MMatchQuest::FetchSurvivalRanking()
{
	/*
	vector<DbData_SurvivalRankingInfo> vtRankingInfo;
	
	if(Db_FetchSurvivalRanking(&vtRankingInfo) == false)
	{
		printf("Fetch survival ranking failed...\n");
		mlog("DB error while fetching survival ranking : check your database.");
		return false;
	}
	
	m_vtSurvivalRanking.clear();
	
	// convert to MTD type.
	for(vector<DbData_SurvivalRankingInfo>::iterator i = vtRankingInfo.begin(); i != vtRankingInfo.end(); i++)
	{
		DbData_SurvivalRankingInfo *pCurr = &(*i);
		
		MTD_SurvivalRanking info;
		ZeroInit(&info, sizeof(MTD_SurvivalRanking));
		
		strcpy(info.szCharName, pCurr->szCharName);
		info.nPoint = (unsigned long)pCurr->nPoint;
		info.nRank = (unsigned long)pCurr->nRanking;
		
		m_vtSurvivalRanking.push_back(info);
	}
	
	printf("Survival ranking fetched.\n");
	mlog("Fetch survival ranking success.");
	*/
	
	AsyncDb_FetchSurvivalRanking();
	
	return true;
}

void MMatchQuest::OnFetchSurvivalRanking(vector<DbData_SurvivalRankingInfo> &vtRankingInfo)
{
	m_vtSurvivalRanking.clear();
	
	// convert to MTD type.
	for(vector<DbData_SurvivalRankingInfo>::iterator i = vtRankingInfo.begin(); i != vtRankingInfo.end(); i++)
	{
		DbData_SurvivalRankingInfo *pCurr = &(*i);
		
		MTD_SurvivalRanking info;
		ZeroInit(&info, sizeof(MTD_SurvivalRanking));
		
		strcpy(info.szCharName, pCurr->szCharName);
		info.nPoint = (unsigned long)pCurr->nPoint;
		info.nRank = (unsigned long)pCurr->nRanking;
		
		m_vtSurvivalRanking.push_back(info);
	}
	
	printf("Survival ranking fetched.\n");
	mlog("Fetch survival ranking success.");
}

bool MMatchQuest::UpdateSurvivalRanking()
{
	/*
	if(Db_UpdateSurvivalRanking() == false)
	{
		printf("Can't update survival ranking.\n");
		mlog("DB error while updating survival ranking : check your database.");
		return false;
	}
	*/
	
	AsyncDb_UpdateSurvivalRanking();
	
	// printf("Survival ranking updated.\n");
	// mlog("Success update survival ranking.");
	
	return FetchSurvivalRanking();
}

void MMatchQuest::SendSurvivalRankingInfo(const MUID &uidStage)
{
	MCmdWriter Cmd;
	Cmd.StartBlob(sizeof(MTD_SurvivalRanking));
	for(vector<MTD_SurvivalRanking>::iterator i = m_vtSurvivalRanking.begin(); i != m_vtSurvivalRanking.end(); i++)
	{
		Cmd.WriteData(&(*i), sizeof(MTD_SurvivalRanking));
	}
	Cmd.EndBlob();
	Cmd.Finalize(MC_SURVIVAL_RANKING_LIST, MCFT_END);
	SendToStage(&Cmd, uidStage);
}

void MMatchQuest::GetSuitableSectorInfo(int to, int *from, int *index)
{
	int nFrom = -1, nIndex = 0;
	
	switch(to)
	{
		// Mansion. ------------
		case 101	:
		{
			switch(RandNum() % 4)
			{
				case 0	:
				{
					nFrom = 102;
					nIndex = 0;
				}
				break;
				case 1	:
				{
					nFrom = 105;
					nIndex = 0;
				}
				break;
				case 2	:
				{
					nFrom = 107;
					nIndex = 2;
				}
				break;
				case 3	:
				{
					nFrom = 109;
					nIndex = 2;
				}
				break;
			}
		}
		break;
		case 102	:
		{
			switch(RandNum() % 5)
			{
				case 0	:
				{
					nFrom = 101;
					nIndex = 0;
				}
				break;
				case 1	:
				{
					nFrom = 104;
					nIndex = 0;
				}
				break;
				case 2	:
				{
					nFrom = 105;
					nIndex = 0;
				}
				break;
				case 3 	:
				{
					nFrom = 107;
					nIndex = 2;
				}
				break;
				case 4	:
				{
					nFrom = 109;
					nIndex = 2;
				}
				break;
			}
		}
		break;
		case 103	:
		{
			switch(RandNum() % 3)
			{
				case 0	:
				{
					nFrom = 107;
					nIndex = 1;
				}
				break;
				case 1	:
				{
					nFrom = 108;
					nIndex = 0;
				}
				break;
				case 2	:
				{
					nFrom = 109;
					nIndex = 1;
				}
				break;
			}
		}
		break;
		case 104	:
		{
			switch(RandNum() % 4)
			{
				case 0	:
				{
					nFrom = 102;
					nIndex = 0;
				}
				break;
				case 1	:
				{
					nFrom = 105;
					nIndex = 0;
				}
				break;
				case 2	:
				{
					nFrom = 107;
					nIndex = 2;
				}
				break;
				case 3	:
				{
					nFrom = 109;
					nIndex = 2;
				}
				break;
			}
		}
		break;
		case 105	:
		{
			switch(RandNum() % 4)
			{
				case 0	:
				{
					nFrom = 102;
					nIndex = 0;
				}
				break;
				case 1	:
				{
					nFrom = 104;
					nIndex = 0;
				}
				break;
				case 2	:
				{
					nFrom = 107;
					nIndex = 2;
				}
				break;
				case 3	:
				{
					nFrom = 109;
					nIndex = 2;
				}
				break;
			}
		}
		break;
		case 106	:
		{
			nFrom = 108;
			nIndex = 2;
		}
		break;
		case 107	:
		{
			switch(RandNum() % 6)
			{
				case 0	:
				{
					nFrom = 101;
					nIndex = 0;
				}
				break;
				case 1	:
				{
					nFrom = 102;
					nIndex = 0;
				}
				break;
				case 2	:
				{
					nFrom = 103;
					nIndex = 1;
				}
				break;
				case 3	:
				{
					nFrom = 103;
					nIndex = 2;
				}
				break;
				case 4	:
				{
					nFrom = 105;
					nIndex = 0;
				}
				break;
				case 5	:
				{
					nFrom = 108;
					nIndex = 1;
				}
				break;
			}
		}
		break;
		case 108	:
		{
			switch(RandNum() % 4)
			{
				case 0	:
				{
					nFrom = 103;
					nIndex = 0;
				}
				break;
				case 1	:
				{
					nFrom = 106;
					nIndex = 0;
				}
				break;
				case 2	:
				{
					nFrom = 107;
					nIndex = 0;
				}
				break;
				case 3	:
				{
					nFrom = 109;
					nIndex = 0;
				}
				break;
			}
		}
		break;
		case 109	:
		{
			switch(RandNum() % 2)
			{
				case 0	:
				{
					nFrom = 103;
					nIndex = 1;
				}
				break;
				case 1	:
				{
					nFrom = 108;
					nIndex = 1;
				}
				break;
			}
		}
		break;
		
		// Prison. ------------
		case 201	:
		{
			nFrom = 209;
			nIndex = 0;
		}
		break;
		case 202	:
		{
			nFrom = 201;
			nIndex = 0;
		}
		break;
		case 203	:
		{
			nFrom = 202;
			nIndex = 0;
		}
		break;
		case 204	:
		{
			nFrom = 203;
			nIndex = 0;
		}
		break;
		case 205	:
		{
			nFrom = 204;
			nIndex = 0;
		}
		break;
		case 206	:
		{
			nFrom = 205;
			nIndex = 0;
		}
		break;
		case 207	:
		{
			nFrom = 206;
			nIndex = 0;
		}
		break;
		case 208	:
		{
			nFrom = 207;
			nIndex = 0;
		}
		break;
		case 209	:
		{
			nFrom = 208;
			nIndex = 0;
		}
		break;
		
		// Dungeon. ------------
		case 301	:
		{
			nFrom = 309;
			nIndex = 0;
		}
		break;
		case 302	:
		{
			nFrom = 301;
			nIndex = 0;
		}
		break;
		case 303	:
		{
			nFrom = 302;
			nIndex = 0;
		}
		break;
		case 304	:
		{
			nFrom = 303;
			nIndex = 0;
		}
		break;
		case 305	:
		{
			nFrom = 304;
			nIndex = 0;
		}
		break;
		case 306	:
		{
			nFrom = 305;
			nIndex = 0;
		}
		break;
		case 307	:
		{
			nFrom = 306;
			nIndex = 0;
		}
		break;
		case 308	:
		{
			nFrom = 307;
			nIndex = 0;
		}
		break;
		case 309	:
		{
			nFrom = 308;
			nIndex = 0;
		}
		break;
	}
	
	(*from) = nFrom;
	(*index) = nIndex;
}
