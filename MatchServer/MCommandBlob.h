#ifndef __MCOMMANDBLOB_H__
#define __MCOMMANDBLOB_H__

#include "MUID.h"

#include "MMatchChannel.h"
#include "MMatchStage.h"

#include "MMatchConstant.h"
#include "MMatchObject_Constant.h"

// ---------- MCommand Blob with-out alignments. ----------
#pragma pack(1)
struct MTD_AccountCharInfo
{
	char				szName[CHARNAME_LEN];
	char				nCharNum;
	unsigned char		nLevel;
};

struct MTD_CharInfo
{
	char				szName[CHARNAME_LEN];
	char				szClanName[CLANNAME_LEN];
	int					nClanGrade;
	unsigned short		nClanContPoint;
	char				nCharNum;
	unsigned short		nLevel;
	char				nSex;
	char				nHair;
	char				nFace;
	unsigned long int	nXP;
	int					nBP;
	float				fBonusRate;
	unsigned short		nPrize;
	unsigned short		nHP;
	unsigned short		nAP;
	unsigned short		nMaxWeight;
	unsigned short		nSafeFalls;
	unsigned short		nFR;
	unsigned short		nCR;
	unsigned short		nER;
	unsigned short		nWR;
	
	unsigned long int	nEquipedItemDesc[MMCIP_END];
	
	int					nUGradeID;
	
	unsigned int		nClanCLID;
	
	int					nDTLastWeekGrade;	
	
	MUID				uidEquipedItem[MMCIP_END];
	unsigned long int	nEquipedItemCount[MMCIP_END];
	
	#if _GAME_VERSION >= 2012
	int					nUnknown1;
	int					nUnknown2;
	#endif
};

struct MTD_MyExtraCharInfo
{
	char	nLevelPercent;
};

struct MTD_SimpleCharInfo
{
	char				szName[CHARNAME_LEN];
	char				nLevel;
	char				nSex;
	char				nHair;
	char				nFace;
	unsigned long int	nEquipedItemDesc[MMCIP_END];
};

struct MTD_MySimpleCharInfo
{
	unsigned char		nLevel;
	unsigned long int	nXP;
	int					nBP;
};

struct MTD_ItemNode
{
	MUID				uidItem;
	unsigned long int	nItemID;
	int					nRentMinutePeriodRemainder;
	int					nMaxUseHour;
	int					nCount;
};

struct MTD_ItemNode2012
{
	MUID				uidItem;
	unsigned long int	nItemID;
	int					nRentMinutePeriodRemainder;
	int					nMaxUseHour;
	int					nCount;
	int 				nMedalPrice;
};

struct MTD_RelayMap
{
	int		nMapID;
};

struct MTD_AccountItemNode
{
	int					nAIID;
	unsigned long int	nItemID;
	int					nRentMinutePeriodRemainder;
	int					nCount;
};

struct MTD_GameInfoPlayerItem
{
	MUID	uidPlayer;
	bool	bAlive;
	int		nKillCount;
	int		nDeathCount;
};

struct MTD_GameInfo
{
	char	nRedTeamScore;
	char	nBlueTeamScore;

	short	nRedTeamKills;
	short	nBlueTeamKills;
};

struct MTD_RuleInfo	
{
	unsigned char	nRuleType;
};

struct MTD_RuleInfo_Assassinate : public MTD_RuleInfo
{
	MUID	uidRedCommander;
	MUID	uidBlueCommander;
};

struct MTD_RuleInfo_Berserker : public MTD_RuleInfo
{
	MUID	uidBerserker;
};


enum MTD_PlayerFlags {
	MTD_PlayerFlags_AdminHide	= 1,
	MTD_PlayerFlags_BridgePeer	= 1<<1,
	MTD_PlayerFlags_Premium		= 1<<2
};

