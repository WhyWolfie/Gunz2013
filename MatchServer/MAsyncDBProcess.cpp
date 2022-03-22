#include "pch.h"
#include "MAsyncDBProcess.h"
#include "MMatchDBMgr.h"

#include "MMatchObject.h"

#include "MMatchDuelTournament.h"
#include "MMatchQuest.h"

#include "MMessageID.h"
#include "MClientAcceptor.h"
#include "MServerSetting.h"

#include "MMatchClan.h"
#include "MMatchExp.h"

#include "MMatchGambleItem.h"

#include "MMatchServer_OnCommand.h"

MAsyncDBProcess g_AsyncDBTaskMgr;

MBaseAsyncDBTask::MBaseAsyncDBTask(const MUID &uidRequestor)
{
	m_uidRequestor = uidRequestor;
}

MBaseAsyncDBTask::~MBaseAsyncDBTask()
{
}

bool MBaseAsyncDBTask::OnExec()
{
	printf("Async DB process error : invalid OnExec() call.\n");
	return false;
}

void MBaseAsyncDBTask::OnDone()
{
	printf("Async DB process error : invalid OnDone() call.\n");
}



MAsyncDBProcess::MAsyncDBProcess()
{
}

MAsyncDBProcess::~MAsyncDBProcess()
{
	for(deque<MBaseAsyncDBTask *>::iterator i = m_AsyncDBTasks.begin(); i != m_AsyncDBTasks.end(); i++)
	{
		delete (*i);
	}
	m_AsyncDBTasks.clear();
}

/*
void MAsyncDBProcess::Add(MBaseAsyncDBTask *pTask)
{
	m_AsyncDBTasks.push_back(pTask);
}
*/

void MAsyncDBProcess::SafeAdd(MBaseAsyncDBTask *pTask)
{
	m_Mutex.lock();
	m_AsyncDBTasks.push_back(pTask);
	m_Mutex.unlock();
}

/*
MBaseAsyncDBTask *MAsyncDBProcess::FrontTask()
{
	MBaseAsyncDBTask *pTask = NULL;
	
	if(m_AsyncDBTasks.empty() == false)
	{
		pTask = m_AsyncDBTasks.front();
		m_AsyncDBTasks.pop_front();
	}
	
	return pTask;
}
*/

MBaseAsyncDBTask *MAsyncDBProcess::SafeFrontTask()
{
	MBaseAsyncDBTask *pTask = NULL;
	
	m_Mutex.lock();
	if(m_AsyncDBTasks.empty() == false)
	{
		pTask = m_AsyncDBTasks.front();
		m_AsyncDBTasks.pop_front();
	}
	m_Mutex.unlock();
	
	return pTask;
}

// -------------------------------------------------------------------------------------.



MAsyncDBTask_UpdateDTDailyRanking::MAsyncDBTask_UpdateDTDailyRanking() : MBaseAsyncDBTask(MUID(0, 0))
{
}

MAsyncDBTask_UpdateDTDailyRanking::~MAsyncDBTask_UpdateDTDailyRanking()
{
}

bool MAsyncDBTask_UpdateDTDailyRanking::OnExec()
{
	return Db_UpdateDTDailyRanking();
}

void MAsyncDBTask_UpdateDTDailyRanking::OnDone()
{
}


MAsyncDBTask_UpdateDTWeeklyRanking::MAsyncDBTask_UpdateDTWeeklyRanking() : MBaseAsyncDBTask(MUID(0, 0))
{
}

MAsyncDBTask_UpdateDTWeeklyRanking::~MAsyncDBTask_UpdateDTWeeklyRanking()
{
}

bool MAsyncDBTask_UpdateDTWeeklyRanking::OnExec()
{
	return Db_UpdateDTWeeklyRanking();
}

void MAsyncDBTask_UpdateDTWeeklyRanking::OnDone()
{
}


MAsyncDBTask_FetchDTTopRanking::MAsyncDBTask_FetchDTTopRanking() : MBaseAsyncDBTask(MUID(0, 0))
{
}

MAsyncDBTask_FetchDTTopRanking::~MAsyncDBTask_FetchDTTopRanking()
{
}

bool MAsyncDBTask_FetchDTTopRanking::OnExec()
{
	return Db_FetchDTTopRanking(&m_vtDbRankingInfo);
}

void MAsyncDBTask_FetchDTTopRanking::OnDone()
{
	g_DTRankingMgr.OnFetchRanking(m_vtDbRankingInfo);
}


MAsyncDBTask_InsertCQRecord::MAsyncDBTask_InsertCQRecord(const MUID &uidPlayer, int nAID, int nMapID, int nRecordTime) : MBaseAsyncDBTask(uidPlayer)
{
	m_nAID = nAID;
	m_nMapID = nMapID;
	m_nRecordTime = nRecordTime;
}

MAsyncDBTask_InsertCQRecord::~MAsyncDBTask_InsertCQRecord()
{
}

bool MAsyncDBTask_InsertCQRecord::OnExec()
{
	return Db_InsertCQRecord(m_nAID, m_nMapID, m_nRecordTime);
}

void MAsyncDBTask_InsertCQRecord::OnDone()
{
}


MAsyncDBTask_SetExp::MAsyncDBTask_SetExp(const MUID &uidPlayer, int nCID, int nLevel, int nXP, int nBP, int nKill, int nDeath) : MBaseAsyncDBTask(uidPlayer)
{
	m_nCID = nCID;
	m_nLevel = nLevel;
	m_nXP = nXP;
	m_nBP = nBP;
	m_nKill = nKill;
	m_nDeath = nDeath;
}

MAsyncDBTask_SetExp::~MAsyncDBTask_SetExp()
{
}

bool MAsyncDBTask_SetExp::OnExec()
{
	return Db_SetExp(m_nCID, m_nLevel, m_nXP, m_nBP, m_nKill, m_nDeath);
}

void MAsyncDBTask_SetExp::OnDone()
{
}


MAsyncDBTask_SetSurvivalPoint::MAsyncDBTask_SetSurvivalPoint(const MUID &uidPlayer, int nCID, int nPoint) : MBaseAsyncDBTask(uidPlayer)
{
	m_nCID = nCID;
	m_nPoint = nPoint;
}

MAsyncDBTask_SetSurvivalPoint::~MAsyncDBTask_SetSurvivalPoint()
{
}

bool MAsyncDBTask_SetSurvivalPoint::OnExec()
{
	return Db_SetSurvivalPoint(m_nCID, m_nPoint);
}

void MAsyncDBTask_SetSurvivalPoint::OnDone()
{
}


MAsyncDBTask_SetDTScore::MAsyncDBTask_SetDTScore(const MUID &uidPlayer, int nCID, int nTP, int nWin, int nLose, int nFinalWin) : MBaseAsyncDBTask(uidPlayer)
{
	m_nCID = nCID;
	m_nTP = nTP;
	m_nWin = nWin;
	m_nLose = nLose;
	m_nFinalWin = nFinalWin;
}

MAsyncDBTask_SetDTScore::~MAsyncDBTask_SetDTScore()
{
}

bool MAsyncDBTask_SetDTScore::OnExec()
{
	return Db_SetDTScore(m_nCID, m_nTP, m_nWin, m_nLose, m_nFinalWin);
}

void MAsyncDBTask_SetDTScore::OnDone()
{
}


MAsyncDBTask_TakeoffItem::MAsyncDBTask_TakeoffItem(const MUID &uidPlayer, int nCID, int nItemParts) : MBaseAsyncDBTask(uidPlayer)
{
	m_nCID = nCID;
	m_nItemParts = nItemParts;
}

MAsyncDBTask_TakeoffItem::~MAsyncDBTask_TakeoffItem()
{
}

bool MAsyncDBTask_TakeoffItem::OnExec()
{
	return Db_TakeoffItem(m_nCID, m_nItemParts);
}

void MAsyncDBTask_TakeoffItem::OnDone()
{
}


MAsyncDBTask_DeleteSpentItem::MAsyncDBTask_DeleteSpentItem(const MUID &uidPlayer, int nCIID) : MBaseAsyncDBTask(uidPlayer)
{
	m_nCIID = nCIID;
}

MAsyncDBTask_DeleteSpentItem::~MAsyncDBTask_DeleteSpentItem()
{
}

bool MAsyncDBTask_DeleteSpentItem::OnExec()
{
	return Db_DeleteSpentItem(m_nCIID);
}

void MAsyncDBTask_DeleteSpentItem::OnDone()
{
}


MAsyncDBTask_UpdateItemCount::MAsyncDBTask_UpdateItemCount(const MUID &uidPlayer, int nCIID, int nItemCount) : MBaseAsyncDBTask(uidPlayer)
{
	m_nCIID = nCIID;
	m_nItemCount = nItemCount;
}

MAsyncDBTask_UpdateItemCount::~MAsyncDBTask_UpdateItemCount()
{
}

bool MAsyncDBTask_UpdateItemCount::OnExec()
{
	return Db_UpdateItemCount(m_nCIID, m_nItemCount);
}

void MAsyncDBTask_UpdateItemCount::OnDone()
{
}


MAsyncDBTask_UpdateCharQuestItem::MAsyncDBTask_UpdateCharQuestItem(const MUID &uidPlayer, int nCID, const char *pszData) : MBaseAsyncDBTask(uidPlayer)
{
	m_nCID = nCID;
	strcpy(m_szData, pszData);
}

