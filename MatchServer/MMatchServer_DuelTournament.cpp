#include "pch.h"
#include "MMatchDuelTournament.h"

#include "MMatchObject.h"
#include "MMatchChannel.h"
#include "MMatchStage.h"

#include "MMessageID.h"

#include "MMatchGame.h"

void OnDuelTournamentRankingInfo(const MUID &uidPlayer)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	MMatchCharDuelTournamentInfo *pInfo = &pObj->m_DuelTournament;
	
	g_DTRankingMgr.SendRankingInfo(uidPlayer);
	
	MCmdWriter Cmd;
	
	Cmd.WriteInt(pInfo->nTP);
	Cmd.WriteInt(pInfo->nWin);
	Cmd.WriteInt(pInfo->nLose);
	Cmd.WriteInt(pInfo->nRanking);
	Cmd.WriteInt(pInfo->nRankingDiff);
	Cmd.WriteInt(pInfo->nFinalWin);
	Cmd.WriteInt(pInfo->nClass);
	Cmd.Finalize(MC_MATCH_DUELTOURNAMENT_CHAR_INFO);
	
	Cmd.WriteInt(pInfo->nPrevTP);
	Cmd.WriteInt(pInfo->nPrevWin);
	Cmd.WriteInt(pInfo->nPrevLose);
	Cmd.WriteInt(pInfo->nPrevRanking);
	Cmd.WriteInt(pInfo->nPrevFinalWin);
	Cmd.Finalize(MC_MATCH_DUELTOURNAMENT_CHAR_INFO_PREVIOUS, MCFT_END);
	
	SendToClient(&Cmd, uidPlayer);
}

// param 2 : nTournamentType is currently not used.
void OnDuelTournamentJoin(const MUID &uidPlayer, int nTournamentType)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	// if(pObj->m_uidChannel == MUID(0, 0)) return;
	// if(pObj->m_uidStage != MUID(0, 0)) return;
	
	if(pObj->m_nPlace != MMP_LOBBY) return;
	
	MMatchChannel *pChannel = g_ChannelMgr.Get(pObj->m_uidChannel);
	if(pChannel == NULL) return;
	
	MCmdWriter Cmd;
	
	if(pChannel->IsDuelTournament() == false)
	{
		Cmd.WriteInt(MERR_NOT_DUELTOURNAMENT_CHANNEL);
		Cmd.Finalize(MC_MATCH_DUELTOURNAMENT_RESPONSE_JOINGAME, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	g_DTQMgr.ReserveChallenge(pObj);
}

// param 2 : nTournamentType is currently not used.
void OnDuelTournamentCancel(const MUID &uidPlayer, int nTournamentType)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	g_DTQMgr.CancelChallenge(pObj);
}

void OnDuelTournamentGamePlayerStatus(const MUID &uidPlayer, float fDamagedPoint, float fHP, float fAP)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	if(pObj->m_uidStage == MUID(0, 0)) return;
	if(pObj->CheckGameFlag(MMOGF_ENTERED) == false) return;
	
	MMatchStage *pStage = g_StageMgr.Get(pObj->m_uidStage);
	if(pStage == NULL) return;
	
	if(pStage->GetGameType() != (int)MMGT_DUEL_TOURNAMENT) return;
	if(pStage->CheckGameInfoExists() == false) return;
	
	MMatchGame_DuelTournament *pGame = (MMatchGame_DuelTournament *)pStage->GetGame();
	pGame->SetPlayerStatus(uidPlayer, (int)fDamagedPoint, (int)fHP, (int)fAP);
}