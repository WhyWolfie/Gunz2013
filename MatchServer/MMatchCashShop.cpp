#include "pch.h"
#include "MMatchCashShop.h"

#include "MMatchItem.h"

MMatchCashShop g_CashShop;

int ParseCashShopItemFilterString(const char *pszFilter)
{
	if(MStricmp(pszFilter, "head") == 0)
		return CASHSHOP_ITEMFILTER_HEAD;
	else if(MStricmp(pszFilter, "chest") == 0)
		return CASHSHOP_ITEMFILTER_CHEST;
	else if(MStricmp(pszFilter, "hand") == 0)
		return CASHSHOP_ITEMFILTER_HAND;
	else if(MStricmp(pszFilter, "legs") == 0)
		return CASHSHOP_ITEMFILTER_LEGS;
	else if(MStricmp(pszFilter, "feet") == 0)
		return CASHSHOP_ITEMFILTER_FEET;
	else if(MStricmp(pszFilter, "ring") == 0)
		return CASHSHOP_ITEMFILTER_RING;
	else if(MStricmp(pszFilter, "avatar") == 0)
		return CASHSHOP_ITEMFILTER_AVATAR;
	else if(MStricmp(pszFilter, "melee") == 0)
		return CASHSHOP_ITEMFILTER_MELEE;
	else if(MStricmp(pszFilter, "range") == 0)
		return CASHSHOP_ITEMFILTER_RANGE;
	else if(MStricmp(pszFilter, "custom") == 0)
		return CASHSHOP_ITEMFILTER_CUSTOM;
	else if(MStricmp(pszFilter, "quest") == 0)
		return CASHSHOP_ITEMFILTER_QUEST;
	else if(MStricmp(pszFilter, "gamble") == 0)
		return CASHSHOP_ITEMFILTER_GAMBLE;
	else if(MStricmp(pszFilter, "enchant") == 0)
		return CASHSHOP_ITEMFILTER_ENCHANT;
	else if(MStricmp(pszFilter, "set") == 0)
		return CASHSHOP_ITEMFILTER_SET;
	else if(MStricmp(pszFilter, "etc") == 0)
		return CASHSHOP_ITEMFILTER_ETC;
		
	return CASHSHOP_ITEMFILTER_MAX;	// error or unknown.
}


MCashShopBlob::MCashShopBlob(int nShopItemID, const char *pszItemName)
{
	Reset(nShopItemID, pszItemName);
}

MCashShopBlob::~MCashShopBlob()
{
}

void MCashShopBlob::Reset(int nShopItemID, const char *pszItemName)
{
	m_nShopItemID = nShopItemID;
	
	ZeroInit(m_szItemName, sizeof(m_szItemName));
	strcpy(m_szItemName, pszItemName);
	
	m_vtRentPeriod.clear();
	m_vtItemID.clear();
}

void MCashShopBlob::AddPeriod(int nHour, int nPrice)
{
	_periodinfo info = {nHour, nPrice};
	m_vtRentPeriod.push_back(info);
}

void MCashShopBlob::AddItem(int nItemID)
{
	m_vtItemID.push_back(nItemID);
}

void MCashShopBlob::Build(unsigned char *pOutData, int *pOutSize)
{
	// TODO : add set item support.
	
	
	int nIndex = sizeof(int);	// size header contains header itself size. start from index 4 (int).
	
	// shop item id.
	(*(int *)&pOutData[nIndex]) = m_nShopItemID;
	nIndex += sizeof(int);
	
	// name - 64 bytes.
	memcpy(&pOutData[nIndex], m_szItemName, sizeof(char) * 64);
	nIndex += sizeof(char) * 64;
	
	// rent period count.
	(*(int *)&pOutData[nIndex]) = (int)m_vtRentPeriod.size();
	nIndex += sizeof(int);
	
	// ?.
	(*(int *)&pOutData[nIndex]) = 1;
	nIndex += sizeof(int);
	
	// ?.
	(*(int *)&pOutData[nIndex]) = 0;
	nIndex += sizeof(int);
	
	// ?.
	(*(int *)&pOutData[nIndex]) = 4;
	nIndex += sizeof(int);
	
	// rent periods.
	for(vector<_periodinfo>::iterator i = m_vtRentPeriod.begin(); i != m_vtRentPeriod.end(); i++)
	{
		_periodinfo *curr = &(*i);
		
		(*(int *)&pOutData[nIndex]) = curr->nHour;
		nIndex += sizeof(int);
		
		(*(int *)&pOutData[nIndex]) = curr->nPrice;
		nIndex += sizeof(int);
	}
	
	// item ids.
	for(vector<int>::iterator i = m_vtItemID.begin(); i != m_vtItemID.end(); i++)
	{
		int id = (*i);
		
		(*(int *)&pOutData[nIndex]) = id;
		nIndex += sizeof(int);
	}
	
	// finalize size header.
	(*(int *)&pOutData[0]) = nIndex;
	
	// output size.
	(*pOutSize) = nIndex;
}


