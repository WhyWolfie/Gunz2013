#ifndef __MMATCHOBJECT_H__
#define __MMATCHOBJECT_H__

#include "MMatchConstant.h"
#include "MMatchObject_Constant.h"

// account.
struct MMatchAccountInfo
{
	int nAID;
	int nUGradeID, nPGradeID;
	
	int nCash;
};

// character level.
struct MMatchCharExp
{
	int nLevel;
	
	int nXP;
	int nBP;
	
	int nKill;
	int nDeath;
	
	int nRanking;
};

// character item.
struct MMatchCharItem
{
	MUID uid;
	int nCIID;
	int nID;
	int nCount;
	int nRentSecPeriod;
	
	// item made time : for calc rent period.
	unsigned long nMadeTime;
	
	int CalcRentSecPeriod()
	{
		if(nRentSecPeriod == 0) return 0;
		
		unsigned long nTimeElapsed = GetTime() - nMadeTime;
		return (nRentSecPeriod - (int)(nTimeElapsed / 1000));
	}
};

// character gamble item.
struct MMatchCharGambleItem
{
	MUID uid;
	int nGIID;
	int nItemID;
	int nCount;
};

// character.
struct MMatchCharInfo
{
	int nCID;
	char szName[CHARNAME_LEN];
	
	int nSex;
	int nHair;
	int nFace;
};

// survival quest.
struct MMatchCharSurvivalInfo
{
	int nPoint;
	int nRanking;
};

// duel tournament.
struct MMatchCharDuelTournamentInfo
{
	int nClass;
	
	int nTP;
	int nWin;
	int nLose;
	int nFinalWin;
	int nRanking;
	
	int nPrevTP;
	int nPrevWin;
	int nPrevLose;
	int nPrevFinalWin;
	int nPrevRanking;
	
	int nRankingDiff;
};

// game.
#define SPENDITEM_SLOT_COUNT	2	// MMCIP_CUSTOM1, MMCIP_CUSTOM2 : total 2.

struct MMatchCharGameInfo
{
	bool bAlive;
	
	int nKill;
	int nDeath;
	
	int nExp;
	
	int nTeam;
	
	int nSpendItemUsed[SPENDITEM_SLOT_COUNT];
};

// clan.
struct MMatchCharClanInfo
{
	int nCLID;
	int nMemberGrade;
};

// friend.
struct MMatchFriend
{
	int nFriendID;
	int nCID;
	char szName[CHARNAME_LEN];
};

// quest item.
struct MMatchQuestItem
{
	int nItemID;
	int nItemCount;
};

struct MMatchChallengeQuestRecordInfo
{
	int nScenarioID;
	int nTime;
};

// account item.
struct MMatchAccItem
{
	int nAIID;
	int nID;
	int nCount;
	int nRentSecPeriod;
	
	// item made time : for calc rent period.
	unsigned long nMadeTime;
	
	int CalcRentSecPeriod()
	{
		if(nRentSecPeriod == 0) return 0;
		
		unsigned long nTimeElapsed = GetTime() - nMadeTime;
		return (nRentSecPeriod - (int)(nTimeElapsed / 1000));
	}
};

// blitzkrieg.
struct MMatchCharBlitzKriegInfo
{
	int nPoint;
	int nWin;
	int nLose;
	
	int nMedal;
};

// mmatchobject game flag.
#define MMOGF_NONE		0
#define MMOGF_INGAME	1
#define MMOGF_ENTERED	2
#define MMOGF_FORCED	4
#define MMOGF_LAUNCHED	8

class MMatchObject
{
public:
	MMatchObject(Socket::socket_type s, const MUID &uid, const unsigned char *pCryptKey);
	~MMatchObject();

	Socket::socket_type GetSocket()	{ return m_Socket; }
	const MUID &GetUID()	{ return m_uid; }
	const unsigned char *GetCryptKey()	{ return m_nCryptKey; }
	
	void SetUDPPort(unsigned short n)	{ m_nUDPPort = n; }
	
	const char *GetIPString()	{ return m_szIP; }
	unsigned long GetIP()	{ return m_nIP; }
	unsigned short GetPort()	{ return m_nPort; }
	unsigned short GetUDPPort()	{ return m_nUDPPort; }
	