MAsyncDBTask_UpdateCharQuestItem::~MAsyncDBTask_UpdateCharQuestItem()
{
}

bool MAsyncDBTask_UpdateCharQuestItem::OnExec()
{
	return Db_UpdateCharQuestItem(m_nCID, m_szData);
}

void MAsyncDBTask_UpdateCharQuestItem::OnDone()
{
}


MAsyncDBTask_FetchSurvivalRanking::MAsyncDBTask_FetchSurvivalRanking() : MBaseAsyncDBTask(MUID(0, 0))
{
}

MAsyncDBTask_FetchSurvivalRanking::~MAsyncDBTask_FetchSurvivalRanking()
{
}

bool MAsyncDBTask_FetchSurvivalRanking::OnExec()
{
	return Db_FetchSurvivalRanking(&m_vtDbRankingInfo);
}

void MAsyncDBTask_FetchSurvivalRanking::OnDone()
{
	g_Quest.OnFetchSurvivalRanking(m_vtDbRankingInfo);
}


MAsyncDBTask_UpdateSurvivalRanking::MAsyncDBTask_UpdateSurvivalRanking() : MBaseAsyncDBTask(MUID(0, 0))
{
}

MAsyncDBTask_UpdateSurvivalRanking::~MAsyncDBTask_UpdateSurvivalRanking()
{
}

bool MAsyncDBTask_UpdateSurvivalRanking::OnExec()
{
	return Db_UpdateSurvivalRanking();
}

void MAsyncDBTask_UpdateSurvivalRanking::OnDone()
{
}


MAsyncDBTask_BanPlayer::MAsyncDBTask_BanPlayer(const MUID &uidPlayer, int nAID) : MBaseAsyncDBTask(uidPlayer)
{
	m_nAID = nAID;
}

MAsyncDBTask_BanPlayer::~MAsyncDBTask_BanPlayer()
{
}

bool MAsyncDBTask_BanPlayer::OnExec()
{
	return Db_BanPlayer(m_nAID);
}

void MAsyncDBTask_BanPlayer::OnDone()
{
}


MAsyncDBTask_ClearExpiredItem::MAsyncDBTask_ClearExpiredItem(const MUID &uidPlayer, int nAID, int nCID) : MBaseAsyncDBTask(uidPlayer)
{
	m_nAID = nAID;
	m_nCID = nCID;
}

MAsyncDBTask_ClearExpiredItem::~MAsyncDBTask_ClearExpiredItem()
{
}

bool MAsyncDBTask_ClearExpiredItem::OnExec()
{
	if(Db_ClearExpiredAccountItem(m_nAID, &m_vtAItemInfo) == false) return false;
	if(Db_ClearExpiredItem(m_nCID, &m_vtItemInfo) == false) return false;
	
	if(m_vtAItemInfo.empty() == true && m_vtItemInfo.empty() == true) return false;
	
	return true;
}

void MAsyncDBTask_ClearExpiredItem::OnDone()
{
	MMatchObject *pObj = g_ObjectMgr.Get(m_uidRequestor);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	MCmdWriter Cmd;
	
	Cmd.StartBlob(sizeof(unsigned long));
	// acc item.
	for(vector<DbData_ExpiredItemNode>::iterator i = m_vtAItemInfo.begin(); i != m_vtAItemInfo.end(); i++)
	{
		DbData_ExpiredItemNode *pNode = &(*i);
		
		pObj->AccItemExpired(pNode->nCIID);	// = AIID.
		Cmd.WriteULong((unsigned long)pNode->nItemID);
	}
	// char item.
	for(vector<DbData_ExpiredItemNode>::iterator i = m_vtItemInfo.begin(); i != m_vtItemInfo.end(); i++)
	{
		DbData_ExpiredItemNode *pNode = &(*i);
		
		pObj->ItemExpired(pNode->nCIID);
		Cmd.WriteULong((unsigned long)pNode->nItemID);
	}
	Cmd.EndBlob();
	
	Cmd.Finalize(MC_MATCH_EXPIRED_RENT_ITEM, MCFT_END);
	SendToClient(&Cmd, m_uidRequestor);
	
	OnCharacterItemList(m_uidRequestor);
	OnAccountItemList(m_uidRequestor);
}


MAsyncDBTask_RemoveFriend::MAsyncDBTask_RemoveFriend(const MUID &uidPlayer, int nFriendID) : MBaseAsyncDBTask(uidPlayer)
{
	m_nFriendID = nFriendID;
}

MAsyncDBTask_RemoveFriend::~MAsyncDBTask_RemoveFriend()
{
}

bool MAsyncDBTask_RemoveFriend::OnExec()
{
	return Db_RemoveFriend(m_nFriendID);
}

void MAsyncDBTask_RemoveFriend::OnDone()
{
}


MAsyncDBTask_JoinClan::MAsyncDBTask_JoinClan(const MUID &uidPlayer, int nCLID, int nCID) : MBaseAsyncDBTask(uidPlayer)
{
	m_nCLID = nCLID;
	m_nCID = nCID;
}

MAsyncDBTask_JoinClan::~MAsyncDBTask_JoinClan()
{
}

bool MAsyncDBTask_JoinClan::OnExec()
{
	return Db_JoinClan(m_nCLID, m_nCID);
}

void MAsyncDBTask_JoinClan::OnDone()
{
}


MAsyncDBTask_UpdateClanMemberGrade::MAsyncDBTask_UpdateClanMemberGrade(const MUID &uidPlayer, int nCLID, int nCID, int nMemberGrade) : MBaseAsyncDBTask(uidPlayer)
{
	m_nCLID = nCLID;
	m_nCID = nCID;
	m_nMemberGrade = nMemberGrade;
}

MAsyncDBTask_UpdateClanMemberGrade::~MAsyncDBTask_UpdateClanMemberGrade()
{
}

bool MAsyncDBTask_UpdateClanMemberGrade::OnExec()
{
	return Db_UpdateClanMemberGrade(m_nCLID, m_nCID, m_nMemberGrade);
}

void MAsyncDBTask_UpdateClanMemberGrade::OnDone()
{
}


MAsyncDBTask_UpdateClanMasterCID::MAsyncDBTask_UpdateClanMasterCID(const MUID &uidPlayer, int nCLID, int nCID) : MBaseAsyncDBTask(uidPlayer)
{
	m_nCLID = nCLID;
	m_nCID = nCID;
}

MAsyncDBTask_UpdateClanMasterCID::~MAsyncDBTask_UpdateClanMasterCID()
{
}

bool MAsyncDBTask_UpdateClanMasterCID::OnExec()
{
	return Db_UpdateClanMasterCID(m_nCLID, m_nCID);
}

void MAsyncDBTask_UpdateClanMasterCID::OnDone()
{
}


MAsyncDBTask_LeaveClan::MAsyncDBTask_LeaveClan(const MUID &uidPlayer, int nCLID, int nCID) : MBaseAsyncDBTask(uidPlayer)
{
	m_nCLID = nCLID;
	m_nCID = nCID;
}

MAsyncDBTask_LeaveClan::~MAsyncDBTask_LeaveClan()
{
}

bool MAsyncDBTask_LeaveClan::OnExec()
{
	return Db_LeaveClan(m_nCLID, m_nCID);
}

void MAsyncDBTask_LeaveClan::OnDone()
{
}


MAsyncDBTask_CloseClan::MAsyncDBTask_CloseClan(const MUID &uidPlayer, int nCLID) : MBaseAsyncDBTask(uidPlayer)
{
	m_nCLID = nCLID;
}

MAsyncDBTask_CloseClan::~MAsyncDBTask_CloseClan()
{
}

bool MAsyncDBTask_CloseClan::OnExec()
{
	return Db_CloseClan(m_nCLID);
}

void MAsyncDBTask_CloseClan::OnDone()
{
}


MAsyncDBTask_UpdateItem::MAsyncDBTask_UpdateItem(const MUID &uidPlayer, int nCID, int nCIID, int nItemCount, int nBounty) : MBaseAsyncDBTask(uidPlayer)
{
	m_nCID = nCID;
	m_nCIID = nCIID;
	m_nItemCount = nItemCount;
	m_nBounty = nBounty;
}

MAsyncDBTask_UpdateItem::~MAsyncDBTask_UpdateItem()
{
}

bool MAsyncDBTask_UpdateItem::OnExec()
{
	return Db_UpdateItem(m_nCID, m_nCIID, m_nItemCount, m_nBounty);
}

void MAsyncDBTask_UpdateItem::OnDone()
{
}


MAsyncDBTask_UpdateGambleItem::MAsyncDBTask_UpdateGambleItem(const MUID &uidPlayer, int nCID, int nCGIID, int nItemCount, int nBounty) : MBaseAsyncDBTask(uidPlayer)
{
	m_nCID = nCID;
	m_nCGIID = nCGIID;
	m_nItemCount = nItemCount;
	m_nBounty = nBounty;
}

MAsyncDBTask_UpdateGambleItem::~MAsyncDBTask_UpdateGambleItem()
{
}

bool MAsyncDBTask_UpdateGambleItem::OnExec()
{
	return Db_UpdateGambleItem(m_nCID, m_nCGIID, m_nItemCount, m_nBounty);
}

void MAsyncDBTask_UpdateGambleItem::OnDone()
{
}