struct MTD_ChannelPlayerListNode 
{
	MUID			uidPlayer;
	char			szName[CHARNAME_LEN];
	char			szClanName[CLANNAME_LEN];
	char			nLevel;
	char			nDTLastWeekGrade;
	int				nPlace;
	unsigned char	nGrade;
	unsigned char	nPlayerFlags;
	unsigned int	nCLID;
	unsigned int	nEmblemChecksum;
};

struct MTD_ClanMemberListNode 
{
	MUID		uidPlayer;
	char		szName[CHARNAME_LEN];
	char		nLevel;
	int			nClanGrade;
	int			nPlace;
};


enum MTD_WorldItemSubType
{
	MTD_WorldItemSubType_Dynamic = 0,
	MTD_WorldItemSubType_Static  = 1
};

struct MTD_WorldItem
{
	unsigned short	nUID;
	unsigned short	nItemID;
	unsigned short  nItemSubType;
	short			x;
	short			y;
	short			z;
};

struct MTD_ActivatedTrap
{
	MUID				uidOwner;
	unsigned short		nItemID;
	unsigned long int	nTimeElapsed;
	short				x;
	short				y;
	short				z;
};


struct MTD_CharClanInfo
{
	char	szClanName[CLANNAME_LEN];
	int		nGrade;
};


struct MTD_CharInfo_Detail
{
	char				szName[CHARNAME_LEN];
	char				szClanName[CLANNAME_LEN];
	int					nClanGrade;
	int					nClanContPoint;
	unsigned short		nLevel;
	char				nSex;
	char				nHair;
	char				nFace;
	unsigned long int	nXP;
	int					nBP;

	int					nKillCount;
	int					nDeathCount;

	unsigned long int	nTotalPlayTimeSec;
	unsigned long int	nConnPlayTimeSec;


	unsigned long int	nEquipedItemDesc[MMCIP_END];

	int					nUGradeID;

	unsigned int		nClanCLID;
};


#define MSTAGENODE_FLAG_FORCEDENTRY_ENABLED		1
#define MSTAGENODE_FLAG_PRIVATE					2
#define MSTAGENODE_FLAG_LIMITLEVEL				4

struct MTD_StageListNode
{
	MUID			uidStage;							
	unsigned char	nNo;								
	char			szStageName[MAX_STAGE_NAME_LENGTH];
	char			nPlayers;							
	char			nMaxPlayers;						
	int				nState;								
	int 			nGameType;							
	char			nMapIndex;							
	int				nSettingFlag;						
	char			nMasterLevel;						
	char			nLimitLevel;						
};


struct MTD_ExtendInfo
{
	char			nTeam;
	unsigned char	nPlayerFlags;
	unsigned char	nReserved1;
	unsigned char	nReserved2;
};

struct MTD_PeerListNode
{
	MUID					uidChar;
	unsigned long			nIP;
	unsigned int			nPort;
	MTD_CharInfo			CharInfo;
	MTD_ExtendInfo			ExtendInfo;
};

struct MTD_ReplierNode
{
	char szName[CHARNAME_LEN];
};

struct MTD_LadderTeamMemberNode
{
	char szName[CHARNAME_LEN];

};

struct MTD_ClanInfo
{
	char				szClanName[CLANNAME_LEN];		
	short				nLevel;								
	int					nPoint;								
	int					nTotalPoint;						
	int					nRanking;							
	char				szMaster[CHARNAME_LEN];	
	unsigned short		nWins;								
	unsigned short		nLosses;							
	unsigned short		nTotalMemberCount;					
	unsigned short		nConnedMember;						
	unsigned int		nCLID;								
	unsigned int		nEmblemChecksum;					
};

struct MTD_StandbyClanList
{
	char				szClanName[CLANNAME_LEN];		
	short				nPlayers;							
	short				nLevel;								
	int					nRanking;							
	unsigned int		nCLID;								
	unsigned int		nEmblemChecksum;					
};


struct MTD_QuestGameInfo
{
	unsigned short		nQL;												
	float				fNPC_TC;											
	unsigned short		nNPCCount;											

