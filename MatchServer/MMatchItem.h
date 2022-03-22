#ifndef __MMATCHITEM_H__
#define __MMATCHITEM_H__

#define MXML_FILENAME_ZITEM			"zitem.xml"
#define MXML_FILENAME_ZITEM_LOCALE	"zitem_locale.xml"
#define MXML_FILENAME_ZQUESTITEM	"zquestitem.xml"
#define MXML_FILENAME_SHOP			"shop.xml"

// item macro.
#define RENT_PERIOD_UNLIMITED			0
#define RENT_MINUTE_PERIOD_UNLIMITED	525600

// currency type.
#define ITEM_CURRENCY_NONE		0
#define ITEM_CURRENCY_BOUNTY	1
#define ITEM_CURRENCY_CASH		2
#define ITEM_CURRENCY_MEDAL		3

int GetCurrencyType(bool bCash);
int GetCurrencyType(const char *pszCurrency);

// item.
class MMatchItem
{
public:
	MMatchItem(int nID, int nLevel, int nWeight, int nMaxWt, int nPrice, int nSex, int nSlotType, int nRentPeriod, int nCurrencyType, bool bSpendable);
	bool Create(int nID, int nLevel, int nWeight, int nMaxWt, int nPrice, int nSex, int nSlotType, int nRentPeriod, int nCurrencyType, bool bSpendable);
	
	int GetID() const	{ return m_nID; }
	int GetLevel() const	{ return m_nLevel; }
	int GetWeight() const	{ return m_nWeight; }
	int GetMaxWt() const	{ return m_nMaxWt; }
	int GetPrice() const	{ return m_nPrice; }
	int GetSex() const	{ return m_nSex; }
	int GetSlotType() const	{ return m_nSlotType; }
	int GetRentPeriod() const	{ return m_nRentPeriod; }
	int GetCurrencyType() const	{ return m_nCurrencyType; }
	bool IsSpendable() const	{ return m_bSpendable; }
	
	bool IsBounty() const	{ return m_nCurrencyType == ITEM_CURRENCY_BOUNTY ? true : false ; }
	bool IsCash() const	{ return m_nCurrencyType == ITEM_CURRENCY_CASH ? true : false ; }
	bool IsMedal() const	{ return m_nCurrencyType == ITEM_CURRENCY_MEDAL ? true : false ; }
	
	bool IsEquipableLevel(int nLevel) const	{ return nLevel >= m_nLevel ? true : false ; }
	bool IsEquipableWeight(int nCurrWt, int nMaxWt) const	{ return (nCurrWt + m_nWeight) <= nMaxWt ? true : false ; }
	bool IsEnoughBP(int nBP, int nCnt = 1) const	{ return (nBP - (m_nPrice * nCnt)) >= 0 ? true : false ; }
	bool IsValidSex(int nCharSex) const;
	bool IsValidSlot(int nCharParts) const;
	
private:
	int m_nID;
	int m_nLevel;
	int m_nWeight;
	int m_nMaxWt;
	int m_nPrice;
	int m_nSex;
	int m_nSlotType;
	int m_nRentPeriod;
	int m_nCurrencyType;
	bool m_bSpendable;
};

enum MMatchItemSex
{
	MMIS_ALL, 
	MMIS_MALE, 
	MMIS_FEMALE, 
	MMIS_END
};

enum MMatchItemSlotType
{
	MMIST_NONE, 
	MMIST_MELEE, 
	MMIST_RANGE, 
	MMIST_CUSTOM, 
	MMIST_HEAD, 
	MMIST_CHEST, 
	MMIST_HANDS, 
	MMIST_LEGS, 
	MMIST_FEET, 
	MMIST_FINGER, 
	MMIST_EXTRA, 
	MMIST_AVATAR, 
	MMIST_QUEST, // special slot type for quest.
	MMIST_END
};

int GetSexFromString(const char *pszSex);
int GetSlotTypeFromString(const char *pszSlot);

// item manager.
class MMatchItemManager
{
public:
	MMatchItemManager();
	~MMatchItemManager();
	
	bool LoadZItem(const char *pszFileName);
	bool LoadZQuestItem();
	
	void Add(MMatchItem *pItem);
	void Add(int nID, int nLevel, int nWeight, int nMaxWt, int nPrice, int nSex, int nSlotType, int nRentPeriod, int nCurrencyType, bool bSpendable);
	
	void ClearAll();
	
	MMatchItem *Get(int nItemID);
	
	vector<MMatchItem *>::iterator Begin()	{ return m_vtItem.begin(); }
	vector<MMatchItem *>::iterator End()	{ return m_vtItem.end(); }

private:
	vector<MMatchItem *> m_vtItem;
};

// shop item.
struct MShopItem
{
	int nID;
	int nCount;
};

class MMatchShopManager
{
public:
	MMatchShopManager();
	~MMatchShopManager();
	
	bool Load();
	
	void AddItem(int nItemID, int nItemCount = 1);
	void ClearAllItem();
	
	bool IsSellingItem(int nItemID);
	
	vector<MShopItem *>::iterator Begin()	{ return m_vtShopItem.begin(); }
	vector<MShopItem *>::iterator End()	{ return m_vtShopItem.end(); }
	
private:
	vector<MShopItem *> m_vtShopItem;
};

extern MMatchItemManager g_ItemMgr;
extern MMatchShopManager g_ShopMgr;

#endif