MAsyncDBTask_DeleteItem::MAsyncDBTask_DeleteItem(const MUID &uidPlayer, int nCID, int nCIID, int nBounty) : MBaseAsyncDBTask(uidPlayer)
{
	m_nCID = nCID;
	m_nCIID = nCIID;
	m_nBounty = nBounty;
}

MAsyncDBTask_DeleteItem::~MAsyncDBTask_DeleteItem()
{
}

bool MAsyncDBTask_DeleteItem::OnExec()
{
	return Db_DeleteItem(m_nCID, m_nCIID, m_nBounty);
}

void MAsyncDBTask_DeleteItem::OnDone()
{
}


MAsyncDBTask_DeleteGambleItem::MAsyncDBTask_DeleteGambleItem(const MUID &uidPlayer, int nCID, int nCGIID, int nBounty) : MBaseAsyncDBTask(uidPlayer)
{
	m_nCID = nCID;
	m_nCGIID = nCGIID;
	m_nBounty = nBounty;
}

MAsyncDBTask_DeleteGambleItem::~MAsyncDBTask_DeleteGambleItem()
{
}

bool MAsyncDBTask_DeleteGambleItem::OnExec()
{
	return Db_DeleteGambleItem(m_nCID, m_nCGIID, m_nBounty);
}

void MAsyncDBTask_DeleteGambleItem::OnDone()
{
}


MAsyncDBTask_EquipItem::MAsyncDBTask_EquipItem(const MUID &uidPlayer, int nCID, int nCIID, int nItemSlot) : MBaseAsyncDBTask(uidPlayer)
{
	m_nCID = nCID;
	m_nCIID = nCIID;
	m_nItemSlot = nItemSlot;
}

MAsyncDBTask_EquipItem::~MAsyncDBTask_EquipItem()
{
}

bool MAsyncDBTask_EquipItem::OnExec()
{
	return Db_EquipItem(m_nCID, m_nCIID, m_nItemSlot);
}

void MAsyncDBTask_EquipItem::OnDone()
{
}


MAsyncDBTask_DecreaseGambleItem::MAsyncDBTask_DecreaseGambleItem(const MUID &uidPlayer, int nCGIID) : MBaseAsyncDBTask(uidPlayer)
{
	m_nCGIID = nCGIID;
}

MAsyncDBTask_DecreaseGambleItem::~MAsyncDBTask_DecreaseGambleItem()
{
}

bool MAsyncDBTask_DecreaseGambleItem::OnExec()
{
	return Db_DecreaseGambleItem(m_nCGIID);
}

void MAsyncDBTask_DecreaseGambleItem::OnDone()
{
}


MAsyncDBTask_MatchLogin::MAsyncDBTask_MatchLogin(const MUID &uidPlayer, const char *pszUserID, const char *pszPassword, int nCommandVersion, const char *pszIP, bool bUseIPBlock) : MBaseAsyncDBTask(uidPlayer)
{
	strcpy(m_szUserID, pszUserID);
	strcpy(m_szPassword, pszPassword);
	m_nCommandVersion = nCommandVersion;
	strcpy(m_szIP, pszIP);
	m_nIP = Socket::InetAddr(pszIP);
	m_bUseIPBlock = false;
	
	m_nAID = 0;
	m_nUGradeID = 0;
	m_nPGradeID = 0;
	m_nCash = 0;
	
	m_nErrorCode = MSG_OK;
}

MAsyncDBTask_MatchLogin::~MAsyncDBTask_MatchLogin()
{
}

bool MAsyncDBTask_MatchLogin::OnExec()
{
	if(m_nCommandVersion != 58)	// MCommand version.
	{
		m_nErrorCode = MERR_WRONG_CMDVERSION;
		return true;
	}
	
	if(m_bUseIPBlock == true)
	{
		int nResult;
		
		if(Db_IsBannedIP(m_szIP, &nResult) == false)
		{
			m_nErrorCode = MERR_LOGIN_FAILED;
			return true;
		}
		
		if(nResult == 1)	// == banned.
		{
			m_nErrorCode = MERR_BANNED_ID;
			return true;
		}
	}
	
	#ifdef _AUTO_REGISTER
	// #ifdef _DEBUG	// Auto account Register.
	if(Db_InsertAccount(m_szUserID, m_szPassword) == false)
	{
		m_nErrorCode = MERR_LOGIN_FAILED;
		return true;
	}
	// #endif	// (_DEBUG)
	#endif	// (_AUTO_REGISTER)
	
	char szDbPassword[MAX_USERPASSWORD_LEN];
	if(Db_GetAccountInfo(m_szUserID, &m_nAID, szDbPassword, &m_nUGradeID, &m_nPGradeID, &m_nCash) == false)
	{
		m_nErrorCode = MERR_UNKNOWN_ACCOUNTINFO;
		return true;
	}

	if(strcmp(szDbPassword, m_szPassword) != 0)
	{
		m_nErrorCode = MERR_WRONG_LOGINID;
		return true;
	}
	
	Db_UpdateConnData(m_nAID, m_szIP);
	
	if(m_nUGradeID == MMUG_BLOCKED)
	{
		m_nErrorCode = MERR_BANNED_ID;
		return true;
	}
	
	// account related in-game info.
	//  - account item.
	Db_GetAccountItemList(m_nAID, &m_vtAccountItem);
	//  - challenge quest.
	Db_GetCQRecord(m_nAID, &m_vtCQRecordInfo);
	
	return true;
}

void MAsyncDBTask_MatchLogin::OnDone()
{
	Socket::socket_type s;
	unsigned char nCryptKey[ENCRYPTIONKEY_LENGTH];

	if(g_ClientAcceptor.GetClientInfo(m_uidRequestor, &s, nCryptKey) == false) return;	// unknown error.
	
	if(m_nErrorCode != MSG_OK)
	{
		SendLoginResultCommand(s, nCryptKey);
		return;
	}
	
	// TODO : Free Login IP (can ignore server max players) check.
	
	if(g_ObjectMgr.Size() >= (int)g_ServerConfig.ServerSetting.nMaxPlayers)
	{
		m_nErrorCode = MERR_SERVER_FULL;
		SendLoginResultCommand(s, nCryptKey);
		return;
	}
	
	#ifdef _RELEASE
	for(list<MMatchObject *>::iterator i = g_ObjectMgr.Begin(); i != g_ObjectMgr.End(); i++)
	{
		MMatchObject *pCurr = (*i);
		
		if(pCurr->GetIP() == m_nIP)
		{
			// block same ip - useful for anti connection flood.
			
			SendLoginResultCommand(MERR_ALREADY_CONNECTED, m_szUserID, m_nUGradeID, m_nPGradeID, pCurr->GetUID());
			pCurr->Disconnect();
		}
		else if(pCurr->m_Account.nAID == m_nAID)
		{
			SendLoginResultCommand(MERR_MULTIPLE_LOGIN, m_szUserID, m_nUGradeID, m_nPGradeID, pCurr->GetUID());
			pCurr->Disconnect();
		}
	}
	#endif
	
	MMatchObject *pObj = g_ObjectMgr.Add(s, m_uidRequestor, nCryptKey);

	// set basic account info.
	pObj->m_Account.nAID = m_nAID;
	pObj->m_Account.nUGradeID = m_nUGradeID;
	pObj->m_Account.nPGradeID = m_nPGradeID;
	pObj->m_Account.nCash = m_nCash;
	
	for(vector<DbData_AccItemList>::iterator i = m_vtAccountItem.begin(); i != m_vtAccountItem.end(); i++)
	{
		DbData_AccItemList *pCurr = &(*i);
		pObj->AddAccItem(pCurr->nAIID, pCurr->nItemID, pCurr->nItemCount, pCurr->nRentSecPeriod);
	}
	
	for(vector<DbData_CQRecordInfo>::iterator i = m_vtCQRecordInfo.begin(); i != m_vtCQRecordInfo.end(); i++)
	{
		DbData_CQRecordInfo *pCurr = &(*i);
		pObj->AddCQRecord(pCurr->nScenarioID, pCurr->nTime);
	}
	
	SendLoginResultCommand(s, nCryptKey);
	
	g_ClientAcceptor.RemoveClient(s);
	
	// send gambleitem info list.
	g_GItemMgr.SendInfo(m_uidRequestor);
}

void MAsyncDBTask_MatchLogin::SendLoginResultCommand(Socket::socket_type s, const unsigned char *pCryptKey)
{	
	MCmdWriter Cmd;
	Cmd.WriteInt(m_nErrorCode);
	Cmd.WriteString(g_ServerConfig.ServerSetting.szName);
	Cmd.WriteUChar(g_ServerConfig.ServerSetting.nServerMode);
	Cmd.WriteString(m_szUserID);
	Cmd.WriteUChar((unsigned char)m_nUGradeID);
	Cmd.WriteUChar((unsigned char)m_nPGradeID);
	Cmd.WriteMUID(m_uidRequestor);
	Cmd.WriteBool(g_ServerConfig.ServerSetting.bSurvival);
	Cmd.WriteBool(g_ServerConfig.ServerSetting.bDuelTournament);
	
	if(CheckGameVersion(2013) == false)
	{
		Cmd.StartBlob(20);
		Cmd.WriteSkip(20);
		Cmd.EndBlob();
	}
	
	Cmd.Finalize(MC_MATCH_RESPONSE_LOGIN, MCFT_END);
	SendToClient(&Cmd, s, pCryptKey);
}

