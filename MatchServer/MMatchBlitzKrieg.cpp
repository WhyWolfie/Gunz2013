#include "pch.h"
#include <time.h>

#include <set>
using namespace std;

#include "MMatchBlitzKrieg.h"

#include "MMatchObject.h"

#include "MMatchDBMgr.h"
#include "MAsyncDBProcess.h"

// from channel.
void SendChannelPlayerList(const MUID &uidChannel);

// from stage.
void SendObjectCache(int nCacheType, MMatchStage *pSrcStage, const MUID &uidDestPlayer = MUID(0, 0));
void CreateStageSettingCommand(MMatchStage *pStage, MCommandWriter *pOut);

void ReserveStageAgent(const MUID &uidStage);


// group. ------------------
MMatchBlitzKriegGroup::MMatchBlitzKriegGroup(bool bAssignID)
{
	for(int i = 0; i < BLITZ_TEAM_PLAYER_COUNT; i++)
	{
		m_pObj[i] = NULL;
	}
	
	if(bAssignID == true)
		m_nID = AssignID();
}

unsigned long MMatchBlitzKriegGroup::GetID()
{
	return m_nID;
}

bool MMatchBlitzKriegGroup::AddObject(MMatchObject *pObj)
{
	if(CheckObjectExists(pObj) == true) return false;
	
	for(int i = 0; i < BLITZ_TEAM_PLAYER_COUNT; i++)
	{
		if(m_pObj[i] == NULL)
		{
			m_pObj[i] = pObj;
			return true;
		}
	}
	
	return false;
}

bool MMatchBlitzKriegGroup::CheckObjectExists(MMatchObject *pFindObj)
{
	for(int i = 0; i < BLITZ_TEAM_PLAYER_COUNT; i++)
	{
		if(m_pObj[i] == pFindObj) return true;
	}
	
	return false;
}

int MMatchBlitzKriegGroup::GetObjectCount()
{
	int nResult = 0;
	
	for(int i = 0; i < BLITZ_TEAM_PLAYER_COUNT; i++)
	{
		if(m_pObj[i] != NULL)
			nResult++;
	}
	
	return nResult;
}

unsigned long MMatchBlitzKriegGroup::AssignID()
{
	static unsigned long nCurrID = 1;
	
	nCurrID++;
	if(nCurrID == 0) nCurrID = 1;	// don't set to invalid zero id.
	
	return nCurrID;
}

// queue mgr.  ------------------
MMatchBlitzKriegQueueManager g_BKQMgr;

MMatchBlitzKriegQueueManager::MMatchBlitzKriegQueueManager()
{
	m_nNextProcessTime = 0;
}

MMatchBlitzKriegQueueManager::~MMatchBlitzKriegQueueManager()
{
	for(list<MMatchBlitzKriegGroup *>::iterator i = m_GroupQueue.begin(); i != m_GroupQueue.end(); i++)
	{
		delete (*i);
	}
	m_GroupQueue.clear();
}

void MMatchBlitzKriegQueueManager::Add(MMatchBlitzKriegGroup* pGroup)
{
	if(Get(pGroup->GetID()) != NULL) return;
	
	// create challenger list command.
	MCmdWriter Cmd;
	Cmd.StartBlob(sizeof(MTD_BlitzWaitingPlayerNode));
	for(int i = 0; i < BLITZ_TEAM_PLAYER_COUNT; i++)
	{
		MMatchObject *pCurrObj = pGroup->GetObjects()[i];
		if(pCurrObj == NULL) continue;
		
		MTD_BlitzWaitingPlayerNode node;
		ZeroInit(&node, sizeof(MTD_BlitzWaitingPlayerNode));
		
		strcpy(node.szPlayerName, strlen(pCurrObj->m_Char.szName) <= 12 ? pCurrObj->m_Char.szName : " ");
		node.nPlayerLevel = pCurrObj->m_Exp.nLevel;
		node.nPlayerWin = (unsigned short)pCurrObj->m_BlitzKrieg.nWin;
		node.nPlayerLose = (unsigned short)pCurrObj->m_BlitzKrieg.nLose;
		node.nPlayerMedal = pCurrObj->m_BlitzKrieg.nMedal;
		
		Cmd.WriteData(&node, sizeof(MTD_BlitzWaitingPlayerNode));
	}
	Cmd.EndBlob();
	Cmd.Finalize(MC_BLITZ_WAITING_CHALLENGER_LIST, MCFT_END);
	
	// send command & set group id.
	for(int i = 0; i < BLITZ_TEAM_PLAYER_COUNT; i++)
	{
		MMatchObject *pCurrObj = pGroup->GetObjects()[i];
		if(pCurrObj == NULL) continue;
		
		pCurrObj->m_nBlitzGroupID = pGroup->GetID();
		pCurrObj->StartMedalBonus();
		
		SendToClient(&Cmd, pCurrObj->GetUID());
	}
	
	m_GroupQueue.push_back(pGroup);
}

