#include "pch.h"

#include "MServerSetting.h"
#include "MClientAcceptor.h"

#include "MMatchConstant.h"
#include "MMatchObject_Constant.h"
#include "MMessageID.h"

#include "MMatchDBMgr.h"
#include "MAsyncDBProcess.h"

#include "MMatchObject.h"

#include "MMatchExp.h"

#include "MMatchServer_OnCommand.h"

#include "MMatchClan.h"

#include "MMatchGambleItem.h"

void SendChannelList();
void CheckExpiredCharItem(MMatchObject *pObj);

/*
void CreateLoginResultCommand(MCmdWriter *pCmd, int nResult, const char *pszUserID, int nUGradeID, int nPGradeID, const MUID &uidPlayer)
{
	pCmd->WriteInt(nResult);
	pCmd->WriteString(g_ServerConfig.ServerSetting.szName);
	pCmd->WriteUChar(g_ServerConfig.ServerSetting.nServerMode);
	pCmd->WriteString(pszUserID);
	pCmd->WriteUChar((unsigned char)nUGradeID);
	pCmd->WriteUChar((unsigned char)nPGradeID);
	pCmd->WriteMUID(uidPlayer);
	pCmd->WriteBool(g_ServerConfig.ServerSetting.bSurvival);
	pCmd->WriteBool(g_ServerConfig.ServerSetting.bDuelTournament);

	pCmd->StartBlob(20);
	pCmd->WriteSkip(20);
	pCmd->EndBlob();

	pCmd->Finalize(MC_MATCH_RESPONSE_LOGIN);
}
*/

