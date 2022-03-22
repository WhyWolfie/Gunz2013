#include "pch.h"

#include "MMatchClan.h"
#include "MMatchObject.h"

#include "MMessageID.h"
#include "MMatchConstant.h"

#include "MMatchDBMgr.h"
#include "MAsyncDBProcess.h"

/*
	TODO : 
	clan join process security enahncement.
*/

void OnClanMemberList(const MUID &uidPlayer);

void SendClanMemberList(int nCLID)
{
	MMatchClan *pClan = g_ClanMgr.Get(nCLID);
	if(pClan == NULL) return;
	
	for(map<MUID, MMatchObject *>::iterator i = pClan->Begin(); i != pClan->End(); i++)
	{
		OnClanMemberList((*i).first);
	}
}

// char clan info. ------
void BuildCharClanInfoCommand(const MTD_CharClanInfo *pInfo, MCmdWriter *pOut)
{
	pOut->StartBlob(sizeof(MTD_CharClanInfo));
	pOut->WriteData(pInfo, sizeof(MTD_CharClanInfo));
	pOut->EndBlob();
	pOut->Finalize(MC_MATCH_CLAN_UPDATE_CHAR_CLANINFO, MCFT_END);
}

void SendCharClanInfoToClient(MMatchObject *pObj)
{
	MMatchClan *pClan = g_ClanMgr.Get(pObj->m_Clan.nCLID);
	// if(pClan == NULL) return;
	
	MCmdWriter Cmd;
	
	MTD_CharClanInfo info;
	ZeroInit(&info, sizeof(MTD_CharClanInfo));
	
	/*
	strcpy(info.szClanName, pClan->GetName());
	info.nGrade = pObj->m_Clan.nMemberGrade;
	*/
	
	if(pClan != NULL)
	{
		strcpy(info.szClanName, pClan->GetName());
		info.nGrade = pObj->m_Clan.nMemberGrade;
	}
	
	BuildCharClanInfoCommand(&info, &Cmd);
	SendToClient(&Cmd, pObj->GetUID());
}

void SendEmptyCharClanInfoToClan(int nCLID)
{
	MCmdWriter Cmd;
	
	MTD_CharClanInfo info;
	ZeroInit(&info, sizeof(MTD_CharClanInfo));	// no clan name & grade MCG_NONE.
	
	BuildCharClanInfoCommand(&info, &Cmd);
	SendToClan(&Cmd, nCLID);
}
// end of char clan info. ------

// validate checks. ----------
int CheckClanCreatable(MMatchObject *pObj, const char *pszClanName)
{
	if(pObj->m_Clan.nCLID != 0)
	{
		return MERR_ALREADY_IN_CLAN;
	}
	
	int nNameLen = (int)strlen(pszClanName);
	if(nNameLen < MIN_CLANNAME_LEN)
	{
		return MERR_NAME_SHORT;
	}
	else if(nNameLen > MAX_CLANNAME_LEN)
	{
		return MERR_NAME_LONG;
	}
	
	if(pObj->m_Exp.nLevel < CLAN_CREATION_REQUIRED_LEVEL)
	{
		return MERR_LACKING_LEVEL_FOR_ESTABLISH_CLAN;
	}
	
	if(pObj->m_Exp.nBP < CLAN_CREATION_REQUIRED_BOUNTY)
	{
		return MERR_LACKING_BOUNTY_FOR_ESTABLISH_CLAN;
	}
	
	int nDbRet;
	
	if(Db_IsClanExists(pszClanName, &nDbRet) == false)
	{
		return MERR_CANNOT_CREATE_CLAN;
	}
	
	if(nDbRet == 1)
	{
		return MERR_CLAN_ALREADY_EXISTS;
	}
	
	return MSG_OK;
}

int CheckClanJoinable(MMatchObject *pInviterObj, MMatchObject *pJoinerObj)
{
	if(pJoinerObj->m_bCharInfoExist == false)
	{
		return MERR_TARGET_USER_NOT_FOUND;
	}
	
	if(IsHaveClanRights(pInviterObj->m_Clan.nMemberGrade, MCG_ADMIN) == false)
	{
		return MERR_NOT_CLAN_MASTER_OR_ADMIN;
	}
	
	if(pJoinerObj->m_Clan.nCLID != 0)
	{
		return MERR_JOINER_ALREADY_IN_CLAN;
	}
	
	if(pJoinerObj->m_nPlace != MMP_LOBBY)
	{
		return MERR_JOINER_NOT_ON_LOBBY;
	}
	
	return MSG_OK;
}

