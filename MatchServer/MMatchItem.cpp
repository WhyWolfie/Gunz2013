#include "pch.h"
#include "MMatchItem.h"

#include "MMatchObject_Constant.h"

// ---------- currency. ----------
int GetCurrencyType(bool bCash)
{
	if(bCash == true) return ITEM_CURRENCY_CASH;
	return ITEM_CURRENCY_BOUNTY;
}

int GetCurrencyType(const char *pszCurrency)
{
	if(MStricmp(pszCurrency, "bounty") == 0)
		return ITEM_CURRENCY_BOUNTY;
	else if(MStricmp(pszCurrency, "cash") == 0)
		return ITEM_CURRENCY_CASH;
	else if(MStricmp(pszCurrency, "medal") == 0)
		return ITEM_CURRENCY_MEDAL;
		
	return ITEM_CURRENCY_BOUNTY;	// defaulted to bounty.
}

// ---------- item. ----------
MMatchItem::MMatchItem(int nID, int nLevel, int nWeight, int nMaxWt, int nPrice, int nSex, int nSlotType, int nRentPeriod, int nCurrencyType, bool bSpendable)
{
	Create(nID, nLevel, nWeight, nMaxWt, nPrice, nSex, nSlotType, nRentPeriod, nCurrencyType, bSpendable);
}

bool MMatchItem::Create(int nID, int nLevel, int nWeight, int nMaxWt, int nPrice, int nSex, int nSlotType, int nRentPeriod, int nCurrencyType, bool bSpendable)
{
	m_nID = nID;
	m_nLevel = nLevel;
	m_nWeight = nWeight;
	m_nMaxWt = nMaxWt;
	m_nPrice = nPrice;
	m_nSex = nSex;
	m_nSlotType = nSlotType;
	m_nRentPeriod = nRentPeriod;
	m_nCurrencyType = nCurrencyType;
	m_bSpendable = bSpendable;
	
	return true;
}

int GetSlotTypeFromString(const char *pszSlot)
{
	if(pszSlot == NULL) return (int)MMIST_NONE;
	
	if(MStricmp(pszSlot, "melee") == 0)
		return (int)MMIST_MELEE;
	else if(MStricmp(pszSlot, "range") == 0)
		return (int)MMIST_RANGE;
	else if(MStricmp(pszSlot, "custom") == 0)
		return (int)MMIST_CUSTOM;
	else if(MStricmp(pszSlot, "head") == 0)
		return (int)MMIST_HEAD;
	else if(MStricmp(pszSlot, "chest") == 0)
		return (int)MMIST_CHEST;
	else if(MStricmp(pszSlot, "hands") == 0)
		return (int)MMIST_HANDS;
	else if(MStricmp(pszSlot, "legs") == 0)
		return (int)MMIST_LEGS;
	else if(MStricmp(pszSlot, "feet") == 0)
		return (int)MMIST_FEET;
	else if(MStricmp(pszSlot, "finger") == 0)
		return (int)MMIST_FINGER;
	else if(MStricmp(pszSlot, "extra") == 0)
		return (int)MMIST_EXTRA;
	else if(MStricmp(pszSlot, "avatar") == 0)
		return (int)MMIST_AVATAR;
		
	return (int)MMIST_NONE;
}

int GetSexFromString(const char *pszSex)
{
	if(pszSex == NULL) return (int)MMIS_ALL;
	
	if(MStricmp(pszSex, "m") == 0)
		return (int)MMIS_MALE;
	else if(MStricmp(pszSex, "f") == 0)
		return (int)MMIS_FEMALE;
	
	return (int)MMIS_ALL;
}

bool MMatchItem::IsValidSex(int nCharSex) const
{
	if(m_nSex == MMIS_ALL) return true;
	
	switch(nCharSex)
	{
		case MMS_MALE	:
			if(m_nSex == MMIS_MALE) return true;
		break;
		
		case MMS_FEMALE	:
			if(m_nSex == MMIS_FEMALE) return true;
		break;
	}
	
	return false;
}

bool MMatchItem::IsValidSlot(int nCharParts) const
{
	switch(nCharParts)
	{
		case MMCIP_HEAD	:
			if(m_nSlotType == (int)MMIST_HEAD) return true;
		break;
		
		case MMCIP_CHEST	:
			if(m_nSlotType == (int)MMIST_CHEST) return true;
		break;
		
		case MMCIP_HANDS	:
			if(m_nSlotType == (int)MMIST_HANDS) return true;
		break;
		
		case MMCIP_LEGS	:
			if(m_nSlotType == (int)MMIST_LEGS) return true;
		break;
		
		case MMCIP_FEET	:
			if(m_nSlotType == (int)MMIST_FEET) return true;
		break;
		
		case MMCIP_FINGERL	:
		case MMCIP_FINGERR	:
			if(m_nSlotType == (int)MMIST_FINGER) return true;
		break;
		
		case MMCIP_MELEE	:
			if(m_nSlotType == (int)MMIST_MELEE) return true;
		break;
		
		case MMCIP_PRIMARY	:
		case MMCIP_SECONDARY	:
			if(m_nSlotType == (int)MMIST_RANGE) return true;
		break;
		
		case MMCIP_CUSTOM1	:
		case MMCIP_CUSTOM2	:
			if(m_nSlotType == (int)MMIST_CUSTOM) return true;
		break;
		
		case MMCIP_AVATAR	:
			if(m_nSlotType == (int)MMIST_AVATAR) return true;
		break;
	}
	
	return false;
}

// item management class.
MMatchItemManager g_ItemMgr;
MMatchShopManager g_ShopMgr;

MMatchItemManager::MMatchItemManager()
{
}

MMatchItemManager::~MMatchItemManager()
{
	ClearAll();
}

