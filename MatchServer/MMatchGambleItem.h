#ifndef __MMATCHGAMBLEITEM_H__
#define __MMATCHGAMBLEITEM_H__

#include "MMatchItem.h"

#define MXML_FILENAME_GAMBLEITEM	"gambleitem.xml"

struct MMatchGambleRewardItemID
{
	int nMaleItemID;
	int nFemaleItemID;
};

class MMatchGambleRewardItemNode
{
public:
	MMatchGambleRewardItemNode(int nMaleItemID, int nFemaleItemID, int nRentHourPeriod = 0, int nQuantity = 1);
	
	// item id.
	void SetItemID(int nMaleItemID, int nFemaleItemID);
	
	int GetMaleItemID()		{ return m_ItemID.nMaleItemID; }
	int GetFemaleItemID()	{ return m_ItemID.nFemaleItemID; }
	
	// rent.
	void SetRentHourPeriod(int nHour)	{ m_nRentHourPeriod = nHour; }
	
	int GetRentHourPeriod()			{ return m_nRentHourPeriod; }
	bool IsRentPeriodUnlimited()	{ return m_nRentHourPeriod == 0 ? true : false ; }
	
	// count.
	void SetQuantity(int nQuantity)	{ m_nQuantity = nQuantity; }
	int GetQuantity()				{ return m_nQuantity; }
	
private:
	MMatchGambleRewardItemID m_ItemID;
	
	int m_nRentHourPeriod;
	int m_nQuantity;
};

// ------------------------------------------------------------.

// currency.
#define GAMBLEITEM_CURRENCY_NONE	ITEM_CURRENCY_NONE
#define GAMBLEITEM_CURRENCY_BOUNTY	ITEM_CURRENCY_BOUNTY//1
#define GAMBLEITEM_CURRENCY_CASH	ITEM_CURRENCY_CASH//2
#define GAMBLEITEM_CURRENCY_MEDAL	ITEM_CURRENCY_MEDAL//3

class MMatchGambleItem
{
public:
	MMatchGambleItem(int nID, const char *pszName, const char *pszDesc, int nPrice, int nCurrencyType, bool bShop);
	~MMatchGambleItem();
	
	int GetID()				{ return m_nID; }
	const char *GetName()	{ return m_szName; }
	const char *GetDesc()	{ return m_szDesc; }
	int GetPrice()			{ return m_nPrice; }
	bool IsSelling()		{ return m_bShop; }
	
	int GetCurrencyType()	{ return m_nCurrencyType; }
	
	bool IsBounty()		{ return m_nCurrencyType == GAMBLEITEM_CURRENCY_BOUNTY ? true : false ; }
	bool IsCash()		{ return m_nCurrencyType == GAMBLEITEM_CURRENCY_CASH ? true : false ; }
	bool IsMedal()		{ return m_nCurrencyType == GAMBLEITEM_CURRENCY_MEDAL ? true : false; }
	
	bool IsEnoughBP(int nBP, int nCnt = 1) const	{ return (nBP - (m_nPrice * nCnt)) >= 0 ? true : false ; }
	
	void AddRewardItem(int nMaleItemID, int nFemaleItemID, int nRentHourPeriod = 0, int nQuantity = 1);
	MMatchGambleRewardItemNode *GetRandomRewardItem();
	
private:
	int m_nID;
	
	char m_szName[256];
	char m_szDesc[256];
	
	int m_nPrice;
	
	int m_nCurrencyType;
	bool m_bShop;
	
	vector<MMatchGambleRewardItemNode *> m_vtRewardItem;
};

// ------------------------------------------------------------.

class MMatchGambleItemManager
{
public:
	bool Load();

	void Add(MMatchGambleItem *pGambleItem);
	void Remove(MMatchGambleItem *pGambleItem);
	void Clear();
	
	MMatchGambleItem *Get(int nItemID);
	
	void SendInfo(const MUID &uidPlayer);
	
	void SendInfo2011(const MUID &uidPlayer);
	void SendInfo2012(const MUID &uidPlayer);
	
	vector<MMatchGambleItem *>::iterator Begin()	{ return m_vtGambleItem.begin(); }
	vector<MMatchGambleItem *>::iterator End()		{ return m_vtGambleItem.end(); }
	
private:
	vector<MMatchGambleItem *> m_vtGambleItem;
};

extern MMatchGambleItemManager g_GItemMgr;

#endif