bool OnMatchLogin(const MUID &uidPlayer, const char *pszUserID, const char *pszPassword, int nCommandVersion)
{
	Socket::socket_type s;
	unsigned char nCryptKey[ENCRYPTIONKEY_LENGTH];

	if(g_ClientAcceptor.GetClientInfo(uidPlayer, &s, nCryptKey) == false) return false;	// unknown error.

	/*
	MCmdWriter Cmd;

	if(nCommandVersion != 58)	 	// MCommand version.
	{
		CreateLoginResultCommand(&Cmd, MERR_WRONG_CMDVERSION, pszUserID, 0, 0, uidPlayer);
		Cmd.Finalize();
		SendToClient(&Cmd, s, nCryptKey);
		return false;
	}

	// TODO : Free Login IP (can ignore server max players) check.
	
	if(g_ObjectMgr.Size() >= (int)g_ServerConfig.ServerSetting.nMaxPlayers)
	{
		CreateLoginResultCommand(&Cmd, MERR_SERVER_FULL, pszUserID, 0, 0, uidPlayer);
		Cmd.Finalize();
		SendToClient(&Cmd, s, nCryptKey);
		return false;
	}
	
	Socket::address_type addr;
	Socket::GetPeerName(s, &addr);
	
	char szIP[64];
	strcpy(szIP, Socket::InetNtoa(&addr));
	
	// unsigned long nIP = Socket::InetAddr(szIP);
	
	if(g_ServerConfig.OtherSetting.bIpBan == true)
	{
		int nResult;
		
		if(Db_IsBannedIP(szIP, &nResult) == false)
		{
			CreateLoginResultCommand(&Cmd, MERR_LOGIN_FAILED, pszUserID, 0, 0, uidPlayer);
			Cmd.Finalize();
			SendToClient(&Cmd, s, nCryptKey);
			return false;
		}
		
		if(nResult == 1)
		{
			CreateLoginResultCommand(&Cmd, MERR_BANNED_ID, pszUserID, 0, 0, uidPlayer);
			Cmd.Finalize();
			SendToClient(&Cmd, s, nCryptKey);
			return false;
		}
	}
	
	#ifdef _UNIQUE_KEY_AUTH		// login with unique keys.
	// TODO : Complete this...
	
	pszUserID = szIP;
	pszPassword = "";
	
	if(Db_InsertAccount(pszUserID, pszPassword) == false)
	{
		CreateLoginResultCommand(&Cmd, MERR_LOGIN_FAILED, pszUserID, 0, 0, uidPlayer);
		Cmd.Finalize();
		SendToClient(&Cmd, s, nCryptKey);
		return false;
	}
	
	int nAID;
	char szDbPassword[MAX_USERPASSWORD_LEN];
	int nUGradeID, nPGradeID;

	if(Db_GetAccountInfo(pszUserID, &nAID, szDbPassword, &nUGradeID, &nPGradeID) == false)
	{
		CreateLoginResultCommand(&Cmd, MERR_UNKNOWN_ACCOUNTINFO, pszUserID, 0, 0, uidPlayer);
		Cmd.Finalize();
		SendToClient(&Cmd, s, nCryptKey);
		return false;
	}

	if(strcmp(szDbPassword, pszPassword) != 0)
	{
		CreateLoginResultCommand(&Cmd, MERR_WRONG_LOGINID, pszUserID, nUGradeID, nPGradeID, uidPlayer);
		Cmd.Finalize();
		SendToClient(&Cmd, s, nCryptKey);
		return false;
	}
	#else
	#ifdef _DEBUG	// Auto account Register.
	if(Db_InsertAccount(pszUserID, pszPassword) == false)
	{
		CreateLoginResultCommand(&Cmd, MERR_LOGIN_FAILED, pszUserID, 0, 0, uidPlayer);
		Cmd.Finalize();
		SendToClient(&Cmd, s, nCryptKey);
		return false;
	}
	#endif	// (_DEBUG)
	
	int nAID;
	char szDbPassword[MAX_USERPASSWORD_LEN];
	int nUGradeID, nPGradeID;

	if(Db_GetAccountInfo(pszUserID, &nAID, szDbPassword, &nUGradeID, &nPGradeID) == false)
	{
		CreateLoginResultCommand(&Cmd, MERR_UNKNOWN_ACCOUNTINFO, pszUserID, 0, 0, uidPlayer);
		Cmd.Finalize();
		SendToClient(&Cmd, s, nCryptKey);
		return false;
	}

	if(strcmp(szDbPassword, pszPassword) != 0)
	{
		CreateLoginResultCommand(&Cmd, MERR_WRONG_LOGINID, pszUserID, nUGradeID, nPGradeID, uidPlayer);
		Cmd.Finalize();
		SendToClient(&Cmd, s, nCryptKey);
		return false;
	}
	#endif	// (_UNIQUE_KEY_AUTH)
	
	Db_UpdateConnData(nAID, szIP);
	
	if(nUGradeID == MMUG_BLOCKED)
	{
		CreateLoginResultCommand(&Cmd, MERR_BANNED_ID, pszUserID, nUGradeID, nPGradeID, uidPlayer);
		Cmd.Finalize();
		SendToClient(&Cmd, s, nCryptKey);
		return false;
	}
	
	#ifdef _RELEASE
	unsigned long nIP = Socket::InetAddr(szIP);
	
	for(list<MMatchObject *>::iterator i = g_ObjectMgr.Begin(); i != g_ObjectMgr.End(); i++)
	{
		MMatchObject *pCurr = (*i);
		
		if(pCurr->GetIP() == nIP)
		{
			// block same ip - useful for anti connection flood.
			
			MCmdWriter Cmd2;
			CreateLoginResultCommand(&Cmd2, MERR_ALREADY_CONNECTED, pszUserID, nUGradeID, nPGradeID, pCurr->GetUID());
			Cmd2.Finalize();
			SendToClient(&Cmd2, pCurr->GetUID());
			
			pCurr->Disconnect();
		}
		else if(pCurr->m_Account.nAID == nAID)
		{
			MCmdWriter Cmd2;
			CreateLoginResultCommand(&Cmd2, MERR_MULTIPLE_LOGIN, pszUserID, nUGradeID, nPGradeID, pCurr->GetUID());
			Cmd2.Finalize();
			SendToClient(&Cmd2, pCurr->GetUID());
			
			// TIP : + disconnect process, if need.
		}
	}
	#endif
	
	MMatchObject *pObj = g_ObjectMgr.Add(s, uidPlayer, nCryptKey);

	// set basic account info.
	pObj->m_Account.nAID = nAID;
	pObj->m_Account.nUGradeID = nUGradeID;
	pObj->m_Account.nPGradeID = nPGradeID;
	
	// account related in-game info.
	//  - account item.
	vector<DbData_AccItemList> vtAccountItem;
	Db_GetAccountItemList(nAID, &vtAccountItem);
	
	for(vector<DbData_AccItemList>::iterator i = vtAccountItem.begin(); i != vtAccountItem.end(); i++)
	{
		DbData_AccItemList *pCurr = &(*i);
		pObj->AddAccItem(pCurr->nAIID, pCurr->nItemID, pCurr->nItemCount, pCurr->nRentSecPeriod);
	}
	
	//  - challenge quest.
	vector<DbData_CQRecordInfo> vtCQRecordInfo;
	Db_GetCQRecord(nAID, &vtCQRecordInfo);
	
	for(vector<DbData_CQRecordInfo>::iterator i = vtCQRecordInfo.begin(); i != vtCQRecordInfo.end(); i++)
	{
		DbData_CQRecordInfo *pCurr = &(*i);
		pObj->AddCQRecord(pCurr->nScenarioID, pCurr->nTime);
	}
	
	CreateLoginResultCommand(&Cmd, MSG_OK, pszUserID, nUGradeID, nPGradeID, uidPlayer);
	Cmd.Finalize();
	SendToClient(&Cmd, s, nCryptKey);
	
	g_ClientAcceptor.RemoveClient(s);
	
	return true;
	*/
	
	Socket::address_type addr;
	Socket::GetPeerName(s, &addr);
	
	char szIP[64];
	strcpy(szIP, Socket::InetNtoa(&addr));
	
	if(g_ServerConfig.OtherSetting.bIpAccount == true)
	{
		MAsyncDBTask_MatchLogin *pNew = new MAsyncDBTask_MatchLogin(uidPlayer, szIP, "", nCommandVersion, szIP, g_ServerConfig.OtherSetting.bIpBan);
		g_AsyncDBTaskMgr.SafeAdd(pNew);
	}
	else
	{
		MAsyncDBTask_MatchLogin *pNew = new MAsyncDBTask_MatchLogin(uidPlayer, pszUserID, pszPassword, nCommandVersion, szIP, g_ServerConfig.OtherSetting.bIpBan);
		g_AsyncDBTaskMgr.SafeAdd(pNew);
	}
	
	return true;
}