void MMatchBlitzKriegQueueManager::Remove(unsigned long nGroupID, const char *pszCancellerName)
{
	for(list<MMatchBlitzKriegGroup *>::iterator i = m_GroupQueue.begin(); i != m_GroupQueue.end(); i++)
	{
		MMatchBlitzKriegGroup *pCurrGroup = (*i);
		
		if(pCurrGroup->GetID() == nGroupID)
		{
			// if canceller name is NULL, the name is overwrited with "Server" but not sent.
			MCmdWriter Cmd;
			Cmd.WriteString(pszCancellerName != NULL ? pszCancellerName : "Server");
			Cmd.Finalize(MC_BLITZ_CANCEL_CHALLENGE, MCFT_END);
			
			for(int j = 0; j < BLITZ_TEAM_PLAYER_COUNT; j++)
			{
				MMatchObject *pCurrObj = pCurrGroup->GetObjects()[j];
				if(pCurrObj == NULL) continue;
				
				pCurrObj->m_nBlitzGroupID = 0;
				pCurrObj->StopMedalBonus();
				
				if(pszCancellerName != NULL)
					SendToClient(&Cmd, pCurrObj->GetUID());
			}
			
			delete pCurrGroup;
			m_GroupQueue.erase(i);
			
			break;
		}
	}
}

MMatchBlitzKriegGroup* MMatchBlitzKriegQueueManager::Get(unsigned long nGroupID)
{
	for(list<MMatchBlitzKriegGroup *>::iterator i = m_GroupQueue.begin(); i != m_GroupQueue.end(); i++)
	{
		MMatchBlitzKriegGroup *pCurr = (*i);
		if(pCurr->GetID() == nGroupID) return pCurr;
	}
	return NULL;
}

