#include "pch.h"
#include "MMatchGambleItem.h"

MMatchGambleItemManager g_GItemMgr;

// reward item.
MMatchGambleRewardItemNode::MMatchGambleRewardItemNode(int nMaleItemID, int nFemaleItemID, int nRentHourPeriod, int nQuantity)
{
	SetItemID(nMaleItemID, nFemaleItemID);
	m_nRentHourPeriod = nRentHourPeriod;
	m_nQuantity = nQuantity;
}

void MMatchGambleRewardItemNode::SetItemID(int nMaleItemID, int nFemaleItemID)
{
	m_ItemID.nMaleItemID = nMaleItemID;
	m_ItemID.nFemaleItemID = nFemaleItemID;
}

// gamble item itself.
MMatchGambleItem::MMatchGambleItem(int nID, const char* pszName, const char* pszDesc, int nPrice, int nCurrencyType, bool bShop)
{
	#ifndef _BOUNTY
	if(nCurrencyType == GAMBLEITEM_CURRENCY_BOUNTY)
		nPrice = 0;
	#endif
		
	m_nID = nID;
	strcpy(m_szName, pszName);
	strcpy(m_szDesc, pszDesc);
	m_nPrice = nPrice;
	m_nCurrencyType = nCurrencyType;
	m_bShop = bShop;
}

MMatchGambleItem::~MMatchGambleItem()
{
	for(vector<MMatchGambleRewardItemNode *>::iterator i = m_vtRewardItem.begin(); i != m_vtRewardItem.end(); i++)
	{
		delete (*i);
	}
	m_vtRewardItem.clear();
}

void MMatchGambleItem::AddRewardItem(int nMaleItemID, int nFemaleItemID, int nRentHourPeriod, int nQuantity)
{
	m_vtRewardItem.push_back(new MMatchGambleRewardItemNode(nMaleItemID, nFemaleItemID, nRentHourPeriod, nQuantity));
}

MMatchGambleRewardItemNode* MMatchGambleItem::GetRandomRewardItem()
{
	if(m_vtRewardItem.size() == 0) return NULL;
	
	int nRand = (int)(RandNum() % (unsigned int)m_vtRewardItem.size());
	return m_vtRewardItem[nRand];
}

// gamble item manager.
int ParseGambleItemCurrencyTypeString(const char *pszCurrency)
{
	if(MStricmp(pszCurrency, "bounty") == 0)
		return GAMBLEITEM_CURRENCY_BOUNTY;
	else if(MStricmp(pszCurrency, "cash") == 0)
		return GAMBLEITEM_CURRENCY_CASH;
	else if(MStricmp(pszCurrency, "medal") == 0)
		return GAMBLEITEM_CURRENCY_MEDAL;
		
	return GAMBLEITEM_CURRENCY_BOUNTY;
}


bool MMatchGambleItemManager::Load()
{
	XMLDocument doc;
	if(doc.LoadFile(MXML_FILENAME_GAMBLEITEM) != XML_SUCCESS) 
	{
		mlog("Gamble items are failure to load.");
		return false;
	}
	
	mlog("Loading gamble item descriptor...");
	
	XMLHandle handle(&doc);
	
	XMLElement *element = handle.FirstChildElement("XML").FirstChildElement("GAMBLEITEM").ToElement();
	while(element != NULL)
	{
		MMatchGambleItem *pNewGItem = new MMatchGambleItem(
			element->IntAttribute("id"), 
			element->Attribute("name") == NULL ? "" : element->Attribute("name"), 
			element->Attribute("desc") == NULL ? "" : element->Attribute("desc"), 
			element->IntAttribute("price"), 
			ParseGambleItemCurrencyTypeString(element->Attribute("Currency") == NULL ? "" : element->Attribute("Currency")), 
			element->BoolAttribute("shop")
		);
		
		XMLElement *rielem = element->FirstChildElement("REWARDITEM");
		while(rielem != NULL)
		{
			int nMaleItemID = rielem->IntAttribute("maleid");
			int nFemaleItemID = rielem->IntAttribute("femaleid");
			int nRentHourPeriod = rielem->IntAttribute("rentperiod");
			int nQuantity;
			
			if(rielem->QueryIntAttribute("quantity", &nQuantity) != XML_SUCCESS)
			{
				nQuantity = 1;
			}
			
			pNewGItem->AddRewardItem(nMaleItemID, nFemaleItemID, nRentHourPeriod, nQuantity);
			
			rielem = rielem->NextSiblingElement();
		}
		
		Add(pNewGItem);
		
		element = element->NextSiblingElement();
	}
	
	mlog("Init gamble item manager success.");
	
	return true;
}