void OnAccountCharList(const MUID &uidPlayer)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;

	/*
	int nCharCount;
	DbData_AccountCharList CharInfo[ACCOUNT_CHARLIST_COUNT];

	if(Db_GetAccountCharList(pObj->m_Account.nAID, &nCharCount, CharInfo) == false)
		return;

	MCmdWriter Cmd;

	Cmd.StartBlob(34);
	for(int i = 0; i < nCharCount; i++)
	{
		Cmd.WriteString(CharInfo[i].szName, CHARNAME_LEN);
		Cmd.WriteChar((char)CharInfo[i].nCharNum);
		Cmd.WriteUChar((unsigned char)CharInfo[i].nLevel);
	}
	Cmd.EndBlob();

	Cmd.Finalize(MC_MATCH_RESPONSE_ACCOUNT_CHARLIST, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
	
	pObj->Clean();
	
	SendChannelList();
	
	#ifdef _DEBUG
	// auto char select (index 0) for faster debugging.
	if(nCharCount >= 1)
	{
		OnSelectCharacter(uidPlayer, 0);
	}
	#endif
	*/
	
	MAsyncDBTask_AccountCharList *pNew = new MAsyncDBTask_AccountCharList(uidPlayer, pObj->m_Account.nAID);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void OnAccountCharInfo(const MUID &uidPlayer, int nCharNum)
{
	if(nCharNum < 0 || nCharNum >= ACCOUNT_CHARLIST_COUNT) return;

	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;

	/*
	int nCID;

	// cid from charnum.
	if(Db_GetCharIndex(pObj->m_Account.nAID, nCharNum, &nCID) == false)
		return;

	// char info.
	DbData_BasicCharInfo CharInfo;

	if(Db_GetBasicCharInfo(nCID, &CharInfo) == false)
		return;

	// clan info.
	DbData_BasicClanInfo ClanInfo = {0, "", 0, 0};
	Db_GetBasicClanInfo(nCID, &ClanInfo);

	// item info.
	DbData_CharItemInfo ItemInfo;
	Db_GetCharItemInfo(nCID, &ItemInfo);

	// dt class info.
	int nDTClass;
	Db_GetDTCharClass(nCID, &nDTClass);

	MCmdWriter Cmd;

	Cmd.WriteChar((char)nCharNum);

	#if _GAME_VERSION >= 2012
	Cmd.StartBlob(382);
	Cmd.WriteString(CharInfo.szName, CHARNAME_LEN);
	Cmd.WriteString(ClanInfo.szClanName, CLANNAME_LEN);
	Cmd.WriteInt(ClanInfo.nMemberGrade);
	Cmd.WriteUShort((unsigned short)ClanInfo.nClanPoint);
	Cmd.WriteChar((char)nCharNum);
	Cmd.WriteUShort((unsigned short)CharInfo.nLevel);
	Cmd.WriteChar((char)CharInfo.nSex);
	Cmd.WriteChar((char)CharInfo.nHair);
	Cmd.WriteChar((char)CharInfo.nFace);
	Cmd.WriteInt(CharInfo.nExp);
	Cmd.WriteInt(CharInfo.nBounty);
	Cmd.WriteSkip(22);
	for(int i = 0; i < MMCIP_END; i++)
	{
		Cmd.WriteInt(ItemInfo.nItemID[i]);
	}
	Cmd.WriteInt(pObj->m_Account.nUGradeID);
	Cmd.WriteInt(ClanInfo.nCLID);
	Cmd.WriteInt(nDTClass);
	for(int i = 0; i < MMCIP_END; i++)
	{
		// item UID. 0:0.
		Cmd.WriteMUID(MUID(0, 0));
	}
	for(int i = 0; i < MMCIP_END; i++)
	{
		Cmd.WriteInt(ItemInfo.nItemCount[i]);
	}
	Cmd.WriteInt(0);	// ...?
	Cmd.WriteInt(0);	// ...?
	Cmd.EndBlob();
	#else
	Cmd.StartBlob(374);
	Cmd.WriteString(CharInfo.szName, CHARNAME_LEN);
	Cmd.WriteString(ClanInfo.szClanName, CLANNAME_LEN);
	Cmd.WriteInt(ClanInfo.nMemberGrade);
	Cmd.WriteUShort((unsigned short)ClanInfo.nClanPoint);
	Cmd.WriteChar((char)nCharNum);
	Cmd.WriteUShort((unsigned short)CharInfo.nLevel);
	Cmd.WriteChar((char)CharInfo.nSex);
	Cmd.WriteChar((char)CharInfo.nHair);
	Cmd.WriteChar((char)CharInfo.nFace);
	Cmd.WriteInt(CharInfo.nExp);
	Cmd.WriteInt(CharInfo.nBounty);
	Cmd.WriteSkip(22);
	for(int i = 0; i < MMCIP_END; i++)
	{
		Cmd.WriteInt(ItemInfo.nItemID[i]);
	}
	Cmd.WriteInt(pObj->m_Account.nUGradeID);
	Cmd.WriteInt(ClanInfo.nCLID);
	Cmd.WriteInt(nDTClass);
	for(int i = 0; i < MMCIP_END; i++)
	{
		// item UID. 0:0.
		Cmd.WriteMUID(MUID(0, 0));
	}
	for(int i = 0; i < MMCIP_END; i++)
	{
		Cmd.WriteInt(ItemInfo.nItemCount[i]);
	}
	Cmd.EndBlob();
	#endif

	Cmd.Finalize(MC_MATCH_RESPONSE_ACCOUNT_CHARINFO, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
	*/
	
	MAsyncDBTask_AccountCharInfo *pNew = new MAsyncDBTask_AccountCharInfo(uidPlayer, pObj->m_Account.nAID, nCharNum);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void OnCreateCharacter(const MUID &uidPlayer, int nCharIndex, const char *pszName, int nSex, int nHair, int nFace, int nCostume)
{
	if(nCharIndex < 0 || nCharIndex >= ACCOUNT_CHARLIST_COUNT) return;
	if(nSex != MMS_MALE && nSex != MMS_FEMALE) return;
	if(nHair < 0 || nHair >= CHAR_HAIR_COUNT) return;
	if(nFace < 0 || nFace >= CHAR_FACE_COUNT) return;
	if(nCostume < 0 || nCostume >= CHAR_COSTUME_COUNT) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
		
	if(pObj->m_bCharInfoExist == true) return;

	int nCharCount;

	if(Db_GetCharacterCount(pObj->m_Account.nAID, &nCharCount) == false)
		return;

	if(nCharCount >= ACCOUNT_CHARLIST_COUNT)
		return;

	int nResult = MSG_OK;

	int nNameSize = (int)strlen(pszName);
	if(nNameSize < MIN_CHARNAME_LEN)
	{
		nResult = MERR_NAME_SHORT;
	}
	else if(nNameSize > MAX_CHARNAME_LEN)
	{
		nResult = MERR_NAME_LONG;
	}
	else
	{
		int nRet;

		if(Db_IsCharacterExists(pszName, &nRet) == false)
		{
			return;
		}
		if(nRet == 1)
		{
			nResult = MERR_CHARACTER_EXISTS;
		}
	}

	if(nResult == MSG_OK)
	{
		if(Db_InsertCharacter(pObj->m_Account.nAID, pszName, nSex, nHair, nFace, nCostume) == false)
		{
			return;
		}
	}

	MCmdWriter Cmd;

	Cmd.WriteInt(nResult);
	Cmd.WriteString(pszName);
	Cmd.Finalize(MC_MATCH_RESPONSE_CREATE_CHAR, MCFT_END);

	SendToClient(&Cmd, uidPlayer);
}

void OnDeleteCharacter(const MUID &uidPlayer, int nCharIndex, const char *pszCharName)
{
	if(nCharIndex < 0 || nCharIndex >= ACCOUNT_CHARLIST_COUNT) return;

	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
		
	if(pObj->m_bCharInfoExist == true) return;

	MCmdWriter Cmd;

	int nCID;

	if(Db_GetCharIndex(pObj->m_Account.nAID, nCharIndex, &nCID) == false)
	{
		CreateResultCommand(&Cmd, MC_MATCH_RESPONSE_DELETE_CHAR, MERR_CHARACTER_NOT_EXISTS);
		Cmd.Finalize();
		SendToClient(&Cmd, uidPlayer);
		return;
	}

	char szName[CHARNAME_LEN];

	if(Db_GetCharacterName(nCID, szName) == false)
	{
		CreateResultCommand(&Cmd, MC_MATCH_RESPONSE_DELETE_CHAR, MERR_CHARACTER_NOT_EXISTS);
		Cmd.Finalize();
		SendToClient(&Cmd, uidPlayer);
		return;
	}

	if(strcmp(szName, pszCharName) != 0)
	{
		CreateResultCommand(&Cmd, MC_MATCH_RESPONSE_DELETE_CHAR, MERR_DELETE_CHARACTER_FAIL);
		Cmd.Finalize();
		SendToClient(&Cmd, uidPlayer);
		return;
	}

	/*
	TODO :
	[done] clan joined or not check,
	cash item having check,
	[done] remove from friend,
	[done] remove from survival ranking,
	[done] remove from dueltournament ranking.
	*/
	
	DbData_BasicClanInfo ClanInfo = {0, "", 0, 0};
	if(Db_GetBasicClanInfo(nCID, &ClanInfo) == false) return;
	
	if(ClanInfo.nCLID != 0)
	{
		CreateResultCommand(&Cmd, MC_MATCH_RESPONSE_DELETE_CHAR, MERR_DELETE_CHARACTER_FAIL);
		Cmd.Finalize();
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	// clear friend : removed from here and moved to deletecharacter() function on DB.
	/*
	if(Db_ClearTargetFriend(nCID) == false)
	{
		CreateResultCommand(&Cmd, MC_MATCH_RESPONSE_DELETE_CHAR, MERR_DELETE_CHARACTER_FAIL);
		Cmd.Finalize();
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	*/

	if(Db_DeleteCharacter(nCID) == false)
	{
		CreateResultCommand(&Cmd, MC_MATCH_RESPONSE_DELETE_CHAR, MERR_DELETE_CHARACTER_FAIL);
		Cmd.Finalize();
		SendToClient(&Cmd, uidPlayer);
		return;
	}

	CreateResultCommand(&Cmd, MC_MATCH_RESPONSE_DELETE_CHAR, MSG_OK);
	Cmd.Finalize();
	SendToClient(&Cmd, uidPlayer);
}

void OnSelectCharacter(const MUID &uidPlayer, int nCharIndex)
{
	if(nCharIndex < 0 || nCharIndex >= ACCOUNT_CHARLIST_COUNT) return;
		
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
		
	if(pObj->m_bCharInfoExist == true) return;

	/*
	int nCID;

	// cid from charnum.
	if(Db_GetCharIndex(pObj->m_Account.nAID, nCharIndex, &nCID) == false)
		return;

	// char info.
	DbData_BasicCharInfo CharInfo;

	if(Db_GetBasicCharInfo(nCID, &CharInfo) == false)
		return;
		
	// clan info.
	DbData_BasicClanInfo ClanInfo = {0, "", 0, 0};
	Db_GetBasicClanInfo(nCID, &ClanInfo);

	// item info.
	DbData_CharEquippedItemCIID ItemInfo;
	Db_GetCharEquipmentSlotCIID(nCID, &ItemInfo);

	// dt character info.
	DbData_DTCharInfo DTInfo;
	ZeroInit(&DTInfo, sizeof(DbData_DTCharInfo));
	
	Db_GetDTCharInfo(nCID, &DTInfo);
	
	// all item list.
	struct _eif
	{
		MUID uidItem;
		int nItemID;
		int nItemCount;
	} EquipItemInfo[MMCIP_END];
	ZeroInit(EquipItemInfo, sizeof(_eif) * MMCIP_END);
	
	vector<DbData_CharItemList> vtItemList;
	
	if(Db_GetCharacterItemList(nCID, &vtItemList) == false)
		return;
		
	for(vector<DbData_CharItemList>::iterator i = vtItemList.begin(); i != vtItemList.end(); i++)
	{
		DbData_CharItemList *pci = &(*i);
		MUID uidAssigned = MUID::Assign();
		
		pObj->AddCharItem(uidAssigned, pci->nCIID, pci->nItemID, pci->nItemCount, pci->nRentSecPeriod);
		
		for(int j = 0; j < MMCIP_END; j++)
		{
			if(ItemInfo.nCIID[j] == pci->nCIID)
			{
				pObj->SetItemUID(uidAssigned, j);
				
				EquipItemInfo[j].uidItem = uidAssigned;
				EquipItemInfo[j].nItemID = pci->nItemID;
				EquipItemInfo[j].nItemCount = pci->nItemCount;
				
				break;
			}
		}
	}
	
	// gamble item list.
	vector<DbData_CharGambleItem> vtCharGambleItem;
	if(Db_GetCharGambleItem(nCID, &vtCharGambleItem) == false) return;
	
	for(vector<DbData_CharGambleItem>::iterator i = vtCharGambleItem.begin(); i != vtCharGambleItem.end(); i++)
	{
		DbData_CharGambleItem *pgi = &(*i);
		MUID uidAssigned = MUID::Assign();
		
		pObj->AddCharGambleItem(uidAssigned, pgi->nGIID, pgi->nItemID, pgi->nCount);
	}
	
	// advanced clan info.
	if(ClanInfo.nCLID != 0)
	{
		DbData_ClanInfo AdvClanInfo;
		
		if(Db_GetClanInfo(ClanInfo.nCLID, &AdvClanInfo) == true)
		{
			MMatchClan *pClan = g_ClanMgr.Add(AdvClanInfo.nCLID, 
										AdvClanInfo.szName, 
										AdvClanInfo.nLevel, 
										AdvClanInfo.nPoint, 
										AdvClanInfo.nMasterCID, 
										AdvClanInfo.nWin, 
										AdvClanInfo.nLose, 
										AdvClanInfo.nDraw, 
										AdvClanInfo.nTotalPoint, 
										AdvClanInfo.nRanking, 
										AdvClanInfo.szEmblemURL, 
										AdvClanInfo.nEmblemChecksum);
			if(pClan != NULL)
			{
				pClan->Join(pObj);
			}
		}
	}
	
	// friend list info.
	pObj->ClearFriendList();	// clear existing info.
	
	vector<DbData_FriendListNode> vtFriendListNode;
	
	if(Db_GetFriendList(nCID, &vtFriendListNode) == true)
	{
		for(vector<DbData_FriendListNode>::iterator i = vtFriendListNode.begin(); i != vtFriendListNode.end(); i++)
		{
			DbData_FriendListNode *pNode = &(*i);
			pObj->AddFriend(pNode->nFriendID, pNode->nCID, pNode->szCharName);
		}
	}
	
	// quest item.
	pObj->InitQuestItem(CharInfo.szQuestItemData);
	
	// survival info.
	DbData_CharSurvivalInfo SurvivalInfo;
	Db_GetSurvivalCharInfo(nCID, &SurvivalInfo);
	
	pObj->m_Survival.nPoint = SurvivalInfo.nPoint;
	pObj->m_Survival.nRanking = SurvivalInfo.nRanking;
	
		
		
	MCmdWriter Cmd;
	
	Cmd.WriteInt(0);	// result = ok.
	
	#if _GAME_VERSION >= 2012
	Cmd.StartBlob(382);
	Cmd.WriteString(CharInfo.szName, CHARNAME_LEN);
	Cmd.WriteString(ClanInfo.szClanName, CLANNAME_LEN);
	Cmd.WriteInt(ClanInfo.nMemberGrade);
	Cmd.WriteUShort((unsigned short)ClanInfo.nClanPoint);
	Cmd.WriteChar((char)nCharIndex);
	Cmd.WriteUShort((unsigned short)CharInfo.nLevel);
	Cmd.WriteChar((char)CharInfo.nSex);
	Cmd.WriteChar((char)CharInfo.nHair);
	Cmd.WriteChar((char)CharInfo.nFace);
	Cmd.WriteInt(CharInfo.nExp);
	Cmd.WriteInt(CharInfo.nBounty);
	Cmd.WriteSkip(22);
	for(int i = 0; i < MMCIP_END; i++)
	{
		Cmd.WriteInt(EquipItemInfo[i].nItemID);
	}
	Cmd.WriteInt(pObj->m_Account.nUGradeID);
	Cmd.WriteInt(ClanInfo.nCLID);
	Cmd.WriteInt(DTInfo.nGrade);
	for(int i = 0; i < MMCIP_END; i++)
	{
		// item uid. assigned above.
		Cmd.WriteMUID(EquipItemInfo[i].uidItem);
	}
	for(int i = 0; i < MMCIP_END; i++)
	{
		Cmd.WriteInt(EquipItemInfo[i].nItemCount);
	}
	Cmd.WriteInt(0);	// ...?
	Cmd.WriteInt(0);	// ...?
	Cmd.EndBlob();
	#else
	Cmd.StartBlob(374);
	Cmd.WriteString(CharInfo.szName, CHARNAME_LEN);
	Cmd.WriteString(ClanInfo.szClanName, CLANNAME_LEN);
	Cmd.WriteInt(ClanInfo.nMemberGrade);
	Cmd.WriteUShort((unsigned short)ClanInfo.nClanPoint);
	Cmd.WriteChar((char)nCharIndex);
	Cmd.WriteUShort((unsigned short)CharInfo.nLevel);
	Cmd.WriteChar((char)CharInfo.nSex);
	Cmd.WriteChar((char)CharInfo.nHair);
	Cmd.WriteChar((char)CharInfo.nFace);
	Cmd.WriteInt(CharInfo.nExp);
	Cmd.WriteInt(CharInfo.nBounty);
	Cmd.WriteSkip(22);
	for(int i = 0; i < MMCIP_END; i++)
	{
		Cmd.WriteInt(EquipItemInfo[i].nItemID);
	}
	Cmd.WriteInt(pObj->m_Account.nUGradeID);
	Cmd.WriteInt(ClanInfo.nCLID);
	Cmd.WriteInt(DTInfo.nGrade);
	for(int i = 0; i < MMCIP_END; i++)
	{
		// item uid. assigned above.
		Cmd.WriteMUID(EquipItemInfo[i].uidItem);
	}
	for(int i = 0; i < MMCIP_END; i++)
	{
		Cmd.WriteInt(EquipItemInfo[i].nItemCount);
	}
	Cmd.EndBlob();
	#endif
	
	Cmd.StartBlob(1);
	Cmd.WriteChar((char)g_FormulaMgr.GetExpPercent(CharInfo.nLevel, CharInfo.nExp));
	Cmd.EndBlob();
	
	if(CheckGameVersion(2012) == true)
	{
		// TODO : 2013 cmd.
		Cmd.StartBlob(20);
		Cmd.WriteShort(0);	// win.
		Cmd.WriteShort(0);	// lose.
		Cmd.WriteInt(0);		// blitz point.
		Cmd.WriteInt(0);	// medals.
		Cmd.WriteInt(0);
		Cmd.WriteInt(0);
		Cmd.EndBlob();
		
		// Use MTD_BlitzPlayerScore.
		// 
		// 1, UShort Wins
		// 2, UShort Losses, 
		// 3, Int Points.
		// 4, Int Medals.
		// 5, Int ???. (Mine is 1)
		// 6, Int ???. (Mine is 5)
	}
	
	Cmd.Finalize(MC_MATCH_RESPONSE_SELECT_CHAR, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
	
	// set some info.
	pObj->m_Char.nCID = nCID;
	strcpy(pObj->m_Char.szName, CharInfo.szName);
	pObj->m_Char.nSex = CharInfo.nSex;
	pObj->m_Char.nHair = CharInfo.nHair;
	pObj->m_Char.nFace = CharInfo.nFace;
	
	pObj->m_Exp.nLevel = CharInfo.nLevel;
	pObj->m_Exp.nXP = CharInfo.nExp;
	pObj->m_Exp.nBP = CharInfo.nBounty;
	pObj->m_Exp.nKill = CharInfo.nKillCount;
	pObj->m_Exp.nDeath = CharInfo.nDeathCount;
	pObj->m_Exp.nRanking = CharInfo.nRanking;
	
	MMatchCharDuelTournamentInfo *pCharDTInfo = &pObj->m_DuelTournament;
	pCharDTInfo->nClass = DTInfo.nGrade;
	pCharDTInfo->nTP = DTInfo.nTournamentPoint;
	pCharDTInfo->nWin = DTInfo.nWins;
	pCharDTInfo->nLose = DTInfo.nLosses;
	pCharDTInfo->nFinalWin = DTInfo.nFinalMatchWin;
	pCharDTInfo->nRanking = DTInfo.nRanking;
	pCharDTInfo->nPrevTP = DTInfo.nPreviousTournamentPoint;
	pCharDTInfo->nPrevWin = DTInfo.nPreviousWins;
	pCharDTInfo->nPrevLose = DTInfo.nPreviousLosses;
	pCharDTInfo->nPrevFinalWin = DTInfo.nPreviousFinalMatchWin;
	pCharDTInfo->nPrevRanking = DTInfo.nPreviousRanking;
	pCharDTInfo->nRankingDiff = DTInfo.nRankingDifferent;
	
	pObj->m_Clan.nCLID = ClanInfo.nCLID;
	pObj->m_Clan.nMemberGrade = ClanInfo.nMemberGrade;
	
	pObj->m_bCharInfoExist = true;
	
	// check & delete expired rent item.
	CheckExpiredCharItem(pObj);
	
	// send gamble item info.
	// g_GItemMgr.SendInfo(uidPlayer);
	
	// send shop item list.
	OnShopItemList(uidPlayer);
	
	// send char item list.
	OnCharacterItemList(uidPlayer);
	
	// notify member logged-in, to clan members.
	MCmdWriter Cmd2Clan;
	Cmd2Clan.WriteString(CharInfo.szName);
	Cmd2Clan.Finalize(MC_MATCH_CLAN_MEMBER_CONNECTED, MCFT_END);
	SendToClan(&Cmd2Clan, ClanInfo.nCLID);
	
	SendChannelList();
	*/
	
	MAsyncDBTask_SelectCharacter *pNew = new MAsyncDBTask_SelectCharacter(uidPlayer, pObj->m_Account.nAID, nCharIndex);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}
