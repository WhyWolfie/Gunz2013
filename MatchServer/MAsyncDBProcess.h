#ifndef __MASYNCDBPROCESS_H__
#define __MASYNCDBPROCESS_H__

#include "MMatchDBMgr.h"

#include "MMatchObject.h"

class MBaseAsyncDBTask
{
public:
	MBaseAsyncDBTask(const MUID &uidRequestor = MUID(0, 0));
	virtual ~MBaseAsyncDBTask();

	virtual bool OnExec();	// non-thread safe.
	virtual void OnDone();	// thread safe.
	
protected:
	MUID m_uidRequestor;
};



class MAsyncDBProcess
{
public:
	MAsyncDBProcess();
	~MAsyncDBProcess();
	
	// void Add(MBaseAsyncDBTask *pTask);
	void SafeAdd(MBaseAsyncDBTask *pTask);
	
	// MBaseAsyncDBTask *FrontTask();
	MBaseAsyncDBTask *SafeFrontTask();
	
private:
	mutex m_Mutex;
	deque<MBaseAsyncDBTask *> m_AsyncDBTasks;
};

extern MAsyncDBProcess g_AsyncDBTaskMgr;

// -------------------------------------------------------------------------------------.
class MAsyncDBTask_UpdateDTDailyRanking : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_UpdateDTDailyRanking();
	virtual ~MAsyncDBTask_UpdateDTDailyRanking();
	
	virtual bool OnExec();
	virtual void OnDone();
};

class MAsyncDBTask_UpdateDTWeeklyRanking : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_UpdateDTWeeklyRanking();
	virtual ~MAsyncDBTask_UpdateDTWeeklyRanking();
	
	virtual bool OnExec();
	virtual void OnDone();
};

class MAsyncDBTask_FetchDTTopRanking : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_FetchDTTopRanking();
	virtual ~MAsyncDBTask_FetchDTTopRanking();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	vector<DbData_DTRankingInfo> m_vtDbRankingInfo;
};

class MAsyncDBTask_InsertCQRecord : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_InsertCQRecord(const MUID &uidPlayer, int nAID, int nMapID, int nRecordTime);
	virtual ~MAsyncDBTask_InsertCQRecord();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nAID;
	int m_nMapID;
	int m_nRecordTime;
};

class MAsyncDBTask_SetExp : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_SetExp(const MUID &uidPlayer, int nCID, int nLevel, int nXP, int nBP, int nKill, int nDeath);
	virtual ~MAsyncDBTask_SetExp();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nCID;
	int m_nLevel;
	int m_nXP;
	int m_nBP;
	int m_nKill;
	int m_nDeath;
};

class MAsyncDBTask_SetSurvivalPoint : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_SetSurvivalPoint(const MUID &uidPlayer, int nCID, int nPoint);
	virtual ~MAsyncDBTask_SetSurvivalPoint();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nCID;
	int m_nPoint;
};

class MAsyncDBTask_SetDTScore : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_SetDTScore(const MUID &uidPlayer, int nCID, int nTP, int nWin, int nLose, int nFinalWin);
	virtual ~MAsyncDBTask_SetDTScore();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nCID;
	int m_nTP;
	int m_nWin;
	int m_nLose;
	int m_nFinalWin;
};

class MAsyncDBTask_TakeoffItem : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_TakeoffItem(const MUID &uidPlayer, int nCID, int nItemParts);
	virtual ~MAsyncDBTask_TakeoffItem();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nCID;
	int m_nItemParts;
};

class MAsyncDBTask_DeleteSpentItem : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_DeleteSpentItem(const MUID &uidPlayer, int nCIID);
	virtual ~MAsyncDBTask_DeleteSpentItem();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nCIID;
};

class MAsyncDBTask_UpdateItemCount : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_UpdateItemCount(const MUID &uidPlayer, int nCIID, int nItemCount);
	virtual ~MAsyncDBTask_UpdateItemCount();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nCIID;
	int m_nItemCount;
};

class MAsyncDBTask_UpdateCharQuestItem : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_UpdateCharQuestItem(const MUID &uidPlayer, int nCID, const char *pszData);
	virtual ~MAsyncDBTask_UpdateCharQuestItem();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nCID;
	char m_szData[1024];
};