void MMatchBlitzKriegQueueManager::ProcessGroup(unsigned long nTime)
{
	#ifdef _DEBUG
		#define BLITZKRIEG_GROUP_QUEUE_PROCESS_INTERVAL	1000	// 1 sec.
	#else
		#define BLITZKRIEG_GROUP_QUEUE_PROCESS_INTERVAL	30000	// 30 sec.
	#endif
	
	if(m_nNextProcessTime > nTime) return;
	
	deque<MUID> PlayerChannelUIDs;
	
	while(1)
	{
		// team array index number.
		#define RED_TEAM	0
		#define BLUE_TEAM	1
		#define MAX_TEAM	2
		
		bool bSuccess = false;
		
		vector<MMatchBlitzKriegGroup *> vtGroup[MAX_TEAM];
		int nMemberCount[MAX_TEAM] = {0, 0};
		
		// decide match team members.
		for(list<MMatchBlitzKriegGroup *>::iterator i = m_GroupQueue.begin(); i != m_GroupQueue.end(); i++)
		{
			MMatchBlitzKriegGroup *pGroup = (*i);
			
			int nGroupObjCount = pGroup->GetObjectCount();
			
			for(int i = 0; i < MAX_TEAM; i++)
			{
				int nTotal = nMemberCount[i] + nGroupObjCount;
				
				if(nTotal <= BLITZ_TEAM_PLAYER_COUNT)
				{
					vtGroup[i].push_back(pGroup);
					nMemberCount[i] = nTotal;
					break;
				}
			}
			
			if(nMemberCount[RED_TEAM] >= BLITZ_TEAM_PLAYER_COUNT && nMemberCount[BLUE_TEAM] >= BLITZ_TEAM_PLAYER_COUNT)
			{
				bSuccess = true;
				break;
			}
		}
		
		// can't make team.
		if(bSuccess == false) break;
		
		// begin create stage.
		MMatchMap *pMap = g_MapMgr.GetMapFromName(DEFAULT_BLITZKRIEG_MAP_NAME, MMMST_GENERAL);
		if(pMap == NULL) 
		{
			mlog("BlitzKrieg match launch failed! - There is no map named '%s'. (Please set-up your map info correctly.)", DEFAULT_BLITZKRIEG_MAP_NAME);
			break;
		}
		
		MMatchStage *pNewStage = g_StageMgr.Create(0);
		pNewStage->Create(MUID(0, 0), 0, "Blitzkrieg", "pwd");
		
		pNewStage->SetMap(pMap);
		pNewStage->SetGameType((int)MMGT_BLITZKRIEG);
		pNewStage->SetMaxPlayer(BLITZ_PLAYER_COUNT);
		pNewStage->SetRound(1);
		pNewStage->SetTimeLimit(0);
		pNewStage->SetForcedEntry(false);
		
		// set object info & delete group info.
		for(int i = 0; i < MAX_TEAM; i++)
		{
			for(vector<MMatchBlitzKriegGroup *>::iterator j = vtGroup[i].begin(); j != vtGroup[i].end(); j++)
			{
				MMatchBlitzKriegGroup *pCurrGroup = (*j);
				
				for(int k = 0; k < BLITZ_TEAM_PLAYER_COUNT; k++)
				{
					MMatchObject *pCurrObj = pCurrGroup->GetObjects()[k];
					if(pCurrObj == NULL) continue;
					
					int nTeam = MMT_ALL;
					if(i == RED_TEAM) 		nTeam = MMT_RED;
					else if(i == BLUE_TEAM)	nTeam = MMT_BLUE;
					
					pCurrObj->m_GameInfo.nTeam = nTeam;
					
					pCurrObj->m_uidStage = pNewStage->GetUID();
					pNewStage->Enter(pCurrObj);
					
					pCurrObj->m_nGameFlag = MMOGF_INGAME | MMOGF_LAUNCHED;
					pCurrObj->m_nStageState = MOSS_WAIT;
					
					pCurrObj->m_nPlace = MMP_STAGE;
					
					pCurrObj->ReserveStageKick(60000);	// 1 min.
					
					if(find(PlayerChannelUIDs.begin(), PlayerChannelUIDs.end(), pCurrObj->m_uidChannel) == PlayerChannelUIDs.end())
					{
						PlayerChannelUIDs.push_back(pCurrObj->m_uidChannel);
					}
				}
				
				Remove(pCurrGroup->GetID());
			}
		}
		
		// 1 : stage setting.
		MCmdWriter CmdStageSetting;
		CreateStageSettingCommand(pNewStage, &CmdStageSetting);
		SendToStage(&CmdStageSetting, pNewStage->GetUID());
		
		// 2. reserve agent.
		ReserveStageAgent(pNewStage->GetUID());
		
		// 3, game start command.
		MCmdWriter CmdStartGame;
		if(CheckGameVersion(2013) == true)
		{
			CmdStartGame.WriteInt(0);
			CmdStartGame.Finalize(MC_MATCH_STAGE_START);
		}
		else
		{
			CmdStartGame.WriteMUID(MUID(0, 0));
			CmdStartGame.WriteMUID(pNewStage->GetUID());
			CmdStartGame.WriteInt(0);
			CmdStartGame.Finalize(MC_MATCH_STAGE_START);
		}
		CmdStartGame.Finalize();
		SendToStage(&CmdStartGame, pNewStage->GetUID());
		
		// 4, obj cache.
		SendObjectCache((int)MMOCT_UPDATE, pNewStage);
		
		// 5, launch match.
		MCmdWriter CmdLaunchMatch;
		CmdLaunchMatch.WriteMUID(pNewStage->GetUID());
		CmdLaunchMatch.WriteString(pMap->szName);
		CmdLaunchMatch.Finalize(MC_BLITZ_GAME_LAUNCH, MCFT_END);
		SendToStage(&CmdLaunchMatch, pNewStage->GetUID());
		
		// 6, run game.
		pNewStage->SetState((int)MMSS_RUN);
		
		if(pNewStage->CreateGame() == true)
		{
			// after game created : do nothing.
		}
		else
		{
			// error...
			mlog("Error while launching BlitzKrieg match stage...");
			
			pNewStage->DestroyGame();
			g_StageMgr.Remove(pNewStage);
			break;
		}
	}
	
	while(PlayerChannelUIDs.size() != 0)
	{
		SendChannelPlayerList(PlayerChannelUIDs.front());
		PlayerChannelUIDs.pop_front();
	}
	
	m_nNextProcessTime = nTime + BLITZKRIEG_GROUP_QUEUE_PROCESS_INTERVAL;
}

// ------------------------------------
#define BLITZKRIEG_SHOP_RESTOCK_TIME_FILENAME	"./blitzkrieg_shop.conf"

