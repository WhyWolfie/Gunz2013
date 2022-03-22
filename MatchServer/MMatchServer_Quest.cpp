#include "pch.h"

#include "MMatchObject.h"

#include "MMatchStage.h"
#include "MMatchGame.h"

#include "MMatchQuest.h"

#include "MMatchMap.h"

#include "MMatchItem.h"
#include "MMatchConstant.h"

#include "MMatchDBMgr.h"
#include "MAsyncDBProcess.h"

#include "MMessageID.h"

void BuildQuestStageInfoCommand(int nScenarioID, MCmdWriter *pOut)
{
	int nQL = 0, nMapsetID = 0;
	
	if(nScenarioID != 0)
	{
		MQuestScenario *pScenario = g_Quest.GetScenario(nScenarioID);
		if(pScenario != NULL)
		{
			nQL = pScenario->nQL;
			
			MMatchMap *pMap = g_MapMgr.GetMapFromName(pScenario->szMapName, MMMST_QUEST);
			if(pMap != NULL)
			{
				nMapsetID = pMap->nIndex;
			}
		}
	}
	
	pOut->WriteChar((char)nQL);
	pOut->WriteChar((char)nMapsetID);
	pOut->WriteUInt((unsigned int)nScenarioID);
	pOut->Finalize(MC_QUEST_STAGE_GAME_INFO, MCFT_END);
}

void SendQuestStageInfoToStage(const MUID &uidStage, int nScenarioID)
{
	MCmdWriter Cmd;
	BuildQuestStageInfoCommand(nScenarioID, &Cmd);
	SendToStage(&Cmd, uidStage, true);
}

void SendQuestStageInfoToClient(const MUID &uidPlayer, int nScenarioID)
{
	MCmdWriter Cmd;
	BuildQuestStageInfoCommand(nScenarioID, &Cmd);
	SendToClient(&Cmd, uidPlayer);
}

void OnCharQuestItemList(const MUID &uidPlayer)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	MCmdWriter Cmd;
	
	Cmd.StartBlob(sizeof(MTD_QuestItemNode));
	for(list<MMatchQuestItem>::iterator i = pObj->m_QuestItemList.begin(); i != pObj->m_QuestItemList.end(); i++)
	{
		MMatchQuestItem *pCurr = &(*i);
		
		MTD_QuestItemNode node;
		node.nItemID = pCurr->nItemID;
		node.nCount = pCurr->nItemCount;
		
		Cmd.WriteData(&node, sizeof(MTD_QuestItemNode));
	}
	Cmd.EndBlob();
	
	Cmd.Finalize(MC_MATCH_RESPONSE_CHAR_QUEST_ITEMLIST, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
}