	void Disconnect();

private:
	Socket::socket_type m_Socket;

	MUID m_uid;
	unsigned char m_nCryptKey[ENCRYPTIONKEY_LENGTH];
	
	char m_szIP[64];
	unsigned long m_nIP;
	unsigned short m_nPort;
	unsigned short m_nUDPPort;
	
	bool m_bDisconnected;
	
public:
	void Update(unsigned long nTime);
	
public:
	void Clean();
	void SaveExp();
	
	void SaveSpentItem();
	
public:
	bool m_bCharInfoExist;
	
	MMatchAccountInfo m_Account;
	MMatchCharInfo m_Char;
	MMatchCharExp m_Exp;
	// MMatchCharItem m_Item[MMCIP_END];
	MMatchCharSurvivalInfo m_Survival;
	MMatchCharDuelTournamentInfo m_DuelTournament;
	MMatchCharGameInfo m_GameInfo;
	MMatchCharClanInfo m_Clan;
	MMatchCharBlitzKriegInfo m_BlitzKrieg;
	
	unsigned long m_nPlayerFlag;
	unsigned long m_nOptionFlag;
	unsigned long m_nGameFlag;
	
	bool CheckGameFlag(unsigned long nFlag)	{ return (m_nGameFlag & nFlag) != 0 ? true : false ; }
	void ResetInGameInfo();
	
	int m_nPlace;
	int m_nStageState;
	
	MUID m_uidEquippedItem[MMCIP_END];
	void SetItemUID(const MUID &uidItem, int nItemSlot);
	bool IsEquippedItem(const MUID &uidItem);
	
	int CalcCurrWeight();
	int CalcMaxWeight();
	
	void ResetDTScore();
	
	bool IsAdmin();
	
	// admin_hide.
	void Hide(bool b = true);
	bool IsHide();
	
public:
	MUID m_uidChannel;
	MUID m_uidStage;
	
public:
	int m_nLastChannelPlayerListPage;
	int m_nLastStageListPage;
	
	int m_nRequestedChannelListType;
	
	// checksums related. (same infos are not sent.)
public:
	unsigned int MakeChecksum();
	
	unsigned int m_nLastChannelPlayerListChecksum;
	unsigned int m_nLastStageListChecksum;
	
public:
	// account item.
	list<MMatchAccItem *> m_AccountItemList;
	
	void AddAccItem(int nAIID, int nItemID, int nItemCount, int nRentPeriod);
	void RemoveAccItem(int nAIID);
	MMatchAccItem *GetAccItemByItemID(int nItemID);
	MMatchAccItem *GetAccItemByAIID(int nAIID);
	void AccItemExpired(int nAIID);
	void ClearAccItemList();
	
	bool MoveItemToStorage(int nCIID);	// normal item.
	bool MoveItemToStorage(int nCIID, int nCount);	// consumable item.
	bool MoveItemToInventory(int nAIID);	// normal item.
	bool MoveItemToInventory(int nAIID, int nCount);	// consumable item.
	
public:
	// char item.
	list<MMatchCharItem *> m_ItemList;
	
	void AddCharItem(const MUID &uid, int nCIID, int nItemID, int nItemCount, int nRentPeriod);
	void RemoveCharItem(const MUID &uid);
	void RemoveCharItem(int nCIID);
	MMatchCharItem *GetCharItem(const MUID &uid);
	MMatchCharItem *GetCharItemByItemID(int nItemID);
	MMatchCharItem *GetCharItemByCIID(int nCIID);
	MMatchCharItem *GetCharItemBySlotIndex(int nSlotIndex);
	void ItemExpired(int nCIID);
	void ClearCharItemList();
	
public:
	// gamble item.
	vector<MMatchCharGambleItem *> m_vtGambleItem;
	
	void AddCharGambleItem(const MUID &uid, int nGIID, int nItemID, int nCount);
	void RemoveCharGambleItem(const MUID &uid);
	void RemoveCharGambleItem(int nGIID);
	MMatchCharGambleItem *GetCharGambleItem(const MUID &uid);
	MMatchCharGambleItem *GetCharGambleItemByItemID(int nItemID);
	MMatchCharGambleItem *GetCharGambleItemByGIID(int nGIID);
	void ClearAllCharGambleItem();
	
