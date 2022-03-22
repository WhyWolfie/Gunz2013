#include "pch.h"
#include "MMatchBlitzKrieg.h"

#include "MMatchObject.h"
#include "MMatchStage.h"
#include "MMatchGame.h"

#include "MMatchClan.h"

void OnBlitzMatchCollaboratorList(const MUID &uidPlayer)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	MMatchChannel *pChannel = g_ChannelMgr.Get(pObj->m_uidChannel);
	if(pChannel == NULL) return;
	
	if(pChannel->IsBlitzKrieg() == false) return;
	
	map<MUID, MTD_BlitzFriendNode> BlitzFriends;
	
	// from clan members.
	MMatchClan *pClan = g_ClanMgr.Get(pObj->m_Clan.nCLID);
	if(pClan != NULL)
	{
		for(map<MUID, MMatchObject *>::iterator i = pClan->Begin(); i != pClan->End(); i++)
		{
			MMatchObject *pCurr = (*i).second;
			if(pCurr == pObj) continue;
			
			if(pCurr->m_bCharInfoExist == false) continue;
			
			// if(pCurr->m_uidChannel != pObj->m_uidChannel) continue;
			// if(pCurr->m_uidStage != MUID(0, 0)) continue;
			
			// (channel type == blitz) check. ----------
			if(pCurr->m_uidChannel == MUID(0, 0)) continue;
			
			MMatchChannel *pCurrChannel = g_ChannelMgr.Get(pCurr->m_uidChannel);
			if(pCurrChannel == NULL) continue;
			
			if(pCurrChannel->IsBlitzKrieg() == false) continue;
			// --------------------
			
			if(pCurr->m_nPlace != MMP_LOBBY) continue;
			
			if(pCurr->m_nBlitzGroupID != 0) continue;
			
			// check friend already exists in map.
			if(BlitzFriends.find(pCurr->GetUID()) != BlitzFriends.end()) continue;
			
			// add friend data.
			MTD_BlitzFriendNode node;
			ZeroInit(&node, sizeof(MTD_BlitzFriendNode));
			
			node.uidPlayer = pCurr->GetUID();
			strcpy(node.szPlayerName, pCurr->m_Char.szName);
			
			BlitzFriends.insert(pair<MUID, MTD_BlitzFriendNode>(node.uidPlayer, node));
		}
	}
	
	// from friends.
	for(list<MMatchObject *>::iterator i = g_ObjectMgr.Begin(); i != g_ObjectMgr.End(); i++)
	{
		MMatchObject *pCurrObj = (*i);
		if(pCurrObj == pObj) continue;
		
		if(pCurrObj->m_bCharInfoExist == false) continue;
		
		// if(pCurrObj->m_uidChannel != pObj->m_uidChannel) continue;
		// if(pCurrObj->m_uidStage != MUID(0, 0)) continue;
		
		// (channel type == blitz) check. ----------
		if(pCurrObj->m_uidChannel == MUID(0, 0)) continue;
			
		MMatchChannel *pCurrChannel = g_ChannelMgr.Get(pCurrObj->m_uidChannel);
		if(pCurrChannel == NULL) continue;
			
		if(pCurrChannel->IsBlitzKrieg() == false) continue;
		// --------------------
		
		if(pCurrObj->m_nPlace != MMP_LOBBY) continue;
		
		if(pCurrObj->m_nBlitzGroupID != 0) continue;
		
		for(list<MMatchFriend *>::iterator j = pObj->m_FriendList.begin(); j != pObj->m_FriendList.end(); j++)
		{
			MMatchFriend *pCurrFriend = (*j);
			
			if(pCurrFriend->nCID == pCurrObj->m_Char.nCID)
			{
				// check friend already exists in map.
				if(BlitzFriends.find(pCurrObj->GetUID()) != BlitzFriends.end()) break;
			
				// add friend data.
				MTD_BlitzFriendNode node;
				ZeroInit(&node, sizeof(MTD_BlitzFriendNode));
			
				node.uidPlayer = pCurrObj->GetUID();
				strcpy(node.szPlayerName, pCurrObj->m_Char.szName);
			
				BlitzFriends.insert(pair<MUID, MTD_BlitzFriendNode>(node.uidPlayer, node));
			}
		}
	}
	
	MCmdWriter Cmd;
	Cmd.StartBlob(sizeof(MTD_BlitzFriendNode));
	for(map<MUID, MTD_BlitzFriendNode>::iterator i = BlitzFriends.begin(); i != BlitzFriends.end(); i++)
	{
		Cmd.WriteData(&(*i).second, sizeof(MTD_BlitzFriendNode));
	}
	Cmd.EndBlob();
	Cmd.Finalize(MC_BLITZ_RESPONSE_MATCH_COLLABORATOR_LIST, MCFT_END);
	
	SendToClient(&Cmd, uidPlayer);
}