MBlitzShop g_BlitzShop;

MBlitzShop::MBlitzShop()
{
	m_nNextRestockTime = 0;
}

MBlitzShop::~MBlitzShop()
{
	ClearSellingItem();
	ClearAllItem();
}

bool MBlitzShop::LoadXML()
{
	XMLDocument doc;
	if(doc.LoadFile("blitzshop.xml") != XML_SUCCESS) 
	{
		mlog("Error : could not read Blitz medal shop items.");
		return false;
	}
	
	mlog("Loading BlitzKrieg medal shop items...");
	
	XMLHandle handle(&doc);
	XMLElement *element = handle.FirstChildElement("XML").FirstChildElement("SELL").ToElement();
	
	while(element != NULL)
	{
		MBlitzShopItem *pNew = new MBlitzShopItem(element->IntAttribute("itemid"), 
												  element->FloatAttribute("stockrate"), 
												  element->IntAttribute("rentperiod"), 
												  element->IntAttribute("price"), 
												  element->FloatAttribute("maxdc"), 
												  element->FloatAttribute("dcrate"), 
												  element->BoolAttribute("coupon"));
		m_vtItem.push_back(pNew);
		
		element = element->NextSiblingElement();
	}
	
	mlog("Medal shop has been successfully initialized.");
	
	return true;
}

bool MBlitzShop::LoadFromDB()
{
	// sync item list with db.
	
	vector<DbData_BlitzShopItem> vtDbItem;
	if(Db_GetBlitzShopItem(&vtDbItem) == false) return false;
	
	set<int> CIDs;
	
	for(vector<DbData_BlitzShopItem>::iterator i = vtDbItem.begin(); i != vtDbItem.end(); i++)
	{
		DbData_BlitzShopItem *pItem = &(*i);
		if(CIDs.find(pItem->nCID) == CIDs.end()) CIDs.insert(pItem->nCID);
	}
	
	for(set<int>::iterator i = CIDs.begin(); i != CIDs.end(); i++)
	{
		int nCID = (*i);
		vector<MBlitzShopSellingItem *> vtSellingItem;
		
		for(vector<DbData_BlitzShopItem>::iterator j = vtDbItem.begin(); j != vtDbItem.end(); j++)
		{
			DbData_BlitzShopItem *pItem = &(*j);
			
			if(pItem->nCID == nCID)
			{
				MBlitzShopSellingItem *pNewItem = new MBlitzShopSellingItem(pItem->nItemID, 
																			pItem->nPrice, 
																			pItem->nBasePrice, 
																			pItem->nCount, 
																			pItem->nRentHourPeriod);
				vtSellingItem.push_back(pNewItem);
			}
		}
		
		if(nCID == MBLITZSHOP_CID_ALL)
		{
			m_vtSellingItem = vtSellingItem;
		}
		else
		{
			vector<MBlitzShopSellingItem *> *pNew = new vector<MBlitzShopSellingItem *>;
			*pNew = vtSellingItem;
			
			m_CharSellingItem.insert(pair<int, vector<MBlitzShopSellingItem *> *>(nCID, pNew));
		}
	}
	
	return true;
}

void MBlitzShop::Run()
{
	time_t nNowTime = time(NULL);
	
	if(m_nNextRestockTime <= nNowTime)
	{
		SyncDBData_Remove();
		
		ClearSellingItem();
		Restock(&m_vtSellingItem);
		
		SyncDBData_Add(MBLITZSHOP_CID_ALL, m_vtSellingItem);
		
		m_nNextRestockTime = MakeNextRestockTime();
		SaveRestockTime();
		
		SendItemList(MUID(0, 0), MBLITZSHOP_CID_ALL);
		
		printf("Medal shop has restocked.\n");
		mlog("Blitz shop is now restocked. Next restock will be %ld.", m_nNextRestockTime);
	}
}

bool MBlitzShop::LoadRestockTime()
{
	FILE *pFile = fopen(BLITZKRIEG_SHOP_RESTOCK_TIME_FILENAME, "r+");
	if(pFile == NULL)
	{
		mlog("Failed to open blitz shop config file. "
			 "Create an empty file named as %s, and give read/write permission to it.", 
			 BLITZKRIEG_SHOP_RESTOCK_TIME_FILENAME);
		return false;
	}
	
	time_t nRestockTime;
	if(fscanf(pFile, "%ld", &nRestockTime) != 1)
	{
		fputs("0", pFile);
	}
	
	m_nNextRestockTime = nRestockTime;
	
	fclose(pFile);
	return true;
}

