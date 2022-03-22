#ifndef __MMATCHCASHSHOP_H__
#define __MMATCHCASHSHOP_H__

#include "MMatchItem.h"

// for cash coin auto-increasing.
#define CASH_BONUS_DEATHMATCH			0	// DM & TDM.
#define CASH_BONUS_QUEST				1	// Quest & Survival.
#define CASH_BONUS_CHALLENGE_QUEST		2	// Challenge Quest.
#define CASH_BONUS_BLITZKRIEG			3	// BlitzKrieg.
#define CASH_BONUS_SPY					4	// Spy Mode.
#define CASH_BONUS_END					5

static const int g_nCashBonus[CASH_BONUS_END] = {5, 8, 8, 30, 6};

// for command MC_CASHSHOP_RESPONSE_ITEMINFO.
class MCashShopBlob
{
public:
	struct _periodinfo
	{
		int nHour;
		int nPrice;
	};
	
public:
	MCashShopBlob(int nShopItemID, const char *pszItemName);
	~MCashShopBlob();
	
	void Reset(int nShopItemID, const char *pszItemName);
	
	void AddPeriod(int nHour, int nPrice);
	void AddItem(int nItemID);
	
	void Build(unsigned char *pOutData, int *pOutSize);
	
private:
	int m_nShopItemID;
	char m_szItemName[64];
	
	vector<_periodinfo> m_vtRentPeriod;
	vector<int> m_vtItemID;
};

// for item set id.
#define CASHITEM_SET_HEAD		0
#define CASHITEM_SET_CHEST		1
#define CASHITEM_SET_HAND		2
#define CASHITEM_SET_LEGS		3
#define CASHITEM_SET_FEET		4
#define CASHITEM_SET_MAX		5

// item gender.
#define CASHSHOP_SEX_MALE		0
#define CASHSHOP_SEX_FEMALE		1

// sort type.
#define CASHSHOP_SORT_NEWEST	1
#define CASHSHOP_SORT_LEVEL		0

class MMatchCashShopItem
{
public:
	MMatchCashShopItem(int nItemID, const char *pszItemName, int nCashPrice, int nItemCount, const int *pItemSetID, int nItemFilter);
	~MMatchCashShopItem();
	
	// nShopSex : CASHSHOP_SEX_MALE, CASHSHOP_SEX_FEMALE.
	bool IsValidShopSex(int nShopSex);
	
public:
	int m_nID;
	char m_szName[64];
	int m_nPrice;
	int m_nQuantity;
	int m_nSetID[CASHITEM_SET_MAX];
	int m_nFilter;
	
	unsigned short m_nShopID;
	MMatchItem *m_pItem;
};

class MMatchCashShop
{
public:
	MMatchCashShop();
	~MMatchCashShop();
	
	bool Load();
	
	void Add(MMatchCashShopItem *pShopItem);
	MMatchCashShopItem *Get(int nShopItemID);
	void DeleteAll();
	
	void Organize(vector<MMatchCashShopItem *> &vtShopItem);
	
	void MakeItemList(int nSex, int nSortType, vector<MMatchCashShopItem *> *pOut);
	
	void SexFiltering(int nSex, vector<MMatchCashShopItem *> *pDest);
	void Sort(int nSortType, vector<MMatchCashShopItem *> *pDest);
	
	void SendItemList(const MUID &uidPlayer, int nSex, int nSortType);
	void SendItemInfo(const MUID &uidPlayer, int nSex, int nSortType, int nStartIndex, int nCount);
	
private:
	vector<MMatchCashShopItem *> m_vtShopItem;
};

extern MMatchCashShop g_CashShop;

#endif