void OnBlitzReserveChallenge(const MUID &uidPlayer, vector<MTD_BlitzChallengerNode> *pChallengers)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	// if(pObj->m_uidChannel == MUID(0, 0)) return;
	// if(pObj->m_uidStage != MUID(0, 0)) return;
	
	if(pObj->m_nPlace != MMP_LOBBY) return;
	
	if(pObj->m_nBlitzGroupID != 0) return;
	
	MMatchChannel *pChannel = g_ChannelMgr.Get(pObj->m_uidChannel);
	if(pChannel == NULL) return;
	
	if(pChannel->IsBlitzKrieg() == false) return;
	
	MMatchBlitzKriegGroup group;
	
	group.AddObject(pObj);
	
	for(vector<MTD_BlitzChallengerNode>::iterator i = pChallengers->begin(); i != pChallengers->end(); i++)
	{
		MTD_BlitzChallengerNode *pCurr = &(*i);
		
		MMatchObject *pTargetObj = g_ObjectMgr.Get(pCurr->szPlayerName);
		if(pTargetObj == NULL) return;
		
		if(pObj == pTargetObj) continue;
		
		if(pTargetObj->m_bCharInfoExist == false) return;
		
		// if(pTargetObj->m_uidChannel != pObj->m_uidChannel) return;
		// if(pTargetObj->m_uidStage != MUID(0, 0)) return;
		
		// (channel type == blitz) check. ----------
		if(pTargetObj->m_uidChannel == MUID(0, 0)) continue;
			
		MMatchChannel *pCurrChannel = g_ChannelMgr.Get(pTargetObj->m_uidChannel);
		if(pCurrChannel == NULL) continue;
			
		if(pCurrChannel->IsBlitzKrieg() == false) continue;
		// --------------------
		
		if(pTargetObj->m_nPlace != MMP_LOBBY) return;
		
		if(pTargetObj->m_nBlitzGroupID != 0) return;
		
		group.AddObject(pTargetObj);
	}
	
	MMatchBlitzKriegGroup *pNew = new MMatchBlitzKriegGroup(false);
	*pNew = group;
	
	g_BKQMgr.Add(pNew);
}

void OnBlitzCancelChallenge(const MUID &uidPlayer)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	if(pObj->m_nBlitzGroupID == 0) return;
	
	g_BKQMgr.Remove(pObj->m_nBlitzGroupID, pObj->m_Char.szName);
}