void MAsyncDBTask_MatchLogin::SendLoginResultCommand(int nResult, const char *pszUserID, int nUGradeID, int nPGradeID, const MUID &uidPlayer)
{
	MCmdWriter Cmd;
	Cmd.WriteInt(nResult);
	Cmd.WriteString(g_ServerConfig.ServerSetting.szName);
	Cmd.WriteUChar(g_ServerConfig.ServerSetting.nServerMode);
	Cmd.WriteString(pszUserID);
	Cmd.WriteUChar((unsigned char)nUGradeID);
	Cmd.WriteUChar((unsigned char)nPGradeID);
	Cmd.WriteMUID(uidPlayer);
	Cmd.WriteBool(g_ServerConfig.ServerSetting.bSurvival);
	Cmd.WriteBool(g_ServerConfig.ServerSetting.bDuelTournament);
	
	Cmd.StartBlob(20);
	Cmd.WriteSkip(20);
	Cmd.EndBlob();
	
	Cmd.Finalize(MC_MATCH_RESPONSE_LOGIN, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
}


MAsyncDBTask_AccountCharList::MAsyncDBTask_AccountCharList(const MUID &uidPlayer, int nAID) : MBaseAsyncDBTask(uidPlayer)
{
	m_nAID = nAID;
	
	m_nCharCount = 0;
	ZeroInit(m_CharInfo, sizeof(m_CharInfo));
}

MAsyncDBTask_AccountCharList::~MAsyncDBTask_AccountCharList()
{
}

bool MAsyncDBTask_AccountCharList::OnExec()
{
	return Db_GetAccountCharList(m_nAID, &m_nCharCount, m_CharInfo);
}

void MAsyncDBTask_AccountCharList::OnDone()
{
	void SendChannelList();
	
	MMatchObject *pObj = g_ObjectMgr.Get(m_uidRequestor);
	if(pObj == NULL) return;
	
	MCmdWriter Cmd;
	
	Cmd.StartBlob(sizeof(MTD_AccountCharInfo));
	for(int i = 0; i < m_nCharCount; i++)
	{
		MTD_AccountCharInfo info;
		ZeroInit(&info, sizeof(MTD_AccountCharInfo));
		
		strcpy(info.szName, m_CharInfo[i].szName);
		info.nCharNum = (char)m_CharInfo[i].nCharNum;
		info.nLevel = (unsigned char)m_CharInfo[i].nLevel;
		
		Cmd.WriteData(&info, sizeof(MTD_AccountCharInfo));
	}
	Cmd.EndBlob();

	Cmd.Finalize(MC_MATCH_RESPONSE_ACCOUNT_CHARLIST, MCFT_END);
	SendToClient(&Cmd, m_uidRequestor);
	
	pObj->Clean();
	
	SendChannelList();
	
	#ifdef _DEBUG
	// auto char select (index 0) for faster debugging.
	if(m_nCharCount >= 1)
	{
		OnSelectCharacter(m_uidRequestor, 0);
	}
	#endif
}


MAsyncDBTask_AccountCharInfo::MAsyncDBTask_AccountCharInfo(const MUID &uidPlayer, int nAID, int nCharNum) : MBaseAsyncDBTask(uidPlayer)
{
	m_nAID = nAID;
	m_nCharNum = nCharNum;
	
	m_nCID = 0;
	ZeroInit(&m_CharInfo, sizeof(m_CharInfo));
	ZeroInit(&m_ClanInfo, sizeof(m_ClanInfo));
	ZeroInit(&m_ItemInfo, sizeof(m_ItemInfo));
	m_nDTClass = 10;
}

MAsyncDBTask_AccountCharInfo::~MAsyncDBTask_AccountCharInfo()
{
}

bool MAsyncDBTask_AccountCharInfo::OnExec()
{
	// cid from charnum.
	if(Db_GetCharIndex(m_nAID, m_nCharNum, &m_nCID) == false)
		return false;

	// char info.
	if(Db_GetBasicCharInfo(m_nCID, &m_CharInfo) == false)
		return false;

	// clan info.
	Db_GetBasicClanInfo(m_nCID, &m_ClanInfo);

	// item info.
	Db_GetCharItemInfo(m_nCID, &m_ItemInfo);

	// dt class info.
	Db_GetDTCharClass(m_nCID, &m_nDTClass);
	
	return true;
}

void MAsyncDBTask_AccountCharInfo::OnDone()
{
	MMatchObject *pObj = g_ObjectMgr.Get(m_uidRequestor);
	if(pObj == NULL) return;
	
	MCmdWriter Cmd;

	Cmd.WriteChar((char)m_nCharNum);
	
	MTD_CharInfo info;
	ZeroInit(&info, sizeof(MTD_CharInfo));
	
	strcpy(info.szName, m_CharInfo.szName);
	strcpy(info.szClanName, m_ClanInfo.szClanName);
	info.nClanGrade = m_ClanInfo.nMemberGrade;
	info.nClanContPoint = (unsigned short)m_ClanInfo.nClanPoint;
	info.nCharNum = (char)m_nCharNum;
	info.nLevel = (unsigned short)m_CharInfo.nLevel;
	info.nSex = (char)m_CharInfo.nSex;
	info.nHair = (char)m_CharInfo.nHair;
	info.nFace = (char)m_CharInfo.nFace;
	info.nXP = (unsigned long)m_CharInfo.nExp;
	info.nBP = m_CharInfo.nBounty;
	for(int i = 0; i < MMCIP_END; i++)
	{
		info.nEquipedItemDesc[i] = (unsigned long)m_ItemInfo.nItemID[i];
		info.nEquipedItemCount[i] = (unsigned long)m_ItemInfo.nItemCount[i];
	}
	info.nUGradeID = pObj->m_Account.nUGradeID;
	info.nClanCLID = (unsigned int)m_ClanInfo.nCLID;
	info.nDTLastWeekGrade = m_nDTClass;
	
	Cmd.StartBlob(sizeof(MTD_CharInfo));
	Cmd.WriteData(&info, sizeof(MTD_CharInfo));
	Cmd.EndBlob();
	
	Cmd.Finalize(MC_MATCH_RESPONSE_ACCOUNT_CHARINFO, MCFT_END);
	SendToClient(&Cmd, m_uidRequestor);
}


MAsyncDBTask_SelectCharacter::MAsyncDBTask_SelectCharacter(const MUID &uidPlayer, int nAID, int nCharIndex) : MBaseAsyncDBTask(uidPlayer)
{
	m_nAID = nAID;
	m_nCharIndex = nCharIndex;
	
	m_nCID = 0;
	ZeroInit(&m_CharInfo, sizeof(m_CharInfo));
	ZeroInit(&m_ClanInfo, sizeof(m_ClanInfo));
	ZeroInit(&m_ItemInfo, sizeof(m_ItemInfo));
	ZeroInit(&m_DTInfo, sizeof(m_DTInfo));
	ZeroInit(&m_EquipItemInfo, sizeof(m_EquipItemInfo));
	ZeroInit(&m_AdvClanInfo, sizeof(m_AdvClanInfo));
	ZeroInit(&m_SurvivalInfo, sizeof(m_SurvivalInfo));
	ZeroInit(&m_BlitzInfo, sizeof(m_BlitzInfo));
}

MAsyncDBTask_SelectCharacter::~MAsyncDBTask_SelectCharacter()
{
}

bool MAsyncDBTask_SelectCharacter::OnExec()
{
	// cid from charnum.
	if(Db_GetCharIndex(m_nAID, m_nCharIndex, &m_nCID) == false)
		return false;

	// char info.
	if(Db_GetBasicCharInfo(m_nCID, &m_CharInfo) == false)
		return false;
		
	// clan info.
	Db_GetBasicClanInfo(m_nCID, &m_ClanInfo);

	// item info.
	Db_GetCharEquipmentSlotCIID(m_nCID, &m_ItemInfo);

	// dt character info.
	Db_GetDTCharInfo(m_nCID, &m_DTInfo);
	
	if(Db_GetCharacterItemList(m_nCID, &m_vtItemList) == false)
		return false;
		
	// gamble item list.
	if(Db_GetCharGambleItem(m_nCID, &m_vtCharGambleItem) == false) return false;
	
	// advanced clan info.
	if(m_ClanInfo.nCLID != 0)
	{
		if(Db_GetClanInfo(m_ClanInfo.nCLID, &m_AdvClanInfo) == false)
		{
			m_AdvClanInfo.nCLID = 0;
		}
	}
	
	// friend list info.
	Db_GetFriendList(m_nCID, &m_vtFriendListNode);
	
	// survival info.
	Db_GetSurvivalCharInfo(m_nCID, &m_SurvivalInfo);
	
	// blitzkrieg char info.
	Db_GetBlitzScore(m_nCID, &m_BlitzInfo);
	
	return true;
}

void MAsyncDBTask_SelectCharacter::OnDone()
{
	void CheckExpiredCharItem(MMatchObject *pObj);
	void SendChannelList();
	
	MMatchObject *pObj = g_ObjectMgr.Get(m_uidRequestor);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == true) return;
	
	for(vector<DbData_CharItemList>::iterator i = m_vtItemList.begin(); i != m_vtItemList.end(); i++)
	{
		DbData_CharItemList *pci = &(*i);
		MUID uidAssigned = MUID::Assign();
		
		pObj->AddCharItem(uidAssigned, pci->nCIID, pci->nItemID, pci->nItemCount, pci->nRentSecPeriod);
		
		for(int j = 0; j < MMCIP_END; j++)
		{
			if(m_ItemInfo.nCIID[j] == pci->nCIID)
			{
				pObj->SetItemUID(uidAssigned, j);
				
				m_EquipItemInfo[j].uidItem = uidAssigned;
				m_EquipItemInfo[j].nItemID = pci->nItemID;
				m_EquipItemInfo[j].nItemCount = pci->nItemCount;
				
				break;
			}
		}
	}
	
	// gamble item list.
	for(vector<DbData_CharGambleItem>::iterator i = m_vtCharGambleItem.begin(); i != m_vtCharGambleItem.end(); i++)
	{
		DbData_CharGambleItem *pgi = &(*i);
		MUID uidAssigned = MUID::Assign();
		
		pObj->AddCharGambleItem(uidAssigned, pgi->nGIID, pgi->nItemID, pgi->nCount);
	}
	
	// advanced clan info.
	if(m_AdvClanInfo.nCLID != 0)
	{
		MMatchClan *pClan = g_ClanMgr.Add(m_AdvClanInfo.nCLID, 
									m_AdvClanInfo.szName, 
									m_AdvClanInfo.nLevel, 
									m_AdvClanInfo.nPoint, 
									m_AdvClanInfo.nMasterCID, 
									m_AdvClanInfo.nWin, 
									m_AdvClanInfo.nLose, 
									m_AdvClanInfo.nDraw, 
									m_AdvClanInfo.nTotalPoint, 
									m_AdvClanInfo.nRanking, 
									m_AdvClanInfo.szEmblemURL, 
									m_AdvClanInfo.nEmblemChecksum);
		if(pClan != NULL)
		{
			pClan->Join(pObj);
		}
	}
	
	// friend list info.
	pObj->ClearFriendList();	// clear existing info.
	
	for(vector<DbData_FriendListNode>::iterator i = m_vtFriendListNode.begin(); i != m_vtFriendListNode.end(); i++)
	{
		DbData_FriendListNode *pNode = &(*i);
		pObj->AddFriend(pNode->nFriendID, pNode->nCID, pNode->szCharName);
	}
	
	// quest item.
	pObj->InitQuestItem(m_CharInfo.szQuestItemData);
	
	// survival info.
	pObj->m_Survival.nPoint = m_SurvivalInfo.nPoint;
	pObj->m_Survival.nRanking = m_SurvivalInfo.nRanking;
	
		
		
	MCmdWriter Cmd;
	
	Cmd.WriteInt(0);	// result = ok.
	
	MTD_CharInfo info;
	ZeroInit(&info, sizeof(MTD_CharInfo));
	
	strcpy(info.szName, m_CharInfo.szName);
	strcpy(info.szClanName, m_ClanInfo.szClanName);
	info.nClanGrade = m_ClanInfo.nMemberGrade;
	info.nClanContPoint = (unsigned short)m_ClanInfo.nClanPoint;
	info.nCharNum = (char)m_nCharIndex;
	info.nLevel = (unsigned short)m_CharInfo.nLevel;
	info.nSex = (char)m_CharInfo.nSex;
	info.nHair = (char)m_CharInfo.nHair;
	info.nFace = (char)m_CharInfo.nFace;
	info.nXP = (unsigned long)m_CharInfo.nExp;
	info.nBP = m_CharInfo.nBounty;
	for(int i = 0; i < MMCIP_END; i++)
	{
		info.uidEquipedItem[i] = m_EquipItemInfo[i].uidItem;
		info.nEquipedItemDesc[i] = (unsigned long)m_EquipItemInfo[i].nItemID;
		info.nEquipedItemCount[i] = (unsigned long)m_EquipItemInfo[i].nItemCount;
	}
	info.nUGradeID = pObj->m_Account.nUGradeID;
	info.nClanCLID = (unsigned int)m_ClanInfo.nCLID;
	info.nDTLastWeekGrade = m_DTInfo.nGrade;
	
	Cmd.StartBlob(sizeof(MTD_CharInfo));
	Cmd.WriteData(&info, sizeof(MTD_CharInfo));
	Cmd.EndBlob();
	
	// extra info.
	MTD_MyExtraCharInfo eci;
	eci.nLevelPercent = (char)g_FormulaMgr.GetExpPercent(m_CharInfo.nLevel, m_CharInfo.nExp);
	
	Cmd.StartBlob(sizeof(MTD_MyExtraCharInfo));
	Cmd.WriteData(&eci, sizeof(MTD_MyExtraCharInfo));
	Cmd.EndBlob();
	
	if(CheckGameVersion(2012) == true)
	{
		MTD_BlitzPlayerScore score;
		ZeroInit(&score, sizeof(MTD_BlitzPlayerScore));
		
		score.nWin = (unsigned short)m_BlitzInfo.nWin;
		score.nLose = (unsigned short)m_BlitzInfo.nLose;
		score.nPoint = m_BlitzInfo.nPoint;
		score.nMedal = m_BlitzInfo.nMedal;
		
		Cmd.StartBlob(sizeof(MTD_BlitzPlayerScore));
		Cmd.WriteData(&score, sizeof(MTD_BlitzPlayerScore));
		Cmd.EndBlob();
	}
	
	Cmd.Finalize(MC_MATCH_RESPONSE_SELECT_CHAR, MCFT_END);
	SendToClient(&Cmd, m_uidRequestor);
	
	// set some info.
	pObj->m_Char.nCID = m_nCID;
	strcpy(pObj->m_Char.szName, m_CharInfo.szName);
	pObj->m_Char.nSex = m_CharInfo.nSex;
	pObj->m_Char.nHair = m_CharInfo.nHair;
	pObj->m_Char.nFace = m_CharInfo.nFace;
	
	pObj->m_Exp.nLevel = m_CharInfo.nLevel;
	pObj->m_Exp.nXP = m_CharInfo.nExp;
	pObj->m_Exp.nBP = m_CharInfo.nBounty;
	pObj->m_Exp.nKill = m_CharInfo.nKillCount;
	pObj->m_Exp.nDeath = m_CharInfo.nDeathCount;
	pObj->m_Exp.nRanking = m_CharInfo.nRanking;
	
	MMatchCharDuelTournamentInfo *pCharDTInfo = &pObj->m_DuelTournament;
	pCharDTInfo->nClass = m_DTInfo.nGrade;
	pCharDTInfo->nTP = m_DTInfo.nTournamentPoint;
	pCharDTInfo->nWin = m_DTInfo.nWins;
	pCharDTInfo->nLose = m_DTInfo.nLosses;
	pCharDTInfo->nFinalWin = m_DTInfo.nFinalMatchWin;
	pCharDTInfo->nRanking = m_DTInfo.nRanking;
	pCharDTInfo->nPrevTP = m_DTInfo.nPreviousTournamentPoint;
	pCharDTInfo->nPrevWin = m_DTInfo.nPreviousWins;
	pCharDTInfo->nPrevLose = m_DTInfo.nPreviousLosses;
	pCharDTInfo->nPrevFinalWin = m_DTInfo.nPreviousFinalMatchWin;
	pCharDTInfo->nPrevRanking = m_DTInfo.nPreviousRanking;
	pCharDTInfo->nRankingDiff = m_DTInfo.nRankingDifferent;
	
	pObj->m_Clan.nCLID = m_ClanInfo.nCLID;
	pObj->m_Clan.nMemberGrade = m_ClanInfo.nMemberGrade;
	
	// blitz.
	pObj->m_BlitzKrieg.nWin = m_BlitzInfo.nWin;
	pObj->m_BlitzKrieg.nLose = m_BlitzInfo.nLose;
	pObj->m_BlitzKrieg.nPoint = m_BlitzInfo.nPoint;
	pObj->m_BlitzKrieg.nMedal = m_BlitzInfo.nMedal;
	
	pObj->m_bCharInfoExist = true;
	
	// check & delete expired rent item.
	CheckExpiredCharItem(pObj);
	
	// send gamble item info.
	// g_GItemMgr.SendInfo(m_uidRequestor);
	
	// send shop item list.
	OnShopItemList(m_uidRequestor);
	
	// send char item list.
	OnCharacterItemList(m_uidRequestor);
	
	// notify member logged-in, to clan members.
	MCmdWriter Cmd2Clan;
	Cmd2Clan.WriteString(m_CharInfo.szName);
	Cmd2Clan.Finalize(MC_MATCH_CLAN_MEMBER_CONNECTED, MCFT_END);
	SendToClan(&Cmd2Clan, m_ClanInfo.nCLID);
	
	SendChannelList();
	
	// check received gift.
	MAsyncDBTask_CheckItemGift *pNew = new MAsyncDBTask_CheckItemGift(m_uidRequestor, m_nCID);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}