bool MBlitzShop::SaveRestockTime()
{
	FILE *pFile = fopen(BLITZKRIEG_SHOP_RESTOCK_TIME_FILENAME, "w");
	if(pFile == NULL)
	{
		mlog("Failed to open %s file for write. Couldn't save config.", 
			 BLITZKRIEG_SHOP_RESTOCK_TIME_FILENAME);
		return false;
	}
	
	if(fprintf(pFile, "%ld", m_nNextRestockTime) < 0)
	{
		fclose(pFile);
		return false;
	}
	
	fclose(pFile);
	return true;
}

time_t MBlitzShop::MakeNextRestockTime()
{
	time_t nNowTime = time(NULL);
	
	tm TimeInfo = *localtime(&nNowTime);
	
	TimeInfo.tm_sec = 0;
	TimeInfo.tm_min = 0;
	TimeInfo.tm_hour = 0;
	
	time_t nNextTime = mktime(&TimeInfo) + 86400;	// + 1 day.
	
	return nNextTime;
}

MBlitzShopItem *MBlitzShop::GetItem(int nItemID)
{
	for(vector<MBlitzShopItem *>::iterator i = m_vtItem.begin(); i != m_vtItem.end(); i++)
	{
		MBlitzShopItem *pCurr = (*i);
		if(pCurr->m_nItemID == nItemID) return pCurr;
	}
	
	return NULL;
}

MBlitzShopSellingItem *MBlitzShop::GetSellingItem(int nItemID, int nCID)
{
	if(nCID != MBLITZSHOP_CID_ALL)
	{
		for(map<int, vector<MBlitzShopSellingItem *> *>::iterator i = m_CharSellingItem.begin(); i != m_CharSellingItem.end(); i++)
		{
			if((*i).first == nCID)
			{
				vector<MBlitzShopSellingItem *> *pvtSellingItem = (*i).second;
				
				for(vector<MBlitzShopSellingItem *>::iterator j = pvtSellingItem->begin(); j != pvtSellingItem->end(); j++)
				{
					MBlitzShopSellingItem *pCurrItem = (*j);
					if(pCurrItem->m_nItemID == nItemID) return pCurrItem;
				}
				
				return NULL;
			}
		}
	}
	
	for(vector<MBlitzShopSellingItem *>::iterator i = m_vtSellingItem.begin(); i != m_vtSellingItem.end(); i++)
	{
		MBlitzShopSellingItem *pCurrItem = (*i);
		if(pCurrItem->m_nItemID == nItemID) return pCurrItem;
	}
	
	return NULL;
}

bool MBlitzShop::Restock(vector<MBlitzShopSellingItem *> *pOut)
{
	Unstock(pOut);
	
	for(vector<MBlitzShopItem *>::iterator i = m_vtItem.begin(); i != m_vtItem.end(); i++)
	{
		MBlitzShopItem *pItem = (*i);
		if(pItem->IsSellable() == false) continue;
		
		if(pItem->m_fStockRate < RandNum(0.0f, 1.0f)) continue;
		
		float fDiscount = 0.0f;
		if(pItem->m_fDiscountRate != 0.0f)
		{
			if(pItem->m_fDiscountRate >= RandNum(0.0f, 1.0f))
			{
				fDiscount = RandNum(0.0f, pItem->m_fMaxDiscount);
			}
		}
		
		int nRealPrice = (int)((float)pItem->m_nPrice * (1.0f - fDiscount));
		
		MBlitzShopSellingItem *pNewItem = new MBlitzShopSellingItem(pItem->m_nItemID, 
																	nRealPrice, 
																	pItem->m_nPrice, 
																	pItem->m_nQuantity, 
																	pItem->m_nRentHourPeriod);
		pOut->push_back(pNewItem);
	}
	
	return true;
}

void MBlitzShop::Unstock(vector<MBlitzShopSellingItem *> *pDest)
{
	for(vector<MBlitzShopSellingItem *>::iterator i = pDest->begin(); i != pDest->end(); i++)
	{
		delete (*i);
	}
	
	pDest->clear();
}