	unsigned char		nNPCInfoCount;										
	unsigned char		nNPCInfo[MAX_QUEST_NPC_INFO_COUNT];					
	unsigned short		nMapSectorCount;									
	unsigned short		nMapSectorID[MAX_QUEST_MAP_SECTOR_COUNT];			
	char				nMapSectorLinkIndex[MAX_QUEST_MAP_SECTOR_COUNT];	
	unsigned char		nRepeat;											
	int					eGameType;											
};

struct MTD_QuestReward
{
	MUID		uidPlayer;	
	int			nXP;		
	int			nBP;		
};

struct MTD_QuestItemNode
{
	int		nItemID;
	int		nCount;
};

struct MTD_QuestZItemNode
{
	unsigned int	nItemID;
	int				nRentPeriodHour;
	int				nItemCnt;
};

struct MTD_NPCINFO
{
	unsigned char	nNPCTID;
	unsigned short	nMaxHP;
	unsigned short	nMaxAP;
	unsigned char	nInt;
	unsigned char	nAgility;
	float			fAngle;
	float			fDyingTime;

	float			fCollisonRadius;
	float			fCollisonHight;

	unsigned char	nAttackType;
	float			fAttackRange;
	unsigned long	nWeaponItemID;
	float			fDefaultSpeed;
};

struct MTD_SurvivalRanking
{
	char			szCharName[CHARNAME_LEN];
	unsigned long	nPoint;
	unsigned long	nRank;		
};

/*
#if defined(_AGENT_PING) //defined(LOCALE_NHNUSA)
struct MTD_ServerStatusInfo
{
	unsigned long		nIP;
	unsigned long		nAgentIP;
	int					nPort;
	unsigned char		nServerID;
	short				nMaxPlayer;
	short				nCurPlayer;
	char				nType;
	bool				bIsLive;
	char				szServerName[64];
};
#else
struct MTD_ServerStatusInfo
{
	unsigned long		nIP;
	int					nPort;
	unsigned char		nServerID;
	short				nMaxPlayer;
	short				nCurPlayer;
	char				nType;
	bool				bIsLive;
	char				szServerName[64];
};
#endif
*/

struct MTD_ResetTeamMembersData
{
	MUID		uidPlayer;
	char		nTeam;		
};


struct MTD_DuelQueueInfo
{
	MUID		uidChampion;
	MUID		uidChallenger;
	MUID		uidWaitQueue[14];
	char		nQueueLength;
	char		nVictory;		
	bool		bIsRoundEnd;	
};

struct MTD_DuelTournamentGameInfo
{
	MUID				uidPlayer1;		
	MUID				uidPlayer2;		
	int					nMatchType;		
	int					nMatchNumber;	
	int					nRoundCount;	
	bool				bIsRoundEnd;	
	char				nWaitPlayerListLength;	
	unsigned char		dummy[2];				
	MUID				uidWaitPlayerList[8];		
};

struct MTD_DuelTournamentNextMatchPlayerInfo
{
	MUID		uidPlayer1;					
	MUID		uidPlayer2;					
};


struct MTD_DuelTournamentRoundResultInfo
{
	MUID				uidWinnerPlayer;			
	MUID				uidLoserPlayer;				
	bool				bIsTimeOut;					
	bool				bDraw;						
	bool				bIsMatchFinish;				
	unsigned char		dummy[2];					
};

struct MTD_DuelTournamentMatchResultInfo
{
	int			nMatchNumber;
	int			nMatchType;
	MUID		uidWinnerPlayer;			
	MUID		uidLoserPlayer;				
	int			nGainTP;
	int			nLoseTP;
};

struct MTD_QuickJoinParam
{
	unsigned long int	nMapEnum;
	unsigned long int	nModeEnum;
};

struct MFRIENDLISTNODE 
{
	unsigned char	nState;
	char			szName[CHARNAME_LEN];
	char			szDescription[/*MATCH_SIMPLE_DESC_LENGTH*/ 64];
};

struct MTD_CQRecordInfo
{
	int nScenarioID;
	int nTime;
};

struct MTD_CashShopItemID
{
	unsigned short nID;
};