class MAsyncDBTask_FetchSurvivalRanking : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_FetchSurvivalRanking();
	virtual ~MAsyncDBTask_FetchSurvivalRanking();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	vector<DbData_SurvivalRankingInfo> m_vtDbRankingInfo;
};

class MAsyncDBTask_UpdateSurvivalRanking : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_UpdateSurvivalRanking();
	virtual ~MAsyncDBTask_UpdateSurvivalRanking();
	
	virtual bool OnExec();
	virtual void OnDone();
};

class MAsyncDBTask_BanPlayer : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_BanPlayer(const MUID &uidPlayer, int nAID);
	virtual ~MAsyncDBTask_BanPlayer();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nAID;
};

class MAsyncDBTask_ClearExpiredItem : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_ClearExpiredItem(const MUID &uidPlayer, int nAID, int nCID);
	virtual ~MAsyncDBTask_ClearExpiredItem();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nAID;
	int m_nCID;
	
	vector<DbData_ExpiredItemNode> m_vtAItemInfo;
	vector<DbData_ExpiredItemNode> m_vtItemInfo;
};

class MAsyncDBTask_RemoveFriend : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_RemoveFriend(const MUID &uidPlayer, int nFriendID);
	virtual ~MAsyncDBTask_RemoveFriend();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nFriendID;
};

class MAsyncDBTask_JoinClan : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_JoinClan(const MUID &uidPlayer, int nCLID, int nCID);
	virtual ~MAsyncDBTask_JoinClan();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nCLID;
	int m_nCID;
};

class MAsyncDBTask_UpdateClanMemberGrade : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_UpdateClanMemberGrade(const MUID &uidPlayer, int nCLID, int nCID, int nMemberGrade);
	virtual ~MAsyncDBTask_UpdateClanMemberGrade();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nCLID;
	int m_nCID;
	int m_nMemberGrade;
};

class MAsyncDBTask_UpdateClanMasterCID : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_UpdateClanMasterCID(const MUID &uidPlayer, int nCLID, int nCID);
	virtual ~MAsyncDBTask_UpdateClanMasterCID();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nCLID;
	int m_nCID;
};

class MAsyncDBTask_LeaveClan : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_LeaveClan(const MUID &uidPlayer, int nCLID, int nCID);
	virtual ~MAsyncDBTask_LeaveClan();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nCLID;
	int m_nCID;
};

class MAsyncDBTask_CloseClan : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_CloseClan(const MUID &uidPlayer, int nCLID);
	virtual ~MAsyncDBTask_CloseClan();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nCLID;
};

class MAsyncDBTask_UpdateItem : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_UpdateItem(const MUID &uidPlayer, int nCID, int nCIID, int nItemCount, int nBounty);
	virtual ~MAsyncDBTask_UpdateItem();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nCID;
	int m_nCIID;
	int m_nItemCount;
	int m_nBounty;
};

class MAsyncDBTask_UpdateGambleItem : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_UpdateGambleItem(const MUID &uidPlayer, int nCID, int nCGIID, int nItemCount, int nBounty);
	virtual ~MAsyncDBTask_UpdateGambleItem();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nCID;
	int m_nCGIID;
	int m_nItemCount;
	int m_nBounty;
};

class MAsyncDBTask_DeleteItem : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_DeleteItem(const MUID &uidPlayer, int nCID, int nCIID, int nBounty);
	virtual ~MAsyncDBTask_DeleteItem();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nCID;
	int m_nCIID;
	int m_nBounty;
};

class MAsyncDBTask_DeleteGambleItem : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_DeleteGambleItem(const MUID &uidPlayer, int nCID, int nCGIID, int nBounty);
	virtual ~MAsyncDBTask_DeleteGambleItem();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nCID;
	int m_nCGIID;
	int m_nBounty;
};

class MAsyncDBTask_EquipItem : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_EquipItem(const MUID &uidPlayer, int nCID, int nCIID, int nItemSlot);
	virtual ~MAsyncDBTask_EquipItem();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nCID;
	int m_nCIID;
	int m_nItemSlot;
};

class MAsyncDBTask_DecreaseGambleItem : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_DecreaseGambleItem(const MUID &uidPlayer, int nCGIID);
	virtual ~MAsyncDBTask_DecreaseGambleItem();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nCGIID;
};