int CheckClanGradeChangable(MMatchObject *pMasterObj, MMatchObject *pTargetObj)
{
	if(pTargetObj->m_bCharInfoExist == false)
	{
		return MERR_TARGET_USER_NOT_FOUND;
	}
	
	if(pTargetObj->m_Clan.nCLID == 0)
	{
		return MERR_NOT_JOINED_TO_CLAN;
	}
	
	if(pMasterObj->m_Clan.nCLID != pTargetObj->m_Clan.nCLID)
	{
		return MERR_NOT_SAME_CLAN;
	}
	
	if(IsHaveClanRights(pMasterObj->m_Clan.nMemberGrade, MCG_MASTER) == false)
	{
		return MERR_NOT_CLAN_MASTER;
	}
	
	return MSG_OK;
}

int CheckClanLeavable(MMatchObject *pLeaverObj)
{
	if(pLeaverObj->m_bCharInfoExist == false)
	{
		return MERR_TARGET_USER_NOT_FOUND;
	}
	
	if(pLeaverObj->m_Clan.nCLID == 0)
	{
		return MERR_NOT_JOINED_TO_CLAN;
	}
	
	if(pLeaverObj->m_Clan.nMemberGrade == MCG_MASTER)
	{
		return MERR_CANNOT_LEAVE_CLAN;
	}
	
	return MSG_OK;
}
// end of validate checks. ----------