bool MBlitzShop::Restock(int nCID)
{
	if(nCID == MBLITZSHOP_CID_ALL) return false;
	
	SyncDBData_Remove(nCID);
	
	// find for existing stock.
	for(map<int, vector<MBlitzShopSellingItem *> *>::iterator i = m_CharSellingItem.begin(); i != m_CharSellingItem.end(); i++)
	{
		if((*i).first == nCID)
		{
			Restock((*i).second);
			SyncDBData_Add(nCID, *(*i).second);
			return true;
		}
	}
	
	// add new stock.
	vector<MBlitzShopSellingItem *> *pNew = new vector<MBlitzShopSellingItem *>;
	Restock(pNew);
	
	m_CharSellingItem.insert(pair<int, vector<MBlitzShopSellingItem *> *>(nCID, pNew));
	
	SyncDBData_Add(nCID, *pNew);
	
	return true;
}

void MBlitzShop::SendItemList(const MUID &uidPlayer, int nCID)
{
	vector<MBlitzShopSellingItem *> vtSellingItem;
	EnumSellingItems(nCID, &vtSellingItem);
	
	MCmdWriter Cmd;
	
	int nIndex = 0;
	
	Cmd.StartBlob(sizeof(MTD_BlitzShopItemNode));
	for(vector<MBlitzShopSellingItem *>::iterator i = vtSellingItem.begin(); i != vtSellingItem.end(); i++)
	{
		MBlitzShopSellingItem *pCurr = (*i);
		
		MTD_BlitzShopItemNode node;
		node.nIndex = nIndex;
		node.nItemID = pCurr->m_nItemID;
		node.nRentHourPeriod = pCurr->m_nRentHourPeriod;
		node.nPrice = pCurr->m_nPrice;
		node.fPriceDown = (float)(((float)pCurr->m_nPrice / 4.0f) * (pCurr->CalcDCPercent() + 1.0f));
		node.nCount = pCurr->m_nCount;
		
		Cmd.WriteData(&node, sizeof(MTD_BlitzShopItemNode));
		
		nIndex++;
	}
	Cmd.EndBlob();
	
	Cmd.Finalize(MC_MATCH_RESPONSE_BLITZSHOP_ITEMLIST, MCFT_END);
	
	if(uidPlayer == MUID(0, 0))
	{
		SendToAll(&Cmd);
	}
	else
	{
		SendToClient(&Cmd, uidPlayer);
	}
}

void MBlitzShop::EnumSellingItems(int nCID, vector<MBlitzShopSellingItem *> *pOut)
{
	if(nCID != MBLITZSHOP_CID_ALL)
	{
		for(map<int, vector<MBlitzShopSellingItem *> *>::iterator i = m_CharSellingItem.begin(); i != m_CharSellingItem.end(); i++)
		{
			if((*i).first == nCID)
			{
				vector<MBlitzShopSellingItem *> *pvtSellingItem = (*i).second;
				
				for(vector<MBlitzShopSellingItem *>::iterator j = pvtSellingItem->begin(); j != pvtSellingItem->end(); j++)
				{
					pOut->push_back(*j);
				}
				
				return;
			}
		}
	}
	
	for(vector<MBlitzShopSellingItem *>::iterator i = m_vtSellingItem.begin(); i != m_vtSellingItem.end(); i++)
	{
		pOut->push_back(*i);
	}
	
	return;
}

void MBlitzShop::SyncDBData_Add(int nCID, vector<MBlitzShopSellingItem *> &vtSellingItem)
{
	for(vector<MBlitzShopSellingItem *>::iterator i = vtSellingItem.begin(); i != vtSellingItem.end(); i++)
	{
		MBlitzShopSellingItem *pCurrItem = (*i);
		AsyncDb_AddBlitzShopItem(nCID, pCurrItem->m_nItemID, pCurrItem->m_nPrice, pCurrItem->m_nBasePrice, pCurrItem->m_nCount, pCurrItem->m_nRentHourPeriod);
	}
}

void MBlitzShop::SyncDBData_Remove(int nCID)
{
	AsyncDb_ClearBlitzShop(nCID);
}

void MBlitzShop::ClearAllItem()
{
	for(vector<MBlitzShopItem *>::iterator i = m_vtItem.begin(); i != m_vtItem.end(); i++)
	{
		delete (*i);
	}
	
	m_vtItem.clear();
}

void MBlitzShop::ClearSellingItem()
{
	Unstock(&m_vtSellingItem);
	
	for(map<int, vector<MBlitzShopSellingItem *> *>::iterator i = m_CharSellingItem.begin(); i != m_CharSellingItem.end(); i++)
	{
		vector<MBlitzShopSellingItem *> *pCurr = (*i).second;
		
		Unstock(pCurr);
		delete pCurr;
	}
	m_CharSellingItem.clear();
}