class MAsyncDBTask_MatchLogin : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_MatchLogin(const MUID &uidPlayer, const char *pszUserID, const char *pszPassword, int nCommandVersion, const char *pszIP, bool bUseIPBlock);
	virtual ~MAsyncDBTask_MatchLogin();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	char m_szUserID[MAX_USERID_LEN];
	char m_szPassword[MAX_USERPASSWORD_LEN];
	int m_nCommandVersion;
	char m_szIP[64];
	unsigned long m_nIP;
	bool m_bUseIPBlock;
	
	int m_nAID;
	int m_nUGradeID;
	int m_nPGradeID;
	int m_nCash;
	vector<DbData_AccItemList> m_vtAccountItem;
	vector<DbData_CQRecordInfo> m_vtCQRecordInfo;
	
	int m_nErrorCode;
	
	void SendLoginResultCommand(Socket::socket_type s, const unsigned char *pCryptKey);
	void SendLoginResultCommand(int nResult, const char *pszUserID, int nUGradeID, int nPGradeID, const MUID &uidPlayer);
};

class MAsyncDBTask_AccountCharList : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_AccountCharList(const MUID &uidPlayer, int nAID);
	virtual ~MAsyncDBTask_AccountCharList();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nAID;
	
	int m_nCharCount;
	DbData_AccountCharList m_CharInfo[ACCOUNT_CHARLIST_COUNT];
};

class MAsyncDBTask_AccountCharInfo : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_AccountCharInfo(const MUID &uidPlayer, int nAID, int nCharNum);
	virtual ~MAsyncDBTask_AccountCharInfo();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nAID;
	int m_nCharNum;
	
	int m_nCID;
	DbData_BasicCharInfo m_CharInfo;
	DbData_BasicClanInfo m_ClanInfo;
	DbData_CharItemInfo m_ItemInfo;
	int m_nDTClass;
};

class MAsyncDBTask_SelectCharacter : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_SelectCharacter(const MUID &uidPlayer, int nAID, int nCharIndex);
	virtual ~MAsyncDBTask_SelectCharacter();
	
	virtual bool OnExec();
	virtual void OnDone();
	
	struct _tagEquipItemInfo
	{
		MUID uidItem;
		int nItemID;
		int nItemCount;
	};
	
protected:
	int m_nAID;
	int m_nCharIndex;
	
	int m_nCID;
	DbData_BasicCharInfo m_CharInfo;
	DbData_BasicClanInfo m_ClanInfo;
	DbData_CharEquippedItemCIID m_ItemInfo;
	DbData_DTCharInfo m_DTInfo;
	vector<DbData_CharItemList> m_vtItemList;
	_tagEquipItemInfo m_EquipItemInfo[MMCIP_END];
	vector<DbData_CharGambleItem> m_vtCharGambleItem;
	DbData_ClanInfo m_AdvClanInfo;
	vector<DbData_FriendListNode> m_vtFriendListNode;
	DbData_CharSurvivalInfo m_SurvivalInfo;
	DbData_BlitzCharInfo m_BlitzInfo;
};

class MAsyncDBTask_UpdateBounty : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_UpdateBounty(const MUID &uidPlayer, int nCID, int nBounty);
	virtual ~MAsyncDBTask_UpdateBounty();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nCID;
	int m_nBounty;
};

class MAsyncDBTask_UpdateServerStatus : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_UpdateServerStatus(int nServerID, int nCurrPlayers);
	virtual ~MAsyncDBTask_UpdateServerStatus();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nServerID;
	int m_nCurrPlayers;
};

class MAsyncDBTask_UpdateIndividualRanking : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_UpdateIndividualRanking();
	virtual ~MAsyncDBTask_UpdateIndividualRanking();
	
	virtual bool OnExec();
	virtual void OnDone();
};

class MAsyncDBTask_SetBlitzScore : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_SetBlitzScore(const MUID &uidPlayer, int nCID, int nWin, int nLose, int nPoint, int nMedal);
	virtual ~MAsyncDBTask_SetBlitzScore();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nCID;
	int m_nWin;
	int m_nLose;
	int m_nPoint;
	int m_nMedal;
};