bool MMatchItemManager::LoadZItem(const char *pszFileName)
{
	XMLDocument doc;
	doc.LoadFile(pszFileName);
	
	if(doc.Error() == true)
	{
		mlog("Can't open %s file.", pszFileName);
		return false;
	}
	
	mlog("Item descriptor loading... (%s)", pszFileName);
	
	XMLHandle handle(&doc);
	handle = handle.FirstChildElement("XML");
	
	XMLElement *element = handle.FirstChildElement("ITEM").ToElement();
	
	while(element != NULL)
	{
		Add(
			element->IntAttribute("id"), 
			#ifdef _ITEM_LEVEL_LIMITATION
			element->IntAttribute("res_level"), 
			#else
			0, 
			#endif
			element->IntAttribute("weight"), 
			element->IntAttribute("maxwt"), 
			#ifdef _BOUNTY
			element->IntAttribute("bt_price"), 
			#else
			0, 
			#endif
			GetSexFromString(element->Attribute("res_sex") == NULL ? "" : element->Attribute("res_sex")), 
			GetSlotTypeFromString(element->Attribute("slot") == NULL ? "" : element->Attribute("slot")), 
			element->IntAttribute("rent_period") * 24, 	// hour period (day * hour).
			#if _GAME_VERSION >= 2012
			GetCurrencyType(element->Attribute("Currency") == NULL ? "" : element->Attribute("Currency")), 
			#else
			GetCurrencyType(element->BoolAttribute("iscashitem")), 
			#endif
			element->BoolAttribute("spendable")
		);
		element = element->NextSiblingElement();
	}
	
	mlog("%s file loaded successfully.", pszFileName);
	
	return true;
}

bool MMatchItemManager::LoadZQuestItem()
{
	XMLDocument doc;
	doc.LoadFile(MXML_FILENAME_ZQUESTITEM);
	
	if(doc.Error() == true)
	{
		mlog("Fail to open %s.", MXML_FILENAME_ZQUESTITEM);
		return false;
	}
	
	mlog("Loading Quest Item descriptor... (%s)", MXML_FILENAME_ZQUESTITEM);
	
	XMLHandle handle(&doc);
	handle = handle.FirstChildElement("XML");
	
	XMLElement *element = handle.FirstChildElement("ITEM").ToElement();
	
	while(element != NULL)
	{
		MMatchItem *pNewItem = new MMatchItem(
			element->IntAttribute("id"), 
			#ifdef _ITEM_LEVEL_LIMITATION
			element->IntAttribute("level"), 
			#else
			0, 
			#endif
			0, 
			0, 
			#ifdef _BOUNTY
			element->IntAttribute("price"), 
			#else
			0, 
			#endif
			(int)MMIS_ALL, 
			(int)MMIST_QUEST, 
			0, 
			false, 
			false
		);
		Add(pNewItem);
		
		element = element->NextSiblingElement();
	}
	
	mlog("Quest item Descriptor has been loaded.");
	
	return true;
}

void MMatchItemManager::Add(MMatchItem *pItem)
{
	m_vtItem.push_back(pItem);
}

void MMatchItemManager::Add(int nID, int nLevel, int nWeight, int nMaxWt, int nPrice, int nSex, int nSlotType, int nRentPeriod, int nCurrencyType, bool bSpendable)
{
	MMatchItem *pNewItem = new MMatchItem(nID, nLevel, nWeight, nMaxWt, nPrice, nSex, nSlotType, nRentPeriod, nCurrencyType, bSpendable);
	m_vtItem.push_back(pNewItem);
}

void MMatchItemManager::ClearAll()
{
	for(vector<MMatchItem *>::iterator i = m_vtItem.begin(); i != m_vtItem.end(); i++)
	{
		delete (*i);
	}
	m_vtItem.clear();
}

MMatchItem *MMatchItemManager::Get(int nItemID)
{
	for(vector<MMatchItem *>::iterator i = m_vtItem.begin(); i != m_vtItem.end(); i++)
	{
		MMatchItem *pCurr = (*i);
		if(pCurr->GetID() == nItemID) return pCurr;
	}
	
	return NULL;
}

// ---------- shop. ----------
MMatchShopManager::MMatchShopManager()
{
}

MMatchShopManager::~MMatchShopManager()
{
	ClearAllItem();
}

bool MMatchShopManager::Load()
{
	XMLDocument doc;
	doc.LoadFile(MXML_FILENAME_SHOP);
	
	if(doc.Error() == true) 
	{
		mlog("Fail to open Shop Item list.");
		return false;
	}
	
	mlog("Loading shop items...");
	
	XMLHandle handle(&doc);
	handle = handle.FirstChildElement("XML");
	
	XMLElement *element = handle.FirstChildElement("SELL").ToElement();
	
	while(element != NULL)
	{
		AddItem(element->IntAttribute("itemid"));
		element = element->NextSiblingElement();
	}
	
	mlog("Shop item list loading success. (%d items are collected.)", (int)m_vtShopItem.size());
	
	return true;
}

void MMatchShopManager::AddItem(int nItemID, int nItemCount)
{
	MShopItem *pNew = new MShopItem;
	pNew->nID = nItemID;
	pNew->nCount = nItemCount;
	m_vtShopItem.push_back(pNew);
}

void MMatchShopManager::ClearAllItem()
{
	for(vector<MShopItem *>::iterator i = m_vtShopItem.begin(); i != m_vtShopItem.end(); i++)
	{
		delete (*i);
	}
	m_vtShopItem.clear();
}

bool MMatchShopManager::IsSellingItem(int nItemID)
{
	for(vector<MShopItem *>::iterator i = m_vtShopItem.begin(); i != m_vtShopItem.end(); i++)
	{
		if((*i)->nID == nItemID) return true;
	}
	
	return false;
}