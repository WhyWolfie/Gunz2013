#ifndef __MMATCHBLITZKRIEG_H__
#define __MMATCHBLITZKRIEG_H__

#include "MMatchObject.h"

#define DEFAULT_BLITZKRIEG_MAP_NAME	"BlitzKrieg"

#ifdef _DEBUG
	#define BLITZ_PLAYER_COUNT	2
#else
	#define BLITZ_PLAYER_COUNT	8
#endif
#define BLITZ_TEAM_PLAYER_COUNT	(BLITZ_PLAYER_COUNT / 2)	// two teams.

#define BLITZ_MAX_ASSISTANT		3	// for killing assist.

class MMatchBlitzKriegGroup
{
public:
	MMatchBlitzKriegGroup(bool bAssignID = true);
	
	unsigned long GetID();
	
	bool AddObject(MMatchObject *pObj);
	bool CheckObjectExists(MMatchObject *pFindObj);
	
	int GetObjectCount();
	
	MMatchObject **GetObjects()	{ return m_pObj; }
	
private:
	unsigned long AssignID();
	
private:
	unsigned long m_nID;
	MMatchObject *m_pObj[BLITZ_TEAM_PLAYER_COUNT];
};

class MMatchBlitzKriegQueueManager
{
public:
	MMatchBlitzKriegQueueManager();
	~MMatchBlitzKriegQueueManager();
	
	void Add(MMatchBlitzKriegGroup *pGroup);
	void Remove(unsigned long nGroupID, const char *pszCancellerName = NULL);
	MMatchBlitzKriegGroup *Get(unsigned long nGroupID);
	
	void ProcessGroup(unsigned long nTime);
	
private:
	unsigned long m_nNextProcessTime;
	
	list<MMatchBlitzKriegGroup *> m_GroupQueue;
};

extern MMatchBlitzKriegQueueManager g_BKQMgr;

// blitz shop. ------------------------------------
class MBlitzShopItem
{
public:
	MBlitzShopItem(int nItemID, float fStockRate, int nRentHourPeriod, int nPrice, float fMaxDiscount, float fDiscountRate, bool bCoupon)
	{
		m_nItemID = nItemID;
		m_fStockRate = fStockRate;
		m_nRentHourPeriod = nRentHourPeriod;
		m_nPrice = nPrice;
		m_fMaxDiscount = fMaxDiscount;
		m_fDiscountRate = fDiscountRate;
		m_nQuantity = 1;	// currently always 1.
		m_bCoupon = bCoupon;
	}
	
	bool IsSellable()	{ return m_fStockRate > 0.0f ? true : false ; }
	bool IsPeriodUnlimited()	{ return m_nRentHourPeriod == 0 ? true : false; }
	
public:
	int m_nItemID;
	float m_fStockRate;
	int m_nRentHourPeriod;
	int m_nPrice;
	float m_fMaxDiscount;
	float m_fDiscountRate;
	int m_nQuantity;
	bool m_bCoupon;
};

class MBlitzShopSellingItem
{
public:
	MBlitzShopSellingItem(int nItemID, int nPrice, int nBasePrice, int nCount, int nRentHourPeriod)
	{
		m_nItemID = nItemID;
		m_nPrice = nPrice;
		m_nBasePrice = nBasePrice;
		m_nCount = nCount;
		m_nRentHourPeriod = nRentHourPeriod;
	}
	
	bool IsEnoughMedal(int nMedal, int nCnt = 1)	{ return (nMedal - (m_nPrice * nCnt)) >= 0 ? true : false ; }
	float CalcDCPercent()	{ return (1.0f - ((float)m_nPrice / (float)m_nBasePrice)); }
	bool IsPeriodUnlimited()	{ return m_nRentHourPeriod == 0 ? true : false; }
	
public:
	int m_nItemID;
	int m_nPrice;
	int m_nBasePrice;
	int m_nCount;
	int m_nRentHourPeriod;
};


#define MBLITZSHOP_CID_ALL	0

class MBlitzShop
{
public:
	MBlitzShop();
	~MBlitzShop();
	
	bool LoadXML();
	bool LoadFromDB();
	
	void Run();
	
	bool LoadRestockTime();
	bool SaveRestockTime();
	time_t MakeNextRestockTime();
	
	MBlitzShopItem *GetItem(int nItemID);
	bool IsItemEmpty()	{ return m_vtItem.empty(); }
	
	MBlitzShopSellingItem *GetSellingItem(int nItemID, int nCID = MBLITZSHOP_CID_ALL);
	
	bool Restock(vector<MBlitzShopSellingItem *> *pOut);
	void Unstock(vector<MBlitzShopSellingItem *> *pDest);
	
	bool Restock(int nCID);
	
	void SendItemList(const MUID &uidPlayer /* pass MUID(0, 0) to send to all. */, int nCID = MBLITZSHOP_CID_ALL);
	void EnumSellingItems(int nCID, vector<MBlitzShopSellingItem *> *pOut);
	
	void SyncDBData_Add(int nCID, vector<MBlitzShopSellingItem *> &vtSellingItem);
	void SyncDBData_Remove(int nCID = MBLITZSHOP_CID_ALL);
	
	void ClearAllItem();
	void ClearSellingItem();
	
private:
	time_t m_nNextRestockTime;
	
	vector<MBlitzShopItem *> m_vtItem;
	
	vector<MBlitzShopSellingItem *> m_vtSellingItem;
	map<int, vector<MBlitzShopSellingItem *> *> m_CharSellingItem;	// <CID, info>.
};

extern MBlitzShop g_BlitzShop;

#endif