MMatchCashShopItem::MMatchCashShopItem(int nItemID, const char *pszItemName, int nCashPrice, int nItemCount, const int *pItemSetID, int nItemFilter)
{
	m_nID = nItemID;
	strcpy(m_szName, pszItemName);
	m_nPrice = nCashPrice;
	m_nQuantity = nItemCount;
	for(int i = 0; i < CASHITEM_SET_MAX; i++) m_nSetID[i] = pItemSetID[i];
	m_nFilter = nItemFilter;
	
	// cash shop internal item ID.
	static unsigned short nShopItemID = 1;
	m_nShopID = nShopItemID++;
	
	// ZItem info. if this returned NULL, the server may crash in future.
	m_pItem = g_ItemMgr.Get(nItemID);
	if(m_pItem == NULL)
	{
		mlog("FATAL ERROR [MMatchCashShopItem::MMatchCashShopItem()] - item info not found! (ID : %d)", nItemID);
	}
}

MMatchCashShopItem::~MMatchCashShopItem()
{
}

bool MMatchCashShopItem::IsValidShopSex(int nShopSex)
{
	switch(m_pItem->GetSex())
	{
		case (int)MMIS_ALL	:
			return true;
			
		case (int)MMIS_MALE	:
			if(nShopSex == CASHSHOP_SEX_MALE) return true;
		break;
		
		case (int)MMIS_FEMALE	:
			if(nShopSex == CASHSHOP_SEX_FEMALE) return true;
		break;
	}
	
	return false;
}


MMatchCashShop::MMatchCashShop()
{
}

MMatchCashShop::~MMatchCashShop()
{
	DeleteAll();
}

bool MMatchCashShop::Load()
{
	XMLDocument doc;
	if(doc.LoadFile("cashshop.xml") != XML_NO_ERROR) 
	{
		mlog("Couldn't load cash shop item info.");
		return false;
	}
	
	mlog("Cash shop item info list loading...");
	
	XMLHandle handle(&doc);
	XMLElement *element = handle.FirstChildElement("XML").FirstChildElement("SELL").ToElement();
	
	while(element != NULL)
	{
		int nItemID;
		char szItemName[64] = "";
		int nPrice;
		int nQuantity;
		int nItemSetID[CASHITEM_SET_MAX] = {0, 0, 0, 0, 0};
		char szItemFilter[32] = "";
		
		if(element->QueryIntAttribute("itemid", &nItemID) != XML_NO_ERROR)
		{
			mlog("Skipping an invalid Cash Item...");
			
			element = element->NextSiblingElement();
			continue;
		}
		
		if(element->Attribute("name") != NULL)
		{
			strcpy(szItemName, element->Attribute("name"));
		}
		
		if(element->QueryIntAttribute("price", &nPrice) != XML_NO_ERROR)
		{
			mlog("Skipping an invalid price set Cash Item...");
			
			element = element->NextSiblingElement();
			continue;
		}
		
		if(element->QueryIntAttribute("quantity", &nQuantity) != XML_NO_ERROR)
		{
			nQuantity = 1;
		}
		
		if(element->Attribute("setid") != NULL)
		{
			char szScanFormat[256] = "";
			
			for(int i = 0; i < CASHITEM_SET_MAX; i++)
				strcat(szScanFormat, "%d;");
				
			if(sscanf(element->Attribute("setid"), szScanFormat, 
				&nItemSetID[CASHITEM_SET_HEAD], 
				&nItemSetID[CASHITEM_SET_CHEST], 
				&nItemSetID[CASHITEM_SET_HAND], 
				&nItemSetID[CASHITEM_SET_LEGS], 
				&nItemSetID[CASHITEM_SET_FEET]
			  ) != CASHITEM_SET_MAX)
			{
				mlog("Skipping an invalid bulk purchase Cash Item...");
			
				element = element->NextSiblingElement();
				continue;
			}
		}
		
		if(element->Attribute("filter") != NULL)
		{
			strcpy(szItemFilter, element->Attribute("filter"));
		}
		
		MMatchCashShopItem *pNewItem = new MMatchCashShopItem(nItemID, szItemName, nPrice, nQuantity, nItemSetID, ParseCashShopItemFilterString(szItemFilter));
		Add(pNewItem);
		
		element = element->NextSiblingElement();
	}
	
	Organize(m_vtShopItem);
	
	mlog("Loaded cash shop info success.");
	
	return true;
}