// for MTD_CashShopItem.
struct MTD_CashShopItemNode
{
	int nStartIndex;
	int nItemCount;
};

struct MTD_CashShopItem
{
	#define CASHSHOP_ITEMFILTER_ALL			0
	#define CASHSHOP_ITEMFILTER_HEAD		1
	#define CASHSHOP_ITEMFILTER_CHEST		2
	#define CASHSHOP_ITEMFILTER_HAND		3
	#define CASHSHOP_ITEMFILTER_LEGS		4
	#define CASHSHOP_ITEMFILTER_FEET		5
	#define CASHSHOP_ITEMFILTER_RING		6
	#define CASHSHOP_ITEMFILTER_AVATAR		7
	#define CASHSHOP_ITEMFILTER_MELEE		8
	#define CASHSHOP_ITEMFILTER_RANGE		9
	#define CASHSHOP_ITEMFILTER_CUSTOM		10
	#define CASHSHOP_ITEMFILTER_QUEST		11
	#define CASHSHOP_ITEMFILTER_GAMBLE		12
	#define CASHSHOP_ITEMFILTER_ENCHANT		13
	#define CASHSHOP_ITEMFILTER_SET			14
	#define CASHSHOP_ITEMFILTER_ETC			15
	#define CASHSHOP_ITEMFILTER_MAX			16
	
	MTD_CashShopItemNode node[CASHSHOP_ITEMFILTER_MAX];	// 16 = max cash shop item filter count.
};

struct MTD_GiftCashItem
{
	int 						nGiftID;
	unsigned char 	nDummy1[4];
	char 						szSenderName[24];
	int 						nRentHourPeriod;
	char 						szMessage[128];
	unsigned char 	nDummy2[2];
	unsigned short 				year;	// received year.
	unsigned char 				month;	// received month.
	unsigned char 				day;	// received day.
	unsigned char 				hour;	// received hour.
	unsigned char 	nDummy3[2];
	unsigned char 				nItemNodeCount;
	unsigned char 	nDummy4[2];
	int 						nItemID[5];
};

struct MTD_BlitzShopItemNode
{
	int 	nIndex;
	int 	nItemID;
	int 	nRentHourPeriod;
	int 	nPrice;
	float 	fPriceDown;	// (selling price / 4) * (discounting percent + 100%)
	int 	nCount;
};

struct MTD_BlitzPlayerScore
{
	unsigned short 	nWin;
	unsigned short 	nLose;
	int 			nPoint;
	int 			nMedal;
	int 			nUnknown1;
	int 			nUnknown2;
};

struct MTD_BlitzFriendNode
{
	MUID 	uidPlayer;
	char 	szPlayerName[CHARNAME_LEN];
};

struct MTD_BlitzChallengerNode
{
	char 	szPlayerName[CHARNAME_LEN];
};

struct MTD_BlitzWaitingPlayerNode
{
	char 			szPlayerName[12 + 1];
	int 			nPlayerLevel;
	unsigned short 	nPlayerWin;
	unsigned short 	nPlayerLose;
	int 			nPlayerMedal;
	int 			nUnknown1;
};

struct MTD_BlitzAvailableClassInfo
{
	bool bActive[MBLITZ_CLASS_MAX];
};

struct MTD_BlitzAssisterInfoNode
{
	MUID 			uidAssister;
	unsigned int 	nGivenHonor;
};

struct MTD_BlitzPlayerResultInfoNode
{
	MUID uidPlayer;
	int nLevelPercent;	// EXP %.
	int nExp;			// Gain EXP.
	int nBounty;		// Gain BP.
	int nMedal;			// Gain Medal.
	int nHonorPoint;	// Honor point In-game.
	int nBlitzPoint;	// Total Blitz point.
};

struct MTD_SpyItemNode
{
	int nItemID;
	int nItemCount;
};

struct MTD_SpyRoundFinishInfo
{
	MUID uidPlayer;
	int nXP;
	int nPercent;
	int nBP;
	int nPoint;
};

