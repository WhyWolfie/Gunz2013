#include "pch.h"

#include "MMatchDBMgr.h"
#include "MMatchConstant.h"

#include "MMatchObject_Constant.h"

#include "MServerSetting.h"

otl_connect db;

bool Db_GetAccountInfo(const char *pszInUserID, int *pOutAID, char *pszOutPassword, int *pOutUGradeID, int *pOutPGradeID, int *pOutCash)
{
	if(Db_IsValidString(pszInUserID) == false)
		return false;

	try
	{
		otl_stream a(1, "SELECT aid, password, ugradeid, pgradeid, cash FROM getaccountinfo(:id<char[32]>);", db, otl_implicit_select);
		a << pszInUserID;
		
		if(a.get_prefetched_row_count() == 0)
			return false;
			
		a >> *pOutAID >> pszOutPassword >> *pOutUGradeID >> *pOutPGradeID >> *pOutCash;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}

	return true;
}

bool Db_GetAccountCharList(int nInAID, int *pOutCount, DbData_AccountCharList *pOutInfo)
{
	*pOutCount = 0;

	try
	{
		otl_stream a(4 /*ACCOUNT_CHARLIST_COUNT*/, "SELECT charname, charnum, charlevel FROM getcharacterlist(:id<int>);", db, otl_implicit_select);
		a << nInAID;
		
		for(int i = 0; a.eof() == 0 && i < ACCOUNT_CHARLIST_COUNT; i++)
		{
			a >> pOutInfo[i].szName >> pOutInfo[i].nCharNum >> pOutInfo[i].nLevel;
			(*pOutCount)++;
		}
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}

	return true;
}

bool Db_GetCharIndex(int nInAID, int nInCharNum, int *pOutCID)
{
	try
	{
		otl_stream a(1, "SELECT charid FROM getcharindex(:id<int>) WHERE charnum = :num<int>;", db, otl_implicit_select);
		a << nInAID << nInCharNum;
		
		if(a.get_prefetched_row_count() == 0)
			return false;
			
		a >> *pOutCID;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}

	return true;
}

bool Db_GetBasicCharInfo(int nInCID, DbData_BasicCharInfo *pOutInfo)
{
	try
	{
		otl_stream a(1, "SELECT name, level, sex, hair, face, exp, bounty, killcount, deathcount, ranking, questiteminfo "
						"FROM getcharacterinfo(:id<int>);", db, otl_implicit_select);
		a << nInCID;
		
		if(a.get_prefetched_row_count() == 0)
			return false;
			
		a >> pOutInfo->szName >> pOutInfo->nLevel >> pOutInfo->nSex >> pOutInfo->nHair >> pOutInfo->nFace >> pOutInfo->nExp >> pOutInfo->nBounty >> 
				pOutInfo->nKillCount >> pOutInfo->nDeathCount >> pOutInfo->nRanking >> pOutInfo->szQuestItemData;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}

	return true;
}

bool Db_GetBasicClanInfo(int nInCID, DbData_BasicClanInfo *pOutInfo)
{
	try
	{
		otl_stream a(1, "SELECT clid_out, name_out, grade_out, point_out FROM getbasicclaninfo(:id<int>);", db, otl_implicit_select);
		a << nInCID;
		
		// if(a.get_prefetched_row_count() == 0)
		// 	return false;
			
		if(a.get_prefetched_row_count() == 1)
		{
			a >> pOutInfo->nCLID >> pOutInfo->szClanName >> pOutInfo->nMemberGrade >> pOutInfo->nClanPoint;
		}
		else
		{
			// CLID == 0 : means clan not found.
			pOutInfo->nCLID = 0;
		}
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}

	return true;
}

bool Db_GetCharItemInfo(int nInCID, DbData_CharItemInfo *pOutInfo)
{
	ZeroInit(pOutInfo, sizeof(DbData_CharItemInfo));
	
	try
	{
		otl_stream a(17 /*MMCIP_END*/, "SELECT itemid_, quantity_, slotnum_ FROM getcharalliteminfo(:cid<int>);", db, otl_implicit_select);
		a << nInCID;
		
		while(a.eof() == 0)
		{
			int nItemID, nQuantity, nSlot;
			a >> nItemID >> nQuantity >> nSlot;
			
			if(nItemID != 0)
			{
				if(nSlot < 0 || nSlot >= MMCIP_END) continue;
				
				pOutInfo->nItemID[nSlot] = nItemID;
				pOutInfo->nItemCount[nSlot] = nQuantity;
			}
		}
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}

	return true;
}

bool Db_GetCharEquipmentSlotCIID(int nInCID, DbData_CharEquippedItemCIID *pOutInfo)
{
	ZeroInit(pOutInfo, sizeof(DbData_CharEquippedItemCIID));
	
	try
	{
		otl_stream a(17 /*MMCIP_END*/, "SELECT ciid_, slotnum_ FROM getcharequipmentslotciid(:cid<int>);", db, otl_implicit_select);
		a << nInCID;
		
		while(a.eof() == 0)
		{
			int nCIID, nSlot;
			a >> nCIID >> nSlot;
			
			if(nCIID != 0)
			{
				if(nSlot < 0 || nSlot >= MMCIP_END) continue;
				pOutInfo->nCIID[nSlot] = nCIID;
			}
		}
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}

	return true;
}