MAsyncDBTask_UpdateBounty::MAsyncDBTask_UpdateBounty(const MUID &uidPlayer, int nCID, int nBounty) : MBaseAsyncDBTask(uidPlayer)
{
	m_nCID = nCID;
	m_nBounty = nBounty;
}

MAsyncDBTask_UpdateBounty::~MAsyncDBTask_UpdateBounty()
{
}

bool MAsyncDBTask_UpdateBounty::OnExec()
{
	return Db_UpdateBounty(m_nCID, m_nBounty);
}

void MAsyncDBTask_UpdateBounty::OnDone()
{
}


MAsyncDBTask_UpdateServerStatus::MAsyncDBTask_UpdateServerStatus(int nServerID, int nCurrPlayers) : MBaseAsyncDBTask(MUID(0, 0))
{
	m_nServerID = nServerID;
	m_nCurrPlayers = nCurrPlayers;
}

MAsyncDBTask_UpdateServerStatus::~MAsyncDBTask_UpdateServerStatus()
{
}

bool MAsyncDBTask_UpdateServerStatus::OnExec()
{
	return Db_UpdateServerStatus(m_nServerID, m_nCurrPlayers);
}

void MAsyncDBTask_UpdateServerStatus::OnDone()
{
}


MAsyncDBTask_UpdateIndividualRanking::MAsyncDBTask_UpdateIndividualRanking() : MBaseAsyncDBTask(MUID(0, 0))
{
}

