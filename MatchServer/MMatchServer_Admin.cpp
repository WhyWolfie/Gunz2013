#include "pch.h"
#include "MMatchAdmin.h"

#include "MMatchObject.h"
#include "MMatchStage.h"

#include "MMatchDBMgr.h"
#include "MAsyncDBProcess.h"

#include "MMatchServer_Etc.h"

void OnAdminAnnounce(const MUID &uidAdmin, const char *pszMsg, unsigned int nMsgType)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidAdmin);
	if(pObj == NULL) return;
	
	if(pObj->IsAdmin() == false) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	char szMsg[512];
	sprintf(szMsg, "[%s] %s", pObj->m_Char.szName, pszMsg);
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidAdmin);
	Cmd.WriteString(szMsg);
	Cmd.WriteUInt(nMsgType);
	Cmd.Finalize(MC_ADMIN_ANNOUNCE, MCFT_END | MCFT_RAW);
	SendToAll(&Cmd);
}

void OnAdminHalt(const MUID &uidAdmin)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidAdmin);
	if(pObj == NULL) return;
	
	if(pObj->IsAdmin() == false) return;
	
	// shutdown the server immediately (without messages).
	ShutdownServer();
	
	AnnounceToClient("Your server shutdown request is accepted. Now processing...", uidAdmin);
}

void OnAdminKick(const MUID &uidAdmin, const char *pszTargetName)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidAdmin);
	if(pObj == NULL) return;
	
	if(pObj->IsAdmin() == false) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	MMatchObject *pTargetObj = g_ObjectMgr.Get(pszTargetName);
	if(pTargetObj == NULL) return;
	
	if(pTargetObj->m_bCharInfoExist == false) return;
	
	if(pTargetObj->IsAdmin() == true)
	{
		AnnounceToClient("You can't kick (or ban) administrators.", uidAdmin);
		return;
	}
	
	mlog("Admin player %s (AID %d) is requested to kick %s (AID %d) from the server. Processing this request...", 
		  pObj->m_Char.szName, pObj->m_Account.nAID, pTargetObj->m_Char.szName, pTargetObj->m_Account.nAID);
	
	/*
	if(Db_BanPlayer(pTargetObj->m_Account.nAID) == false)
	{
		// db error, but do nothing.
	}
	*/
	AsyncDb_BanPlayer(uidAdmin, pTargetObj->m_Account.nAID);
	
	pTargetObj->Disconnect();
}

void OnAdminHide(const MUID &uidAdmin)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidAdmin);
	if(pObj == NULL) return;
	
	if(pObj->IsAdmin() == false) return;
	
	if(pObj->IsHide() == true)
	{
		pObj->Hide(false);
		AnnounceToClient("Hide mode is disabled.", uidAdmin);
	}
	else
	{
		pObj->Hide(true);
		AnnounceToClient("Hide mode is enabled.", uidAdmin);
	}
}

void OnAdminAssassin(const MUID &uidAdmin)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidAdmin);
	if(pObj == NULL) return;
	
	if(pObj->IsAdmin() == false) return;
	
	if(pObj->m_uidStage == MUID(0, 0)) return;
	if(pObj->CheckGameFlag(MMOGF_ENTERED) == false) return;
	
	MMatchStage *pStage = g_StageMgr.Get(pObj->m_uidStage);
	if(pStage == NULL) return;
	
	if(pStage->GetGameType() != (int)MMGT_ASSASSINATE) return;
	
	if(pStage->CheckGameInfoExists() == false) return;
	
	MMatchGame_Assassinate *pGame = (MMatchGame_Assassinate *)pStage->GetGame();
	
	if(pObj->m_GameInfo.nTeam == MMT_RED)
	{
		pGame->SetRedTeamCommanderUID(uidAdmin);
	}
	else if(pObj->m_GameInfo.nTeam == MMT_BLUE)
	{
		pGame->SetBlueTeamCommanderUID(uidAdmin);
	}
}