void MMatchCashShop::Add(MMatchCashShopItem *pShopItem)
{
	m_vtShopItem.push_back(pShopItem);
}

MMatchCashShopItem *MMatchCashShop::Get(int nShopItemID)
{
	for(vector<MMatchCashShopItem *>::iterator i = m_vtShopItem.begin(); i != m_vtShopItem.end(); i++)
	{
		MMatchCashShopItem *pCurr = (*i);
		if((int)pCurr->m_nShopID == nShopItemID) return pCurr;
	}
	
	return NULL;
}

void MMatchCashShop::DeleteAll()
{
	for(vector<MMatchCashShopItem *>::iterator i = m_vtShopItem.begin(); i != m_vtShopItem.end(); i++)
	{
		delete (*i);
	}
	
	m_vtShopItem.clear();
}

void MMatchCashShop::Organize(vector<MMatchCashShopItem *> &vtShopItem)
{
	vector<MMatchCashShopItem *> vtTemp;
	
	// sort to head, chest, hand, ..... set, etc.
	for(int i = CASHSHOP_ITEMFILTER_HEAD; i < CASHSHOP_ITEMFILTER_MAX; i++)
	{
		vector<MMatchCashShopItem *>::iterator j = vtShopItem.begin();
		
		while(j != vtShopItem.end())
		{
			MMatchCashShopItem *pCurr = (*j);
			
			if(pCurr->m_nFilter != i)
			{
				j++;
				continue;
			}
			
			vtTemp.push_back(pCurr);
			
			j = vtShopItem.erase(j);
		}
	}
	
	// push remained node.
	for(vector<MMatchCashShopItem *>::iterator i = vtShopItem.begin(); i != vtShopItem.end(); i++)
	{
		vtTemp.push_back(*i);
	}
	
	vtShopItem.clear();
	
	// update original value.
	vtShopItem = vtTemp;
}

void MMatchCashShop::MakeItemList(int nSex, int nSortType, vector<MMatchCashShopItem *> *pOut)
{
	(*pOut) = m_vtShopItem;
	
	SexFiltering(nSex, pOut);
	Sort(nSortType, pOut);
	
	Organize(*pOut);
}

void MMatchCashShop::SexFiltering(int nSex, vector<MMatchCashShopItem *> *pDest)
{
	 vector<MMatchCashShopItem *>::iterator i = pDest->begin();
	 
	 while(i != pDest->end())
	 {
		 if((*i)->IsValidShopSex(nSex) == true)
		 {
			 i++;
			 continue;
		 }
		 
		 i = pDest->erase(i);
	 }
}

void MMatchCashShop::Sort(int nSortType, vector<MMatchCashShopItem *> *pDest)
{
	if(nSortType == CASHSHOP_SORT_NEWEST)
	{
		// sort by newest.
		for(int i = 0; i < (int)pDest->size() - 1; i++)
		{
			for(int j = i + 1; j < (int)pDest->size(); j++)
			{
				if(((*pDest)[i])->m_pItem->GetID() > ((*pDest)[j])->m_pItem->GetID())
				{
					MMatchCashShopItem *temp = ((*pDest)[i]);
					((*pDest)[i]) = ((*pDest)[j]);
					((*pDest)[j]) = temp;
				}
			}
		}
	}
	else if(nSortType == CASHSHOP_SORT_LEVEL)
	{
		// sort by level.
		for(int i = 0; i < (int)pDest->size() - 1; i++)
		{
			for(int j = i + 1; j < (int)pDest->size(); j++)
			{
				if(((*pDest)[i])->m_pItem->GetLevel() > ((*pDest)[j])->m_pItem->GetLevel())
				{
					MMatchCashShopItem *temp = ((*pDest)[i]);
					((*pDest)[i]) = ((*pDest)[j]);
					((*pDest)[j]) = temp;
				}
			}
		}
	}
}