MAsyncDBTask_UpdateIndividualRanking::~MAsyncDBTask_UpdateIndividualRanking()
{
}

bool MAsyncDBTask_UpdateIndividualRanking::OnExec()
{
	return Db_UpdateIndividualRanking();
}

void MAsyncDBTask_UpdateIndividualRanking::OnDone()
{
}


MAsyncDBTask_SetBlitzScore::MAsyncDBTask_SetBlitzScore(const MUID &uidPlayer, int nCID, int nWin, int nLose, int nPoint, int nMedal) : MBaseAsyncDBTask(uidPlayer)
{
	m_nCID = nCID;
	m_nWin = nWin;
	m_nLose = nLose;
	m_nPoint = nPoint;
	m_nMedal = nMedal;
}

MAsyncDBTask_SetBlitzScore::~MAsyncDBTask_SetBlitzScore()
{
}

bool MAsyncDBTask_SetBlitzScore::OnExec()
{
	return Db_SetBlitzScore(m_nCID, m_nWin, m_nLose, m_nPoint, m_nMedal);
}

void MAsyncDBTask_SetBlitzScore::OnDone()
{
}


MAsyncDBTask_AddBlitzShopItem::MAsyncDBTask_AddBlitzShopItem(int nCID, int nItemID, int nPrice, int nBasePrice, int nCount, int nRentHourPeriod) : MBaseAsyncDBTask(MUID(0, 0))
{
	m_nCID = nCID;
	m_nItemID = nItemID;
	m_nPrice = nPrice;
	m_nBasePrice = nBasePrice;
	m_nCount = nCount;
	m_nRentHourPeriod = nRentHourPeriod;
}

MAsyncDBTask_AddBlitzShopItem::~MAsyncDBTask_AddBlitzShopItem()
{
}

bool MAsyncDBTask_AddBlitzShopItem::OnExec()
{
	return Db_AddBlitzShopItem(m_nCID, m_nItemID, m_nPrice, m_nBasePrice, m_nCount, m_nRentHourPeriod);
}

void MAsyncDBTask_AddBlitzShopItem::OnDone()
{
}


MAsyncDBTask_ClearBlitzShop::MAsyncDBTask_ClearBlitzShop(int nCID) : MBaseAsyncDBTask(MUID(0, 0))
{
	m_nCID = nCID;
}

MAsyncDBTask_ClearBlitzShop::~MAsyncDBTask_ClearBlitzShop()
{
}

bool MAsyncDBTask_ClearBlitzShop::OnExec()
{
	bool bRet = false;
	
	if(m_nCID == 0)
	{
		bRet = Db_ClearBlitzShop();
	}
	else
	{
		bRet = Db_ClearBlitzShop(m_nCID);
	}
	
	return bRet;
}


void MAsyncDBTask_ClearBlitzShop::OnDone()
{
}


MAsyncDBTask_InsertItem::MAsyncDBTask_InsertItem(const MUID &uidPlayer, int nCID, int nItemID, int nItemCount, int nRentHourPeriod, int nBounty) : MBaseAsyncDBTask(uidPlayer)
{
	m_nCID = nCID;
	m_nItemID = nItemID;
	m_nItemCount = nItemCount;
	m_nRentHourPeriod = nRentHourPeriod;
	m_nBounty = nBounty;
	
	m_nCIID = 0;
}

MAsyncDBTask_InsertItem::~MAsyncDBTask_InsertItem()
{
}

bool MAsyncDBTask_InsertItem::OnExec()
{
	bool bRet = false;
	
	if(m_nRentHourPeriod == 0)
	{
		bRet = Db_InsertItem(m_nCID, m_nItemID, m_nItemCount, m_nBounty, &m_nCIID);
	}
	else
	{
		bRet = Db_InsertItem(m_nCID, m_nItemID, m_nItemCount, m_nRentHourPeriod, m_nBounty, &m_nCIID);
	}
	
	return bRet;
}

void MAsyncDBTask_InsertItem::OnDone()
{
	MMatchObject *pObj = g_ObjectMgr.Get(m_uidRequestor);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	pObj->AddCharItem(MUID::Assign(), m_nCIID, m_nItemID, m_nItemCount, m_nRentHourPeriod * 60 * 60);
	
	OnCharacterItemList(m_uidRequestor);
}


MAsyncDBTask_InsertGambleItem::MAsyncDBTask_InsertGambleItem(const MUID &uidPlayer, int nCID, int nItemID, int nItemCount, int nBounty) : MBaseAsyncDBTask(uidPlayer)
{
	m_nCID = nCID;
	m_nItemID = nItemID;
	m_nItemCount = nItemCount;
	m_nBounty = nBounty;
	
	m_nGIID = 0;
}

MAsyncDBTask_InsertGambleItem::~MAsyncDBTask_InsertGambleItem()
{
}

bool MAsyncDBTask_InsertGambleItem::OnExec()
{
	return Db_InsertGambleItem(m_nCID, m_nItemID, m_nItemCount, m_nBounty, &m_nGIID);
}

void MAsyncDBTask_InsertGambleItem::OnDone()
{
	MMatchObject *pObj = g_ObjectMgr.Get(m_uidRequestor);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	pObj->AddCharGambleItem(MUID::Assign(), m_nGIID, m_nItemID, m_nItemCount);
	
	OnCharacterItemList(m_uidRequestor);
}


MAsyncDBTask_MoveCItemToAItem::MAsyncDBTask_MoveCItemToAItem(const MUID &uidPlayer, int nCIID, int nAID, int nItemCount, MMatchCharItem &CharItem) : MBaseAsyncDBTask(uidPlayer)
{
	m_nCIID = nCIID;
	m_nAID = nAID;
	m_nItemCount = nItemCount;
	m_CharItem = CharItem;
	
	m_nAIID = 0;
}

MAsyncDBTask_MoveCItemToAItem::~MAsyncDBTask_MoveCItemToAItem()
{
}

bool MAsyncDBTask_MoveCItemToAItem::OnExec()
{
	bool bRet = false;
	
	if(m_nItemCount == 0)
	{
		bRet = Db_MoveCItemToAItem(m_nCIID, m_nAID, &m_nAIID);
	}
	else
	{
		bRet = Db_MoveCItemToAItem(m_nCIID, m_nAID, m_nItemCount, &m_nAIID);
	}
	
	return bRet;
}

void MAsyncDBTask_MoveCItemToAItem::OnDone()
{
	if(m_nAIID == 0) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(m_uidRequestor);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	if(m_nItemCount == 0)
	{
		pObj->AddAccItem(m_nAIID, m_CharItem.nID, m_CharItem.nCount, m_CharItem.CalcRentSecPeriod());
	}
	else
	{
		MMatchAccItem *pAccItem = pObj->GetAccItemByItemID(m_CharItem.nID);
		if(pAccItem == NULL)
		{
			pObj->AddAccItem(m_nAIID, m_CharItem.nID, m_nItemCount, m_CharItem.CalcRentSecPeriod());
		}
		else
		{
			pAccItem->nCount += m_nItemCount;
		}
	}
	
	OnAccountItemList(m_uidRequestor);
}


MAsyncDBTask_MoveAItemToCItem::MAsyncDBTask_MoveAItemToCItem(const MUID &uidPlayer, int nAIID, int nCID, int nItemCount, MMatchAccItem &AccItem) : MBaseAsyncDBTask(uidPlayer)
{
	m_nAIID = nAIID;
	m_nCID = nCID;
	m_nItemCount = nItemCount;
	m_AccItem = AccItem;
	
	m_nCIID = 0;
}

MAsyncDBTask_MoveAItemToCItem::~MAsyncDBTask_MoveAItemToCItem()
{
}

bool MAsyncDBTask_MoveAItemToCItem::OnExec()
{
	bool bRet = false;
	
	if(m_nItemCount == 0)
	{
		bRet = Db_MoveAItemToCItem(m_nAIID, m_nCID, &m_nCIID);
	}
	else
	{
		bRet = Db_MoveAItemToCItem(m_nAIID, m_nCID, m_nItemCount, &m_nCIID);
	}
	
	return bRet;
}

