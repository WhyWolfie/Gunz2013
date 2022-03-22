#include "pch.h"
#include "MMatchChallengeQuest.h"

#include "MMatchObject.h"
#include "MMatchStage.h"
#include "MMatchGame.h"

void OnChallengeQuestBestRecord(const MUID &uidPlayer)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	MCmdWriter Cmd;
	
	Cmd.StartBlob(sizeof(MTD_CQRecordInfo));
	for(vector<MMatchChallengeQuestRecordInfo>::iterator i = pObj->m_vtCQRecordInfo.begin(); i != pObj->m_vtCQRecordInfo.end(); i++)
	{
		MMatchChallengeQuestRecordInfo *pCurr = &(*i);
		
		MTD_CQRecordInfo info = {pCurr->nScenarioID, pCurr->nTime};
		Cmd.WriteData(&info, sizeof(MTD_CQRecordInfo));
	}
	Cmd.EndBlob();
	
	Cmd.Finalize(MC_CHALLENGE_QUEST_RESPONSE_TOP_RECORD, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
}

void OnChallengeQuestNPCDead(const MUID &uidPlayer, const MUID &uidKiller, const MUID &uidNPC, const ShortVector *pPos)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	if(pObj->m_uidStage == MUID(0, 0)) return;
	if(pObj->CheckGameFlag(MMOGF_ENTERED) == false) return;
	
	MMatchStage *pStage = g_StageMgr.Get(pObj->m_uidStage);
	if(pStage == NULL) return;
	
	if(pStage->GetGameType() != (int)MMGT_CHALLENGE_QUEST) return;

	if(pStage->CheckGameInfoExists() == false) return;
	
	MMatchGame_ChallengeQuest *pGame = (MMatchGame_ChallengeQuest *)pStage->GetGame();
	
	if(pGame->IsValidActorOwner(uidPlayer, uidNPC) == false) return;
	if(pGame->ActorDead(uidKiller, uidNPC) == false) return;
	
	int nWorldItemID = 0;
	switch(RandNum() % 4)
	{
		case 0	: nWorldItemID = 2; break;	// hp02.
		case 1	: nWorldItemID = 5; break;	// ap02.
		case 2	: nWorldItemID = 8; break;	// bullet02.
		default	:
		case 3	: break;					// nothing.
	}
	
	if(nWorldItemID != 0)
	{
		pGame->AddWorldItem(nWorldItemID, (float)pPos->x, (float)pPos->y, (float)pPos->z, (int)MTD_WorldItemSubType_Dynamic);
	}
}

void OnChallengeQuestPlayerDead(const MUID &uidPlayer)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	if(pObj->m_uidStage == MUID(0, 0)) return;
	if(pObj->CheckGameFlag(MMOGF_ENTERED) == false) return;
	
	MMatchStage *pStage = g_StageMgr.Get(pObj->m_uidStage);
	if(pStage == NULL) return;
	
	if(pStage->GetGameType() != (int)MMGT_CHALLENGE_QUEST) return;
	
	pObj->m_GameInfo.bAlive = false;
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidPlayer);
	Cmd.Finalize(MC_CHALLENGE_QUEST_PLAYER_DEAD, MCFT_END);
	SendToBattle(&Cmd, pObj->m_uidStage);
}

void OnChallengeQuestNPCSpawn(const MUID &uidPlayer, const MUID &uidOwnerNPC, const char *pszActorName, const FloatVector *pPos, const FloatVector *pDir)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	if(pObj->m_uidStage == MUID(0, 0)) return;
	if(pObj->CheckGameFlag(MMOGF_ENTERED) == false) return;
	
	MMatchStage *pStage = g_StageMgr.Get(pObj->m_uidStage);
	if(pStage == NULL) return;
	
	// if(pStage->GetGameType() != (int)MMGT_CHALLENGE_QUEST) return;

	if(pStage->CheckGameInfoExists() == false) return;
	
	if(pStage->GetGameType() == (int)MMGT_CHALLENGE_QUEST)
	{
		MMatchGame_ChallengeQuest *pGame = (MMatchGame_ChallengeQuest *)pStage->GetGame();
	
		if(pGame->IsValidActorOwner(uidPlayer, uidOwnerNPC) == false) return;
		if(pGame->SpawnActorByPlayer(uidOwnerNPC, pszActorName, pPos, pDir) == false) return;
	}
	else if(pStage->GetGameType() == (int)MMGT_BLITZKRIEG)
	{
		MMatchGame_BlitzKrieg *pGame = (MMatchGame_BlitzKrieg *)pStage->GetGame();
		
		int nTeamID = pGame->GetActorTeamID(uidOwnerNPC);
		
		if(pGame->IsValidActorOwner(uidPlayer, uidOwnerNPC) == false) return;
		if(pGame->SpawnActorByPlayer(uidOwnerNPC, pszActorName, nTeamID, pPos, pDir) == false) return;
	}
}