bool Db_GetDTCharClass(int nInCID, int *pOutClass)
{
	*pOutClass = 10;

	try
	{
		otl_stream a(1, "SELECT class FROM getdtcharinfo(:id<int>);", db, otl_implicit_select);
		a << nInCID;
		
		if(a.get_prefetched_row_count() == 0)
			return false;
			
		a >> *pOutClass;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}

	return true;
}

bool Db_GetCharacterCount(int nInAID, int *pOutCount)
{
	try
	{
		otl_stream a(1, "SELECT getcharactercount(:id<int>);", db, otl_implicit_select);
		a << nInAID;
		
		a >> *pOutCount;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}

	return true;
}

bool Db_IsCharacterExists(const char *pszInSearchName, int *pOutResult)
{
	if(Db_IsValidString(pszInSearchName) == false)
		return false;

	if(strlen(pszInSearchName) >= DB_CHARNAME_LEN)
		return false;

	try
	{
		otl_stream a(1, "SELECT ischaracterexists(:name<char[24]>);", db, otl_implicit_select);
		a << pszInSearchName;
		
		a >> *pOutResult;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}

	return true;
}

bool Db_InsertCharacter(int nInAID, const char *pszInName, int nInSex, int nInHair, int nInFace, int nInCostume)
{
	if(Db_IsValidString(pszInName) == false)
		return false;

	if(strlen(pszInName) >= DB_CHARNAME_LEN)
		return false;

	int nItemID_Head = 0, nItemID_Chest = 0, nItemID_Hands = 0, nItemID_Legs = 0, nItemID_Feets = 0;	// initial armor.
	int nItemID_FingerL = 0, nItemID_FingerR = 0;	// initial ring.
	int nItemID_Melee = 0, nItemID_Primary = 0, nItemID_Secondary = 0;	// initial weapon.
	int nItemID_Custom1 = 0, nItemID_Custom2 = 0;	// initial greande or kit.
	int nItemID_Avatar = 0;	// initial avatar.

	switch(nInSex)
	{
		case MMS_MALE	:
		#ifdef _PERU_INIT_EQUIP
			nItemID_Chest = 21017;
			nItemID_Hands = 22013;
			nItemID_Legs = 23018;
			nItemID_Feets = 24014;
		#else
			nItemID_Chest = 21001;
			nItemID_Legs = 23001;
		#endif
			break;
		case MMS_FEMALE	:
		#ifdef _PERU_INIT_EQUIP
			nItemID_Chest = 21517;
			nItemID_Hands = 22513;
			nItemID_Legs = 23518;
			nItemID_Feets = 24514;
		#else
			nItemID_Chest = 21501;
			nItemID_Legs = 23501;
		#endif
			break;
		default	:
			return false;
	}

	switch(nInCostume)
	{
		case 0	:
			nItemID_Melee = 1;
			nItemID_Primary = 5001;
			nItemID_Secondary = 4001;
			nItemID_Custom1 = 30301;
			break;
		case 1	:
			nItemID_Melee = 2;
			nItemID_Primary = 5002;
			nItemID_Custom1 = 30301;
			break;
		case 2	:
			nItemID_Melee = 1;
			nItemID_Primary = 4005;
			nItemID_Secondary = 5001;
			nItemID_Custom1 = 30401;
			break;
		case 3	:
			nItemID_Melee = 2;
			nItemID_Primary = 4001;
			nItemID_Custom1 = 30401;
			break;
		case 4	:
			nItemID_Melee = 2;
			nItemID_Primary = 4002;
			nItemID_Custom1 = 30401;
			nItemID_Custom2 = 30001;
			break;
		case 5	:
			nItemID_Melee = 1;
			nItemID_Primary = 4006;
			nItemID_Custom1 = 30101;
			nItemID_Custom2 = 30001;
			break;
		default	:
			return false;
	}


	try
	{
		otl_stream a(1, "SELECT insertcharacter(:id<int>, :name<char[24]>, :sex<int>, :hair<int>, :face<int>, "
		             ":head<int>, :chest<int>, :hands<int>, :legs<int>, :feet<int>, :fingerl<int>, :fingerr<int>, "
		             ":melee<int>, :primary<int>, :secondary<int>, :custom1<int>, :custom2<int>, :avatar<int>, "
		             ":community1<int>, :community2<int>, :longbuff1<int>, :longbuff2<int>);", db, otl_implicit_select);
		a << nInAID << pszInName << nInSex << nInHair << nInFace <<
		  nItemID_Head << nItemID_Chest << nItemID_Hands << nItemID_Legs << nItemID_Feets <<
		  nItemID_FingerL << nItemID_FingerR << nItemID_Melee << nItemID_Primary << nItemID_Secondary <<
		  nItemID_Custom1 << nItemID_Custom2 << nItemID_Avatar << 0 /* Community1. */ << 0 /* Community2. */ <<
		  0 /* LongBuff1. */ << 0 /* LongBuff2. */;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}

	return true;
}