void MMatchCashShop::SendItemList(const MUID &uidPlayer, int nSex, int nSortType)
{
	vector<MMatchCashShopItem *> vtItemList;
	MakeItemList(nSex, nSortType, &vtItemList);
	
	MCmdWriter Cmd;
	
	Cmd.WriteUChar((unsigned char)nSex);
	Cmd.WriteUChar((unsigned char)nSortType);
	
	Cmd.StartBlob(sizeof(MTD_CashShopItemID));
	for(vector<MMatchCashShopItem *>::iterator i = vtItemList.begin(); i != vtItemList.end(); i++)
	{
		MTD_CashShopItemID id;
		id.nID = (*i)->m_nShopID;
		
		Cmd.WriteData(&id, sizeof(MTD_CashShopItemID));
	}
	Cmd.EndBlob();
	
	MTD_CashShopItem shopitem;
	
	// ALL : just everything.
	shopitem.node[CASHSHOP_ITEMFILTER_ALL].nStartIndex = 0;
	shopitem.node[CASHSHOP_ITEMFILTER_ALL].nItemCount = (int)vtItemList.size();
	
	for(int i = CASHSHOP_ITEMFILTER_HEAD; i < CASHSHOP_ITEMFILTER_MAX; i++)
	{
		int nStartIndex = -1;
		int nItemCount = 0;
		
		for(int j = 0; j < (int)vtItemList.size(); j++)
		{
			if(vtItemList[j]->m_nFilter == i)
			{
				// set startindex once (when -1).
				if(nStartIndex == -1) nStartIndex = j;
				nItemCount++;
			}
		}
		
		shopitem.node[i].nStartIndex = nStartIndex == -1 ? 0 : nStartIndex;
		shopitem.node[i].nItemCount = nItemCount;
	}
	
	Cmd.WriteBlob(&shopitem, sizeof(MTD_CashShopItem));
	
	Cmd.Finalize(MC_CASHSHOP_RESPONSE_ITEMLIST, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
}

void MMatchCashShop::SendItemInfo(const MUID &uidPlayer, int nSex, int nSortType, int nStartIndex, int nCount)
{
	vector<MMatchCashShopItem *> vtItemList;
	MakeItemList(nSex, nSortType, &vtItemList);
	
	MCmdWriter Cmd;
	
	Cmd.WriteUChar((unsigned char)nSex);
	Cmd.WriteUChar((unsigned char)nSortType);
	
	Cmd.WriteInt(nStartIndex);
	
	Cmd.StartBlob();
	for(int i = nStartIndex; i < (int)vtItemList.size() && nCount > 0; i++)
	{
		MMatchCashShopItem *pCurr = vtItemList[i];
		
		// prepare blob builder.
		MCashShopBlob blob(pCurr->m_nShopID, pCurr->m_szName);
		
		// add rent period.
		if(pCurr->m_pItem->IsSpendable() == true)
		{
			// spendable item : unlimited.
			blob.AddPeriod(0, pCurr->m_nPrice);
		}
		else
		{
			blob.AddPeriod(72, pCurr->m_nPrice * 3);	// 3 day.
			blob.AddPeriod(120, pCurr->m_nPrice * 5);	// 5 day.
			blob.AddPeriod(168, pCurr->m_nPrice * 7);	// 7 day.
			blob.AddPeriod(240, pCurr->m_nPrice * 10);	// 10 day.
			blob.AddPeriod(360, pCurr->m_nPrice * 15);	// 15 day.
			blob.AddPeriod(480, pCurr->m_nPrice * 20);	// 20 day.
			blob.AddPeriod(720, pCurr->m_nPrice * 30);	// 30 day.
		}
		
		// set item id.
		blob.AddItem(pCurr->m_nID);
		
		// build blob data.
		unsigned char nData[1024];
		int nDataSize;
		
		blob.Build(nData, &nDataSize);
		
		// write to command.
		Cmd.WriteData(nData, nDataSize);
		
		nCount--;
	}
	Cmd.EndBlob();
	
	Cmd.Finalize(MC_CASHSHOP_RESPONSE_ITEMINFO, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
}