class MAsyncDBTask_AddBlitzShopItem : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_AddBlitzShopItem(int nCID, int nItemID, int nPrice, int nBasePrice, int nCount, int nRentHourPeriod);
	virtual ~MAsyncDBTask_AddBlitzShopItem();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nCID;
	int m_nItemID;
	int m_nPrice;
	int m_nBasePrice;
	int m_nCount;
	int m_nRentHourPeriod;
};

class MAsyncDBTask_ClearBlitzShop : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_ClearBlitzShop(int nCID = 0);
	virtual ~MAsyncDBTask_ClearBlitzShop();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nCID;
};

class MAsyncDBTask_InsertItem : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_InsertItem(const MUID &uidPlayer, int nCID, int nItemID, int nItemCount, int nRentHourPeriod, int nBounty);
	virtual ~MAsyncDBTask_InsertItem();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nCID;
	int m_nItemID;
	int m_nItemCount;
	int m_nRentHourPeriod;
	int m_nBounty;
	
	int m_nCIID;
};

class MAsyncDBTask_InsertGambleItem : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_InsertGambleItem(const MUID &uidPlayer, int nCID, int nItemID, int nItemCount, int nBounty);
	virtual ~MAsyncDBTask_InsertGambleItem();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nCID;
	int m_nItemID;
	int m_nItemCount;
	int m_nBounty;
	
	int m_nGIID;
};

class MAsyncDBTask_MoveCItemToAItem : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_MoveCItemToAItem(const MUID &uidPlayer, int nCIID, int nAID, int nItemCount, MMatchCharItem &CharItem);
	virtual ~MAsyncDBTask_MoveCItemToAItem();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nCIID;
	int m_nAID;
	int m_nItemCount;
	MMatchCharItem m_CharItem;
	
	int m_nAIID;
};

class MAsyncDBTask_MoveAItemToCItem : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_MoveAItemToCItem(const MUID &uidPlayer, int nAIID, int nCID, int nItemCount, MMatchAccItem &AccItem);
	virtual ~MAsyncDBTask_MoveAItemToCItem();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nAIID;
	int m_nCID;
	int m_nItemCount;
	MMatchAccItem m_AccItem;
	
	int m_nCIID;
};

/*
class MAsyncDBTask_CreateClan : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_CreateClan(const MUID &uidPlayer, int nCID, const char *pszClanName);
	virtual ~MAsyncDBTask_CreateClan();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nCID;
	char m_szClanName[CLANNAME_LEN];
	
	int m_nCLID;
};
*/

class MAsyncDBTask_BuyCashItem : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_BuyCashItem(const MUID &uidPlayer, int nCID, int nItemID, int nItemCount, int nRentHourPeriod);
	virtual ~MAsyncDBTask_BuyCashItem();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nCID;
	int m_nItemID;
	int m_nItemCount;
	int m_nRentHourPeriod;
	
	int m_nCIID;
};

class MAsyncDBTask_SetAccountCash : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_SetAccountCash(const MUID &uidPlayer, int nAID, int nCash);
	virtual ~MAsyncDBTask_SetAccountCash();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nAID;
	int m_nCash;
};

class MAsyncDBTask_SendItemGift : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_SendItemGift(const MUID &uidPlayer, const char *pszReceiver, const char *pszSender, const char *pszMessage, const int *pItemID, int nRentHourPeriod, int nQuantity, int nAID, int nCash);
	virtual ~MAsyncDBTask_SendItemGift();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	char m_szReceiver[24];
	char m_szSender[24];
	char m_szMessage[128];
	int m_nItemID[5];
	int m_nRentHourPeriod;
	int m_nQuantity;
	
	int m_nAID;
	int m_nCash;
	
	int m_nResult;
};

class MAsyncDBTask_CheckItemGift : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_CheckItemGift(const MUID &uidPlayer, int nCID);
	virtual ~MAsyncDBTask_CheckItemGift();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nCID;
	vector<DbData_ReceivedGiftItem> m_vtGiftItem;
};

class MAsyncDBTask_AcceptItemGift : public MBaseAsyncDBTask
{
public:
	MAsyncDBTask_AcceptItemGift(const MUID &uidPlayer, int nID, int nCID);
	virtual ~MAsyncDBTask_AcceptItemGift();
	