void OnBlitzChooseClass(const MUID &uidPlayer, unsigned int nClass)
{
	if(nClass >= MBLITZ_CLASS_MAX) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	if(pObj->m_uidStage == MUID(0, 0)) return;
	if(pObj->CheckGameFlag(MMOGF_ENTERED) == false) return;
	
	MMatchStage *pStage = g_StageMgr.Get(pObj->m_uidStage);
	if(pStage == NULL) return;
	
	if(pStage->GetGameType() != (int)MMGT_BLITZKRIEG) return;
	
	if(pStage->CheckGameInfoExists() == false) return;
	
	MMatchGame_BlitzKrieg *pGame = (MMatchGame_BlitzKrieg *)pStage->GetGame();
	if(pGame->GetBlitzRoundState() != (int)MMBRS_PREPARE) return;
	
	if(g_nBlitzManualItemID[nClass] != 0 && pObj->GetCharItemByItemID(g_nBlitzManualItemID[nClass]) == NULL) 
		return;
		
	MBlitzPlayerStatus *pPlayerStatus = pGame->GetPlayerStatus(uidPlayer);
	if(pPlayerStatus == NULL) return;
	
	pPlayerStatus->m_nClass = (int)nClass;
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidPlayer);
	Cmd.WriteUInt(nClass);
	Cmd.Finalize(MC_BLITZ_CHOOSE_CLASS, MCFT_END);
	SendToBattle(&Cmd, pObj->m_uidStage);
}

void OnBlitzNPCDead(const MUID &uidPlayer, const MUID &uidKiller, const MUID &uidNPC)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	if(pObj->m_uidStage == MUID(0, 0)) return;
	if(pObj->CheckGameFlag(MMOGF_ENTERED) == false) return;
	
	MMatchStage *pStage = g_StageMgr.Get(pObj->m_uidStage);
	if(pStage == NULL) return;
	
	if(pStage->GetGameType() != (int)MMGT_BLITZKRIEG) return;

	if(pStage->CheckGameInfoExists() == false) return;
	
	MMatchGame_BlitzKrieg *pGame = (MMatchGame_BlitzKrieg *)pStage->GetGame();
	
	if(pGame->IsValidActorOwner(uidPlayer, uidNPC) == false) return;
	if(pGame->ActorDead(uidKiller, uidNPC) == false) return;
}

void OnBlitzPlayerDead(const MUID &uidAttacker, const MUID &uidVictim, vector<MTD_BlitzAssisterInfoNode> *pAssisters)
{
	MMatchObject *pVictimObj = g_ObjectMgr.Get(uidVictim);
	if(pVictimObj == NULL) return;
	
	if(pVictimObj->m_uidStage == MUID(0, 0)) return;
	if(pVictimObj->CheckGameFlag(MMOGF_ENTERED) == false) return;
	
	if(pVictimObj->m_GameInfo.bAlive == false) return;
	
	MMatchStage *pStage = g_StageMgr.Get(pVictimObj->m_uidStage);
	if(pStage == NULL) return;
	
	if(pStage->GetGameType() != (int)MMGT_BLITZKRIEG) return;
	
	if(pStage->CheckGameInfoExists() == false) return;
	
	MMatchGame_BlitzKrieg *pGame = (MMatchGame_BlitzKrieg *)pStage->GetGame();
	unsigned long nElapsedTime = pGame->GetElapsedTime();
	
	unsigned int nKillPoint = 0, nAssistPoint = 0;
	unsigned int nNextSpawnTime = (unsigned int)((nElapsedTime / 60) + 7000);
	
	MMatchObject *pAttackerObj = g_ObjectMgr.Get(uidAttacker);
	if(pAttackerObj == NULL)
	{
		// attacker is npc, but do nothing.
	}
	else
	{
		if(pAttackerObj->m_uidStage != pVictimObj->m_uidStage) return;
		if(pAttackerObj->CheckGameFlag(MMOGF_ENTERED) == false) return;
	
		bool bSuicided = uidAttacker == uidVictim ? true : false ;
		
		if(bSuicided == false) 
		{
			pAttackerObj->m_GameInfo.nKill++;
			
			nKillPoint = (unsigned int)((pVictimObj->m_GameInfo.nKill + 1) * 50);
			nAssistPoint = nKillPoint / 4;
			
			if(pAttackerObj->m_GameInfo.nTeam == pVictimObj->m_GameInfo.nTeam)
			{
				nKillPoint = 0;
				nAssistPoint = 0;
			}
		}
		else 
		{
			nNextSpawnTime += (nNextSpawnTime / 2);
		}
	}
	
	pVictimObj->m_GameInfo.bAlive = false;
	pVictimObj->m_GameInfo.nDeath++;
	pVictimObj->UpdateSpawnTime();	// spawn timer update for next spawn.
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidVictim);
	Cmd.WriteMUID(uidAttacker);
	Cmd.StartBlob(sizeof(MTD_BlitzAssisterInfoNode));
	if(pAssisters != NULL)
	{
		for(vector<MTD_BlitzAssisterInfoNode>::iterator i = pAssisters->begin(); i != pAssisters->end(); i++)
		{
			MTD_BlitzAssisterInfoNode node = (*i);
			node.nGivenHonor = nAssistPoint;
		
			Cmd.WriteData(&node, sizeof(MTD_BlitzAssisterInfoNode));
		}
	}
	Cmd.EndBlob();
	Cmd.WriteUInt(nKillPoint);
	Cmd.WriteUInt(nNextSpawnTime);
	Cmd.WriteBool(false);
	Cmd.Finalize(MC_BLITZ_PLAYER_DEAD, MCFT_END);
	SendToBattle(&Cmd, pStage->GetUID());
	
	// to attacker.
	MBlitzPlayerStatus *pAttackerStatus = pGame->GetPlayerStatus(uidAttacker);
	if(pAttackerStatus != NULL)
	{
		pAttackerStatus->AddHonorPoint((int)nKillPoint);
	}
	
	if(pAssisters != NULL)
	{
		// to assister.
		for(vector<MTD_BlitzAssisterInfoNode>::iterator i = pAssisters->begin(); i != pAssisters->end(); i++)
		{
			MTD_BlitzAssisterInfoNode *pInfo = &(*i);
		
			MBlitzPlayerStatus *pAssisterStatus = pGame->GetPlayerStatus(pInfo->uidAssister);
			if(pAssisterStatus != NULL)
			{
				if(pAssisterStatus->m_nTeamID == pVictimObj->m_GameInfo.nTeam) continue;
				pAssisterStatus->AddHonorPoint((int)nAssistPoint);
			}
		}
	}
}

