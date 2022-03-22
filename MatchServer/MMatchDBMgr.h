#ifndef __MMATCHDBMGR_H__
#define __MMATCHDBMGR_H__

#ifdef _UNIX_BUILD
#define OTL_ODBC_UNIX
#else
#define OTL_ODBC
#endif
#define OTL_DEFAULT_NUMERIC_NULL_TO_VAL 0
#include "otlv4.h"

extern otl_connect db;

#include "MMatchConstant.h"
#include "MMatchObject_Constant.h"

// #include "MCommandBlob.h"

struct DbData_AccountCharList
{
	char szName[CHARNAME_LEN];
	int nCharNum;
	int nLevel;
};

struct DbData_BasicCharInfo
{
	// basic.
	char szName[CHARNAME_LEN];
	int nLevel;
	int nSex;
	int nHair;
	int nFace;
	int nExp;
	int nBounty;
	
	// additional.
	int nKillCount;
	int nDeathCount;
	int nRanking;
	
	// quest.
	char szQuestItemData[1024];
};

struct DbData_BasicClanInfo
{
	int nCLID;
	char szClanName[DB_CLANNAME_LEN];
	int nMemberGrade;
	int nClanPoint;
};

struct DbData_CharItemInfo
{
	int nItemID[MMCIP_END];
	int nItemCount[MMCIP_END];
};

struct DbData_CharEquippedItemCIID
{
	int nCIID[MMCIP_END];
};

struct DbData_DTCharInfo
{
	int nGrade;
	
	int nTournamentPoint;
	int nWins;
	int nLosses;
	int nFinalMatchWin;
	int nRanking;
	
	int nPreviousTournamentPoint;
	int nPreviousWins;
	int nPreviousLosses;
	int nPreviousFinalMatchWin;
	int nPreviousRanking;
	
	int nRankingDifferent;
};

struct DbData_CharItemList
{
	int nCIID;
	int nItemID;
	int nItemCount;
	int nRentSecPeriod;
};

struct DbData_ExpiredItemNode
{
	int nCIID;
	int nItemID;
};

struct DbData_ClanInfo
{
	int nCLID;
	char szName[DB_CLANNAME_LEN];
	int nLevel;
	int nPoint;
	int nMasterCID;
	int nWin;
	int nLose;
	int nDraw;
	int nTotalPoint;
	int nRanking;
	char szEmblemURL[256];
	int nEmblemChecksum;
};

struct DbData_FriendListNode
{
	int nFriendID;
	int nCID;
	char szCharName[CHARNAME_LEN];
};

struct DbData_CharGambleItem
{
	int nGIID;
	int nItemID;
	int nCount;
};

struct DbData_CharSurvivalInfo
{
	int nPoint;
	int nRanking;
};

struct DbData_SurvivalRankingInfo
{
	char szCharName[DB_CHARNAME_LEN];
	int nPoint;
	int nRanking;
};

struct DbData_DTRankingInfo
{
	char szCharName[DB_CHARNAME_LEN];
	int nTP;
	int nWin;
	int nLose;
	int nRanking;
	int nRankingDiff;
	int nFinalWin;
	int nClass;
};

struct DbData_CQRecordInfo
{
	int nScenarioID;
	int nTime;
};

struct DbData_AccItemList
{
	int nAIID;
	int nItemID;
	int nItemCount;
	int nRentSecPeriod;
};

struct DbData_BlitzCharInfo
{
	int nWin;
	int nLose;
	int nPoint;
	int nMedal;
};

struct DbData_BlitzShopItem
{
	int nCID;
	int nItemID;
	int nPrice;
	int nBasePrice;
	int nCount;
	int nRentHourPeriod;
};

struct DbData_ReceivedGiftItem
{
	int nGiftID;
	char szSenderName[24];
	char szMessage[128];
	int nItemID[5];
	int nRentHourPeriod;
	char szGiftDate[32];
	
	int nItemNodeCount;
	