void OnCreateClan(const MUID &uidPlayer, const char *pszClanName)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	MCmdWriter Cmd;
	
	int nRet = CheckClanCreatable(pObj, pszClanName);
	if(nRet != MSG_OK)
	{
		Cmd.WriteInt(nRet);
		Cmd.Finalize(MC_MATCH_CLAN_RESPONSE_CREATE_CLAN, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	int nResult = MSG_OK;
	
	int nCLID;
	if(Db_CreateClan(pObj->m_Char.nCID, pszClanName, &nCLID) == true)
	{
		MMatchClan *pClan = g_ClanMgr.Add(
							nCLID, 
							pszClanName, 
							1, 
							1000, 
							pObj->m_Char.nCID, 
							0, 
							0, 
							0, 
							0, 
							0, 
							"", 
							0
							);
		if(pClan != NULL)
		{
			pClan->Join(pObj);
			
			pObj->m_Clan.nCLID = nCLID;
			pObj->m_Clan.nMemberGrade = MCG_MASTER;
			
			pObj->m_Exp.nBP -= CLAN_CREATION_REQUIRED_BOUNTY;
		}
		else
		{
			nResult = MERR_CANNOT_CREATE_CLAN;
		}
	}
	else
	{
		nResult = MERR_CANNOT_CREATE_CLAN;
	}
	
	Cmd.WriteInt(nResult);
	Cmd.Finalize(MC_MATCH_CLAN_RESPONSE_CREATE_CLAN, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
	
	if(nResult == MSG_OK)
	{
		SendCharClanInfoToClient(pObj);
	}
}

void OnJoinClan(const MUID &uidPlayer, const char *pszClanName, const char *pszJoinerName)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	MMatchClan *pClan = g_ClanMgr.Get(pObj->m_Clan.nCLID);
	if(pClan == NULL)
	{
		MCmdWriter tmp;
		tmp.WriteInt(MERR_THE_CLAN_NOT_FOUND);
		tmp.Finalize(MC_MATCH_CLAN_RESPONSE_JOIN_CLAN, MCFT_END);
		SendToClient(&tmp, uidPlayer);
		return;
	}
	
	MCmdWriter Cmd;
	
	if(MStricmp(pClan->GetName(), pszClanName) != 0)
	{
		Cmd.WriteInt(MERR_WRONG_CLAN_NAME);
		Cmd.Finalize(MC_MATCH_CLAN_RESPONSE_JOIN_CLAN, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	MMatchObject *pJoinerObj = g_ObjectMgr.Get(pszJoinerName);
	if(pJoinerObj == NULL)
	{
		Cmd.WriteInt(MERR_TARGET_USER_NOT_FOUND);
		Cmd.Finalize(MC_MATCH_CLAN_RESPONSE_JOIN_CLAN, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	int nRet = CheckClanJoinable(pObj, pJoinerObj);
	if(nRet != MSG_OK)
	{
		Cmd.WriteInt(nRet);
		Cmd.Finalize(MC_MATCH_CLAN_RESPONSE_JOIN_CLAN, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	if((pJoinerObj->m_nOptionFlag & MBITFLAG_USEROPTION_REJECT_INVITE) != 0)
	{
		Cmd.WriteInt(MATCHNOTIFY_USER_INVITE_REJECTED);
		Cmd.Finalize(MC_MATCH_NOTIFY, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	Cmd.WriteString(pClan->GetName());
	Cmd.WriteMUID(uidPlayer);
	Cmd.WriteString(pObj->m_Char.szName);
	Cmd.Finalize(MC_MATCH_CLAN_ASK_JOIN_AGREEMENT, MCFT_END);
	SendToClient(&Cmd, pJoinerObj->GetUID());
	
	MCmdWriter Cmd2;
	Cmd2.WriteInt(MSG_OK);
	Cmd2.Finalize(MC_MATCH_CLAN_RESPONSE_JOIN_CLAN, MCFT_END);
	SendToClient(&Cmd2, uidPlayer);
}

void OnAnswerJoinClanAgreement(const MUID &uidPlayer, const MUID &uidClanAdmin, const char *pszJoinerName, bool bAnswer)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	MMatchObject *pInviterObj = g_ObjectMgr.Get(uidClanAdmin);
	if(pInviterObj == NULL) return;
	
	MCmdWriter Cmd;
	
	Cmd.WriteMUID(uidClanAdmin);
	Cmd.WriteString(pszJoinerName);
	Cmd.WriteBool(bAnswer);
	Cmd.Finalize(MC_MATCH_CLAN_ANSWER_JOIN_AGREEMENT, MCFT_END);
	SendToClient(&Cmd, uidClanAdmin);
}

void OnJoinClanAgreed(const MUID &uidPlayer, const char *pszClanName, const char *pszJoinerName)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	MMatchClan *pClan = g_ClanMgr.Get(pObj->m_Clan.nCLID);
	if(pClan == NULL)
	{
		MCmdWriter tmp;
		tmp.WriteInt(MERR_THE_CLAN_NOT_FOUND);
		tmp.Finalize(MC_MATCH_CLAN_RESPONSE_AGREED_JOIN_CLAN, MCFT_END);
		SendToClient(&tmp, uidPlayer);
		return;
	}
	
	MCmdWriter Cmd;
	
	if(MStricmp(pClan->GetName(), pszClanName) != 0)
	{
		Cmd.WriteInt(MERR_WRONG_CLAN_NAME);
		Cmd.Finalize(MC_MATCH_CLAN_RESPONSE_AGREED_JOIN_CLAN, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	MMatchObject *pJoinerObj = g_ObjectMgr.Get(pszJoinerName);
	if(pJoinerObj == NULL)
	{
		Cmd.WriteInt(MERR_TARGET_USER_NOT_FOUND);
		Cmd.Finalize(MC_MATCH_CLAN_RESPONSE_AGREED_JOIN_CLAN, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	int nRet = CheckClanJoinable(pObj, pJoinerObj);
	if(nRet != MSG_OK)
	{
		Cmd.WriteInt(nRet);
		Cmd.Finalize(MC_MATCH_CLAN_RESPONSE_AGREED_JOIN_CLAN, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	if((pJoinerObj->m_nOptionFlag & MBITFLAG_USEROPTION_REJECT_INVITE) != 0)
	{
		Cmd.WriteInt(MATCHNOTIFY_USER_INVITE_REJECTED);
		Cmd.Finalize(MC_MATCH_NOTIFY, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	int nResult = MSG_OK;
	
	/*
	if(Db_JoinClan(pObj->m_Clan.nCLID, pJoinerObj->m_Char.nCID) == true)
	{
		pClan->Join(pJoinerObj);
		
		pJoinerObj->m_Clan.nCLID = pObj->m_Clan.nCLID;
		pJoinerObj->m_Clan.nMemberGrade = MCG_MEMBER;
	}
	else
	{
		nResult = MERR_INVALID_JOIN_PROCESS;
	}
	*/
	
	AsyncDb_JoinClan(uidPlayer, pObj->m_Clan.nCLID, pJoinerObj->m_Char.nCID);
	
	pClan->Join(pJoinerObj);
		
	pJoinerObj->m_Clan.nCLID = pObj->m_Clan.nCLID;
	pJoinerObj->m_Clan.nMemberGrade = MCG_MEMBER;
	
	Cmd.WriteInt(nResult);
	Cmd.Finalize(MC_MATCH_CLAN_RESPONSE_AGREED_JOIN_CLAN, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
	
	MCmdWriter Cmd2;
	
	if(nResult == MSG_OK)
	{
		Cmd2.WriteInt(MRESULT_CLAN_JOINED);
		Cmd2.Finalize(MC_MATCH_RESPONSE_RESULT, MCFT_END);
		SendToClient(&Cmd2, pJoinerObj->GetUID());
		
		SendCharClanInfoToClient(pJoinerObj);
		
		SendClanMemberList(pObj->m_Clan.nCLID);
	}
	else
	{
		Cmd2.WriteInt(nResult);
		Cmd2.Finalize(MC_MATCH_RESPONSE_RESULT, MCFT_END);
		SendToClient(&Cmd2, pJoinerObj->GetUID());
	}
}

void OnChangeClanMemberGrade(const MUID &uidPlayer, const char *pszMemberName, int nGrade)
{
	if(nGrade != MCG_MASTER && nGrade != MCG_ADMIN && nGrade != MCG_MEMBER) return;
		
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	if(pObj->m_Clan.nCLID == 0) return;
	
	MCmdWriter Cmd;
	
	MMatchObject *pTargetObj = g_ObjectMgr.Get(pszMemberName);
	if(pTargetObj == NULL)
	{
		Cmd.WriteInt(MERR_TARGET_USER_NOT_FOUND);
		Cmd.Finalize(MC_MATCH_CLAN_MASTER_RESPONSE_CHANGE_GRADE, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	int nRet = CheckClanGradeChangable(pObj, pTargetObj);
	if(nRet != MSG_OK)
	{
		Cmd.WriteInt(nRet);
		Cmd.Finalize(MC_MATCH_CLAN_MASTER_RESPONSE_CHANGE_GRADE, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	int nResult = MSG_OK;
	
	/*
	if(Db_UpdateClanMemberGrade(pObj->m_Clan.nCLID, pTargetObj->m_Char.nCID, nGrade) == true)
	{
		// success update member grade.
		
		if(nGrade == MCG_MASTER)	// new grade == master.
		{
			if(Db_UpdateClanMemberGrade(pObj->m_Clan.nCLID, pObj->m_Char.nCID, MCG_MEMBER) == true)
			{
				// success update master grade to member.
				
				if(Db_UpdateClanMasterCID(pObj->m_Clan.nCLID, pTargetObj->m_Char.nCID) == true)
				{
					// success update clan master cid : do nothing.
				}
				else
				{
					nResult = MC_MATCH_CLAN_MASTER_RESPONSE_CHANGE_GRADE;
				}
			}
			else
			{
				nResult = MC_MATCH_CLAN_MASTER_RESPONSE_CHANGE_GRADE;
			}
		}
	}
	else
	{
		nResult = MC_MATCH_CLAN_MASTER_RESPONSE_CHANGE_GRADE;
	}
	*/
	
	AsyncDb_UpdateClanMemberGrade(uidPlayer, pObj->m_Clan.nCLID, pTargetObj->m_Char.nCID, nGrade);
	
	if(nGrade == MCG_MASTER)
	{
		AsyncDb_UpdateClanMemberGrade(uidPlayer, pObj->m_Clan.nCLID, pObj->m_Char.nCID, MCG_MEMBER);
		AsyncDb_UpdateClanMasterCID(uidPlayer, pObj->m_Clan.nCLID, pTargetObj->m_Char.nCID);
	}
	
	Cmd.WriteInt(nResult);
	Cmd.Finalize(MC_MATCH_CLAN_MASTER_RESPONSE_CHANGE_GRADE, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
	
	if(nResult == MSG_OK)
	{
		pTargetObj->m_Clan.nMemberGrade = nGrade;
		SendCharClanInfoToClient(pTargetObj);
		
		if(nGrade == MCG_MASTER)
		{
			pObj->m_Clan.nMemberGrade = MCG_MEMBER;
			SendCharClanInfoToClient(pObj);
		}
		
		SendClanMemberList(pObj->m_Clan.nCLID);
	}
}

void OnExpelClanMember(const MUID &uidPlayer, const char *pszMemberName)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	if(pObj->m_Clan.nCLID == 0) return;
	
	MCmdWriter Cmd;
	
	if(IsHaveClanRights(pObj->m_Clan.nMemberGrade, MCG_ADMIN) == false)
	{
		Cmd.WriteInt(MERR_NOT_CLAN_MASTER_OR_ADMIN);
		Cmd.Finalize(MC_MATCH_CLAN_ADMIN_RESPONSE_EXPEL_MEMBER, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	MMatchObject *pLeaverObj = g_ObjectMgr.Get(pszMemberName);
	if(pLeaverObj == NULL)
	{
		Cmd.WriteInt(MERR_TARGET_USER_NOT_FOUND);
		Cmd.Finalize(MC_MATCH_CLAN_ADMIN_RESPONSE_EXPEL_MEMBER, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	int nRet = CheckClanLeavable(pLeaverObj);
	if(nRet != MSG_OK)
	{
		Cmd.WriteInt(nRet);
		Cmd.Finalize(MC_MATCH_CLAN_ADMIN_RESPONSE_EXPEL_MEMBER, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	if(pObj->m_Clan.nCLID != pLeaverObj->m_Clan.nCLID)
	{
		Cmd.WriteInt(MERR_NOT_SAME_CLAN);
		Cmd.Finalize(MC_MATCH_CLAN_ADMIN_RESPONSE_EXPEL_MEMBER, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	int nResult = MSG_OK;
	
	/*
	if(Db_LeaveClan(pObj->m_Clan.nCLID, pLeaverObj->m_Char.nCID) == true)
	{
		MMatchClan *pClan = g_ClanMgr.Get(pObj->m_Clan.nCLID);
		if(pClan != NULL)
		{
			pClan->Leave(pLeaverObj);
		}
		
		pLeaverObj->m_Clan.nCLID = 0;
		pLeaverObj->m_Clan.nMemberGrade = MCG_NONE;
	}
	else
	{
		nResult = MERR_CANNOT_LEAVE_CLAN;
	}
	*/
	
	AsyncDb_LeaveClan(uidPlayer, pObj->m_Clan.nCLID, pLeaverObj->m_Char.nCID);
	
	MMatchClan *pClan = g_ClanMgr.Get(pObj->m_Clan.nCLID);
	if(pClan != NULL)
	{
		pClan->Leave(pLeaverObj);
	}
		
	pLeaverObj->m_Clan.nCLID = 0;
	pLeaverObj->m_Clan.nMemberGrade = MCG_NONE;
	
	Cmd.WriteInt(nResult);
	Cmd.Finalize(MC_MATCH_CLAN_ADMIN_RESPONSE_EXPEL_MEMBER, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
	
	if(nResult == MSG_OK)
	{
		SendCharClanInfoToClient(pLeaverObj);
		SendClanMemberList(pObj->m_Clan.nCLID);
	}
}

void OnLeaveClan(const MUID &uidPlayer)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	MCmdWriter Cmd;
	
	int nRet = CheckClanLeavable(pObj);
	if(nRet != MSG_OK)
	{
		Cmd.WriteInt(nRet);
		Cmd.Finalize(MC_MATCH_CLAN_RESPONSE_LEAVE_CLAN, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	int nResult = MSG_OK;
	
	/*
	if(Db_LeaveClan(pObj->m_Clan.nCLID, pObj->m_Char.nCID) == true)
	{
		MMatchClan *pClan = g_ClanMgr.Get(pObj->m_Clan.nCLID);
		if(pClan != NULL)
		{
			pClan->Leave(pObj);
		}
		
		SendClanMemberList(pObj->m_Clan.nCLID);
		
		pObj->m_Clan.nCLID = 0;
		pObj->m_Clan.nMemberGrade = MCG_NONE;
	}
	else
	{
		nResult = MERR_CANNOT_LEAVE_CLAN;
	}
	*/
	
	AsyncDb_LeaveClan(uidPlayer, pObj->m_Clan.nCLID, pObj->m_Char.nCID);
	
	MMatchClan *pClan = g_ClanMgr.Get(pObj->m_Clan.nCLID);
	if(pClan != NULL)
	{
		pClan->Leave(pObj);
	}
		
	SendClanMemberList(pObj->m_Clan.nCLID);
		
	pObj->m_Clan.nCLID = 0;
	pObj->m_Clan.nMemberGrade = MCG_NONE;
	
	Cmd.WriteInt(nResult);
	Cmd.Finalize(MC_MATCH_CLAN_RESPONSE_LEAVE_CLAN, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
	
	if(nResult == MSG_OK)
	{
		SendCharClanInfoToClient(pObj);
	}
}

void OnCloseClan(const MUID &uidPlayer, const char *pszClanName)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	MMatchClan *pClan = g_ClanMgr.Get(pObj->m_Clan.nCLID);
	if(pClan == NULL) return;
	
	MCmdWriter Cmd;
	
	if(pObj->m_Clan.nMemberGrade != MCG_MASTER)
	{
		Cmd.WriteInt(MERR_CANNOT_CLOSE_CLAN);
		Cmd.Finalize(MC_MATCH_CLAN_RESPONSE_CLOSE_CLAN, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	if(MStricmp(pClan->GetName(), pszClanName) != 0)
	{
		Cmd.WriteInt(MERR_WRONG_CLAN_NAME);
		Cmd.Finalize(MC_MATCH_CLAN_RESPONSE_CLOSE_CLAN, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	int nResult = MSG_OK;
	
	/*
	if(Db_CloseClan(pObj->m_Clan.nCLID) == true)
	{
		// do nothing.
	}
	else
	{
		nResult = MERR_CANNOT_CLOSE_CLAN;
	}
	*/
	
	AsyncDb_CloseClan(uidPlayer, pObj->m_Clan.nCLID);
	
	Cmd.WriteInt(nResult);
	Cmd.Finalize(MC_MATCH_CLAN_RESPONSE_CLOSE_CLAN, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
	
	if(nResult == MSG_OK)
	{
		SendEmptyCharClanInfoToClan(pObj->m_Clan.nCLID);
		
		for(map<MUID, MMatchObject *>::iterator i = pClan->Begin(); i != pClan->End(); i++)
		{
			MMatchObject *pCurr = (*i).second;
			
			pCurr->m_Clan.nCLID = 0;
			pCurr->m_Clan.nMemberGrade = MCG_NONE;
		}
		pClan->AllLeave();
	}
}

void OnClanMemberList(const MUID &uidPlayer)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	MMatchClan *pClan = g_ClanMgr.Get(pObj->m_Clan.nCLID);
	if(pClan == NULL) return;
	
	MCmdWriter Cmd;
	Cmd.StartBlob(sizeof(MTD_ClanMemberListNode));
	
	for(map<MUID, MMatchObject *>::iterator i = pClan->Begin(); i != pClan->End(); i++)
	{
		MMatchObject *pCurr = (*i).second;
		
		MTD_ClanMemberListNode node;
		node.uidPlayer = pCurr->GetUID();
		strcpy(node.szName, pCurr->m_Char.szName);
		node.nLevel = (char)pCurr->m_Exp.nLevel;
		node.nClanGrade = pCurr->m_Clan.nMemberGrade;
		node.nPlace = pCurr->m_nPlace;
		
		Cmd.WriteData(&node, sizeof(MTD_ClanMemberListNode));
	}
	
	Cmd.EndBlob();
	Cmd.Finalize(MC_MATCH_CLAN_RESPONSE_MEMBER_LIST, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
}

void OnClanMsg(const MUID &uidPlayer, const char *pszMsg)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	if(pObj->m_Clan.nCLID == 0) return;
	
	MCmdWriter Cmd;
	Cmd.WriteString(pObj->m_Char.szName);
	Cmd.WriteString(pszMsg);
	Cmd.Finalize(MC_MATCH_CLAN_MSG, MCFT_END);
	SendToClan(&Cmd, pObj->m_Clan.nCLID);
}

void OnClanInfo(const MUID &uidPlayer, const char *pszClanName)
{
	// cw channel info.
}

void OnClanEmblemURL(const MUID &uidPlayer, vector<int> *pClanIDs)
{
	MCmdWriter Cmd(8192);
	
	for(vector<int>::iterator i = pClanIDs->begin(); i != pClanIDs->end(); i++)
	{
		int nCLID = (*i);
		
		MMatchClan *pCurr = g_ClanMgr.Get(nCLID);
		if(pCurr == NULL) continue;
		
		Cmd.WriteInt(nCLID);
		Cmd.WriteInt(pCurr->GetEmblemChecksum());
		Cmd.WriteString(pCurr->GetEmblemURL());
		Cmd.Finalize(MC_MATCH_CLAN_RESPONSE_EMBLEMURL);
	}
	
	Cmd.Finalize();
	SendToClient(&Cmd, uidPlayer);
}