	virtual bool OnExec();
	virtual void OnDone();
	
protected:
	int m_nID;
	int m_nCID;
};
// -------------------------------------------------------------------------------------.
void AsyncDb_UpdateDTDailyRanking();
void AsyncDb_UpdateDTWeeklyRanking();
void AsyncDb_FetchDTTopRanking();
void AsyncDb_InsertCQRecord(const MUID &uidPlayer, int nAID, int nMapID, int nRecordTime);
void AsyncDb_SetExp(const MUID &uidPlayer, int nCID, int nLevel, int nXP, int nBP, int nKill, int nDeath);
void AsyncDb_SetSurvivalPoint(const MUID &uidPlayer, int nCID, int nPoint);
void AsyncDb_SetDTScore(const MUID &uidPlayer, int nCID, int nTP, int nWin, int nLose, int nFinalWin);
void AsyncDb_TakeoffItem(const MUID &uidPlayer, int nCID, int nItemParts);
void AsyncDb_DeleteSpentItem(const MUID &uidPlayer, int nCIID);
void AsyncDb_UpdateItemCount(const MUID &uidPlayer, int nCIID, int nItemCount);
void AsyncDb_UpdateCharQuestItem(const MUID &uidPlayer, int nCID, const char *pszData);
void AsyncDb_FetchSurvivalRanking();
void AsyncDb_UpdateSurvivalRanking();
void AsyncDb_BanPlayer(const MUID &uidPlayer, int nAID);
void AsyncDb_ClearExpiredItem(const MUID &uidPlayer, int nAID, int nCID);
void AsyncDb_RemoveFriend(const MUID &uidPlayer, int nFriendID);
void AsyncDb_JoinClan(const MUID &uidPlayer, int nCLID, int nCID);
void AsyncDb_UpdateClanMemberGrade(const MUID &uidPlayer, int nCLID, int nCID, int nMemberGrade);
void AsyncDb_UpdateClanMasterCID(const MUID &uidPlayer, int nCLID, int nCID);
void AsyncDb_LeaveClan(const MUID &uidPlayer, int nCLID, int nCID);
void AsyncDb_CloseClan(const MUID &uidPlayer, int nCLID);
void AsyncDb_UpdateItem(const MUID &uidPlayer, int nCID, int nCIID, int nItemCount, int nBounty);
void AsyncDb_UpdateGambleItem(const MUID &uidPlayer, int nCID, int nCGIID, int nItemCount, int nBounty);
void AsyncDb_DeleteItem(const MUID &uidPlayer, int nCID, int nCIID, int nBounty);
void AsyncDb_DeleteGambleItem(const MUID &uidPlayer, int nCID, int nCGIID, int nBounty);
void AsyncDb_EquipItem(const MUID &uidPlayer, int nCID, int nCIID, int nItemSlot);
void AsyncDb_DecreaseGambleItem(const MUID &uidPlayer, int nCGIID);
void AsyncDb_UpdateBounty(const MUID &uidPlayer, int nCID, int nBounty);
void AsyncDb_UpdateServerStatus(int nServerID, int nCurrPlayers);
void AsyncDb_UpdateIndividualRanking();
void AsyncDb_SetBlitzScore(const MUID &uidPlayer, int nCID, int nWin, int nLose, int nPoint, int nMedal);
void AsyncDb_AddBlitzShopItem(int nCID, int nItemID, int nPrice, int nBasePrice, int nCount, int nRentHourPeriod);
void AsyncDb_ClearBlitzShop(int nCID = 0);
void AsyncDb_InsertItem(const MUID &uidPlayer, int nCID, int nItemID, int nItemCount, int nRentHourPeriod, int nBounty);
void AsyncDb_InsertGambleItem(const MUID &uidPlayer, int nCID, int nItemID, int nItemCount, int nBounty);
void AsyncDb_MoveCItemToAItem(const MUID &uidPlayer, int nCIID, int nAID, int nItemCount, MMatchCharItem &CharItem);
void AsyncDb_MoveAItemToCItem(const MUID &uidPlayer, int nAIID, int nCID, int nItemCount, MMatchAccItem &AccItem);
void AsyncDb_BuyCashItem(const MUID &uidPlayer, int nCID, int nItemID, int nItemCount, int nRentHourPeriod);
void AsyncDb_SetAccountCash(const MUID &uidPlayer, int nAID, int nCash);
// -------------------------------------------------------------------------------------.

void MAsyncDBProcessThread(void *pArg);

#endif