	int nGiftYear;
	int nGiftMonth;
	int nGiftDay;
	int nGiftHour;
};

// ----------------------------------------------------------------------------------------------------

bool Db_GetAccountInfo(const char *pszInUserID, int *pOutAID, char *pszOutPassword, int *pOutUGradeID, int *pOutPGradeID, int *pOutCash);
bool Db_GetAccountCharList(int nInAID, int *pOutCount, DbData_AccountCharList *pOutInfo);
bool Db_GetCharIndex(int nInAID, int nInCharNum, int *pOutCID);
bool Db_GetBasicCharInfo(int nInCID, DbData_BasicCharInfo *pOutInfo);
bool Db_GetBasicClanInfo(int nInCID, DbData_BasicClanInfo *pOutInfo);
bool Db_GetCharItemInfo(int nInCID, DbData_CharItemInfo *pOutInfo);
bool Db_GetCharEquipmentSlotCIID(int nInCID, DbData_CharEquippedItemCIID *pOutInfo);
bool Db_GetDTCharClass(int nInCID, int *pOutClass);
bool Db_GetCharacterCount(int nInAID, int *pOutCount);
bool Db_IsCharacterExists(const char *pszInSearchName, int *pOutResult);
bool Db_InsertCharacter(int nInAID, const char *pszInName, int nInSex, int nInHair, int nInFace, int nInCostume);
bool Db_GetCharacterName(int nInCID, char *pszOutName);
bool Db_DeleteCharacter(int nInCID);
bool Db_GetDTCharInfo(int nInCID, DbData_DTCharInfo *pOutInfo);
bool Db_GetCharacterItemList(int nInCID, vector<DbData_CharItemList> *pOut);
bool Db_ClearExpiredItem(int nInCID, vector<DbData_ExpiredItemNode> *pOutItemInfo);
bool Db_ClearExpiredAccountItem(int nInAID, vector<DbData_ExpiredItemNode> *pOutItemInfo);
bool Db_InsertItem(int nInCID, int nInItemID, int nInCount, int nInRentHourPeriod, int nInBP, int *pOutCIID);
bool Db_InsertItem(int nInCID, int nInItemID, int nInCount, int nInBP, int *pOutCIID);
bool Db_UpdateItem(int nInCID, int nInCIID, int nInCount, int nInBP);
bool Db_DeleteItem(int nInCID, int nInCIID, int nInBP);
bool Db_EquipItem(int nInCID, int nInCIID, int nInSlot);
bool Db_TakeoffItem(int nInCID, int nInSlot);
bool Db_SetExp(int nInCID, int nInLevel, int nInXP, int nInBP, int nInKill, int nInDeath);
bool Db_SetSurvivalPoint(int nInCID, int nInPoint);
bool Db_SetDTScore(int nInCID, int nInTP, int nInWin, int nInLose, int nInFinalWin);
bool Db_GetClanInfo(int nInCLID, DbData_ClanInfo *pOutInfo);
bool Db_IsClanExists(const char *pszInName, int *pOutResult);
bool Db_CreateClan(int nInCID, const char *pszInClanName, int *pOutCLID);
bool Db_JoinClan(int nInCLID, int nInCID);
bool Db_UpdateClanMemberGrade(int nInCLID, int nInCID, int nGrade);
bool Db_UpdateClanMasterCID(int nInCLID, int nInCID);
bool Db_LeaveClan(int nInCLID, int nInCID);
bool Db_CloseClan(int nInCLID);
bool Db_UpdateItemCount(int nInCIID, int nInCount);
bool Db_DeleteSpentItem(int nInCIID);
bool Db_AddFriend(int nInCID, int nInTargetCID, int *pOutID);
bool Db_RemoveFriend(int nInFriendID);
bool Db_GetFriendList(int nInCID, vector<DbData_FriendListNode> *pOut);
bool Db_ClearTargetFriend(int nInTargetCID);
bool Db_UpdateCharQuestItem(int nInCID, const char *pszInData);
bool Db_UpdateBounty(int nInCID, int nInBounty);
bool Db_GetCharGambleItem(int nInCID, vector<DbData_CharGambleItem> *pOut);
bool Db_InsertGambleItem(int nInCID, int nInItemID, int nInCount, int nInBP, int *pOutGIID);
bool Db_UpdateGambleItem(int nInCID, int nInGIID, int nInCount, int nInBP);
bool Db_DeleteGambleItem(int nInCID, int nInGIID, int nInBP);
bool Db_DecreaseGambleItem(int nInGIID);
bool Db_GetCQRecord(int nInAID, vector<DbData_CQRecordInfo> *pOut);
bool Db_InsertCQRecord(int nInAID, int nInScenarioID, int nInTime);
bool Db_GetSurvivalCharInfo(int nInCID, DbData_CharSurvivalInfo *pOutInfo);
bool Db_GetAccountItemList(int nInAID, vector<DbData_AccItemList> *pOut);
bool Db_MoveAItemToCItem(int nInAIID, int nInCID, int *pOutCIID);
bool Db_MoveAItemToCItem(int nInAIID, int nInCID, int nInCount, int *pOutCIID);
bool Db_MoveCItemToAItem(int nInCIID, int nInAID, int *pOutAIID);
bool Db_MoveCItemToAItem(int nInCIID, int nInAID, int nInCount, int *pOutAIID);
bool Db_SetBlitzScore(int nInCID, int nInWin, int nInLose, int nInPoint, int nInMedal);
bool Db_GetBlitzScore(int nInCID, DbData_BlitzCharInfo *pOutInfo);
bool Db_AddBlitzShopItem(int nInCID, int nInItemID, int nInPrice, int nInBasePrice, int nInCount, int nInRentHourPeriod);
bool Db_GetBlitzShopItem(vector<DbData_BlitzShopItem> *pOut);
bool Db_GetBlitzShopItem(int nInCID, vector<DbData_BlitzShopItem> *pOut);
bool Db_ClearBlitzShop();
bool Db_ClearBlitzShop(int nInCID);
bool Db_BuyCashItem(int nInCID, int nInItemID, int nInCount, int nInRentHourPeriod, int *pOutCIID);
bool Db_SetAccountCash(int nInAID, int nInCash);
bool Db_SendItemGift(const char *pszInReceiver, const char *pszInSender, const char *pszInMessage, const int *pInItemID, int nInRentHourPeriod, int nInQuantity, int *pOutResult);
bool Db_CheckItemGift(int nInCID, vector<DbData_ReceivedGiftItem> *pOut);
bool Db_AcceptItemGift(int nInID, int nInCID);

bool Db_BanPlayer(int nInAID);

bool Db_IsBannedIP(const char *pszInIPAddr, int *pOutResult);
bool Db_InsertAccount(const char *pszInUserID, const char *pszInPassword, int *pOutResult = NULL);
bool Db_UpdateConnData(int nInAID, const char *pszInIPAddr);

bool Db_UpdateIndividualRanking();

bool Db_UpdateSurvivalRanking();
bool Db_FetchSurvivalRanking(vector<DbData_SurvivalRankingInfo> *pOut);

bool Db_UpdateDTDailyRanking();
bool Db_UpdateDTWeeklyRanking();
bool Db_FetchDTTopRanking(vector<DbData_DTRankingInfo> *pOut);

// locator.
// bool Db_GetServerStatus(vector<MTD_ServerStatusInfo> *pOut);
bool Db_UpdateServerStatus(int nInServerID, int nInCurrPlayer);

// ----------------------------------------------------------------------------------------------------

void Db_Error(otl_exception &e);
bool Db_Connect(const char *pszUsername, const char *pszPassword, const char *pszDSN);
void Db_Disconnect();
bool Db_IsValidString(const char *pszStr);

#endif