void OnBlitzSkillUpgrade(const MUID &uidPlayer, int nSkillType)
{
	if(nSkillType < 0 || nSkillType >= MBLITZ_SKILLTYPE_MAX) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	if(pObj->m_uidStage == MUID(0, 0)) return;
	if(pObj->CheckGameFlag(MMOGF_ENTERED) == false) return;
	
	MMatchStage *pStage = g_StageMgr.Get(pObj->m_uidStage);
	if(pStage == NULL) return;
	
	if(pStage->GetGameType() != (int)MMGT_BLITZKRIEG) return;
	
	if(pStage->CheckGameInfoExists() == false) return;
	
	MMatchGame_BlitzKrieg *pGame = (MMatchGame_BlitzKrieg *)pStage->GetGame();
	
	MBlitzPlayerStatus *pPlayerStatus = pGame->GetPlayerStatus(uidPlayer);
	if(pPlayerStatus == NULL) return;
	
	if(pPlayerStatus->m_nSkillGrade[nSkillType] >= MBLITZ_SKILLGRADE_MAX) return;
	
	static const int nExpendPoint[MBLITZ_SKILLGRADE_MAX] = 
		{250, 325, 400, 1200};
		
	if(pPlayerStatus->UseHonorPoint(nExpendPoint[pPlayerStatus->m_nSkillGrade[nSkillType]]) == false) return;
		
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidPlayer);
	Cmd.WriteInt(nSkillType);
	Cmd.WriteUInt((const unsigned int)nExpendPoint[pPlayerStatus->m_nSkillGrade[nSkillType]]);
	Cmd.Finalize(MC_BLITZ_PLAYER_SKILL_UPGRADE, MCFT_END);
	SendToBattle(&Cmd, pStage->GetUID());
	
	pPlayerStatus->m_nSkillGrade[nSkillType]++;
}