void MAsyncDBTask_MoveAItemToCItem::OnDone()
{
	if(m_nCIID == 0) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(m_uidRequestor);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	if(m_nItemCount == 0)
	{
		pObj->AddCharItem(MUID::Assign(), m_nCIID, m_AccItem.nID, m_AccItem.nCount, m_AccItem.CalcRentSecPeriod());
	}
	else
	{
		MMatchCharItem *pCharItem = pObj->GetCharItemByItemID(m_AccItem.nID);
		if(pCharItem == NULL)
		{
			pObj->AddCharItem(MUID::Assign(), m_nCIID, m_AccItem.nID, m_AccItem.nCount, m_AccItem.CalcRentSecPeriod());
		}
		else
		{
			pCharItem->nCount += m_nItemCount;
		}
	}
	
	OnCharacterItemList(m_uidRequestor);
}


/*
MAsyncDBTask_CreateClan::MAsyncDBTask_CreateClan(const MUID &uidPlayer, int nCID, const char *pszClanName) : MBaseAsyncDBTask(uidPlayer)
{
	m_nCID = nCID;
	strcpy(m_szClanName, pszClanName);
	
	m_nCLID = 0;
}

MAsyncDBTask_CreateClan::~MAsyncDBTask_CreateClan()
{
}

bool MAsyncDBTask_CreateClan::OnExec()
{
	return Db_CreateClan(m_nCID, m_szClanName, &m_nCLID);
}

void MAsyncDBTask_CreateClan::OnDone()
{
	void SendCharClanInfoToClient(MMatchObject *pObj);
	
	MMatchObject *pObj = g_ObjectMgr.Get(m_uidRequestor);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	MMatchClan *pClan = g_ClanMgr.Add(m_nCLID, 
									  m_szClanName, 
									  1, 
									  1000, 
									  m_nCID, 
									  0, 
									  0, 
									  0, 
									  0, 
									  0, 
									  "", 
									  0);
	if(pClan != NULL)
	{
		pClan->Join(pObj);
		
		pObj->m_Clan.nCLID = m_nCLID;
		pObj->m_Clan.nMemberGrade = MCG_MASTER;
		
		SendCharClanInfoToClient(pObj);
	}
}
*/


MAsyncDBTask_BuyCashItem::MAsyncDBTask_BuyCashItem(const MUID &uidPlayer, int nCID, int nItemID, int nItemCount, int nRentHourPeriod) : MBaseAsyncDBTask(uidPlayer)
{
	m_nCID = nCID;
	m_nItemID = nItemID;
	m_nItemCount = nItemCount;
	m_nRentHourPeriod = nRentHourPeriod;
	
	m_nCIID = 0;
}

MAsyncDBTask_BuyCashItem::~MAsyncDBTask_BuyCashItem()
{
}

bool MAsyncDBTask_BuyCashItem::OnExec()
{
	return Db_BuyCashItem(m_nCID, m_nItemID, m_nItemCount, m_nRentHourPeriod, &m_nCIID);
}

void MAsyncDBTask_BuyCashItem::OnDone()
{
	MMatchObject *pObj = g_ObjectMgr.Get(m_uidRequestor);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	pObj->AddCharItem(MUID::Assign(), m_nCIID, m_nItemID, m_nItemCount, m_nRentHourPeriod * 60 * 60);
	
	OnCharacterItemList(m_uidRequestor);
}


MAsyncDBTask_SetAccountCash::MAsyncDBTask_SetAccountCash(const MUID &uidPlayer, int nAID, int nCash) : MBaseAsyncDBTask(uidPlayer)
{
	m_nAID = nAID;
	m_nCash = nCash;
}

MAsyncDBTask_SetAccountCash::~MAsyncDBTask_SetAccountCash()
{
}

bool MAsyncDBTask_SetAccountCash::OnExec()
{
	return Db_SetAccountCash(m_nAID, m_nCash);
}

void MAsyncDBTask_SetAccountCash::OnDone()
{
}


MAsyncDBTask_SendItemGift::MAsyncDBTask_SendItemGift(const MUID &uidPlayer, const char *pszReceiver, const char *pszSender, const char *pszMessage, const int *pItemID, int nRentHourPeriod, int nQuantity, int nAID, int nCash) : MBaseAsyncDBTask(uidPlayer)
{
	strcpy(m_szReceiver, pszReceiver);
	strcpy(m_szSender, pszSender);
	strcpy(m_szMessage, pszMessage);
	for(int i = 0; i < 5; i++) m_nItemID[i] = pItemID[i];
	m_nRentHourPeriod = nRentHourPeriod;
	m_nQuantity = nQuantity;
	
	m_nAID = nAID;
	m_nCash = nCash;
	
	m_nResult = 0;
}

MAsyncDBTask_SendItemGift::~MAsyncDBTask_SendItemGift()
{
}

bool MAsyncDBTask_SendItemGift::OnExec()
{
	if(Db_SendItemGift(m_szReceiver, m_szSender, m_szMessage, m_nItemID, m_nRentHourPeriod, m_nQuantity, &m_nResult) == false) return false;
	
	if(m_nResult == 0)	// 0 = success.
	{
		Db_SetAccountCash(m_nAID, m_nCash);
	}
	
	return true;
}

void MAsyncDBTask_SendItemGift::OnDone()
{
	MMatchObject *pObj = g_ObjectMgr.Get(m_uidRequestor);
	if(pObj == NULL) return;
	
	int nErrorID = MSG_OK;
	
	switch(m_nResult)
	{
		case -1	: nErrorID = MERR_NOT_EXISTING_GIFT_TARGET; break;
		case 0  : pObj->m_Account.nCash = m_nCash;          break;
	}
	
	MCmdWriter Cmd;
	Cmd.WriteInt(nErrorID);
	Cmd.Finalize(MC_MATCH_RESPONSE_BUY_ITEM, MCFT_END);
	SendToClient(&Cmd, m_uidRequestor);
	
	pObj->SyncCash(false);
}


MAsyncDBTask_CheckItemGift::MAsyncDBTask_CheckItemGift(const MUID &uidPlayer, int nCID) : MBaseAsyncDBTask(uidPlayer)
{
	m_nCID = nCID;
}

MAsyncDBTask_CheckItemGift::~MAsyncDBTask_CheckItemGift()
{
}

bool MAsyncDBTask_CheckItemGift::OnExec()
{
	return Db_CheckItemGift(m_nCID, &m_vtGiftItem);
}

void MAsyncDBTask_CheckItemGift::OnDone()
{
	MCmdWriter Cmd;
	
	Cmd.StartBlob(sizeof(MTD_GiftCashItem));
	for(vector<DbData_ReceivedGiftItem>::iterator i = m_vtGiftItem.begin(); i != m_vtGiftItem.end(); i++)
	{
		DbData_ReceivedGiftItem *pCurr = &(*i);
		
		MTD_GiftCashItem item;
		ZeroInit(&item, sizeof(MTD_GiftCashItem));
		
		item.nGiftID = pCurr->nGiftID;
		strcpy(item.szSenderName, pCurr->szSenderName);
		item.nRentHourPeriod = pCurr->nRentHourPeriod;
		strcpy(item.szMessage, pCurr->szMessage);
		item.year = (unsigned short)pCurr->nGiftYear;
		item.month = (unsigned char)pCurr->nGiftMonth;	// 1 ~ 12.
		item.day = (unsigned char)pCurr->nGiftDay;	// 1 ~ 31.
		item.hour = (unsigned char)pCurr->nGiftHour;	// 0 ~ 23.
		item.nItemNodeCount = pCurr->nItemNodeCount;
		for(int i = 0; i < pCurr->nItemNodeCount; i++) item.nItemID[i] = pCurr->nItemID[i];
		
		Cmd.WriteData(&item, sizeof(MTD_GiftCashItem));
	}
	Cmd.EndBlob();
	
	Cmd.Finalize(MC_MATCH_GIFTITEM_LIST, MCFT_END);
	SendToClient(&Cmd, m_uidRequestor);
}


MAsyncDBTask_AcceptItemGift::MAsyncDBTask_AcceptItemGift(const MUID &uidPlayer, int nID, int nCID) : MBaseAsyncDBTask(uidPlayer)
{
	m_nID = nID;
	m_nCID = nCID;
}

MAsyncDBTask_AcceptItemGift::~MAsyncDBTask_AcceptItemGift()
{
}

bool MAsyncDBTask_AcceptItemGift::OnExec()
{
	return Db_AcceptItemGift(m_nID, m_nCID);
}

void MAsyncDBTask_AcceptItemGift::OnDone()
{
}



// -------------------------------------------------------------------------------------.