void MMatchGambleItemManager::Add(MMatchGambleItem *pGambleItem)
{
	m_vtGambleItem.push_back(pGambleItem);
}

void MMatchGambleItemManager::Remove(MMatchGambleItem *pGambleItem)
{
	for(vector<MMatchGambleItem *>::iterator i = m_vtGambleItem.begin(); i != m_vtGambleItem.end(); i++)
	{
		MMatchGambleItem *pCurr = (*i);
		
		if(pCurr == pGambleItem)
		{
			delete pCurr;
			m_vtGambleItem.erase(i);
			break;
		}
	}
}

void MMatchGambleItemManager::Clear()
{
	for(vector<MMatchGambleItem *>::iterator i = m_vtGambleItem.begin(); i != m_vtGambleItem.end(); i++)
	{
		delete (*i);
	}
	m_vtGambleItem.clear();
}

MMatchGambleItem *MMatchGambleItemManager::Get(int nItemID)
{
	for(vector<MMatchGambleItem *>::iterator i = m_vtGambleItem.begin(); i != m_vtGambleItem.end(); i++)
	{
		MMatchGambleItem *pCurr = (*i);
		
		if(pCurr->GetID() == nItemID)
		{
			return pCurr;
		}
	}
	
	return NULL;
}

void MMatchGambleItemManager::SendInfo(const MUID &uidPlayer)
{
	if(CheckGameVersion(2012) == true)
	{
		SendInfo2012(uidPlayer);
	}
	else
	{
		SendInfo2011(uidPlayer);
	}
}

void MMatchGambleItemManager::SendInfo2011(const MUID &uidPlayer)
{
	MCmdWriter Cmd;
	
	Cmd.StartBlob(sizeof(MTD_DBGambleItemNode));
	for(vector<MMatchGambleItem *>::iterator i = m_vtGambleItem.begin(); i != m_vtGambleItem.end(); i++)
	{
		MMatchGambleItem *pCurr = (*i);
		
		MTD_DBGambleItemNode node;
		ZeroInit(&node, sizeof(MTD_DBGambleItemNode));
		
		node.nItemID = (unsigned int)pCurr->GetID();
		strcpy(node.szName, pCurr->GetName());
		strcpy(node.szDesc, pCurr->GetDesc());
		node.nBuyPrice = pCurr->GetPrice();
		node.bIsCash = pCurr->IsCash();
		
		Cmd.WriteData(&node, sizeof(MTD_DBGambleItemNode));
	}
	Cmd.EndBlob();
	
	Cmd.Finalize(MC_MATCH_RESPONSE_GAMBLEITEM_LIST, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
}

void MMatchGambleItemManager::SendInfo2012(const MUID &uidPlayer)
{
	MCmdWriter Cmd;
	
	Cmd.StartBlob(sizeof(MTD_DBGambleItemNode2012));
	for(vector<MMatchGambleItem *>::iterator i = m_vtGambleItem.begin(); i != m_vtGambleItem.end(); i++)
	{
		MMatchGambleItem *pCurr = (*i);
		
		MTD_DBGambleItemNode2012 node;
		ZeroInit(&node, sizeof(MTD_DBGambleItemNode2012));
		
		node.nItemID = (unsigned int)pCurr->GetID();
		strcpy(node.szName, pCurr->GetName());
		strcpy(node.szDesc, pCurr->GetDesc());
		node.nBuyPrice = pCurr->GetPrice();
		node.nCurrencyType = pCurr->GetCurrencyType();
		
		Cmd.WriteData(&node, sizeof(MTD_DBGambleItemNode2012));
	}
	Cmd.EndBlob();
	
	Cmd.WriteULong(GetTime());
	
	Cmd.Finalize(MC_MATCH_RESPONSE_GAMBLEITEM_LIST, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
}