struct MTD_SpyPlayerScoreInfo
{
	MUID uidPlayer;
	int nWin;
	int nLose;
	int nPoint;
};
#pragma pack()

// ---------- MCommand Blob with alignments. ----------
struct MTD_ShopItemInfo
{
	unsigned int	nItemID;
	int				nItemCount;
};

struct MCHANNELLISTNODE
{
	MUID			uidChannel;
	short			nNo;
	unsigned char	nPlayers;
	short			nMaxPlayers;
	short			nLevelMin;
	short			nLevelMax;
	char			nChannelType;
	char			szChannelName[/*CHANNELNAME_LEN*/ 64];
	char			szChannelNameStrResId[/*CHANNELNAME_STRINGRESID_LEN*/ 64];
	bool			bIsUseTicket;
	unsigned int	nTicketID;
};

struct MSTAGE_SETTING_NODE 
{
	MUID		uidStage;
	char		szMapName[MAX_STAGE_MAPNAME_LENGTH];
	char		nMapIndex;
	int			nGameType;
	int			nRoundMax;
	int			nLimitTime;
	int			nLimitLevel;
	int			nMaxPlayers;
	bool		bTeamKillEnabled;
	bool		bTeamWinThePoint;
	bool		bForcedEntryEnabled;
	bool		bAutoTeamBalancing;
	bool		bIsRelayMap;
	bool		bIsStartRelayMap;
	int			nRelayMapListCount;
	int			nMapList[MAX_RELAYMAP_ELEMENT_COUNT];
	int			nRelayMapType;
	int			nRelayMapRepeatCount;
	
	/*
	bool		bVoteEnabled;
	bool		bObserverEnabled;
	*/
};

struct MTD_ObjectCache
{
	MUID					uidObject;
	char					szName[CHARNAME_LEN];
	char					szClanName[CLANNAME_LEN];
	char					nLevel;
	int						nUGrade;
	int						nPGrade;
	unsigned char			nPlayerFlags;		
	unsigned int			nCLID;			
	unsigned int			nEmblemChecksum;	
	int 					nSex;
	unsigned char			nHair;
	unsigned char			nFace;
	unsigned long int 		nEquipedItemID[MMCIP_END];
	unsigned int			nRank;
	int						nKillCount;
	int						nDeathCount;
	int						nDTGrade;			
};

struct MSTAGE_CHAR_SETTING_NODE 
{
	MUID	uidChar;
	int		nTeam;
	int		nState;
};

struct MTD_DTPlayerInfo
{
	char szName[CHARNAME_LEN];
	MUID uidPlayer;
	int nTP;
};

struct MTD_DTRankingInfo
{
	char szCharName[CHARNAME_LEN];
	int nTP;
	int nWins;
	int nLoses;
	int nRanking;
	int nRankingIncrease;
	int nFinalWins;
	int nGrade;
};

#define MAX_GAMBLEITEM_NAME_LEN		65
#if _GAME_VERSION >= 2012
	#define MAX_GAMBLEITEM_DESC_LEN		128
#else
	#define MAX_GAMBLEITEM_DESC_LEN		65
#endif

struct MTD_GambleItemNode
{
	MUID			uidItem;
	unsigned int	nItemID;							
	unsigned int	nItemCnt;
	
	#if _GAME_VERSION >= 2012
	int				nUnknown1;
	#endif
};

struct MTD_DBGambleItemNode
{
	unsigned int	nItemID;							
	char			szName[MAX_GAMBLEITEM_NAME_LEN];	
	char			szDesc[MAX_GAMBLEITEM_DESC_LEN];	
	int				nBuyPrice;							
	bool			bIsCash;							
};

struct MTD_DBGambleItemNode2012
{
	unsigned int	nItemID;							
	char			szName[MAX_GAMBLEITEM_NAME_LEN];	
	char			szDesc[MAX_GAMBLEITEM_DESC_LEN];	
	int				nBuyPrice;							
	int 			nCurrencyType;
};

#endif