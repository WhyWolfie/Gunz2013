#include "pch.h"

#include "MMatchObject.h"
#include "MMatchItem.h"

#include "MMessageID.h"

#include "MAsyncDBProcess.h"
#include "MMatchDBMgr.h"

#include "MMatchServer_OnCommand.h"

#include "MMatchStage.h"

#include "MMatchGambleItem.h"

#include "MMatchBlitzKrieg.h"

#include "MMatchCashShop.h"

#define MAX_CHAR_ITEM_COUNT			200
#define MAX_CHAR_GAMBLEITEM_COUNT	1000
#define MAX_CHAR_SPENDITEM_COUNT	30000 //999

bool IsMaxCharItem(int n)
{
	return n > MAX_CHAR_ITEM_COUNT ? true : false ;
}

bool IsMaxCharGambleItem(int n)
{
	return n > MAX_CHAR_GAMBLEITEM_COUNT ? true : false ;
}

bool IsMaxCharSpendItem(int n)
{
	return n > MAX_CHAR_SPENDITEM_COUNT ? true : false ;
}

void SendStageCharEquipItemUpdate(const MUID &uidPlayer, int nParts, int nItemID, const MUID &uidStage)
{
	if(uidStage == MUID(0, 0)) return;
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidPlayer);
	Cmd.WriteInt(nParts);
	Cmd.WriteInt(nItemID);
	Cmd.Finalize(MC_MATCH_STAGE_UPDATE_CHARACTER_EQUIPITEM, MCFT_END);
	
	SendToStage(&Cmd, uidStage);
}

void OnShopItemList(const MUID &uidPlayer)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	// g_GItemMgr.SendInfo(uidPlayer);
	
	MCmdWriter Cmd(8192);	// shop item list may be big. set default buff size to 8192.
	
	Cmd.StartBlob(sizeof(MTD_GambleItemNode));
	for(vector<MMatchGambleItem *>::iterator i = g_GItemMgr.Begin(); i != g_GItemMgr.End(); i++)
	{
		MMatchGambleItem *pCurr = (*i);
		if(pCurr->IsSelling() == false) continue;
		
		MTD_GambleItemNode node;
		ZeroInit(&node, sizeof(MTD_GambleItemNode));
		
		node.uidItem = MUID(0, 0);	// used for char gitem list only.
		node.nItemID = pCurr->GetID();
		node.nItemCnt = 1;
		
		Cmd.WriteData(&node, sizeof(MTD_GambleItemNode));
	}
	Cmd.EndBlob();
	
	Cmd.StartBlob(sizeof(MTD_ShopItemInfo));
	for(vector<MShopItem *>::iterator i = g_ShopMgr.Begin(); i != g_ShopMgr.End(); i++)
	{
		MShopItem *pShopItem = (*i);
		
		MTD_ShopItemInfo info;
		info.nItemID = (unsigned int)pShopItem->nID;
		info.nItemCount = pShopItem->nCount;
		
		Cmd.WriteData(&info, sizeof(MTD_ShopItemInfo));
	}
	Cmd.EndBlob();
	
	Cmd.Finalize(MC_MATCH_RESPONSE_SHOP_ITEMLIST, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
	
	g_BlitzShop.SendItemList(uidPlayer, pObj->m_Char.nCID);
}