void AsyncDb_UpdateDTDailyRanking()
{
	MAsyncDBTask_UpdateDTDailyRanking *pNew = new MAsyncDBTask_UpdateDTDailyRanking;
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_UpdateDTWeeklyRanking()
{
	MAsyncDBTask_UpdateDTWeeklyRanking *pNew = new MAsyncDBTask_UpdateDTWeeklyRanking;
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_FetchDTTopRanking()
{
	MAsyncDBTask_FetchDTTopRanking *pNew = new MAsyncDBTask_FetchDTTopRanking;
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_InsertCQRecord(const MUID &uidPlayer, int nAID, int nMapID, int nRecordTime)
{
	MAsyncDBTask_InsertCQRecord *pNew = new MAsyncDBTask_InsertCQRecord(uidPlayer, nAID, nMapID, nRecordTime);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_SetExp(const MUID &uidPlayer, int nCID, int nLevel, int nXP, int nBP, int nKill, int nDeath)
{
	MAsyncDBTask_SetExp *pNew = new MAsyncDBTask_SetExp(uidPlayer, nCID, nLevel, nXP, nBP, nKill, nDeath);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_SetSurvivalPoint(const MUID &uidPlayer, int nCID, int nPoint)
{
	MAsyncDBTask_SetSurvivalPoint *pNew = new MAsyncDBTask_SetSurvivalPoint(uidPlayer, nCID, nPoint);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_SetDTScore(const MUID &uidPlayer, int nCID, int nTP, int nWin, int nLose, int nFinalWin)
{
	MAsyncDBTask_SetDTScore *pNew = new MAsyncDBTask_SetDTScore(uidPlayer, nCID, nTP, nWin, nLose, nFinalWin);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_TakeoffItem(const MUID &uidPlayer, int nCID, int nItemParts)
{
	MAsyncDBTask_TakeoffItem *pNew = new MAsyncDBTask_TakeoffItem(uidPlayer, nCID, nItemParts);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_DeleteSpentItem(const MUID &uidPlayer, int nCIID)
{
	MAsyncDBTask_DeleteSpentItem *pNew = new MAsyncDBTask_DeleteSpentItem(uidPlayer, nCIID);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_UpdateItemCount(const MUID &uidPlayer, int nCIID, int nItemCount)
{
	MAsyncDBTask_UpdateItemCount *pNew = new MAsyncDBTask_UpdateItemCount(uidPlayer, nCIID, nItemCount);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_UpdateCharQuestItem(const MUID &uidPlayer, int nCID, const char *pszData)
{
	MAsyncDBTask_UpdateCharQuestItem *pNew = new MAsyncDBTask_UpdateCharQuestItem(uidPlayer, nCID, pszData);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_FetchSurvivalRanking()
{
	MAsyncDBTask_FetchSurvivalRanking *pNew = new MAsyncDBTask_FetchSurvivalRanking;
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_UpdateSurvivalRanking()
{
	MAsyncDBTask_UpdateSurvivalRanking *pNew = new MAsyncDBTask_UpdateSurvivalRanking;
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_BanPlayer(const MUID &uidPlayer, int nAID)
{
	MAsyncDBTask_BanPlayer *pNew = new MAsyncDBTask_BanPlayer(uidPlayer, nAID);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_ClearExpiredItem(const MUID &uidPlayer, int nAID, int nCID)
{
	MAsyncDBTask_ClearExpiredItem *pNew = new MAsyncDBTask_ClearExpiredItem(uidPlayer, nAID, nCID);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_RemoveFriend(const MUID &uidPlayer, int nFriendID)
{
	MAsyncDBTask_RemoveFriend *pNew = new MAsyncDBTask_RemoveFriend(uidPlayer, nFriendID);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_JoinClan(const MUID &uidPlayer, int nCLID, int nCID)
{
	MAsyncDBTask_JoinClan *pNew = new MAsyncDBTask_JoinClan(uidPlayer, nCLID, nCID);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_UpdateClanMemberGrade(const MUID &uidPlayer, int nCLID, int nCID, int nMemberGrade)
{
	MAsyncDBTask_UpdateClanMemberGrade *pNew = new MAsyncDBTask_UpdateClanMemberGrade(uidPlayer, nCLID, nCID, nMemberGrade);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_UpdateClanMasterCID(const MUID &uidPlayer, int nCLID, int nCID)
{
	MAsyncDBTask_UpdateClanMasterCID *pNew = new MAsyncDBTask_UpdateClanMasterCID(uidPlayer, nCLID, nCID);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_LeaveClan(const MUID &uidPlayer, int nCLID, int nCID)
{
	MAsyncDBTask_LeaveClan *pNew = new MAsyncDBTask_LeaveClan(uidPlayer, nCLID, nCID);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_CloseClan(const MUID &uidPlayer, int nCLID)
{
	MAsyncDBTask_CloseClan *pNew = new MAsyncDBTask_CloseClan(uidPlayer, nCLID);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_UpdateItem(const MUID &uidPlayer, int nCID, int nCIID, int nItemCount, int nBounty)
{
	MAsyncDBTask_UpdateItem *pNew = new MAsyncDBTask_UpdateItem(uidPlayer, nCID, nCIID, nItemCount, nBounty);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_UpdateGambleItem(const MUID &uidPlayer, int nCID, int nCGIID, int nItemCount, int nBounty)
{
	MAsyncDBTask_UpdateGambleItem *pNew = new MAsyncDBTask_UpdateGambleItem(uidPlayer, nCID, nCGIID, nItemCount, nBounty);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_DeleteItem(const MUID &uidPlayer, int nCID, int nCIID, int nBounty)
{
	MAsyncDBTask_DeleteItem *pNew = new MAsyncDBTask_DeleteItem(uidPlayer, nCID, nCIID, nBounty);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_DeleteGambleItem(const MUID &uidPlayer, int nCID, int nCGIID, int nBounty)
{
	MAsyncDBTask_DeleteGambleItem *pNew = new MAsyncDBTask_DeleteGambleItem(uidPlayer, nCID, nCGIID, nBounty);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_EquipItem(const MUID &uidPlayer, int nCID, int nCIID, int nItemSlot)
{
	MAsyncDBTask_EquipItem *pNew = new MAsyncDBTask_EquipItem(uidPlayer, nCID, nCIID, nItemSlot);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_DecreaseGambleItem(const MUID &uidPlayer, int nCGIID)
{
	MAsyncDBTask_DecreaseGambleItem *pNew = new MAsyncDBTask_DecreaseGambleItem(uidPlayer, nCGIID);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_UpdateBounty(const MUID &uidPlayer, int nCID, int nBounty)
{
	MAsyncDBTask_UpdateBounty *pNew = new MAsyncDBTask_UpdateBounty(uidPlayer, nCID, nBounty);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_UpdateServerStatus(int nServerID, int nCurrPlayers)
{
	MAsyncDBTask_UpdateServerStatus *pNew = new MAsyncDBTask_UpdateServerStatus(nServerID, nCurrPlayers);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_UpdateIndividualRanking()
{
	MAsyncDBTask_UpdateIndividualRanking *pNew = new MAsyncDBTask_UpdateIndividualRanking;
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_SetBlitzScore(const MUID &uidPlayer, int nCID, int nWin, int nLose, int nPoint, int nMedal)
{
	MAsyncDBTask_SetBlitzScore *pNew = new MAsyncDBTask_SetBlitzScore(uidPlayer, nCID, nWin, nLose, nPoint, nMedal);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_AddBlitzShopItem(int nCID, int nItemID, int nPrice, int nBasePrice, int nCount, int nRentHourPeriod)
{
	MAsyncDBTask_AddBlitzShopItem *pNew = new MAsyncDBTask_AddBlitzShopItem(nCID, nItemID, nPrice, nBasePrice, nCount, nRentHourPeriod);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_ClearBlitzShop(int nCID)
{
	MAsyncDBTask_ClearBlitzShop *pNew = new MAsyncDBTask_ClearBlitzShop(nCID);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_InsertItem(const MUID &uidPlayer, int nCID, int nItemID, int nItemCount, int nRentHourPeriod, int nBounty)
{
	MAsyncDBTask_InsertItem *pNew = new MAsyncDBTask_InsertItem(uidPlayer, nCID, nItemID, nItemCount, nRentHourPeriod, nBounty);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_InsertGambleItem(const MUID &uidPlayer, int nCID, int nItemID, int nItemCount, int nBounty)
{
	MAsyncDBTask_InsertGambleItem *pNew = new MAsyncDBTask_InsertGambleItem(uidPlayer, nCID, nItemID, nItemCount, nBounty);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_MoveCItemToAItem(const MUID &uidPlayer, int nCIID, int nAID, int nItemCount, MMatchCharItem &CharItem)
{
	MAsyncDBTask_MoveCItemToAItem *pNew = new MAsyncDBTask_MoveCItemToAItem(uidPlayer, nCIID, nAID, nItemCount, CharItem);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_MoveAItemToCItem(const MUID &uidPlayer, int nAIID, int nCID, int nItemCount, MMatchAccItem &AccItem)
{
	MAsyncDBTask_MoveAItemToCItem *pNew = new MAsyncDBTask_MoveAItemToCItem(uidPlayer, nAIID, nCID, nItemCount, AccItem);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_BuyCashItem(const MUID &uidPlayer, int nCID, int nItemID, int nItemCount, int nRentHourPeriod)
{
	MAsyncDBTask_BuyCashItem *pNew = new MAsyncDBTask_BuyCashItem(uidPlayer, nCID, nItemID, nItemCount, nRentHourPeriod);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

void AsyncDb_SetAccountCash(const MUID &uidPlayer, int nAID, int nCash)
{
	MAsyncDBTask_SetAccountCash *pNew = new MAsyncDBTask_SetAccountCash(uidPlayer, nAID, nCash);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
}

// -------------------------------------------------------------------------------------.

#include "MMatchAdmin.h"

void MAsyncDBProcessThread(void *pArg)
{
	while(1)
	{
		MBaseAsyncDBTask *pTask = g_AsyncDBTaskMgr.SafeFrontTask();
		if(pTask != NULL)
		{
			if(pTask->OnExec() == true)
			{
				g_Mutex.lock();
				pTask->OnDone();
				g_Mutex.unlock();
			}
			
			delete pTask;
		}
		else
		{
			if(CheckServerHalt() == true)
			{
				break;
			}
		}
		
		this_thread::sleep_for(chrono::milliseconds(10));
	}
}