bool Db_GetCharacterName(int nInCID, char *pszOutName)
{
	try
	{
		otl_stream a(1, "SELECT name FROM getcharacterinfo(:id<int>);", db, otl_implicit_select);
		a << nInCID;
		
		if(a.get_prefetched_row_count() == 0)
			return false;
			
		a >> pszOutName;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}

	return true;
}

bool Db_DeleteCharacter(int nInCID)
{
	try
	{
		otl_stream a(1, "SELECT deletecharacter(:id<int>);", db, otl_implicit_select);
		a << nInCID;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}

	return true;
}

bool Db_GetDTCharInfo(int nInCID, DbData_DTCharInfo *pOutInfo)
{
	// set to default dt score.
	pOutInfo->nGrade = 10;
	pOutInfo->nTournamentPoint = 1000;
	pOutInfo->nWins = pOutInfo->nLosses = pOutInfo->nFinalMatchWin = 0;
	pOutInfo->nRanking = 0;
	pOutInfo->nPreviousTournamentPoint = 0;
	pOutInfo->nPreviousWins = pOutInfo->nPreviousLosses = pOutInfo->nPreviousFinalMatchWin = 0;
	pOutInfo->nPreviousRanking = 0;
	pOutInfo->nRankingDifferent = 0;
	
	try
	{
		otl_stream a(1, "SELECT class, tp, win, lose, finalwin, ranking, tpprev, winprev, loseprev, finalwinprev, rankingprev, rankingdiff "
						"FROM getdtcharinfo(:id<int>);", db, otl_implicit_select);
		a << nInCID;
		
		if(a.get_prefetched_row_count() == 0)
			return false;
			
		a >> pOutInfo->nGrade >> pOutInfo->nTournamentPoint >> pOutInfo->nWins >> pOutInfo->nLosses >> pOutInfo->nFinalMatchWin >> 
			pOutInfo->nRanking >> pOutInfo->nPreviousTournamentPoint >> pOutInfo->nPreviousWins >> pOutInfo->nPreviousLosses >> 
			pOutInfo->nPreviousFinalMatchWin >> pOutInfo->nPreviousRanking >> pOutInfo->nRankingDifferent;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}

	return true;
}

bool Db_GetCharacterItemList(int nInCID, vector<DbData_CharItemList> *pOut)
{
	try
	{
		otl_stream a(100 /*MAX_CHAR_ITEM_COUNT*/, "SELECT ciid_, itemid_, period_, count_ FROM getcharitemlist(:cid<int>);", db, otl_implicit_select);
		a << nInCID;
		
		while(a.eof() == 0)
		{
			DbData_CharItemList ci;
			a >> ci.nCIID >> ci.nItemID >> ci.nRentSecPeriod >> ci.nItemCount;
			
			pOut->push_back(ci);
		}
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}

	return true;
}

bool Db_ClearExpiredItem(int nInCID, vector<DbData_ExpiredItemNode> *pOutItemInfo)
{
	try
	{
		otl_stream a(50, "SELECT ciid_, itemid_ FROM checkexpireditem(:cid<int>);", db, otl_implicit_select);
		a << nInCID;
		
		while(a.eof() == 0)
		{
			DbData_ExpiredItemNode node;
			a >> node.nCIID >> node.nItemID;
			
			pOutItemInfo->push_back(node);
		}
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}

	return true;
}

bool Db_ClearExpiredAccountItem(int nInAID, vector<DbData_ExpiredItemNode> *pOutItemInfo)
{
	try
	{
		otl_stream a(50, "SELECT aiid_, itemid_ FROM checkexpiredaccountitem(:aid<int>);", db, otl_implicit_select);
		a << nInAID;
		
		while(a.eof() == 0)
		{
			DbData_ExpiredItemNode node;
			a >> node.nCIID >> node.nItemID;	// in this case, 'node.nCIID' means AIID.
			
			pOutItemInfo->push_back(node);
		}
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}

	return true;
}