void OnBuyQuestItem(const MUID &uidPlayer, int nItemID, int nCount)
{
	if(nCount <= 0 || nCount > MAX_QUEST_ITEM_COUNT) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	if(g_ShopMgr.IsSellingItem(nItemID) == false) return;
	
	MMatchItem *pItem = g_ItemMgr.Get(nItemID);
	if(pItem == NULL) return;
	
	if(pItem->GetSlotType() != (int)MMIST_QUEST) return;
	
	MCmdWriter Cmd;
	
	int nCurrBP = pObj->m_Exp.nBP;
	
	if(pItem->IsEnoughBP(nCurrBP, nCount) == false)
	{
		Cmd.WriteInt(MERR_LACKING_BOUNTY);
		Cmd.WriteInt(nCurrBP);
		Cmd.Finalize(MC_MATCH_RESPONSE_BUY_QUEST_ITEM, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	if(pObj->AddQuestItem(nItemID, nCount) == false)
	{
		Cmd.WriteInt(MERR_TOO_MANY_ITEM);
		Cmd.WriteInt(nCurrBP);
		Cmd.Finalize(MC_MATCH_RESPONSE_BUY_QUEST_ITEM, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	nCurrBP -= pItem->GetPrice() * nCount;

	/*
	if(Db_UpdateBounty(pObj->m_Char.nCID, nCurrBP) == false)
	{
		// db error.
		return;
	}
	*/
	AsyncDb_UpdateBounty(uidPlayer, pObj->m_Char.nCID, nCurrBP);
	
	pObj->m_Exp.nBP = nCurrBP;
	
	Cmd.WriteInt(MSG_OK);
	Cmd.WriteInt(nCurrBP);
	Cmd.Finalize(MC_MATCH_RESPONSE_BUY_QUEST_ITEM, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
	
	OnCharQuestItemList(uidPlayer);
}

void OnSellQuestItem(const MUID &uidPlayer, int nItemID, int nCount)
{
	if(nCount <= 0 || nCount > MAX_QUEST_ITEM_COUNT) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	MMatchItem *pItem = g_ItemMgr.Get(nItemID);
	if(pItem == NULL) return;
	
	if(pItem->GetSlotType() != (int)MMIST_QUEST) return;
	
	MCmdWriter Cmd;
	
	int nCurrBP = pObj->m_Exp.nBP;
	
	if(pObj->SubQuestItem(nItemID, nCount) == false)
	{
		Cmd.WriteInt(MERR_CANNOT_SELL_ITEM);
		Cmd.WriteInt(nCurrBP);
		Cmd.Finalize(MC_MATCH_RESPONSE_SELL_QUEST_ITEM, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	nCurrBP += (pItem->GetPrice() / 4) * nCount;
	
	/*
	if(Db_UpdateBounty(pObj->m_Char.nCID, nCurrBP) == false)
	{
		// db error.
		return;
	}
	*/
	AsyncDb_UpdateBounty(uidPlayer, pObj->m_Char.nCID, nCurrBP);
	
	pObj->m_Exp.nBP = nCurrBP;
	
	Cmd.WriteInt(MSG_OK);
	Cmd.WriteInt(nCurrBP);
	Cmd.Finalize(MC_MATCH_RESPONSE_SELL_QUEST_ITEM, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
	
	OnCharQuestItemList(uidPlayer);
}

void OnQuestSlotInfo(const MUID &uidPlayer)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	if(pObj->m_uidStage == MUID(0, 0)) return;
	
	MMatchStage *pStage = g_StageMgr.Get(pObj->m_uidStage);
	if(pStage == NULL) return;
	
	if(pStage->GetGameType() != (int)MMGT_QUEST) return;
	
	MSacriItemSlot *pSlot1 = pStage->GetQuestSlotItem(0);
	if(pSlot1 == NULL) return;
	
	MSacriItemSlot *pSlot2 = pStage->GetQuestSlotItem(1);
	if(pSlot2 == NULL) return;
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(pSlot1->uidOwner);
	Cmd.WriteInt(pSlot1->nItemID);
	Cmd.WriteMUID(pSlot2->uidOwner);
	Cmd.WriteInt(pSlot2->nItemID);
	Cmd.Finalize(MC_MATCH_RESPONSE_SLOT_INFO, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
}

void OnQuestLevel(const MUID &uidPlayer)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	if(pObj->m_uidStage == MUID(0, 0)) return;
	
	MMatchStage *pStage = g_StageMgr.Get(pObj->m_uidStage);
	if(pStage == NULL) return;
	
	if(pStage->GetGameType() != (int)MMGT_QUEST) return;
	
	MQuestScenario *pScenario = g_Quest.GetScenario(pStage->GetQuestScenarioID());
	if(pScenario == NULL) return;
	
	MCmdWriter Cmd;
	Cmd.WriteInt(pScenario->nQL);
	Cmd.Finalize(MC_QUEST_RESPONSE_LEVEL, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
}

void OnQuestDropSacrificeItem(const MUID &uidPlayer, int nSlotIndex, int nItemID)
{
	if(nSlotIndex < 0 || nSlotIndex >= SACRIITEM_SLOT_COUNT) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	if(pObj->m_uidStage == MUID(0, 0)) return;
	
	MMatchStage *pStage = g_StageMgr.Get(pObj->m_uidStage);
	if(pStage == NULL) return;
	
	if(pStage->GetGameType() != (int)MMGT_QUEST) return;
	
	// check the char has quest item.
	if(pObj->GetQuestItem(nItemID) == NULL) return;
	
	pStage->SetQuestSlotItem(uidPlayer, nItemID, nSlotIndex);
	
	MCmdWriter Cmd;
	Cmd.WriteInt(0);	// ok.
	Cmd.WriteMUID(uidPlayer);
	Cmd.WriteInt(nSlotIndex);
	Cmd.WriteInt(nItemID);
	Cmd.Finalize(MC_MATCH_RESPONSE_DROP_SACRIFICE_ITEM, MCFT_END);
	SendToStage(&Cmd, pObj->m_uidStage);
	
	SendQuestStageInfoToStage(pObj->m_uidStage, pStage->GetQuestScenarioID());
}

// param 3 : nItemID is not used.
void OnQuestCallbackSacrificeItem(const MUID &uidPlayer, int nSlotIndex, int nItemID)
{
	if(nSlotIndex < 0 || nSlotIndex >= SACRIITEM_SLOT_COUNT) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	if(pObj->m_uidStage == MUID(0, 0)) return;
	
	MMatchStage *pStage = g_StageMgr.Get(pObj->m_uidStage);
	if(pStage == NULL) return;
	
	if(pStage->GetGameType() != (int)MMGT_QUEST) return;
	
	pStage->SetQuestSlotItem(MUID(0, 0), 0, nSlotIndex);
	
	MCmdWriter Cmd;
	Cmd.WriteInt(0);	// ok.
	Cmd.WriteMUID(uidPlayer);
	Cmd.WriteInt(nSlotIndex);
	Cmd.WriteInt(nItemID);
	Cmd.Finalize(MC_MATCH_RESPONSE_CALLBACK_SACRIFICE_ITEM, MCFT_END);
	SendToStage(&Cmd, pObj->m_uidStage);
	
	SendQuestStageInfoToStage(pObj->m_uidStage, pStage->GetQuestScenarioID());
}

void OnQuestPlayerDead(const MUID &uidPlayer)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	if(pObj->m_uidStage == MUID(0, 0)) return;
	if(pObj->CheckGameFlag(MMOGF_ENTERED) == false) return;
	
	MMatchStage *pStage = g_StageMgr.Get(pObj->m_uidStage);
	if(pStage == NULL) return;
	
	if(IsQuestDerived(pStage->GetGameType()) == false) return;
	
	if(pStage->CheckGameInfoExists() == false) return;
	
	pObj->m_GameInfo.bAlive = false;
	
	// force sector ready.
	MMatchGame_Quest *pGame = (MMatchGame_Quest *)pStage->GetGame();
	pGame->SectorPlayerReady(uidPlayer);
		
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidPlayer);
	Cmd.Finalize(MC_MATCH_QUEST_PLAYER_DEAD, MCFT_END);
	SendToBattle(&Cmd, pObj->m_uidStage);
}

void OnQuestNPCDead(const MUID &uidPlayer, const MUID &uidKiller, const MUID &uidNPC, const ShortVector *pPos)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	if(pObj->m_uidStage == MUID(0, 0)) return;
	if(pObj->CheckGameFlag(MMOGF_ENTERED) == false) return;
	
	MMatchStage *pStage = g_StageMgr.Get(pObj->m_uidStage);
	if(pStage == NULL) return;
	
	if(IsQuestDerived(pStage->GetGameType()) == false) return;

	if(pStage->CheckGameInfoExists() == false) return;
	
	MMatchGame_Quest *pGame = (MMatchGame_Quest *)pStage->GetGame();
		
	if(pGame->IsValidNPCOwner(uidPlayer, uidNPC) == false) return;
	if(pGame->NPCDead(uidKiller, uidNPC) == false) return;
	
	/*	
	int nWorldItemID = 0;
	switch(RandNum() % 4)
	{
		case 0	: nWorldItemID = 1; break;	// hp01.
		case 1	: nWorldItemID = 4; break;	// ap01.
		case 2	: nWorldItemID = 7; break;	// bullet01.
		default	:
		case 3	: break;					// nothing.
	}
	
	if(nWorldItemID != 0)
	{
		pStage->GetGame()->AddWorldItem(nWorldItemID, (float)pPos->x, (float)pPos->y, (float)pPos->z, (int)MTD_WorldItemSubType_Dynamic);
	}
	*/
	
	int nWorldItemID = pGame->GetNPCDropWorldItemID(uidNPC);
	
	if(nWorldItemID == -1)	// == not found, overwrite with default dropset.
	{
		switch(RandNum() % 4)
		{
			case 0	: nWorldItemID = 1; break;	// hp01.
			case 1	: nWorldItemID = 4; break;	// ap01.
			case 2	: nWorldItemID = 7; break;	// bullet01.
			default	:
			case 3	: nWorldItemID = 0; break;	// nothing.
		}
	}
	
	if(nWorldItemID != 0)
	{
		unsigned short nWItemUID = pGame->AddWorldItem(nWorldItemID, (float)pPos->x, (float)pPos->y, (float)pPos->z, (int)MTD_WorldItemSubType_Dynamic);
		pGame->SetNPCDropWorldItemUID(uidNPC, nWItemUID);
	}
}

// param 2 : nCurrSectorIndex is not used.
void OnMoveToPortal(const MUID &uidPlayer, int nCurrSectorIndex)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	if(pObj->m_uidStage == MUID(0, 0)) return;
	if(pObj->CheckGameFlag(MMOGF_ENTERED) == false) return;
	
	MMatchStage *pStage = g_StageMgr.Get(pObj->m_uidStage);
	if(pStage == NULL) return;
	
	if(IsQuestDerived(pStage->GetGameType()) == false) return;
	
	if(pStage->CheckGameInfoExists() == false) return;
	
	int nSectorIndex = 0;
	int nRepeatIndex = 0;
	
	MMatchGame_Quest *pGame = (MMatchGame_Quest *)pStage->GetGame();
		
	nSectorIndex = pGame->GetSectorIndex();
	nRepeatIndex = pGame->GetRepeatIndex();
		
	MCmdWriter Cmd;
	Cmd.WriteChar((char)nSectorIndex);
	Cmd.WriteUChar((unsigned char)nRepeatIndex);
	Cmd.WriteMUID(uidPlayer);
	Cmd.Finalize(MC_QUEST_MOVETO_PORTAL, MCFT_END);
	SendToBattle(&Cmd, pObj->m_uidStage);
}

void OnReadyToNewSector(const MUID &uidPlayer)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	if(pObj->m_uidStage == MUID(0, 0)) return;
	if(pObj->CheckGameFlag(MMOGF_ENTERED) == false) return;
	
	MMatchStage *pStage = g_StageMgr.Get(pObj->m_uidStage);
	if(pStage == NULL) return;
	
	if(IsQuestDerived(pStage->GetGameType()) == false) return;
	
	if(pStage->CheckGameInfoExists() == false) return;
	
	MMatchGame_Quest *pGame = (MMatchGame_Quest *)pStage->GetGame();
	pGame->SectorPlayerReady(uidPlayer);
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidPlayer);
	Cmd.Finalize(MC_QUEST_READYTO_NEWSECTOR, MCFT_END);
	SendToBattle(&Cmd, pObj->m_uidStage);
}