// Buy function separated. -----------------------------------
void OnBuyNormalItem(const MUID &uidPlayer, int nItemID, int nItemCount)
{
	if(nItemCount <= 0 || nItemCount > MAX_CHAR_SPENDITEM_COUNT) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	if(g_ShopMgr.IsSellingItem(nItemID) == false) return;
	
	MMatchItem *pItem = g_ItemMgr.Get(nItemID);
	if(pItem == NULL) return;
	
	MCmdWriter Cmd;
	
	if(pItem->IsSpendable() == false && nItemCount != 1)
	{
		Cmd.WriteInt(MERR_CANNOT_BUY_ITEM);
		Cmd.Finalize(MC_MATCH_RESPONSE_BUY_ITEM, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	if(pItem->IsBounty() == false)
	{
		Cmd.WriteInt(MERR_CANNOT_BUY_ITEM);
		Cmd.Finalize(MC_MATCH_RESPONSE_BUY_ITEM, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	int nCurrBP = pObj->m_Exp.nBP;
	
	if(pItem->IsEnoughBP(nCurrBP, nItemCount) == false)
	{
		Cmd.WriteInt(MERR_LACKING_BOUNTY);
		Cmd.Finalize(MC_MATCH_RESPONSE_BUY_ITEM, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	MMatchCharItem *pCurrSpendItem = pObj->GetCharItemByItemID(nItemID);
	bool bCharSpendItemExists = (pItem->IsSpendable() == false || pCurrSpendItem == NULL) ? false : true;
	
	if(bCharSpendItemExists == true)
	{
		if(IsMaxCharSpendItem(pCurrSpendItem->nCount + nItemCount) == true)
		{
			Cmd.WriteInt(MERR_TOO_MANY_ITEM);
			Cmd.Finalize(MC_MATCH_RESPONSE_BUY_ITEM, MCFT_END);
			SendToClient(&Cmd, uidPlayer);
			return;
		}
	}
	else
	{
		if(IsMaxCharItem((int)(pObj->m_ItemList.size() + 1)) == true)
		{
			Cmd.WriteInt(MERR_TOO_MANY_ITEM);
			Cmd.Finalize(MC_MATCH_RESPONSE_BUY_ITEM, MCFT_END);
			SendToClient(&Cmd, uidPlayer);
			return;
		}
	}
	
	nCurrBP -= pItem->GetPrice() * nItemCount;
	
	if(bCharSpendItemExists == false)
	{
		/*
		bool bResult = false;
		
		int nCIID;
		if(pItem->GetRentPeriod() == 0)
		{
			bResult = Db_InsertItem(pObj->m_Char.nCID, nItemID, nItemCount, nCurrBP, &nCIID);
		}
		else
		{
			bResult = Db_InsertItem(pObj->m_Char.nCID, nItemID, nItemCount, pItem->GetRentPeriod(), nCurrBP, &nCIID);
		}
		
		if(bResult == false)
		{
			Cmd.WriteInt(MERR_CANNOT_BUY_ITEM);
			Cmd.Finalize(MC_MATCH_RESPONSE_BUY_ITEM, MCFT_END);
			SendToClient(&Cmd, uidPlayer);
			return;
		}
		
		pObj->AddCharItem(MUID::Assign(), nCIID, nItemID, nItemCount, pItem->GetRentPeriod() * 60 * 60);
		*/
		
		AsyncDb_InsertItem(uidPlayer, pObj->m_Char.nCID, nItemID, nItemCount, pItem->GetRentPeriod(), nCurrBP);
	}
	else
	{
		int nNewCount = pCurrSpendItem->nCount + nItemCount;
		
		/*
		if(Db_UpdateItem(pObj->m_Char.nCID, pCurrSpendItem->nCIID, nNewCount, nCurrBP) == false)
		{
			Cmd.WriteInt(MERR_CANNOT_BUY_ITEM);
			Cmd.Finalize(MC_MATCH_RESPONSE_BUY_ITEM, MCFT_END);
			SendToClient(&Cmd, uidPlayer);
			return;
		}
		*/
		AsyncDb_UpdateItem(uidPlayer, pObj->m_Char.nCID, pCurrSpendItem->nCIID, nNewCount, nCurrBP);
		
		pCurrSpendItem->nCount = nNewCount;
		
		// AsyncDb_UpdateItem() doesn't do auto-update item list info.
		OnCharacterItemList(uidPlayer);
	}
	
	pObj->m_Exp.nBP = nCurrBP;
	
	Cmd.WriteInt(MSG_OK);
	Cmd.Finalize(MC_MATCH_RESPONSE_BUY_ITEM, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
	
	// OnCharacterItemList(uidPlayer);
}

void OnBuyGambleItem(const MUID &uidPlayer, int nItemID, int nItemCount)
{
	if(nItemCount <= 0 || nItemCount > MAX_CHAR_GAMBLEITEM_COUNT) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	MMatchGambleItem *pItem = g_GItemMgr.Get(nItemID);
	if(pItem == NULL) return;
	
	if(pItem->IsSelling() == false) return;
	
	MCmdWriter Cmd;
	
	if(pItem->IsCash() == true)
	{
		Cmd.WriteInt(MERR_CANNOT_BUY_ITEM);
		Cmd.Finalize(MC_MATCH_RESPONSE_BUY_ITEM, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	int nCurrBP = pObj->m_Exp.nBP;
	
	if(pItem->IsEnoughBP(nCurrBP, nItemCount) == false)
	{
		Cmd.WriteInt(MERR_LACKING_BOUNTY);
		Cmd.Finalize(MC_MATCH_RESPONSE_BUY_ITEM, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	MMatchCharGambleItem *pCharItem = pObj->GetCharGambleItemByItemID(nItemID);
	bool bCharItemExists = pCharItem == NULL ? false : true ;
	
	int nNewCount = nItemCount;
	if(bCharItemExists == true) nNewCount += pCharItem->nCount;
	
	if(IsMaxCharGambleItem(nNewCount) == true)
	{
		Cmd.WriteInt(MERR_TOO_MANY_ITEM);
		Cmd.Finalize(MC_MATCH_RESPONSE_BUY_ITEM, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
		
	nCurrBP -= pItem->GetPrice() * nItemCount;
	
	if(bCharItemExists == false)
	{
		/*
		int nGIID;
		
		if(Db_InsertGambleItem(pObj->m_Char.nCID, nItemID, nNewCount, nCurrBP, &nGIID) == false)
		{
			Cmd.WriteInt(MERR_CANNOT_BUY_ITEM);
			Cmd.Finalize(MC_MATCH_RESPONSE_BUY_ITEM, MCFT_END);
			SendToClient(&Cmd, uidPlayer);
			return;
		}
		
		pObj->AddCharGambleItem(MUID::Assign(), nGIID, nItemID, nNewCount);
		*/
		
		AsyncDb_InsertGambleItem(uidPlayer, pObj->m_Char.nCID, nItemID, nNewCount, nCurrBP);
	}
	else
	{
		/*
		if(Db_UpdateGambleItem(pObj->m_Char.nCID, pCharItem->nGIID, nNewCount, nCurrBP) == false)
		{
			Cmd.WriteInt(MERR_CANNOT_BUY_ITEM);
			Cmd.Finalize(MC_MATCH_RESPONSE_BUY_ITEM, MCFT_END);
			SendToClient(&Cmd, uidPlayer);
			return;
		}
		*/
		AsyncDb_UpdateGambleItem(uidPlayer, pObj->m_Char.nCID, pCharItem->nGIID, nNewCount, nCurrBP);
		
		pCharItem->nCount = nNewCount;
		
		// AsyncDb_UpdateGambleItem() doesn't do auto-update item list info.
		OnCharacterItemList(uidPlayer);
	}
	
	pObj->m_Exp.nBP = nCurrBP;
	
	Cmd.WriteInt(MSG_OK);
	Cmd.Finalize(MC_MATCH_RESPONSE_BUY_ITEM, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
	
	// OnCharacterItemList(uidPlayer);
}

// param 3 : nItemCount is not used.
void OnBuyCashItem(const MUID &uidPlayer, int nShopItemID, int nItemCount, const char *pszGiftName, const char *pszGiftMsg, int nRentHourPeriod)
{
	// if(nItemCount <= 0 || nItemCount > MAX_CHAR_SPENDITEM_COUNT) return;
	
	if(nRentHourPeriod < 24 || nRentHourPeriod > 8760) return;	// available buy period is 1 day ~ 1 year.
	if((nRentHourPeriod % 24) != 0) return;	// must be day basis.
	
	int nRentDayPeriod = nRentHourPeriod / 24;
	bool bGift = pszGiftName[0] != '\0' ? true : false ;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	// do not gift to myself.
	if(MStricmp(pObj->m_Char.szName, pszGiftName) == 0) return;
	
	MMatchCashShopItem *pCashItem = g_CashShop.Get(nShopItemID);
	if(pCashItem == NULL) return;
	
	MMatchItem *pItem = g_ItemMgr.Get(pCashItem->m_nID);
	if(pItem == NULL) return;
	
	MCmdWriter Cmd;
	
	if(pItem->IsCash() == false)
	{
		Cmd.WriteInt(MERR_CANNOT_BUY_ITEM);
		Cmd.Finalize(MC_MATCH_RESPONSE_BUY_ITEM, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	int nPrice = pCashItem->m_nPrice * nRentDayPeriod;
	
	int nCurrCash = pObj->m_Account.nCash;
	
	if(nCurrCash < nPrice)
	{
		Cmd.WriteInt(MERR_LACKING_CASH);
		Cmd.Finalize(MC_MATCH_RESPONSE_BUY_ITEM, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	nCurrCash -= nPrice;
	
	// TODO : set item process.
	
	if(bGift == false)
	{
		AsyncDb_BuyCashItem(uidPlayer, pObj->m_Char.nCID, pCashItem->m_nID, pCashItem->m_nQuantity, nRentHourPeriod);
		
		pObj->m_Account.nCash = nCurrCash;
	
		Cmd.WriteInt(MSG_OK);
		Cmd.Finalize(MC_MATCH_RESPONSE_BUY_ITEM, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
	
		pObj->SyncCash(true);
	}
	else
	{
		int nSetItemID[5] = {pCashItem->m_nID, 0, 0, 0, 0};
		
		MAsyncDBTask_SendItemGift *pNew = new MAsyncDBTask_SendItemGift(uidPlayer, pszGiftName, pObj->m_Char.szName, pszGiftMsg, nSetItemID, nRentHourPeriod, pCashItem->m_nQuantity, pObj->m_Account.nAID, nCurrCash);
		g_AsyncDBTaskMgr.SafeAdd(pNew);
	}
	
	/*
	pObj->m_Account.nCash = nCurrCash;
	
	Cmd.WriteInt(MSG_OK);
	Cmd.Finalize(MC_MATCH_RESPONSE_BUY_ITEM, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
	
	pObj->SyncCash();
	*/
}

void OnBuyMedalItem(const MUID &uidPlayer, int nItemID, int nItemCount)
{
	if(nItemCount <= 0 || nItemCount > MAX_CHAR_SPENDITEM_COUNT) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	MBlitzShopSellingItem *pMedalItem = g_BlitzShop.GetSellingItem(nItemID, pObj->m_Char.nCID);
	if(pMedalItem == NULL) return;
	
	MMatchItem *pItem = g_ItemMgr.Get(nItemID);
	if(pItem == NULL) return;
	
	MCmdWriter Cmd;
	
	if(pItem->IsSpendable() == false && nItemCount != 1)
	{
		Cmd.WriteInt(MERR_CANNOT_BUY_ITEM);
		Cmd.Finalize(MC_MATCH_RESPONSE_BUY_ITEM, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	if(pItem->IsMedal() == false)
	{
		Cmd.WriteInt(MERR_CANNOT_BUY_ITEM);
		Cmd.Finalize(MC_MATCH_RESPONSE_BUY_ITEM, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	int nCurrMedal = pObj->m_BlitzKrieg.nMedal;
	
	if(pMedalItem->IsEnoughMedal(nCurrMedal, nItemCount) == false)
	{
		Cmd.WriteInt(MERR_LACKING_MEDAL);
		Cmd.Finalize(MC_MATCH_RESPONSE_BUY_ITEM, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	MMatchCharItem *pCurrSpendItem = pObj->GetCharItemByItemID(nItemID);
	bool bCharSpendItemExists = (pItem->IsSpendable() == false || pCurrSpendItem == NULL) ? false : true;
	
	if(bCharSpendItemExists == true)
	{
		if(IsMaxCharSpendItem(pCurrSpendItem->nCount + nItemCount) == true)
		{
			Cmd.WriteInt(MERR_TOO_MANY_ITEM);
			Cmd.Finalize(MC_MATCH_RESPONSE_BUY_ITEM, MCFT_END);
			SendToClient(&Cmd, uidPlayer);
			return;
		}
	}
	else
	{
		if(IsMaxCharItem((int)(pObj->m_ItemList.size() + 1)) == true)
		{
			Cmd.WriteInt(MERR_TOO_MANY_ITEM);
			Cmd.Finalize(MC_MATCH_RESPONSE_BUY_ITEM, MCFT_END);
			SendToClient(&Cmd, uidPlayer);
			return;
		}
	}
	
	nCurrMedal -= pMedalItem->m_nPrice * nItemCount;
	
	if(bCharSpendItemExists == false)
	{
		/*
		bool bResult = false;
		
		int nCIID;
		if(pMedalItem->m_nRentHourPeriod == 0)
		{
			bResult = Db_InsertItem(pObj->m_Char.nCID, nItemID, nItemCount, pObj->m_Exp.nBP, &nCIID);
		}
		else
		{
			bResult = Db_InsertItem(pObj->m_Char.nCID, nItemID, nItemCount, pMedalItem->m_nRentHourPeriod, pObj->m_Exp.nBP, &nCIID);
		}
		
		if(bResult == false)
		{
			Cmd.WriteInt(MERR_CANNOT_BUY_ITEM);
			Cmd.Finalize(MC_MATCH_RESPONSE_BUY_ITEM, MCFT_END);
			SendToClient(&Cmd, uidPlayer);
			return;
		}
		
		pObj->AddCharItem(MUID::Assign(), nCIID, nItemID, nItemCount, pMedalItem->m_nRentHourPeriod * 60 * 60);
		*/
		
		AsyncDb_InsertItem(uidPlayer, pObj->m_Char.nCID, nItemID, nItemCount, pMedalItem->m_nRentHourPeriod, pObj->m_Exp.nBP);
	}
	else
	{
		int nNewCount = pCurrSpendItem->nCount + nItemCount;
		
		/*
		if(Db_UpdateItem(pObj->m_Char.nCID, pCurrSpendItem->nCIID, nNewCount, pObj->m_Exp.nBP) == false)
		{
			Cmd.WriteInt(MERR_CANNOT_BUY_ITEM);
			Cmd.Finalize(MC_MATCH_RESPONSE_BUY_ITEM, MCFT_END);
			SendToClient(&Cmd, uidPlayer);
			return;
		}
		*/
		AsyncDb_UpdateItem(uidPlayer, pObj->m_Char.nCID, pCurrSpendItem->nCIID, nNewCount, pObj->m_Exp.nBP);
		
		pCurrSpendItem->nCount = nNewCount;
		
		// AsyncDb_UpdateItem() doesn't do auto-update item list info.
		OnCharacterItemList(uidPlayer);
	}
	
	pObj->m_BlitzKrieg.nMedal = nCurrMedal;
	
	Cmd.WriteInt(MSG_OK);
	Cmd.Finalize(MC_MATCH_RESPONSE_BUY_ITEM, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
	
	// OnCharacterItemList(uidPlayer);
	
	pObj->SyncMedal();
}

// 2011 version.
void OnBuyItem(const MUID &uidPlayer, int nItemID, int nItemCount)
{
	/*
	if(nItemCount <= 0 || nItemCount > MAX_CHAR_SPENDITEM_COUNT) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	if(g_ShopMgr.IsSellingItem(nItemID) == false) return;
	
	MMatchItem *pItem = g_ItemMgr.Get(nItemID);
	if(pItem == NULL) return;
	
	MCmdWriter Cmd;
	
	if(pItem->IsSpendable() == false && nItemCount != 1)
	{
		Cmd.WriteInt(MERR_CANNOT_BUY_ITEM);
		Cmd.Finalize(MC_MATCH_RESPONSE_BUY_ITEM, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	if(pItem->IsCash() == true)
	{
		Cmd.WriteInt(MERR_CANNOT_BUY_ITEM);
		Cmd.Finalize(MC_MATCH_RESPONSE_BUY_ITEM, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	int nCurrBP = pObj->m_Exp.nBP;
	
	if(pItem->IsEnoughBP(nCurrBP, nItemCount) == false)
	{
		Cmd.WriteInt(MERR_LACKING_BOUNTY);
		Cmd.Finalize(MC_MATCH_RESPONSE_BUY_ITEM, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	MMatchCharItem *pCurrSpendItem = pObj->GetCharItemByItemID(nItemID);
	bool bCharSpendItemExists = (pItem->IsSpendable() == false || pCurrSpendItem == NULL) ? false : true;
	
	if(bCharSpendItemExists == true)
	{
		if(IsMaxCharSpendItem(pCurrSpendItem->nCount + nItemCount) == true)
		{
			Cmd.WriteInt(MERR_TOO_MANY_ITEM);
			Cmd.Finalize(MC_MATCH_RESPONSE_BUY_ITEM, MCFT_END);
			SendToClient(&Cmd, uidPlayer);
			return;
		}
	}
	else
	{
		if(IsMaxCharItem((int)(pObj->m_ItemList.size() + 1)) == true)
		{
			Cmd.WriteInt(MERR_TOO_MANY_ITEM);
			Cmd.Finalize(MC_MATCH_RESPONSE_BUY_ITEM, MCFT_END);
			SendToClient(&Cmd, uidPlayer);
			return;
		}
	}
	
	nCurrBP -= pItem->GetPrice() * nItemCount;
	
	if(bCharSpendItemExists == false)
	{
		bool bResult = false;
		
		int nCIID;
		if(pItem->GetRentPeriod() == 0)
		{
			bResult = Db_InsertItem(pObj->m_Char.nCID, nItemID, nItemCount, nCurrBP, &nCIID);
		}
		else
		{
			bResult = Db_InsertItem(pObj->m_Char.nCID, nItemID, nItemCount, pItem->GetRentPeriod(), nCurrBP, &nCIID);
		}
		
		if(bResult == false)
		{
			Cmd.WriteInt(MERR_CANNOT_BUY_ITEM);
			Cmd.Finalize(MC_MATCH_RESPONSE_BUY_ITEM, MCFT_END);
			SendToClient(&Cmd, uidPlayer);
			return;
		}
		
		pObj->AddCharItem(MUID::Assign(), nCIID, nItemID, nItemCount, pItem->GetRentPeriod() * 60 * 60);
	}
	else
	{
		int nNewCount = pCurrSpendItem->nCount + nItemCount;
		
		if(Db_UpdateItem(pObj->m_Char.nCID, pCurrSpendItem->nCIID, nNewCount, nCurrBP) == false)
		{
			Cmd.WriteInt(MERR_CANNOT_BUY_ITEM);
			Cmd.Finalize(MC_MATCH_RESPONSE_BUY_ITEM, MCFT_END);
			SendToClient(&Cmd, uidPlayer);
			return;
		}
		
		pCurrSpendItem->nCount = nNewCount;
	}
	
	pObj->m_Exp.nBP = nCurrBP;
	
	Cmd.WriteInt(MSG_OK);
	Cmd.Finalize(MC_MATCH_RESPONSE_BUY_ITEM, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
	
	OnCharacterItemList(uidPlayer);
	*/
	
	/*
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	if(g_BlitzShop.GetSellingItem(nItemID, pObj->m_Char.nCID) != NULL)
	{
		OnBuyMedalItem(uidPlayer, nItemID, nItemCount);
	}
	else if(g_ItemMgr.Get(nItemID) != NULL)
	{
		OnBuyNormalItem(uidPlayer, nItemID, nItemCount);
	}
	else if(g_GItemMgr.Get(nItemID) != NULL)
	{
		OnBuyGambleItem(uidPlayer, nItemID, nItemCount);
	}
	*/
	
	OnBuyNormalItem(uidPlayer, nItemID, nItemCount);
}

// 2012 version.
void OnBuyItem(const MUID &uidPlayer, int nCurrencyType, int nItemID, int nItemCount, const char *pszGiftName, const char *pszGiftMsg, int nRentHourPeriod)
{
	if(nCurrencyType == ITEM_CURRENCY_BOUNTY)
	{
		if(g_GItemMgr.Get(nItemID) != NULL)
		{
			OnBuyGambleItem(uidPlayer, nItemID, nItemCount);
		}
		else
		{
			OnBuyNormalItem(uidPlayer, nItemID, nItemCount);
		}
	}
	else if(nCurrencyType == ITEM_CURRENCY_CASH)
	{
		OnBuyCashItem(uidPlayer, nItemID, nItemCount, pszGiftName, pszGiftMsg, nRentHourPeriod);
	}
	else if(nCurrencyType == ITEM_CURRENCY_MEDAL)
	{
		OnBuyMedalItem(uidPlayer, nItemID, nItemCount);
	}
	else
	{
		// invalid currency.
	}
}

// Sell function separated. -----------------------------------
void OnSellNormalItem(const MUID &uidPlayer, const MUID &uidItem, int nItemCount)
{
	if(nItemCount <= 0 || nItemCount > MAX_CHAR_SPENDITEM_COUNT) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	MMatchCharItem *pCharItem = pObj->GetCharItem(uidItem);
	if(pCharItem == NULL) return;
	
	MMatchItem *pItem = g_ItemMgr.Get(pCharItem->nID);
	if(pItem == NULL) return;
	
	MCmdWriter Cmd;
	
	if(pObj->IsEquippedItem(uidItem) == true)
	{
		Cmd.WriteInt(MERR_CANNOT_SELL_EQUIPPED_ITEM);
		Cmd.Finalize(MC_MATCH_RESPONSE_SELL_ITEM, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	if(pItem->IsSpendable() == false && nItemCount != 1)
	{
		Cmd.WriteInt(MERR_CANNOT_SELL_ITEM);
		Cmd.Finalize(MC_MATCH_RESPONSE_SELL_ITEM, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	if(pItem->IsBounty() == false)
	{
		Cmd.WriteInt(MERR_CANNOT_SELL_CASH_ITEM_ON_SHOP);
		Cmd.Finalize(MC_MATCH_RESPONSE_SELL_ITEM, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	int nNewCount = pCharItem->nCount - nItemCount;
	
	if(pItem->IsSpendable() == true)
	{
		if(nNewCount < 0)
		{
			Cmd.WriteInt(MERR_CANNOT_SELL_ITEM);
			Cmd.Finalize(MC_MATCH_RESPONSE_SELL_ITEM, MCFT_END);
			SendToClient(&Cmd, uidPlayer);
			return;
		}
	}
	
	int nNewBP = pObj->m_Exp.nBP;// + ((pItem->GetPrice() / 4) * nItemCount);
	if(pCharItem->nRentSecPeriod == 0) nNewBP += (pItem->GetPrice() / 4) * nItemCount;
	
	bool bRemoveItem = (pItem->IsSpendable() == false || nNewCount == 0) ? true : false ;
	
	if(bRemoveItem == true)
	{
		/*
		if(Db_DeleteItem(pObj->m_Char.nCID, pCharItem->nCIID, nNewBP) == false)
		{
			Cmd.WriteInt(MERR_CANNOT_SELL_ITEM);
			Cmd.Finalize(MC_MATCH_RESPONSE_SELL_ITEM, MCFT_END);
			SendToClient(&Cmd, uidPlayer);
			return;
		}
		*/
		AsyncDb_DeleteItem(uidPlayer, pObj->m_Char.nCID, pCharItem->nCIID, nNewBP);
		
		pObj->RemoveCharItem(uidItem);
	}
	else
	{
		/*
		if(Db_UpdateItem(pObj->m_Char.nCID, pCharItem->nCIID, nNewCount, nNewBP) == false)
		{
			Cmd.WriteInt(MERR_CANNOT_SELL_ITEM);
			Cmd.Finalize(MC_MATCH_RESPONSE_SELL_ITEM, MCFT_END);
			SendToClient(&Cmd, uidPlayer);
			return;
		}
		*/
		AsyncDb_UpdateItem(uidPlayer, pObj->m_Char.nCID, pCharItem->nCIID, nNewCount, nNewBP);
		
		pCharItem->nCount = nNewCount;
	}
	
	pObj->m_Exp.nBP = nNewBP;
	
	Cmd.WriteInt(MSG_OK);
	Cmd.Finalize(MC_MATCH_RESPONSE_SELL_ITEM, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
	
	OnCharacterItemList(uidPlayer);
}

void OnSellGambleItem(const MUID &uidPlayer, const MUID &uidItem, int nItemCount)
{
	if(nItemCount <= 0 || nItemCount > MAX_CHAR_GAMBLEITEM_COUNT) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	MMatchCharGambleItem *pCharItem = pObj->GetCharGambleItem(uidItem);
	if(pCharItem == NULL) return;
	
	MMatchGambleItem *pItem = g_GItemMgr.Get(pCharItem->nItemID);
	if(pItem == NULL) return;
	
	MCmdWriter Cmd;
	
	if(pItem->IsCash() == true)
	{
		Cmd.WriteInt(MERR_CANNOT_SELL_CASH_ITEM_ON_SHOP);
		Cmd.Finalize(MC_MATCH_RESPONSE_SELL_ITEM, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	int nNewCount = pCharItem->nCount - nItemCount;
	
	if(nNewCount < 0)
	{
		Cmd.WriteInt(MERR_CANNOT_SELL_ITEM);
		Cmd.Finalize(MC_MATCH_RESPONSE_SELL_ITEM, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
		
	int nNewBP = pObj->m_Exp.nBP + ((pItem->GetPrice() / 4) * nItemCount);
	
	bool bRemoveItem = nNewCount == 0 ? true : false ;
	
	if(bRemoveItem == true)
	{
		/*
		if(Db_DeleteGambleItem(pObj->m_Char.nCID, pCharItem->nGIID, nNewBP) == false)
		{
			Cmd.WriteInt(MERR_CANNOT_SELL_ITEM);
			Cmd.Finalize(MC_MATCH_RESPONSE_SELL_ITEM, MCFT_END);
			SendToClient(&Cmd, uidPlayer);
			return;
		}
		*/
		AsyncDb_DeleteGambleItem(uidPlayer, pObj->m_Char.nCID, pCharItem->nGIID, nNewBP);
		
		pObj->RemoveCharGambleItem(uidItem);
	}
	else
	{
		/*
		if(Db_UpdateGambleItem(pObj->m_Char.nCID, pCharItem->nGIID, nNewCount, nNewBP) == false)
		{
			Cmd.WriteInt(MERR_CANNOT_SELL_ITEM);
			Cmd.Finalize(MC_MATCH_RESPONSE_SELL_ITEM, MCFT_END);
			SendToClient(&Cmd, uidPlayer);
			return;
		}
		*/
		AsyncDb_UpdateGambleItem(uidPlayer, pObj->m_Char.nCID, pCharItem->nGIID, nNewCount, nNewBP);
		
		pCharItem->nCount = nNewCount;
	}
	
	pObj->m_Exp.nBP = nNewBP;
	
	Cmd.WriteInt(MSG_OK);
	Cmd.Finalize(MC_MATCH_RESPONSE_SELL_ITEM, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
	
	OnCharacterItemList(uidPlayer);
}

void OnSellMedalItem(const MUID &uidPlayer, const MUID &uidItem, int nItemCount)
{
	if(nItemCount <= 0 || nItemCount > MAX_CHAR_SPENDITEM_COUNT) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	MMatchCharItem *pCharItem = pObj->GetCharItem(uidItem);
	if(pCharItem == NULL) return;
	
	MBlitzShopItem *pMedalItem = g_BlitzShop.GetItem(pCharItem->nID);
	if(pMedalItem == NULL) return;
	
	MMatchItem *pItem = g_ItemMgr.Get(pCharItem->nID);
	if(pItem == NULL) return;
	
	MCmdWriter Cmd;
	
	if(pObj->IsEquippedItem(uidItem) == true)
	{
		Cmd.WriteInt(MERR_CANNOT_SELL_EQUIPPED_ITEM);
		Cmd.Finalize(MC_MATCH_RESPONSE_SELL_ITEM, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	if(pItem->IsSpendable() == false && nItemCount != 1)
	{
		Cmd.WriteInt(MERR_CANNOT_SELL_ITEM);
		Cmd.Finalize(MC_MATCH_RESPONSE_SELL_ITEM, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	if(pItem->IsMedal() == false)
	{
		Cmd.WriteInt(MERR_CANNOT_SELL_CASH_ITEM_ON_SHOP);
		Cmd.Finalize(MC_MATCH_RESPONSE_SELL_ITEM, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	int nNewCount = pCharItem->nCount - nItemCount;
	
	if(pItem->IsSpendable() == true)
	{
		if(nNewCount < 0)
		{
			Cmd.WriteInt(MERR_CANNOT_SELL_ITEM);
			Cmd.Finalize(MC_MATCH_RESPONSE_SELL_ITEM, MCFT_END);
			SendToClient(&Cmd, uidPlayer);
			return;
		}
	}
	
	int nSellPrice = pMedalItem->m_nPrice;
	if(pMedalItem->m_bCoupon == false) nSellPrice /= 5;
	
	int nNewMedal = pObj->m_BlitzKrieg.nMedal;// + (nSellPrice * nItemCount);
	if(pCharItem->nRentSecPeriod == 0) nNewMedal += nSellPrice * nItemCount;
	
	bool bRemoveItem = (pItem->IsSpendable() == false || nNewCount == 0) ? true : false ;
	
	if(bRemoveItem == true)
	{
		/*
		if(Db_DeleteItem(pObj->m_Char.nCID, pCharItem->nCIID, pObj->m_Exp.nBP) == false)
		{
			Cmd.WriteInt(MERR_CANNOT_SELL_ITEM);
			Cmd.Finalize(MC_MATCH_RESPONSE_SELL_ITEM, MCFT_END);
			SendToClient(&Cmd, uidPlayer);
			return;
		}
		*/
		AsyncDb_DeleteItem(uidPlayer, pObj->m_Char.nCID, pCharItem->nCIID, pObj->m_Exp.nBP);
		
		pObj->RemoveCharItem(uidItem);
	}
	else
	{
		/*
		if(Db_UpdateItem(pObj->m_Char.nCID, pCharItem->nCIID, nNewCount, pObj->m_Exp.nBP) == false)
		{
			Cmd.WriteInt(MERR_CANNOT_SELL_ITEM);
			Cmd.Finalize(MC_MATCH_RESPONSE_SELL_ITEM, MCFT_END);
			SendToClient(&Cmd, uidPlayer);
			return;
		}
		*/
		AsyncDb_UpdateItem(uidPlayer, pObj->m_Char.nCID, pCharItem->nCIID, nNewCount, pObj->m_Exp.nBP);
		
		pCharItem->nCount = nNewCount;
	}
	
	pObj->m_BlitzKrieg.nMedal = nNewMedal;
	
	Cmd.WriteInt(MSG_OK);
	Cmd.Finalize(MC_MATCH_RESPONSE_SELL_ITEM, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
	
	OnCharacterItemList(uidPlayer);
	
	pObj->SyncMedal();
}

void OnSellItem(const MUID &uidPlayer, const MUID &uidItem, int nItemCount)
{
	/*
	if(nItemCount <= 0 || nItemCount > MAX_CHAR_SPENDITEM_COUNT) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	MMatchCharItem *pCharItem = pObj->GetCharItem(uidItem);
	if(pCharItem == NULL) return;
	
	MMatchItem *pItem = g_ItemMgr.Get(pCharItem->nID);
	if(pItem == NULL) return;
	
	MCmdWriter Cmd;
	
	if(pObj->IsEquippedItem(uidItem) == true)
	{
		Cmd.WriteInt(MERR_CANNOT_SELL_EQUIPPED_ITEM);
		Cmd.Finalize(MC_MATCH_RESPONSE_SELL_ITEM, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	if(pItem->IsSpendable() == false && nItemCount != 1)
	{
		Cmd.WriteInt(MERR_CANNOT_SELL_ITEM);
		Cmd.Finalize(MC_MATCH_RESPONSE_SELL_ITEM, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	if(pItem->IsCash() == true)
	{
		Cmd.WriteInt(MERR_CANNOT_SELL_CASH_ITEM_ON_SHOP);
		Cmd.Finalize(MC_MATCH_RESPONSE_SELL_ITEM, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	int nNewCount = pCharItem->nCount - nItemCount;
	
	if(pItem->IsSpendable() == true)
	{
		if(nNewCount < 0)
		{
			Cmd.WriteInt(MERR_CANNOT_SELL_ITEM);
			Cmd.Finalize(MC_MATCH_RESPONSE_SELL_ITEM, MCFT_END);
			SendToClient(&Cmd, uidPlayer);
			return;
		}
	}
	
	int nNewBP = pObj->m_Exp.nBP + ((pItem->GetPrice() / 4) * nItemCount);
	
	bool bRemoveItem = (pItem->IsSpendable() == false || nNewCount == 0) ? true : false ;
	
	if(bRemoveItem == true)
	{
		if(Db_DeleteItem(pObj->m_Char.nCID, pCharItem->nCIID, nNewBP) == false)
		{
			Cmd.WriteInt(MERR_CANNOT_SELL_ITEM);
			Cmd.Finalize(MC_MATCH_RESPONSE_SELL_ITEM, MCFT_END);
			SendToClient(&Cmd, uidPlayer);
			return;
		}
		
		pObj->RemoveCharItem(uidItem);
	}
	else
	{
		if(Db_UpdateItem(pObj->m_Char.nCID, pCharItem->nCIID, nNewCount, nNewBP) == false)
		{
			Cmd.WriteInt(MERR_CANNOT_SELL_ITEM);
			Cmd.Finalize(MC_MATCH_RESPONSE_SELL_ITEM, MCFT_END);
			SendToClient(&Cmd, uidPlayer);
			return;
		}
		
		pCharItem->nCount = nNewCount;
	}
	
	pObj->m_Exp.nBP = nNewBP;
	
	Cmd.WriteInt(MSG_OK);
	Cmd.Finalize(MC_MATCH_RESPONSE_SELL_ITEM, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
	
	OnCharacterItemList(uidPlayer);
	*/
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	MMatchCharItem *pCharItem = pObj->GetCharItem(uidItem);
	
	if(pCharItem != NULL)
	{
		if(g_BlitzShop.GetItem(pCharItem->nID) != NULL)
		{
			OnSellMedalItem(uidPlayer, uidItem, nItemCount);
		}
		else
		{
			OnSellNormalItem(uidPlayer, uidItem, nItemCount);
		}
	}
	else if(pObj->GetCharGambleItem(uidItem) != NULL)
	{
		OnSellGambleItem(uidPlayer, uidItem, nItemCount);
	}
}

void OnEquipItem(const MUID &uidPlayer, const MUID &uidItem, int nSlot)
{
	if(nSlot < 0 || nSlot >= MMCIP_END) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	MMatchCharItem *pCharItem = pObj->GetCharItem(uidItem);
	if(pCharItem == NULL) return;
	
	MMatchItem *pItem = g_ItemMgr.Get(pCharItem->nID);
	if(pItem == NULL) return;
	
	MCmdWriter Cmd;
	
	if(pItem->IsValidSlot(nSlot) == false)
	{
		Cmd.WriteInt(MERR_CANNOT_EQUIP_ITEM);
		Cmd.Finalize(MC_MATCH_RESPONSE_EQUIP_ITEM, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	if(pItem->IsValidSex(pObj->m_Char.nSex) == false)
	{
		Cmd.WriteInt(MERR_ITEM_SEX_MISMATCH);
		Cmd.Finalize(MC_MATCH_RESPONSE_EQUIP_ITEM, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	if(pItem->IsEquipableLevel(pObj->m_Exp.nLevel) == false)
	{
		Cmd.WriteInt(MERR_CANNOT_EQUIP_ITEM_BY_LEVEL);
		Cmd.Finalize(MC_MATCH_RESPONSE_EQUIP_ITEM, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	// for 'replace' item, not insert.
	int nPrevItemWt = 0, nPrevItemMaxWt = 0;
	
	MMatchCharItem *pPrevCharItem = pObj->GetCharItemBySlotIndex(nSlot);
	if(pPrevCharItem != NULL)
	{
		MMatchItem *pPrevItem = g_ItemMgr.Get(pPrevCharItem->nID);
		if(pPrevItem != NULL)
		{
			nPrevItemWt = pPrevItem->GetWeight();
			nPrevItemMaxWt = pPrevItem->GetMaxWt();
		}
	}
	
	if(pItem->IsEquipableWeight(pObj->CalcCurrWeight() - nPrevItemWt, pObj->CalcMaxWeight() - nPrevItemMaxWt) == false)
	{
		Cmd.WriteInt(MERR_ITEM_WEIGHT_OVER);
		Cmd.Finalize(MC_MATCH_RESPONSE_EQUIP_ITEM, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	// TIP : If you want to check for same weapons, add here.

	/*
	if(Db_EquipItem(pObj->m_Char.nCID, pCharItem->nCIID, nSlot) == false)
	{
		// some errors on db.
		return;
	}
	*/
	
	AsyncDb_EquipItem(uidPlayer, pObj->m_Char.nCID, pCharItem->nCIID, nSlot);
	
	pObj->SetItemUID(uidItem, nSlot);
	
	Cmd.WriteInt(MSG_OK);
	Cmd.Finalize(MC_MATCH_RESPONSE_EQUIP_ITEM, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
	
	OnCharacterItemList(uidPlayer);
	
	SendStageCharEquipItemUpdate(uidPlayer, nSlot, pItem->GetID(), pObj->m_uidStage);
}

void OnTakeoffItem(const MUID &uidPlayer, int nSlot)
{
	if(nSlot < 0 || nSlot >= MMCIP_END) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	MMatchCharItem *pCharItem = pObj->GetCharItemBySlotIndex(nSlot);
	if(pCharItem == NULL) return;
	
	MMatchItem *pItem = g_ItemMgr.Get(pCharItem->nID);
	if(pItem == NULL) return;
	
	MCmdWriter Cmd;
	
	int nWeight = pObj->CalcCurrWeight() - pItem->GetWeight(), nMaxWt = pObj->CalcMaxWeight() - pItem->GetMaxWt();
	if(nWeight > nMaxWt)
	{
		Cmd.WriteInt(MERR_CANNOT_TAKEOFF_ITEM_BY_WEIGHT);
		Cmd.Finalize(MC_MATCH_RESPONSE_TAKEOFF_ITEM, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	/*
	if(Db_TakeoffItem(pObj->m_Char.nCID, nSlot) == false)
	{
		// some errors on db.
		return;
	}
	*/
	
	AsyncDb_TakeoffItem(uidPlayer, pObj->m_Char.nCID, nSlot);
	
	pObj->SetItemUID(MUID(0, 0), nSlot);
	
	Cmd.WriteInt(MSG_OK);
	Cmd.Finalize(MC_MATCH_RESPONSE_TAKEOFF_ITEM, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
	
	OnCharacterItemList(uidPlayer);
	
	SendStageCharEquipItemUpdate(uidPlayer, nSlot, 0, pObj->m_uidStage);
}

void OnOpenGambleItem(const MUID &uidPlayer, const MUID &uidItem)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	if(IsMaxCharItem((int)(pObj->m_ItemList.size() + 1)) == true)
	{
		MCmdWriter ErrCmd;
		ErrCmd.WriteInt(MERR_TOO_MANY_ITEM);
		ErrCmd.Finalize(MC_MATCH_RESPONSE_RESULT, MCFT_END);
		SendToClient(&ErrCmd, uidPlayer);
		return;
	}
	
	MMatchCharGambleItem *pCharGambleItem = pObj->GetCharGambleItem(uidItem);
	if(pCharGambleItem == NULL) return;
	
	MMatchGambleItem *pGambleItem = g_GItemMgr.Get(pCharGambleItem->nItemID);
	if(pGambleItem == NULL) return;
	
	MMatchGambleRewardItemNode *pGambleRewardItem = pGambleItem->GetRandomRewardItem();
	if(pGambleRewardItem == NULL) return;
	
	int nRewardItemID = 0;
	
	if(pObj->m_Char.nSex == MMS_MALE) nRewardItemID = pGambleRewardItem->GetMaleItemID();
	else if(pObj->m_Char.nSex == MMS_FEMALE) nRewardItemID = pGambleRewardItem->GetFemaleItemID();
	
	if(nRewardItemID == 0) return;
	
	MMatchItem *pItem = g_ItemMgr.Get(nRewardItemID);
	if(pItem == NULL) return;
	
	MMatchCharItem *pCurrSpendItem = pObj->GetCharItemByItemID(nRewardItemID);
	bool bCharSpendItemExists = pItem->IsSpendable() == false || pCurrSpendItem == NULL ? false : true ;
	
	if(bCharSpendItemExists == false)
	{
		/*
		bool bResult = false;
		
		int nCIID;
		if(pGambleRewardItem->IsRentPeriodUnlimited() == true)
		{
			bResult = Db_InsertItem(pObj->m_Char.nCID, nRewardItemID, pGambleRewardItem->GetQuantity(), pObj->m_Exp.nBP, &nCIID);
		}
		else
		{
			bResult = Db_InsertItem(pObj->m_Char.nCID, nRewardItemID, pGambleRewardItem->GetQuantity(), pGambleRewardItem->GetRentHourPeriod(), pObj->m_Exp.nBP, &nCIID);
		}
		
		if(bResult == false) return;
		
		pObj->AddCharItem(MUID::Assign(), nCIID, nRewardItemID, pGambleRewardItem->GetQuantity(), pGambleRewardItem->GetRentHourPeriod() * 60 * 60);
		*/
		
		AsyncDb_InsertItem(uidPlayer, pObj->m_Char.nCID, nRewardItemID, pGambleRewardItem->GetQuantity(), pGambleRewardItem->GetRentHourPeriod(), pObj->m_Exp.nBP);
	}
	else
	{
		int nNewCount = pCurrSpendItem->nCount + pGambleRewardItem->GetQuantity();
		if(nNewCount > MAX_CHAR_SPENDITEM_COUNT) nNewCount = MAX_CHAR_SPENDITEM_COUNT;
		
		// if(Db_UpdateItem(pObj->m_Char.nCID, pCurrSpendItem->nCIID, nNewCount, pObj->m_Exp.nBP) == false) return;
		AsyncDb_UpdateItem(uidPlayer, pObj->m_Char.nCID, pCurrSpendItem->nCIID, nNewCount, pObj->m_Exp.nBP);
		
		pCurrSpendItem->nCount = nNewCount;
	}
	
	// if(Db_DecreaseGambleItem(pCharGambleItem->nGIID) == false) return;
	AsyncDb_DecreaseGambleItem(uidPlayer, pCharGambleItem->nGIID);
	
	pCharGambleItem->nCount--;
	if(pCharGambleItem->nCount <= 0)
	{
		pObj->RemoveCharGambleItem(pCharGambleItem->uid);
	}
	
	MCmdWriter Cmd;
	Cmd.WriteUInt(0);	// ?, should be 0.
	Cmd.WriteUInt((unsigned int)nRewardItemID);
	Cmd.WriteUInt((unsigned int)pGambleRewardItem->GetQuantity());
	Cmd.WriteUInt((unsigned int)(pGambleRewardItem->GetRentHourPeriod() * 60));
	Cmd.Finalize(MC_MATCH_RESPONSE_GAMBLE, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
	
	if(bCharSpendItemExists == true)	// don't update item list when an item not inserted.
	{
		OnCharacterItemList(uidPlayer);
	}
}

// account item -------------------------------------------------- .
void OnAccountItemList(const MUID &uidPlayer)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	MCmdWriter Cmd;
	
	// ...? unknown parameters.
	Cmd.WriteShort(0);
	Cmd.WriteShort(1);
	
	Cmd.StartBlob(sizeof(MTD_AccountItemNode));
	for(list<MMatchAccItem *>::iterator i = pObj->m_AccountItemList.begin(); i != pObj->m_AccountItemList.end(); i++)
	{
		MMatchAccItem *pCurr = (*i);
		
		int nRentPeriodMinute = pCurr->CalcRentSecPeriod() / 60;
		
		MTD_AccountItemNode node;
		node.nAIID = pCurr->nAIID;
		node.nItemID = (unsigned long)pCurr->nID;
		node.nRentMinutePeriodRemainder = nRentPeriodMinute == 0 ? RENT_MINUTE_PERIOD_UNLIMITED : nRentPeriodMinute;
		node.nCount = pCurr->nCount;
		
		Cmd.WriteData(&node, sizeof(MTD_AccountItemNode));
	}
	Cmd.EndBlob();
	
	Cmd.Finalize(MC_MATCH_RESPONSE_ACCOUNT_ITEMLIST, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
}

// param 3 : nItemID is not used.
void OnTakeOutItemFromStorage(const MUID &uidPlayer, int nAIID, int nItemID, int nItemCount)
{
	if(nItemCount <= 0 || nItemCount > MAX_CHAR_SPENDITEM_COUNT) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	MMatchAccItem *pAccItem = pObj->GetAccItemByAIID(nAIID);
	if(pAccItem == NULL) return;
	
	MMatchItem *pItem = g_ItemMgr.Get(pAccItem->nID);
	if(pItem == NULL) return;
	
	if(pItem->IsSpendable() == true)
	{
		MMatchCharItem *pCharItem = pObj->GetCharItemByItemID(pAccItem->nID);
		if(pCharItem != NULL)
		{
			int nNewCount = pCharItem->nCount + nItemCount;
			if(nNewCount > MAX_CHAR_SPENDITEM_COUNT)
			{
				MCmdWriter temp;
				temp.WriteInt(MERR_TOO_MANY_ITEM);
				temp.Finalize(MC_MATCH_RESPONSE_BRING_ACCOUNTITEM, MCFT_END);
				SendToClient(&temp, uidPlayer);
				return;
			}
		}
		
		if(pObj->MoveItemToInventory(nAIID, nItemCount) == false)
		{
			// error...
			return;
		}
	}
	else
	{
		if(nItemCount != 1) return;
		
		if(pObj->MoveItemToInventory(nAIID) == false)
		{
			// error...
			return;
		}
	}
	
	MCmdWriter Cmd;
	Cmd.WriteInt(MSG_OK);
	Cmd.Finalize(MC_MATCH_RESPONSE_BRING_ACCOUNTITEM, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
	
	// OnCharacterItemList(uidPlayer);
	OnAccountItemList(uidPlayer);
}

void OnDepositItemToStorage(const MUID &uidPlayer, const MUID &uidItem, int nItemCount)
{
	if(nItemCount <= 0 || nItemCount > MAX_CHAR_SPENDITEM_COUNT) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	if(pObj->IsEquippedItem(uidItem) == true) return;
	
	MMatchCharItem *pCharItem = pObj->GetCharItem(uidItem);
	if(pCharItem == NULL) return;
	
	MMatchItem *pItem = g_ItemMgr.Get(pCharItem->nID);
	if(pItem == NULL) return;
	
	if(pItem->IsSpendable() == false && nItemCount != 1) return;
	
	if(pItem->IsCash() == false)
	{
		MCmdWriter temp;
		temp.WriteInt(MERR_ONLY_CASH_ITEM_ALLOWED_TO_SEND_BANK);
		temp.Finalize(MC_MATCH_RESPONSE_BRING_BACK_ACCOUNTITEM, MCFT_END);
		SendToClient(&temp, uidPlayer);
		return;
	}
	
	if(pItem->IsSpendable() == true)
	{
		if(pObj->MoveItemToStorage(pCharItem->nCIID, nItemCount) == false)
		{
			MCmdWriter temp;
			temp.WriteInt(MERR_CANNOT_SEND_TO_BANK);
			temp.Finalize(MC_MATCH_RESPONSE_BRING_BACK_ACCOUNTITEM, MCFT_END);
			SendToClient(&temp, uidPlayer);
			return;
		}
	}
	else
	{
		if(pObj->MoveItemToStorage(pCharItem->nCIID) == false)
		{
			MCmdWriter temp;
			temp.WriteInt(MERR_CANNOT_SEND_TO_BANK);
			temp.Finalize(MC_MATCH_RESPONSE_BRING_BACK_ACCOUNTITEM, MCFT_END);
			SendToClient(&temp, uidPlayer);
			return;
		}
	}
	
	MCmdWriter Cmd;
	Cmd.WriteInt(MSG_OK);
	Cmd.Finalize(MC_MATCH_RESPONSE_BRING_BACK_ACCOUNTITEM, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
	
	OnCharacterItemList(uidPlayer);
	// OnAccountItemList(uidPlayer);
}

void OnCashShopItemList(const MUID &uidPlayer, int nSex, int nSortType)
{
	if(nSex != CASHSHOP_SEX_MALE && nSex != CASHSHOP_SEX_FEMALE) return;
	if(nSortType != CASHSHOP_SORT_NEWEST && nSortType != CASHSHOP_SORT_LEVEL) return;
	
	g_CashShop.SendItemList(uidPlayer, nSex, nSortType);
}

void OnCashShopItemInfo(const MUID &uidPlayer, int nSex, int nSortType, int nStartIndex, int nCount)
{
	if(nSex != CASHSHOP_SEX_MALE && nSex != CASHSHOP_SEX_FEMALE) return;
	if(nSortType != CASHSHOP_SORT_NEWEST && nSortType != CASHSHOP_SORT_LEVEL) return;
	if(nStartIndex < 0) return;
	if(nCount <= 0 || nCount > 100) return;
	
	g_CashShop.SendItemInfo(uidPlayer, nSex, nSortType, nStartIndex, nCount);
}

void OnSyncCashRemainder(const MUID &uidPlayer)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	pObj->SyncCash(false);
}

void OnCheckGiftItem(const MUID &uidPlayer)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	// removed because item list doesn't be updated other than selecting char. : i'll check received item at select char only once.
	/*
	MAsyncDBTask_CheckItemGift *pNew = new MAsyncDBTask_CheckItemGift(uidPlayer, pObj->m_Char.nCID);
	g_AsyncDBTaskMgr.SafeAdd(pNew);
	*/
}

void OnReadGiftItem(const MUID &uidPlayer, vector<int> &vtGiftID)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	for(vector<int>::iterator i = vtGiftID.begin(); i != vtGiftID.end(); i++)
	{
		MAsyncDBTask_AcceptItemGift *pNew = new MAsyncDBTask_AcceptItemGift(uidPlayer, *i, pObj->m_Char.nCID);
		g_AsyncDBTaskMgr.SafeAdd(pNew);
	}
}