bool Db_InsertItem(int nInCID, int nInItemID, int nInCount, int nInRentHourPeriod, int nInBP, int *pOutCIID)
{
	try
	{
		otl_stream a(1, "SELECT insertitem(:cid<int>, :itemid<int>, :count<int>, makeperiod(:rentperiod<int>), :bp<int>);", db, otl_implicit_select);
		a << nInCID << nInItemID << nInCount << nInRentHourPeriod << nInBP;
		
		if(a.get_prefetched_row_count() == 0)
			return false;
			
		a >> *pOutCIID;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_InsertItem(int nInCID, int nInItemID, int nInCount, int nInBP, int *pOutCIID)
{
	try
	{
		otl_stream a(1, "SELECT insertitem(:cid<int>, :itemid<int>, :count<int>, NULL, :bp<int>);", db, otl_implicit_select);
		a << nInCID << nInItemID << nInCount << nInBP;
		
		if(a.get_prefetched_row_count() == 0)
			return false;
			
		a >> *pOutCIID;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_UpdateItem(int nInCID, int nInCIID, int nInCount, int nInBP)
{
	try
	{
		otl_stream a(1, "SELECT updateitem(:cid<int>, :ciid<int>, :cnt<int>, :bp<int>);", db, otl_implicit_select);
		a << nInCID << nInCIID << nInCount << nInBP;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_DeleteItem(int nInCID, int nInCIID, int nInBP)
{
	try
	{
		otl_stream a(1, "SELECT deleteitem(:cid<int>, :ciid<int>, :bp<int>);", db, otl_implicit_select);
		a << nInCID << nInCIID << nInBP;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_EquipItem(int nInCID, int nInCIID, int nInSlot)
{
	try
	{
		otl_stream a(1, "SELECT equipitem(:cid<int>, :ciid<int>, :slot<int>);", db, otl_implicit_select);
		a << nInCID << nInCIID << nInSlot;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_TakeoffItem(int nInCID, int nInSlot)
{
	try
	{
		otl_stream a(1, "SELECT removeequippeditem(:cid<int>, :slot<int>);", db, otl_implicit_select);
		a << nInCID << nInSlot;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_SetExp(int nInCID, int nInLevel, int nInXP, int nInBP, int nInKill, int nInDeath)
{
	try
	{
		otl_stream a(1, "SELECT setexp(:cid<int>, :lv<int>, :xp<int>, :bp<int>, :kill<int>, :death<int>);", db, otl_implicit_select);
		a << nInCID << nInLevel << nInXP << nInBP << nInKill << nInDeath;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_SetSurvivalPoint(int nInCID, int nInPoint)
{
	try
	{
		otl_stream a(1, "SELECT setsurvivalpoint(:cid<int>, :point<int>);", db, otl_implicit_select);
		a << nInCID << nInPoint;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_SetDTScore(int nInCID, int nInTP, int nInWin, int nInLose, int nInFinalWin)
{
	try
	{
		otl_stream a(1, "SELECT setdtscore(:cid<int>, :tp<int>, :win<int>, :lose<int>, :finalwin<int>);", db, otl_implicit_select);
		a << nInCID << nInTP << nInWin << nInLose << nInFinalWin;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_GetClanInfo(int nInCLID, DbData_ClanInfo *pOutInfo)
{
	try
	{
		otl_stream a(1, "SELECT clid, name, level, point, mastercid, win, lose, draw, totalpoint, "
						"ranking, emblemurl, emblemchecksum FROM getclaninfo(:clid<int>);", db, otl_implicit_select);
		a << nInCLID;
		
		if(a.get_prefetched_row_count() == 0)
			return false;
			
		a >> pOutInfo->nCLID >> pOutInfo->szName >> pOutInfo->nLevel >> pOutInfo->nPoint >> pOutInfo->nMasterCID >> 
		pOutInfo->nWin >> pOutInfo->nLose >> pOutInfo->nDraw >> pOutInfo->nTotalPoint >> pOutInfo->nRanking >> 
		pOutInfo->szEmblemURL >> pOutInfo->nEmblemChecksum;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_IsClanExists(const char *pszInName, int *pOutResult)
{
	if(Db_IsValidString(pszInName) == false) return false;
	
	try
	{
		otl_stream a(1, "SELECT isclanexists(:name<char[24]>);", db, otl_implicit_select);
		a << pszInName;
		
		a >> *pOutResult;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_CreateClan(int nInCID, const char *pszInClanName, int *pOutCLID)
{
	if(Db_IsValidString(pszInClanName) == false) return false;
	
	try
	{
		otl_stream a(1, "SELECT createclan(:cid<int>, :name<char[24]>);", db, otl_implicit_select);
		a << nInCID << pszInClanName;
		
		a >> *pOutCLID;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_JoinClan(int nInCLID, int nInCID)
{
	try
	{
		otl_stream a(1, "SELECT joinclan(:clid<int>, :cid<int>);", db, otl_implicit_select);
		a << nInCLID << nInCID;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_UpdateClanMemberGrade(int nInCLID, int nInCID, int nGrade)
{
	try
	{
		otl_stream a(1, "SELECT updateclanmembergrade(:clid<int>, :cid<int>, :grade<int>);", db, otl_implicit_select);
		a << nInCLID << nInCID << nGrade;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_UpdateClanMasterCID(int nInCLID, int nInCID)
{
	try
	{
		otl_stream a(1, "SELECT updateclanmaster(:clid<int>, :cid<int>);", db, otl_implicit_select);
		a << nInCLID << nInCID;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_LeaveClan(int nInCLID, int nInCID)
{
	try
	{
		otl_stream a(1, "SELECT leaveclan(:clid<int>, :cid<int>);", db, otl_implicit_select);
		a << nInCLID << nInCID;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_CloseClan(int nInCLID)
{
	try
	{
		otl_stream a(1, "SELECT closeclan(:clid<int>);", db, otl_implicit_select);
		a << nInCLID;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_UpdateItemCount(int nInCIID, int nInCount)
{
	try
	{
		otl_stream a(1, "SELECT updateitemquantity(:ciid<int>, :count<int>);", db, otl_implicit_select);
		a << nInCIID << nInCount;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_DeleteSpentItem(int nInCIID)
{
	try
	{
		otl_stream a(1, "SELECT itemspend(:ciid<int>);", db, otl_implicit_select);
		a << nInCIID;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_AddFriend(int nInCID, int nInTargetCID, int *pOutID)
{
	try
	{
		otl_stream a(1, "SELECT addfriend(:cid<int>, :targetcid<int>);", db, otl_implicit_select);
		a << nInCID << nInTargetCID;
		
		a >> *pOutID;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_RemoveFriend(int nInFriendID)
{
	try
	{
		otl_stream a(1, "SELECT removefriend(:id<int>);", db, otl_implicit_select);
		a << nInFriendID;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_GetFriendList(int nInCID, vector<DbData_FriendListNode> *pOut)
{
	pOut->clear();
	
	try
	{
		otl_stream a(40 /*MAX_FRIEND_LIST_COUNT*/, "SELECT targetid_, targetcid_, targetname_ FROM getfriendlist(:cid<int>);", db, otl_implicit_select);
		a << nInCID;
		
		while(a.eof() == 0)
		{
			DbData_FriendListNode node;
			a >> node.nFriendID >> node.nCID >> node.szCharName;
			
			pOut->push_back(node);
		}
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_ClearTargetFriend(int nInTargetCID)
{
	try
	{
		otl_stream a(1, "SELECT clearfriend(:targetcid<int>);", db, otl_implicit_select);
		a << nInTargetCID;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_UpdateCharQuestItem(int nInCID, const char *pszInData)
{
	try
	{
		otl_stream a(1, "SELECT updatequestitem(:cid<int>, :data<char[1024]>);", db, otl_implicit_select);
		a << nInCID << pszInData;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_UpdateBounty(int nInCID, int nInBounty)
{
	try
	{
		otl_stream a(1, "SELECT updatebounty(:cid<int>, :bp<int>);", db, otl_implicit_select);
		a << nInCID << nInBounty;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_GetCharGambleItem(int nInCID, vector<DbData_CharGambleItem> *pOut)
{
	try
	{
		otl_stream a(30, "SELECT cgiid, itemid, quantity FROM getchargambleitem(:cid<int>);", db, otl_implicit_select);
		a << nInCID;
		
		while(a.eof() == 0)
		{
			DbData_CharGambleItem item;
			a >> item.nGIID >> item.nItemID >> item.nCount;
			
			pOut->push_back(item);
		}
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_InsertGambleItem(int nInCID, int nInItemID, int nInCount, int nInBP, int *pOutGIID)
{
	try
	{
		otl_stream a(1, "SELECT insertgambleitem(:cid<int>, :itemid<int>, :count<int>, :bp<int>);", db, otl_implicit_select);
		a << nInCID << nInItemID << nInCount << nInBP;
		
		a >> *pOutGIID;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_UpdateGambleItem(int nInCID, int nInGIID, int nInCount, int nInBP)
{
	try
	{
		otl_stream a(1, "SELECT updategambleitem(:cid<int>, :cgiid<int>, :count<int>, :bp<int>);", db, otl_implicit_select);
		a << nInCID << nInGIID << nInCount << nInBP;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_DeleteGambleItem(int nInCID, int nInGIID, int nInBP)
{
	try
	{
		otl_stream a(1, "SELECT deletegambleitem(:cid<int>, :cgiid<int>, :bp<int>);", db, otl_implicit_select);
		a << nInCID << nInGIID << nInBP;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_DecreaseGambleItem(int nInGIID)
{
	try
	{
		otl_stream a(1, "SELECT decreasegambleitem(:cgiid<int>);", db, otl_implicit_select);
		a << nInGIID;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_GetCQRecord(int nInAID, vector<DbData_CQRecordInfo> *pOut)
{
	try
	{
		otl_stream a(20, "SELECT scenarioid_, time_ FROM getcqrecord(:aid<int>);", db, otl_implicit_select);
		a << nInAID;
		
		while(a.eof() == 0)
		{
			DbData_CQRecordInfo info;
			a >> info.nScenarioID >> info.nTime;
			
			pOut->push_back(info);
		}
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_InsertCQRecord(int nInAID, int nInScenarioID, int nInTime)
{
	try
	{
		otl_stream a(1, "SELECT insertcqrecord(:aid<int>, :sid<int>, :time<int>);", db, otl_implicit_select);
		a << nInAID << nInScenarioID << nInTime;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_GetSurvivalCharInfo(int nInCID, DbData_CharSurvivalInfo *pOutInfo)
{
	ZeroInit(pOutInfo, sizeof(DbData_CharSurvivalInfo));
	
	try
	{
		otl_stream a(1, "SELECT point, ranking FROM getsurvivalcharinfo(:cid<int>);", db, otl_implicit_select);
		a << nInCID;
		
		if(a.get_prefetched_row_count() == 1)
			a >> pOutInfo->nPoint >> pOutInfo->nRanking;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_GetAccountItemList(int nInAID, vector<DbData_AccItemList> *pOut)
{
	try
	{
		otl_stream a(100, "SELECT aiid_, itemid_, period_, count_ FROM getaccitemlist(:aid<int>);", db, otl_implicit_select);
		a << nInAID;
		
		while(a.eof() == 0)
		{
			DbData_AccItemList ai;
			a >> ai.nAIID >> ai.nItemID >> ai.nRentSecPeriod >> ai.nItemCount;
			
			pOut->push_back(ai);
		}
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}

	return true;
}

bool Db_MoveAItemToCItem(int nInAIID, int nInCID, int *pOutCIID)
{
	*pOutCIID = 0;
	
	try
	{
		otl_stream a(1, "SELECT moveaitemtocitem(:aiid<int>, :cid<int>);", db, otl_implicit_select);
		a << nInAIID << nInCID;
		
		if(a.is_null() == false)
			a >> *pOutCIID;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_MoveAItemToCItem(int nInAIID, int nInCID, int nInCount, int *pOutCIID)
{
	*pOutCIID = 0;
	
	try
	{
		otl_stream a(1, "SELECT moveaitemtocitem(:aiid<int>, :cid<int>, :count<int>);", db, otl_implicit_select);
		a << nInAIID << nInCID << nInCount;
		
		if(a.is_null() == false)
			a >> *pOutCIID;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_MoveCItemToAItem(int nInCIID, int nInAID, int *pOutAIID)
{
	*pOutAIID = 0;
	
	try
	{
		otl_stream a(1, "SELECT movecitemtoaitem(:ciid<int>, :aid<int>);", db, otl_implicit_select);
		a << nInCIID << nInAID;
		
		if(a.is_null() == false)
			a >> *pOutAIID;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_MoveCItemToAItem(int nInCIID, int nInAID, int nInCount, int *pOutAIID)
{
	*pOutAIID = 0;
	
	try
	{
		otl_stream a(1, "SELECT movecitemtoaitem(:ciid<int>, :aid<int>, :count<int>);", db, otl_implicit_select);
		a << nInCIID << nInAID << nInCount;
		
		if(a.is_null() == false)
			a >> *pOutAIID;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_SetBlitzScore(int nInCID, int nInWin, int nInLose, int nInPoint, int nInMedal)
{
	try
	{
		otl_stream a(1, "SELECT setblitzscore(:cid<int>, :win<int>, :lose<int>, :point<int>, :medal<int>);", db, otl_implicit_select);
		a << nInCID << nInWin << nInLose << nInPoint << nInMedal;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_GetBlitzScore(int nInCID, DbData_BlitzCharInfo *pOutInfo)
{
	pOutInfo->nWin = pOutInfo->nLose = 0;
	pOutInfo->nPoint = 1000;
	pOutInfo->nMedal = 0;
	
	try
	{
		otl_stream a(1, "SELECT win, lose, point, medal FROM getblitzscore(:cid<int>);", db, otl_implicit_select);
		a << nInCID;
		
		if(a.get_prefetched_row_count() == 1)
			a >> pOutInfo->nWin >> pOutInfo->nLose >> pOutInfo->nPoint >> pOutInfo->nMedal;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_AddBlitzShopItem(int nInCID, int nInItemID, int nInPrice, int nInBasePrice, int nInCount, int nInRentHourPeriod)
{
	try
	{
		otl_stream a(1, "SELECT addblitzshopitem(:cid<int>, :itemid<int>, :price<int>, :baseprice<int>, :count<int>, :renthourperiod<int>);", db, otl_implicit_select);
		a << nInCID << nInItemID << nInPrice << nInBasePrice << nInCount << nInRentHourPeriod;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_GetBlitzShopItem(vector<DbData_BlitzShopItem> *pOut)
{
	try
	{
		otl_stream a(40, "SELECT cid, itemid, price, baseprice, quantity, renthourperiod FROM getblitzshopitem();", db, otl_implicit_select);
		
		while(a.eof() == 0)
		{
			DbData_BlitzShopItem item;
			a >> item.nCID >> item.nItemID >> item.nPrice >> item.nBasePrice >> item.nCount >> item.nRentHourPeriod;
			
			pOut->push_back(item);
		}
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_GetBlitzShopItem(int nInCID, vector<DbData_BlitzShopItem> *pOut)
{
	try
	{
		otl_stream a(40, "SELECT cid, itemid, price, baseprice, quantity, renthourperiod FROM getblitzshopitem(:cid<int>);", db, otl_implicit_select);
		a << nInCID;
		
		while(a.eof() == 0)
		{
			DbData_BlitzShopItem item;
			a >> item.nCID >> item.nItemID >> item.nPrice >> item.nBasePrice >> item.nCount >> item.nRentHourPeriod;
			
			pOut->push_back(item);
		}
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_ClearBlitzShop()
{
	try
	{
		otl_stream a(1, "SELECT clearblitzshop();", db, otl_implicit_select);
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_ClearBlitzShop(int nInCID)
{
	try
	{
		otl_stream a(1, "SELECT clearblitzshop(;cid<int>);", db, otl_implicit_select);
		a << nInCID;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_BuyCashItem(int nInCID, int nInItemID, int nInCount, int nInRentHourPeriod, int *pOutCIID)
{
	try
	{
		otl_stream a(1, "SELECT insertitem(:cid<int>, :itemid<int>, :count<int>, makeperiod(:rentperiod<int>));", db, otl_implicit_select);
		a << nInCID << nInItemID << nInCount << nInRentHourPeriod;
		
		if(a.get_prefetched_row_count() == 0)
			return false;
			
		a >> *pOutCIID;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_SetAccountCash(int nInAID, int nInCash)
{
	try
	{
		otl_stream a(1, "SELECT setaccountcash(:aid<int>, :cash<int>);", db, otl_implicit_select);
		a << nInAID << nInCash;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_SendItemGift(const char *pszInReceiver, const char *pszInSender, const char *pszInMessage, const int *pInItemID, int nInRentHourPeriod, int nInQuantity, int *pOutResult)
{
	if(Db_IsValidString(pszInReceiver) == false || Db_IsValidString(pszInSender) == false) return false;
	if(Db_IsValidString(pszInMessage) == false) return false;
	
	try
	{
		otl_stream a(1, "SELECT sendgift(:receiver<char[24]>, :sender<char[24]>, :message<char[128]>, :itemid1<int>, :itemid2<int>, :itemid3<int>, :itemid4<int>, :itemid5<int>, :rentperiod<int>, :quantity<int>);", db, otl_implicit_select);
		a << pszInReceiver << pszInSender << pszInMessage << pInItemID[0] << pInItemID[1] << pInItemID[2] << pInItemID[3] << pInItemID[4] << nInRentHourPeriod << nInQuantity;
		
		a >> *pOutResult;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_CheckItemGift(int nInCID, vector<DbData_ReceivedGiftItem> *pOut)
{
	try
	{
		otl_stream a(50, "SELECT id_, sender_, message_, itemid1_, itemid2_, itemid3_, itemid4_, itemid5_, renthourperiod_, giftdate_ FROM checkgift(:cid<int>);", db, otl_implicit_select);
		a << nInCID;
		
		while(a.eof() == 0)
		{
			DbData_ReceivedGiftItem item;
			ZeroInit(&item, sizeof(DbData_ReceivedGiftItem));
			
			a >> item.nGiftID >> item.szSenderName >> item.szMessage >> item.nItemID[0] >> item.nItemID[1] >> item.nItemID[2] >> item.nItemID[3] >> item.nItemID[4] >> item.nRentHourPeriod >> item.szGiftDate;
			
			for(int i = 0; i < 5; i++)
			{
				if(item.nItemID[i] != 0) item.nItemNodeCount++;
			}
			
			if(sscanf(item.szGiftDate, "%d %d %d %d", &item.nGiftYear, &item.nGiftMonth, &item.nGiftDay, &item.nGiftHour) != 4)
			{
				item.nGiftYear = item.nGiftMonth = item.nGiftDay = item.nGiftHour = 0;
			}
			
			pOut->push_back(item);
		}
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_AcceptItemGift(int nInID, int nInCID)
{
	try
	{
		otl_stream a(1, "SELECT acceptgift(:id<int>, :cid<int>);", db, otl_implicit_select);
		a << nInID << nInCID;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_BanPlayer(int nInAID)
{
	try
	{
		otl_stream a(1, "SELECT banplayer(:aid<int>);", db, otl_implicit_select);
		a << nInAID;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_IsBannedIP(const char *pszInIPAddr, int *pOutResult)
{
	if(Db_IsValidString(pszInIPAddr) == false)
		return false;
		
	try
	{
		otl_stream a(1, "SELECT isipbanned(:ip<char[24]>);", db, otl_implicit_select);
		a << pszInIPAddr;
		
		a >> *pOutResult;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}

	return true;
}

bool Db_InsertAccount(const char *pszInUserID, const char *pszInPassword, int *pOutResult)
{
	if(Db_IsValidString(pszInUserID) == false || Db_IsValidString(pszInPassword) == false)
		return false;
		
	try
	{
		otl_stream a(1, "SELECT insertaccount(:id<char[128]>, :pw<char[128]>);", db, otl_implicit_select);
		a << pszInUserID << pszInPassword;
		
		if(pOutResult != NULL) a >> *pOutResult;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}

	return true;
}

bool Db_UpdateConnData(int nInAID, const char *pszInIPAddr)
{
	if(Db_IsValidString(pszInIPAddr) == false)
		return false;
		
	try
	{
		otl_stream a(1, "SELECT updatelastconndata(:aid<int>, :ip<char[64]>);", db, otl_implicit_select);
		a << nInAID << pszInIPAddr;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}
	
	return true;
}

bool Db_UpdateIndividualRanking()
{
	try
	{
		otl_stream a(1, "SELECT updateindividualranking();", db, otl_implicit_select);
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}

	return true;
}

bool Db_UpdateSurvivalRanking()
{
	try
	{
		otl_stream a(1, "SELECT updatesurvivalranking();", db, otl_implicit_select);
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}

	return true;
}

bool Db_FetchSurvivalRanking(vector<DbData_SurvivalRankingInfo> *pOut)
{
	try
	{
		otl_stream a(10, "SELECT charname_, point_, ranking_ FROM fetchsurvivalranking();", db, otl_implicit_select);
		
		while(a.eof() == 0)
		{
			DbData_SurvivalRankingInfo info;
			a >> info.szCharName >> info.nPoint >> info.nRanking;
			
			pOut->push_back(info);
		}
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}

	return true;
}

bool Db_UpdateDTDailyRanking()
{
	try
	{
		otl_stream a(1, "SELECT updatedtdailyranking();", db, otl_implicit_select);
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}

	return true;
}

bool Db_UpdateDTWeeklyRanking()
{
	try
	{
		otl_stream a(1, "SELECT updatedtweeklyranking();", db, otl_implicit_select);
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}

	return true;
}

bool Db_FetchDTTopRanking(vector<DbData_DTRankingInfo> *pOut)
{
	try
	{
		otl_stream a(5, "SELECT charname_, tp_, win_, lose_, ranking_, rankingdiff_, finalwin_, class_ FROM fetchdttopranking();", db, otl_implicit_select);
		
		while(a.eof() == 0)
		{
			DbData_DTRankingInfo info;
			a >> info.szCharName >> info.nTP >> info.nWin >> info.nLose >> info.nRanking >> info.nRankingDiff >> info.nFinalWin >> info.nClass;
			
			pOut->push_back(info);
		}
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}

	return true;
}

/*
bool Db_GetServerStatus(vector<MTD_ServerStatusInfo> *pOut)
{
	try
	{
		#if 0
		otl_stream a(1, "SELECT ip, agentip, port, id, maxplayer, currplayer, type, name FROM getserverstatus();", db, otl_implicit_select);
		#else
		otl_stream a(1, "SELECT ip, port, id, maxplayer, currplayer, type, name FROM getserverstatus();", db, otl_implicit_select);
		#endif
		
		while(a.eof() == 0)
		{
			char szIP[64];
			#if 0
			char szAgentIP[64];
			#endif
			int nPort;
			int nServerID;
			int nMaxPlayer;
			int nCurrPlayer;
			int nType;
			char szServerName[64];
			
			#if 0
			a >> szIP >> szAgentIP >> nPort >> nServerID >> nMaxPlayer >> nCurrPlayer >> nType >> szServerName;
			#else
			a >> szIP >> nPort >> nServerID >> nMaxPlayer >> nCurrPlayer >> nType >> szServerName;
			#endif
			
			MTD_ServerStatusInfo info;
			info.nIP = Socket::InetAddr(szIP);
			#if 0
			info.nAgentIP = Socket::InetAddr(szAgentIP);
			#endif
			info.nPort = nPort;
			info.nServerID = (unsigned char)nServerID;
			info.nMaxPlayer = (short)nMaxPlayer;
			info.nCurPlayer = (short)nCurrPlayer;
			info.nType = (char)nType;
			strcpy(info.szServerName, szServerName);
			
			info.bIsLive = true;
			
			pOut->push_back(info);
		}
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}

	return true;
}
*/

bool Db_UpdateServerStatus(int nInServerID, int nInCurrPlayer)
{
	try
	{
		otl_stream a(1, "SELECT updateserverstatus(:id<int>, :player<int>);", db, otl_implicit_select);
		a << nInServerID << nInCurrPlayer;
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------------------------------------

void Db_Error(otl_exception &e)
{
	printf("%s\n", e.msg);
	printf("%s\n", e.stm_text);
	printf("%s\n", e.var_info);
	
	printf("An error occurred while processing SQL.\n");
	
	mlog("[SQL ERROR]");
	mlog("%s", (const char *)e.msg);
	mlog("%s", (const char *)e.stm_text);
	mlog("%s", (const char *)e.var_info);
	
	if(e.code == 26)	// 26 = connection dead.
	{
		printf("Connection to the database server is lost. Reconnecting...\n");
		
		Db_Disconnect();
		
		if(Db_Connect(g_ServerConfig.DatabaseSetting.szUsername,
					g_ServerConfig.DatabaseSetting.szPassword,
					g_ServerConfig.DatabaseSetting.szDSN) == false)
		{
			printf("Failed to reconnect to database.\n");
			return;
		}
		
		printf("Database server reconnected.\n");
	}
}

bool Db_Connect(const char *pszUsername, const char *pszPassword, const char *pszDSN)
{
	char szConn[128];
	sprintf(szConn, "UID=%s;PWD=%s;DSN=%s;MAXVARCHARSIZE=4096", pszUsername, pszPassword, pszDSN);

	otl_connect::otl_initialize(1);
	try
	{
		db.rlogon(szConn, 1);
	}
	catch(otl_exception &e)
	{
		Db_Error(e);
		return false;	// failed.
	}

	return true;	// succeed.
}

void Db_Disconnect()
{
	db.logoff();
}

bool Db_IsValidString(const char *pszStr)
{
	for(int i = 0; pszStr[i] != '\0'; i++)
	{
		if(pszStr[i] == '\'')	 	// find invalid character.
			return false;	// invalid.
	}
	return true;	// valid.
}