	// friend.
public:
	bool AddFriend(int nID, int nFriendCID, const char *pszCharName);
	bool RemoveFriend(int nFriendID);
	void ClearFriendList();
	
	bool IsFriendListMax();
	bool CheckFriendExists(int nFriendCID);
	
	MMatchFriend *GetFrind(const char *pszCharName);
	
	list<MMatchFriend *> m_FriendList;
	
	// spawn time.
public:
	void UpdateSpawnTime();
	bool IsValidSpawn();
	
private:
	unsigned long m_nSpawnTimer;
	
	// kicks.
public:
	bool m_bVoted;
	
	// chat rooms.
public:
	void AttachChatRoom(unsigned long nID);
	void DetachChatRoom(unsigned long nID);
	bool CheckChatRoomAttached(unsigned long nID);

	vector<unsigned long> m_vtChatRoomID;
	
	unsigned long m_nChatRoomID;	// current selected chat room id.
	
	// quest item.
public:
	list<MMatchQuestItem> m_QuestItemList;
	
	void InitQuestItem(const char *pszData);
	void SaveQuestItem();
	
	MMatchQuestItem *GetQuestItem(int nItemID);
	
	bool AddQuestItem(int nItemID, int nCount);
	bool SubQuestItem(int nItemID, int nCount);
	
private:
	bool RestoreQuestItemData(const char *pszSrcData, list<MMatchQuestItem> *pDstList);
	bool BuildQuestItemData(list<MMatchQuestItem> *pSrcList, char *pszDstData);
	
	// challenge quest.
public:
	vector<MMatchChallengeQuestRecordInfo> m_vtCQRecordInfo;
	
	void AddCQRecord(int nScenarioID, int nTime);
	MMatchChallengeQuestRecordInfo *GetCQRecord(int nScenarioID);
	
	// agent.
public:
	bool m_bAgentUser;
	
	// anti-flood.
private:
	bool m_bFlooding;
	deque<unsigned long> m_CmdRecvTime;
	
public:
	bool IsFlooding()	{ return m_bFlooding; }
	bool UpdateCmdRecvTime();
	
	// blitzkrieg.
public:
	unsigned long m_nBlitzGroupID;
	
	void StartMedalBonus();
	void StopMedalBonus();
	
	void SyncMedal();
	void SyncMedal(int diff);
	
	void BlitzPenalty(int nLostMedal, int nPoint);
	
private:
	bool m_bDistribMedalBonus;
	unsigned long m_nNextMedalBonusTime;
	
	int m_nAddedBonusMedal;	// for medal sync.
	
	#define BLITZ_SCORETYPE_MEDAL_ADD		0
	#define BLITZ_SCORETYPE_PENALTY			1
	#define BLITZ_SCORETYPE_MEDAL_UPDATE	2
	void SendBlitzScore(int nScoreType, int nMedal, int nPoint = 0);
	
	// auto kick timer.
	//   - because of battle entering client lag. : 
	//   - if that client doesn't enter to battle, the game doesn't starts.
public:
	void ReserveStageKick(unsigned long nReserveTime);
	void CancelStageKick();
	
private:
	bool m_bStageKickReserved;
	unsigned long m_nStageKickTime;
	
	// cash.
public:
	void SyncCash(bool bSave = true);
};


class MMatchObjectManager
{
public:
	MMatchObjectManager();
	~MMatchObjectManager();
	
	MMatchObject *Add(Socket::socket_type s, const MUID &uid, const unsigned char *pCryptKey);
	MMatchObject *Get(Socket::socket_type s);
	MMatchObject *Get(const MUID &uid);
	MMatchObject *Get(const char *pszCharName);
	void Erase(Socket::socket_type s);
	void Erase(const MUID &uid);
	void EraseAll();
	
	list<MMatchObject *>::iterator Begin()	{ return m_Objects.begin(); }
	list<MMatchObject *>::iterator End()	{ return m_Objects.end(); }
	
	int Size()	{ return (int)m_Objects.size(); }
	
	bool IsAvailable(const MUID &uid);
	bool IsAvailable(MMatchObject *pObj);
	
private:
	list<MMatchObject *> m_Objects;
};

extern MMatchObjectManager g_ObjectMgr;

#endif
