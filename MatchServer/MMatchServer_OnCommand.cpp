#include "pch.h"

#include "MMatchServer_OnCommand.h"
#include "MMatchConstant.h"

#include "MMatchChatRoom.h"

#include "MMatchDBMgr.h"

#include "MMatchObject.h"
#include "MMatchAgent.h"

#include "MClientAcceptor.h"

#include "MMatchBlitzKrieg.h"

bool MMatchServer_OnCommand(MCommandReader *pRecvCmd)
{
#ifdef _SHOW_RECVCMD_ID
	printf("[%lu:%lu] - Command ID %u is received.\n", pRecvCmd->GetOwnerUID().ulHighID, pRecvCmd->GetOwnerUID().ulLowID, pRecvCmd->GetCommandID());
#endif

	switch(pRecvCmd->GetCommandID())
	{

		case MC_MATCH_LOGIN	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			char szUserID[MAX_USERID_LEN];
			char szPassword[MAX_USERPASSWORD_LEN];
			int nCommandVersion;

			pRecvCmd->ReadString(szUserID, MAX_USERID_LEN);
			pRecvCmd->ReadString(szPassword, MAX_USERPASSWORD_LEN);
			pRecvCmd->ReadInt(&nCommandVersion);
			// UInt (Checksum).
			// Blob (MD5).

			if(OnMatchLogin(uidPlayer, szUserID, szPassword, nCommandVersion) == false)
			{
				return false;
			}
		}
		break;
		
		case MC_MATCH_LOGIN_NETMARBLE	:
		{
			int nCommandVersion;
			
			pRecvCmd->ReadStringSkip(1);	// AuthCookie.
			pRecvCmd->ReadStringSkip(1);	// DataCookie.
			pRecvCmd->ReadStringSkip(1);	// CPCookie.
			pRecvCmd->ReadStringSkip(1);	// SpareParam.
			pRecvCmd->ReadInt(&nCommandVersion);
			// UInt (Checksum).
			
			if(OnMatchLogin(pRecvCmd->GetOwnerUID(), "", "", nCommandVersion) == false)
			{
				return false;
			}
		}
		break;
		
		case MC_MATCH_LOGIN_NHNUSA	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			char szUserID[MAX_USERID_LEN];
			char szPassword[MAX_USERPASSWORD_LEN];
			int nCommandVersion;

			pRecvCmd->ReadString(szUserID, MAX_USERID_LEN);
			pRecvCmd->ReadString(szPassword, MAX_USERPASSWORD_LEN);
			pRecvCmd->ReadInt(&nCommandVersion);
			// UInt (Checksum).
			// Blob (MD5).

			if(OnMatchLogin(uidPlayer, szUserID, szPassword, nCommandVersion) == false)
			{
				return false;
			}
		}
		break;
		
		case MC_MATCH_LOGIN_FHL_PERU	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			char szUserID[MAX_USERID_LEN];
			char szPassword[MAX_USERPASSWORD_LEN];
			int nCommandVersion;

			pRecvCmd->ReadString(szUserID, MAX_USERID_LEN);
			pRecvCmd->ReadString(szPassword, MAX_USERPASSWORD_LEN);
			pRecvCmd->ReadInt(&nCommandVersion);
			// UInt (Checksum).
			// Blob (MD5).
			
			if(OnMatchLogin(uidPlayer, szUserID, szPassword, nCommandVersion) == false)
			{
				return false;
			}
		}
		break;

		case MC_MATCH_REQUEST_ACCOUNT_CHARLIST	:
		{
			// Blob (EMsg).
			
			OnAccountCharList(pRecvCmd->GetOwnerUID());
		}
		break;

		case MC_MATCH_REQUEST_ACCOUNT_CHARINFO	:
		{
			char nCharNum;
			pRecvCmd->ReadChar(&nCharNum);

			OnAccountCharInfo(pRecvCmd->GetOwnerUID(), nCharNum);
		}
		break;

		case MC_MATCH_REQUEST_CREATE_CHAR	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			int nCharIndex;
			char szName[CHARNAME_LEN];
			int nSex;
			int nHair;
			int nFace;
			int nCostume;

			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadInt(&nCharIndex);
			pRecvCmd->ReadString(szName, CHARNAME_LEN);
			pRecvCmd->ReadInt(&nSex);
			pRecvCmd->ReadInt(&nHair);
			pRecvCmd->ReadInt(&nFace);
			pRecvCmd->ReadInt(&nCostume);

			OnCreateCharacter(uidPlayer, nCharIndex, szName, nSex, nHair, nFace, nCostume);
		}
		break;

		case MC_MATCH_REQUEST_DELETE_CHAR	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			int nCharIndex;
			char szName[CHARNAME_LEN];

			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadInt(&nCharIndex);
			pRecvCmd->ReadString(szName, CHARNAME_LEN);

			OnDeleteCharacter(uidPlayer, nCharIndex, szName);
		}
		break;

		case MC_MATCH_REQUEST_SELECT_CHAR	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			int nCharIndex;

			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadInt(&nCharIndex);

			OnSelectCharacter(uidPlayer, nCharIndex);
		}
		break;
		
		case MC_MATCH_USER_OPTION	:
		{
			unsigned long nFlag;
			pRecvCmd->ReadULong(&nFlag);
			
			OnUserOption(pRecvCmd->GetOwnerUID(), nFlag);
		}
		break;
		
		case MC_MATCH_REQUEST_APPROPRIATE_CHANNEL	:
		{
			OnAppropriateChannel(pRecvCmd->GetOwnerUID());
		}
		break;
		
		case MC_MATCH_CHANNEL_REQUEST_JOIN	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID(), uidChannel;
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadMUID(&uidChannel);
			
			OnChannelJoin(uidPlayer, uidChannel);
		}
		break;
		
		case MC_MATCH_CHANNEL_REQUEST_INDIVIDUAL_JOIN	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			int nChannelType;
			char szChannelName[32];
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadInt(&nChannelType);
			pRecvCmd->ReadString(szChannelName, SIZEOFA(szChannelName));
			
			OnPrivateChannelJoin(uidPlayer, nChannelType, szChannelName);
		}
		break;
		
		case MC_MATCH_CHANNEL_REQUEST_PLAYER_LIST	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID(), uidChannel;
			int nPage;
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadMUID(&uidChannel);
			pRecvCmd->ReadInt(&nPage);
			
			OnChannelPlayerList(uidPlayer, uidChannel, nPage, false);
		}
		break;
		
		case MC_MATCH_CHANNEL_REQUEST_CHAT	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID(), uidChannel;
			char szChat[1024];
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadMUID(&uidChannel);
			pRecvCmd->ReadString(szChat, SIZEOFA(szChat));
			
			OnChannelChat(uidPlayer, uidChannel, szChat);
		}
		break;
		
		case MC_MATCH_CHANNEL_LIST_START	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			int nChannelType;
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadInt(&nChannelType);
			
			OnChannelListStart(uidPlayer, nChannelType);
		}
		break;
		
		case MC_MATCH_CHANNEL_LIST_STOP	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			pRecvCmd->ReadSkip(sizeof(MUID));
			
			OnChannelListStop(uidPlayer);
		}
		break;
		
		case MC_MATCH_USER_WHISPER	:
		{
			char szTargetName[CHARNAME_LEN], szMessage[256];
			
			pRecvCmd->ReadStringSkip(1);	// Sender name.
			pRecvCmd->ReadString(szTargetName, CHARNAME_LEN);
			pRecvCmd->ReadString(szMessage, SIZEOFA(szMessage));
			
			OnUserWhisper(pRecvCmd->GetOwnerUID(), szTargetName, szMessage);
		}
		break;
		
		case MC_MATCH_STAGE_FOLLOW	:
		{
			char szTargetName[CHARNAME_LEN];
			pRecvCmd->ReadString(szTargetName, CHARNAME_LEN);
			
			OnStageFollow(pRecvCmd->GetOwnerUID(), szTargetName);
		}
		break;
		
		case MC_MATCH_USER_WHERE	:
		{
			char szTargetName[CHARNAME_LEN];
			pRecvCmd->ReadString(szTargetName, CHARNAME_LEN);
			
			OnUserWhere(pRecvCmd->GetOwnerUID(), szTargetName);
		}
		break;
		
		case MC_MATCH_REQUEST_SHOP_ITEMLIST	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			// int - FirstItemIndex.
			// int - ItemCount.
			
			OnShopItemList(uidPlayer);
		}
		break;
		
		case MC_MATCH_REQUEST_CHARACTER_ITEMLIST	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			pRecvCmd->ReadSkip(sizeof(MUID));
			
			OnCharacterItemList(uidPlayer);
		}
		break;
		
		case MC_MATCH_REQUEST_BUY_ITEM	:
		{
			if(CheckGameVersion(2012) == true)
			{
				unsigned int nCurrencyType;	// 1 = bounty, 2 = cash, 3 = medal.
				unsigned int nItemID, nItemCount;
				char szGiftName[CHARNAME_LEN], szGiftMsg[64];
				unsigned int nRentHourPeriod;
				
				pRecvCmd->ReadUInt(&nCurrencyType);
				pRecvCmd->ReadUInt(&nItemID);
				pRecvCmd->ReadUInt(&nItemCount);
				pRecvCmd->ReadString(szGiftName, CHARNAME_LEN);
				pRecvCmd->ReadString(szGiftMsg, SIZEOFA(szGiftMsg));
				pRecvCmd->ReadUInt(&nRentHourPeriod);
				pRecvCmd->ReadSkip(sizeof(bool));	// ?.
				
				OnBuyItem(pRecvCmd->GetOwnerUID(), (int)nCurrencyType, (int)nItemID, (int)nItemCount, szGiftName, szGiftMsg, (int)nRentHourPeriod);
			}
			else
			{
				MUID uidPlayer = pRecvCmd->GetOwnerUID();
				unsigned int nItemID, nItemCount;
			
				pRecvCmd->ReadSkip(sizeof(MUID));
				pRecvCmd->ReadUInt(&nItemID);
				pRecvCmd->ReadUInt(&nItemCount);
			
				OnBuyItem(uidPlayer, (int)nItemID, (int)nItemCount);
			}
		}
		break;
		
		case MC_MATCH_REQUEST_SELL_ITEM	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID(), uidItem;
			unsigned int nItemCount;
			
			if(CheckGameVersion(2012) == false)
				pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadMUID(&uidItem);
			pRecvCmd->ReadUInt(&nItemCount);
			
			OnSellItem(uidPlayer, uidItem, (int)nItemCount);
		}
		break;
		
		case MC_MATCH_REQUEST_EQUIP_ITEM	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID(), uidItem;
			int nSlot;
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadMUID(&uidItem);
			pRecvCmd->ReadInt(&nSlot);
			
			OnEquipItem(uidPlayer, uidItem, nSlot);
		}
		break;
		
		case MC_MATCH_REQUEST_TAKEOFF_ITEM	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			int nSlot;
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadInt(&nSlot);
			
			OnTakeoffItem(uidPlayer, nSlot);
		}
		break;
		
		case MC_MATCH_REQUEST_ACCOUNT_ITEMLIST	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			pRecvCmd->ReadSkip(sizeof(MUID));
			
			OnAccountItemList(uidPlayer);
		}
		break;
		
		case MC_MATCH_REQUEST_BRING_ACCOUNTITEM	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			int nAIID, nItemID, nItemCount;
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadInt(&nAIID);
			pRecvCmd->ReadInt(&nItemID);
			pRecvCmd->ReadInt(&nItemCount);
			
			OnTakeOutItemFromStorage(uidPlayer, nAIID, nItemID, nItemCount);
		}
		break;
		
		case MC_MATCH_REQUEST_BRING_BACK_ACCOUNTITEM	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID(), uidItem;
			unsigned int nItemCount;
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadMUID(&uidItem);
			pRecvCmd->ReadUInt(&nItemCount);
			
			OnDepositItemToStorage(uidPlayer, uidItem, nItemCount);
		}
		break;
		
		case MC_MATCH_REQUEST_STAGE_LIST	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID(), uidChannel;
			int nStageCursor;
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadMUID(&uidChannel);
			pRecvCmd->ReadInt(&nStageCursor);
			
			OnStageList(uidPlayer, uidChannel, nStageCursor, false);
		}
		break;
		
		case MC_MATCH_STAGE_CREATE	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			char szStageName[MAX_STAGE_NAME_LENGTH], szStagePassword[MAX_STAGE_PASSWORD_LENGTH];
			bool bPrivate;
			
			if(CheckGameVersion(2012) == false)
				pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadString(szStageName, MAX_STAGE_NAME_LENGTH);
			pRecvCmd->ReadBool(&bPrivate);
			pRecvCmd->ReadString(szStagePassword, MAX_STAGE_PASSWORD_LENGTH);
			
			OnCreateStage(uidPlayer, szStageName, szStagePassword, bPrivate);
		}
		break;
		
		case MC_MATCH_REQUEST_STAGE_JOIN	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID(), uidStage;
			
			if(CheckGameVersion(2012) == false)
				pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadMUID(&uidStage);
			
			OnStageJoin(uidPlayer, uidStage);
		}
		break;
		
		case MC_MATCH_REQUEST_PRIVATE_STAGE_JOIN	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID(), uidStage;
			char szStagePassword[MAX_STAGE_PASSWORD_LENGTH];
			
			if(CheckGameVersion(2012) == false)
				pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadMUID(&uidStage);
			pRecvCmd->ReadString(szStagePassword, MAX_STAGE_PASSWORD_LENGTH);
			
			OnPrivateStageJoin(uidPlayer, uidStage, szStagePassword);
		}
		break;
		
		case MC_MATCH_REQUEST_STAGESETTING	:
		{
			MUID uidStage;
			pRecvCmd->ReadMUID(&uidStage);
			
			OnStageSetting(pRecvCmd->GetOwnerUID(), uidStage);
		}
		break;
		
		case MC_MATCH_STAGE_LEAVE	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			pRecvCmd->ReadSkip(sizeof(MUID));
			
			OnStageLeave(uidPlayer);
		}
		break;
		
		case MC_MATCH_STAGE_CHAT	:
		{
			/*
			MUID uidPlayer = pRecvCmd->GetOwnerUID(), uidStage;
			char szChat[256];
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadMUID(&uidStage);
			pRecvCmd->ReadString(szChat, SIZEOFA(szChat));
			
			OnStageChat(uidPlayer, uidStage, szChat);
			*/
			
			char szChat[256];
			
			pRecvCmd->ReadSkip(sizeof(MUID));	// Client UID.
			if(CheckGameVersion(2012) == false)
				pRecvCmd->ReadSkip(sizeof(MUID));	// Stage UID.				
			pRecvCmd->ReadString(szChat, SIZEOFA(szChat));
			
			OnStageChat(pRecvCmd->GetOwnerUID(), szChat);
		}
		break;
		
		case MC_MATCH_STAGE_KICK	:
		{
			// added in 2012, instead of '/kick [name]' using stage chat.
			
			char szCharName[CHARNAME_LEN];
			pRecvCmd->ReadString(szCharName, SIZEOFA(szCharName));
			
			OnStageKick(pRecvCmd->GetOwnerUID(), szCharName);
		}
		break;
		
		case MC_MATCH_STAGESETTING	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID(), uidStage;
			MSTAGE_SETTING_NODE node;
			
			int nNodeSize, nNodeCount;
			
			if(CheckGameVersion(2012) == false)
				pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadMUID(&uidStage);
			if(pRecvCmd->ReadBlobArray(&nNodeCount, &nNodeSize) == false) return false;
			
			if(nNodeSize != sizeof(MSTAGE_SETTING_NODE))
			{
				return false;
			}
			
			if(nNodeCount != 1)
			{
				return false;
			}
			
			if(pRecvCmd->ReadData(&node, sizeof(MSTAGE_SETTING_NODE)) == false)
			{
				return false;
			}
			
			// set last string to NULL for safe.
			node.szMapName[MAX_STAGE_MAPNAME_LENGTH - 1] = '\0';
			
			OnChangeStageSetting(uidPlayer, uidStage, &node);
		}
		break;
		
		case MC_MATCH_STAGE_MAP	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID(), uidStage;
			char szMapName[MAX_STAGE_MAPNAME_LENGTH];
			
			pRecvCmd->ReadMUID(&uidStage);
			pRecvCmd->ReadString(szMapName, MAX_STAGE_MAPNAME_LENGTH);
			
			OnStageMap(uidPlayer, uidStage, szMapName);
		}
		break;
		
		case MC_MATCH_STAGE_RELAY_MAP_ELEMENT_UPDATE	:
		{
			MUID uidStage;
			int nType, nRepeat;
			
			pRecvCmd->ReadMUID(&uidStage);
			pRecvCmd->ReadInt(&nType);
			pRecvCmd->ReadInt(&nRepeat);
			
			OnRelayMapElementUpdate(pRecvCmd->GetOwnerUID(), uidStage, nType, nRepeat);
		}
		break;
		
		case MC_MATCH_STAGE_RELAY_MAP_INFO_UPDATE	:
		{
			MUID uidStage;
			int nType, nRepeat;
			MTD_RelayMap MapList[MAX_RELAYMAP_ELEMENT_COUNT];
			
			pRecvCmd->ReadMUID(&uidStage);
			pRecvCmd->ReadInt(&nType);
			pRecvCmd->ReadInt(&nRepeat);
			
			int nSize, nCount;
			if(pRecvCmd->ReadBlobArray(&nCount, &nSize) == false) return false;
			
			if(nSize != sizeof(MTD_RelayMap))
			{
				return false;
			}
			
			if(nCount > MAX_RELAYMAP_ELEMENT_COUNT)
			{
				return false;
			}
			
			if(pRecvCmd->ReadData(MapList, sizeof(MTD_RelayMap) * nCount) == false)
			{
				return false;
			}
			
			OnRelayMapInfoUpdate(pRecvCmd->GetOwnerUID(), uidStage, nType, nRepeat, MapList, nCount);
		}
		break;
		
		case MC_MATCH_STAGE_TEAM	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID(), uidStage;
			int nTeam;
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadMUID(&uidStage);
			pRecvCmd->ReadInt(&nTeam);
			
			OnStageTeam(uidPlayer, uidStage, nTeam);
		}
		break;
		
		case MC_MATCH_STAGE_PLAYER_STATE	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID(), uidStage;
			int nState;
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadMUID(&uidStage);
			pRecvCmd->ReadInt(&nState);
			
			OnStageState(uidPlayer, uidStage, nState);
		}
		break;
		
		case MC_MATCH_STAGE_START	:
		{
			/*
			MUID uidPlayer = pRecvCmd->GetOwnerUID(), uidStage;
			int nCountdown;
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadMUID(&uidStage);
			pRecvCmd->ReadInt(&nCountdown);
			
			OnStageStart(uidPlayer, uidStage, nCountdown);
			*/
			
			/*
			 ~2012 : 
			    Player UID (MUID).
			    Stage UID (MUID).
			    Countdown (Int).
			 
			 2013~ : 
			    Countdown (Int).
			*/
			
			OnStageStart(pRecvCmd->GetOwnerUID(), 0);
		}
		break;
		
		case MC_MATCH_LOADING_COMPLETE	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			int nPercent;
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadInt(&nPercent);
			
			OnLoadingComplete(uidPlayer, nPercent);
		}
		break;
		
		case MC_MATCH_GAME_REQUEST_TIMESYNC	:
		{
			unsigned long nLocalTimeStamp;
			pRecvCmd->ReadULong(&nLocalTimeStamp);
			
			OnTimeSync(pRecvCmd->GetOwnerUID(), nLocalTimeStamp);
		}
		break;
		
		case MC_MATCH_GAME_REPORT_TIMESYNC	:
		{
			unsigned long nLocalTimeStamp, nDataChecksum;
			
			pRecvCmd->ReadULong(&nLocalTimeStamp);
			pRecvCmd->ReadULong(&nDataChecksum);
			
			OnReportTimeSync(pRecvCmd->GetOwnerUID(), nLocalTimeStamp, nDataChecksum);
		}
		break;
		
		case MC_MATCH_STAGE_REQUEST_ENTERBATTLE	:
		{
			MUID uidStage;
			
			if(CheckGameVersion(2012) == false)
				pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadMUID(&uidStage);
			
			OnStageEnterBattle(pRecvCmd->GetOwnerUID(), uidStage);
		}
		break;
		
		case MC_MATCH_REQUEST_PEERLIST	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID(), uidStage;
			
			if(CheckGameVersion(2012) == false)
				pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadMUID(&uidStage);
			
			OnPeerList(uidPlayer, uidStage);
		}
		break;
		
		case MC_MATCH_STAGE_LEAVEBATTLE_TO_SERVER	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			bool bFinishLeave;
			
			if(CheckGameVersion(2012) == false)
				pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadBool(&bFinishLeave);
			
			OnStageLeaveBattle(uidPlayer, bFinishLeave);
		}
		break;
		
		case MC_MATCH_STAGE_REQUEST_FORCED_ENTRY	:
		{
			/*
			MUID uidPlayer = pRecvCmd->GetOwnerUID(), uidStage;
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadMUID(&uidStage);
			
			OnForceEntry(uidPlayer, uidStage);
			*/
			
			OnForceEntry(pRecvCmd->GetOwnerUID());
		}
		break;
		
		case MC_MATCH_REQUEST_GAME_INFO	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID(), uidStage;
			
			if(CheckGameVersion(2012) == false)
				pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadMUID(&uidStage);
			
			OnGameInfo(uidPlayer, uidStage);
		}
		break;
		
		case MC_MATCH_GAME_REQUEST_SPAWN	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			FloatVector pos, dir;
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadVec(&pos.x, &pos.y, &pos.z);
			pRecvCmd->ReadVec(&dir.x, &dir.y, &dir.z);
			
			OnGameSpawn(uidPlayer, &pos, &dir);
		}
		break;
		
		case MC_MATCH_REQUEST_SPAWN_WORLDITEM	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			int nItemID;
			FloatVector pos;
			float fDropDelayTime;
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadInt(&nItemID);
			pRecvCmd->ReadVec(&pos.x, &pos.y, &pos.z);
			pRecvCmd->ReadFloat(&fDropDelayTime);
			
			OnSpawnWorldItem(uidPlayer, nItemID, &pos, fDropDelayTime);
		}
		break;
		
		case MC_MATCH_REQUEST_OBTAIN_WORLDITEM	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			int nItemUID;
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadInt(&nItemUID);
			
			OnObtainWorldItem(uidPlayer, nItemUID);
		}
		break;
		
		case MC_MATCH_GAME_KILL	:
		{
			MUID uidAttacker, uidVictim = pRecvCmd->GetOwnerUID();
			pRecvCmd->ReadMUID(&uidAttacker);
			
			OnGameKill(uidAttacker, uidVictim);
		}
		break;
		
		case MC_MATCH_REQUEST_SUICIDE	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			pRecvCmd->ReadSkip(sizeof(MUID));
			
			OnGameSuicide(uidPlayer);
		}
		break;
		
		case MC_MATCH_REQUEST_MY_SIMPLE_CHARINFO	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			pRecvCmd->ReadSkip(sizeof(MUID));
			
			OnMySimpleCharInfo(uidPlayer);
		}
		break;
		
		case MC_MATCH_STAGE_REQUEST_QUICKJOIN	:
		{
			if(CheckGameVersion(2012) == true)
			{
				MTD_QuickJoinParam qjparam;
				
				pRecvCmd->ReadULong(&qjparam.nMapEnum);
				pRecvCmd->ReadULong(&qjparam.nModeEnum);
				
				OnStageQuickJoin(pRecvCmd->GetOwnerUID(), &qjparam);
			}
			else
			{
				MUID uidPlayer = pRecvCmd->GetOwnerUID();
				MTD_QuickJoinParam qjparam;
			
				pRecvCmd->ReadSkip(sizeof(MUID));
			
				int nBlobCount, nBlobSize;
				pRecvCmd->ReadBlobArray(&nBlobCount, &nBlobSize);
			
				if(nBlobSize != sizeof(MTD_QuickJoinParam))
				{
					return false;
				}
			
				if(nBlobCount != 1)
				{
					return false;
				}
			
				if(pRecvCmd->ReadData(&qjparam, sizeof(MTD_QuickJoinParam)) == false)
				{
					return false;
				}
			
				OnStageQuickJoin(uidPlayer, &qjparam);
			}
		}
		break;
		
		case MC_MATCH_STAGE_GO	:
		{
			unsigned int nStageNo;
			pRecvCmd->ReadUInt(&nStageNo);
			
			OnStageGo(pRecvCmd->GetOwnerUID(), (int)nStageNo);
		}
		break;
		
		case MC_MATCH_CLAN_REQUEST_CREATE_CLAN	:
		{
			char szClanName[CLANNAME_LEN];
			pRecvCmd->ReadString(szClanName, CLANNAME_LEN);
			
			OnCreateClan(pRecvCmd->GetOwnerUID(), szClanName);
		}
		break;
		
		case MC_MATCH_CLAN_REQUEST_JOIN_CLAN	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			char szClanName[CLANNAME_LEN], szJoinerName[CHARNAME_LEN];
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadString(szClanName, CLANNAME_LEN);
			pRecvCmd->ReadString(szJoinerName, CHARNAME_LEN);
			
			OnJoinClan(uidPlayer, szClanName, szJoinerName);
		}
		break;
		
		case MC_MATCH_CLAN_ANSWER_JOIN_AGREEMENT	:
		{
			MUID uidClanAdmin;
			char szJoinerName[CHARNAME_LEN];
			bool bAnswer;
			
			pRecvCmd->ReadMUID(&uidClanAdmin);
			pRecvCmd->ReadString(szJoinerName, CHARNAME_LEN);
			pRecvCmd->ReadBool(&bAnswer);
			
			OnAnswerJoinClanAgreement(pRecvCmd->GetOwnerUID(), uidClanAdmin, szJoinerName, bAnswer);
		}
		break;
		
		case MC_MATCH_CLAN_REQUEST_AGREED_JOIN_CLAN	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			char szClanName[CLANNAME_LEN], szJoinerName[CHARNAME_LEN];
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadString(szClanName, CLANNAME_LEN);
			pRecvCmd->ReadString(szJoinerName, CHARNAME_LEN);
			
			OnJoinClanAgreed(uidPlayer, szClanName, szJoinerName);
		}
		break;
		
		case MC_MATCH_CLAN_MASTER_REQUEST_CHANGE_GRADE	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			char szMemberName[CHARNAME_LEN];
			int nGrade;
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadString(szMemberName, CHARNAME_LEN);
			pRecvCmd->ReadInt(&nGrade);
			
			OnChangeClanMemberGrade(uidPlayer, szMemberName, nGrade);
		}
		break;
		
		case MC_MATCH_CLAN_ADMIN_REQUEST_EXPEL_MEMBER	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			char szMemberName[CHARNAME_LEN];
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadString(szMemberName, CHARNAME_LEN);
			
			OnExpelClanMember(uidPlayer, szMemberName);
		}
		break;
		
		case MC_MATCH_CLAN_REQUEST_LEAVE_CLAN	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			pRecvCmd->ReadSkip(sizeof(MUID));
			
			OnLeaveClan(uidPlayer);
		}
		break;
		
		case MC_MATCH_CLAN_REQUEST_CLOSE_CLAN	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			char szClanName[CLANNAME_LEN];
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadString(szClanName, CLANNAME_LEN);
			
			OnCloseClan(uidPlayer, szClanName);
		}
		break;
		
		case MC_MATCH_CLAN_REQUEST_MEMBER_LIST	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			pRecvCmd->ReadSkip(sizeof(MUID));
			
			OnClanMemberList(uidPlayer);
		}
		break;
		
		case MC_MATCH_CLAN_REQUEST_MSG	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			char szMsg[256];
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadString(szMsg, SIZEOFA(szMsg));
			
			OnClanMsg(uidPlayer, szMsg);
		}
		break;
		
		case MC_MATCH_CLAN_REQUEST_CLAN_INFO	:
		{
			/*
			P(MPT_UID, "uidChar")
			P(MPT_STR, "ClanName")
			*/
		}
		break;
		
		case MC_MATCH_CLAN_REQUEST_EMBLEMURL	:
		{
			int nNodeSize, nNodeCount;
			if(pRecvCmd->ReadBlobArray(&nNodeCount, &nNodeSize) == false) return false;
			
			if(nNodeSize != sizeof(int)) return false;
			if(nNodeCount <= 0 || nNodeCount > 24) return false;
			
			vector<int> vtCLID;
			
			for(int i = 0; i < nNodeCount; i++)
			{
				int nCLID;
				if(pRecvCmd->ReadData(&nCLID, sizeof(int)) == false) return false;
				
				vtCLID.push_back(nCLID);
			}
			
			OnClanEmblemURL(pRecvCmd->GetOwnerUID(), &vtCLID);
		}
		break;
		
		case MC_MATCH_FRIEND_LIST	:
		{
			OnFriendList(pRecvCmd->GetOwnerUID());
		}
		break;
		
		case MC_MATCH_FRIEND_ADD	:
		{
			char szCharName[CHARNAME_LEN];
			pRecvCmd->ReadString(szCharName, CHARNAME_LEN);
			
			OnFriendAdd(pRecvCmd->GetOwnerUID(), szCharName);
		}
		break;

		case MC_MATCH_FRIEND_REMOVE	:
		{
			char szCharName[CHARNAME_LEN];
			pRecvCmd->ReadString(szCharName, CHARNAME_LEN);
			
			OnFriendRemove(pRecvCmd->GetOwnerUID(), szCharName);
		}
		break;
		
		case MC_MATCH_CALLVOTE	:
		{
			char szDiscuss[256], szArg[256];
			
			pRecvCmd->ReadString(szDiscuss, SIZEOFA(szDiscuss));
			pRecvCmd->ReadString(szArg, SIZEOFA(szArg));
			
			OnCallvote(pRecvCmd->GetOwnerUID(), szDiscuss, szArg);
		}
		break;
		
		#if _GAME_VERSION >= 2012
		case MC_MATCH_VOTE	:
		{
			bool bAgreed;
			pRecvCmd->ReadBool(&bAgreed);
			
			OnVote(pRecvCmd->GetOwnerUID(), bAgreed);
		}
		break;
		#else
		case MC_MATCH_VOTE_YES	:
		{
			OnVoteYes(pRecvCmd->GetOwnerUID());
		}
		break;
		
		case MC_MATCH_VOTE_NO	:
		{
			OnVoteNo(pRecvCmd->GetOwnerUID());
		}
		break;
		#endif
		
		case MC_MATCH_REQUEST_USE_SPENDABLE_NORMAL_ITEM	:
		{
			MUID uidItem;
			pRecvCmd->ReadMUID(&uidItem);
			
			OnUseNormalSpendableItem(pRecvCmd->GetOwnerUID(), uidItem);
		}
		break;
		
		case MC_MATCH_CHATROOM_CREATE	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			char szChatRoomName[MAX_CHATROOM_NAME_LENGTH];
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadString(szChatRoomName, MAX_CHATROOM_NAME_LENGTH);
			
			OnChatRoomCreate(uidPlayer, szChatRoomName);
		}
		break;
		
		case MC_MATCH_CHATROOM_JOIN	:
		{
			char szChatRoomName[MAX_CHATROOM_NAME_LENGTH];
			
			pRecvCmd->ReadStringSkip(1);	// player name.
			pRecvCmd->ReadString(szChatRoomName, MAX_CHATROOM_NAME_LENGTH);
			
			OnChatRoomJoin(pRecvCmd->GetOwnerUID(), szChatRoomName);
		}
		break;
		
		case MC_MATCH_CHATROOM_LEAVE	:
		{
			char szChatRoomName[MAX_CHATROOM_NAME_LENGTH];
			
			pRecvCmd->ReadStringSkip(1);	// player name.
			pRecvCmd->ReadString(szChatRoomName, MAX_CHATROOM_NAME_LENGTH);
			
			OnChatRoomLeave(pRecvCmd->GetOwnerUID(), szChatRoomName);
		}
		break;
		
		case MC_MATCH_CHATROOM_SELECT_WRITE	:
		{
			char szChatRoomName[MAX_CHATROOM_NAME_LENGTH];
			pRecvCmd->ReadString(szChatRoomName, MAX_CHATROOM_NAME_LENGTH);
			
			OnChatRoomSelect(pRecvCmd->GetOwnerUID(), szChatRoomName);
		}
		break;
		
		case MC_MATCH_CHATROOM_INVITE	:
		{
			char szTargetName[CHARNAME_LEN];
			
			pRecvCmd->ReadStringSkip(1);	// sender name.
			pRecvCmd->ReadString(szTargetName, CHARNAME_LEN);
			pRecvCmd->ReadStringSkip(1);	// chat room name.
			
			OnChatRoomInvite(pRecvCmd->GetOwnerUID(), szTargetName);
		}
		break;
		
		case MC_MATCH_CHATROOM_CHAT	:
		{
			char szMessage[256];
			
			pRecvCmd->ReadStringSkip(1);	// chat room name.
			pRecvCmd->ReadStringSkip(1);	// sender name.
			pRecvCmd->ReadString(szMessage, SIZEOFA(szMessage));
			
			OnChatRoomChat(pRecvCmd->GetOwnerUID(), szMessage);
		}
		break;
		
		case MC_MATCH_REQUEST_CHAR_QUEST_ITEMLIST	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			pRecvCmd->ReadSkip(sizeof(MUID));
			
			OnCharQuestItemList(uidPlayer);
		}
		break;
		
		case MC_MATCH_REQUEST_BUY_QUEST_ITEM	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			int nItemID, nItemCount;
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadInt(&nItemID);
			pRecvCmd->ReadInt(&nItemCount);
			
			OnBuyQuestItem(uidPlayer, nItemID, nItemCount);
		}
		break;
		
		case MC_MATCH_REQUEST_SELL_QUEST_ITEM	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			int nItemID, nItemCount;
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadInt(&nItemID);
			pRecvCmd->ReadInt(&nItemCount);
			
			OnSellQuestItem(uidPlayer, nItemID, nItemCount);
		}
		break;
		
		case MC_MATCH_REQUEST_SLOT_INFO	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			pRecvCmd->ReadSkip(sizeof(MUID));
			
			OnQuestSlotInfo(uidPlayer);
		}
		break;
		
		case MC_QUEST_REQUEST_LEVEL	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			pRecvCmd->ReadSkip(sizeof(MUID));
			
			OnQuestLevel(uidPlayer);
		}
		break;
		
		case MC_MATCH_REQUEST_DROP_SACRIFICE_ITEM	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			int nSlotIndex, nItemID;
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadInt(&nSlotIndex);
			pRecvCmd->ReadInt(&nItemID);
			
			OnQuestDropSacrificeItem(uidPlayer, nSlotIndex, nItemID);
		}
		break;
		
		case MC_MATCH_REQUEST_CALLBACK_SACRIFICE_ITEM	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			int nSlotIndex, nItemID;
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadInt(&nSlotIndex);
			pRecvCmd->ReadInt(&nItemID);
			
			OnQuestCallbackSacrificeItem(uidPlayer, nSlotIndex, nItemID);
		}
		break;
		
		case MC_MATCH_QUEST_REQUEST_DEAD	:
		{
			OnQuestPlayerDead(pRecvCmd->GetOwnerUID());
		}
		break;
		
		case MC_QUEST_REQUEST_NPC_DEAD	:
		{
			MUID uidKiller, uidNPC;
			ShortVector vPos;
			
			pRecvCmd->ReadMUID(&uidKiller);
			pRecvCmd->ReadMUID(&uidNPC);
			pRecvCmd->ReadSVec(&vPos.x, &vPos.y, &vPos.z);
			
			OnQuestNPCDead(pRecvCmd->GetOwnerUID(), uidKiller, uidNPC, &vPos);
		}
		break;
		
		case MC_QUEST_REQUEST_MOVETO_PORTAL	:
		{
			char nSectorIndex;
			pRecvCmd->ReadChar(&nSectorIndex);
			
			OnMoveToPortal(pRecvCmd->GetOwnerUID(), nSectorIndex);
		}
		break;
		
		case MC_QUEST_READYTO_NEWSECTOR	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			pRecvCmd->ReadSkip(sizeof(MUID));
			
			OnReadyToNewSector(uidPlayer);
		}
		break;
		
		case MC_MATCH_DUELTOURNAMENT_REQUEST_SIDERANKING_INFO	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			pRecvCmd->ReadSkip(sizeof(MUID));
			
			OnDuelTournamentRankingInfo(uidPlayer);
		}
		break;
		
		case MC_MATCH_DUELTOURNAMENT_REQUEST_JOINGAME	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			int nTournamentType;
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadInt(&nTournamentType);
			
			OnDuelTournamentJoin(uidPlayer, nTournamentType);
		}
		break;
		
		case MC_MATCH_DUELTOURNAMENT_REQUEST_CANCELGAME	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			int nTournamentType;
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadInt(&nTournamentType);
			
			OnDuelTournamentCancel(uidPlayer, nTournamentType);
		}
		break;
		
		case MC_MATCH_DUELTOURNAMENT_GAME_PLAYER_STATUS	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			float fDamaged, fHP, fAP;
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadFloat(&fDamaged);
			pRecvCmd->ReadFloat(&fHP);
			pRecvCmd->ReadFloat(&fAP);
			
			OnDuelTournamentGamePlayerStatus(uidPlayer, fDamaged, fHP, fAP);
		}
		break;
		
		case MC_CHALLENGE_QUEST_REQUEST_TOP_RECORD	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			pRecvCmd->ReadSkip(sizeof(MUID));
			
			OnChallengeQuestBestRecord(uidPlayer);
		}
		break;
		
		case MC_CHALLENGE_QUEST_REQUEST_NPC_DEAD	:
		{
			MUID uidKiller, uidNPC;
			ShortVector vPos;
			
			pRecvCmd->ReadMUID(&uidKiller);
			pRecvCmd->ReadMUID(&uidNPC);
			pRecvCmd->ReadSVec(&vPos.x, &vPos.y, &vPos.z);
			
			OnChallengeQuestNPCDead(pRecvCmd->GetOwnerUID(), uidKiller, uidNPC, &vPos);
		}
		break;
		
		case MC_CHALLENGE_QUEST_PLAYER_REQUEST_DEAD	:
		{
			OnChallengeQuestPlayerDead(pRecvCmd->GetOwnerUID());
		}
		break;
		
		case MC_CHALLENGE_QUEST_PLAYER_REQUEST_NPC_SPAWN	:
		{
			MUID uidOwnerNPC;
			char szActorName[256];
			FloatVector vPos, vDir;
			
			pRecvCmd->ReadMUID(&uidOwnerNPC);
			pRecvCmd->ReadString(szActorName, SIZEOFA(szActorName));
			pRecvCmd->ReadVec(&vPos.x, &vPos.y, &vPos.z);
			pRecvCmd->ReadVec(&vDir.x, &vDir.y, &vDir.z);
			
			OnChallengeQuestNPCSpawn(pRecvCmd->GetOwnerUID(), uidOwnerNPC, szActorName, &vPos, &vDir);
		}
		break;
		
		case MC_MATCH_REQUEST_GAMBLE	:
		{
			MUID uidItem;
			pRecvCmd->ReadMUID(&uidItem);
			
			OnOpenGambleItem(pRecvCmd->GetOwnerUID(), uidItem);
		}
		break;
		
		case MC_BLITZ_REQUEST_MATCH_COLLABORATOR_LIST	:
		{
			OnBlitzMatchCollaboratorList(pRecvCmd->GetOwnerUID());
		}
		break;
		
		case MC_BLITZ_REQUEST_RESERVE_CHALLENGE	:
		{
			pRecvCmd->ReadSkip(sizeof(int));	// ...?
			pRecvCmd->ReadSkip(sizeof(int));	// ...?
			
			int nNodeSize, nNodeCount;
			if(pRecvCmd->ReadBlobArray(&nNodeCount, &nNodeSize) == false) return false;
			
			if(nNodeSize != sizeof(MTD_BlitzChallengerNode)) return false;
			if(nNodeCount <= 0 || nNodeCount > BLITZ_TEAM_PLAYER_COUNT) return false;
			
			vector<MTD_BlitzChallengerNode> vtChallengers;
			
			for(int i = 0; i < nNodeCount; i++)
			{
				MTD_BlitzChallengerNode node;
				if(pRecvCmd->ReadData(&node, sizeof(MTD_BlitzChallengerNode)) == false) return false;
				
				node.szPlayerName[CHARNAME_LEN - 1] = '\0';	// for safe.
				
				vtChallengers.push_back(node);
			}
			
			OnBlitzReserveChallenge(pRecvCmd->GetOwnerUID(), &vtChallengers);
		}
		break;
		
		case MC_BLITZ_REQUEST_CANCEL_CHALLENGE	:
		{
			OnBlitzCancelChallenge(pRecvCmd->GetOwnerUID());
		}
		break;
		
		case MC_BLITZ_CHOOSE_CLASS	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			unsigned int nClass;
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadUInt(&nClass);
			
			OnBlitzChooseClass(uidPlayer, nClass);
		}
		break;
		
		case MC_BLITZ_PLAYER_REQUEST_DEAD	:
		{
			MUID uidAttacker;
			pRecvCmd->ReadMUID(&uidAttacker);
			
			vector<MTD_BlitzAssisterInfoNode> vtAssisters;
			
			int nNodeSize, nNodeCount;
			if(pRecvCmd->ReadBlobArray(&nNodeCount, &nNodeSize) == true)
			{
				if(nNodeSize != sizeof(MTD_BlitzAssisterInfoNode)) return false;
				if(nNodeCount <= 0 || nNodeCount > BLITZ_MAX_ASSISTANT) return false;
				
				for(int i = 0; i < nNodeCount; i++)
				{
					MTD_BlitzAssisterInfoNode node;
					if(pRecvCmd->ReadData(&node, sizeof(MTD_BlitzAssisterInfoNode)) == false) return false;
					
					if(node.uidAssister == MUID(0, 0)) continue;
					
					vtAssisters.push_back(node);
				}
			}
			
			OnBlitzPlayerDead(uidAttacker, pRecvCmd->GetOwnerUID(), &vtAssisters);
		}
		break;
		
		case MC_BLITZ_REQUEST_NPC_DEAD	:
		{
			MUID uidKiller, uidNPC;
			
			pRecvCmd->ReadMUID(&uidKiller);
			pRecvCmd->ReadMUID(&uidNPC);
			
			OnBlitzNPCDead(pRecvCmd->GetOwnerUID(), uidKiller, uidNPC);
		}
		break;
		
		case MC_BLITZ_PLAYER_REQUEST_SKILL_UPGRADE	:
		{
			int nSkillType;
			pRecvCmd->ReadInt(&nSkillType);
			
			OnBlitzSkillUpgrade(pRecvCmd->GetOwnerUID(), nSkillType);
		}
		break;
		
		case MC_MATCH_REQUEST_PROPOSAL	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID();
			int nProposalMode;
			int nRequestID;
			int nResponderCount;
			vector<MTD_ReplierNode> vtResponders;
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadInt(&nProposalMode);
			pRecvCmd->ReadInt(&nRequestID);
			pRecvCmd->ReadInt(&nResponderCount);
			
			int nNodeCount, nNodeSize;
			if(pRecvCmd->ReadBlobArray(&nNodeCount, &nNodeSize) == false) return false;
			
			if(nNodeSize != sizeof(MTD_ReplierNode))
			{
				return false;
			}
			
			if((nNodeCount <= 0 || nNodeCount > MAX_PROPOSAL_REPLIER_COUNT) || (nNodeCount != nResponderCount))
			{
				return false;
			}
			
			for(int i = 0; i < nNodeCount; i++)
			{
				MTD_ReplierNode node;
				if(pRecvCmd->ReadData(&node, sizeof(MTD_ReplierNode)) == false) return false;
				
				// for safe, last character must be NULL.
				node.szName[CHARNAME_LEN - 1] = '\0';
				
				vtResponders.push_back(node);
			}
			
			OnLadderProposal(uidPlayer, nProposalMode, nRequestID, vtResponders);
		}
		break;
		
		case MC_MATCH_REPLY_AGREEMENT	:
		{
#if _GAME_VERSION >= 2012
			MUID uidProposer, uidResponder = pRecvCmd->GetOwnerUID();
			char szResponderName[CHARNAME_LEN];
			vector<MTD_ReplierNode> vtResponders;
			int nProposalMode;
			int nRequestID;
			bool bAnswer;
			bool bBusy;
			
			pRecvCmd->ReadMUID(&uidProposer);
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadString(szResponderName, CHARNAME_LEN);
			
			int nNodeCount, nNodeSize;
			if(pRecvCmd->ReadBlobArray(&nNodeCount, &nNodeSize) == false) return false;
			
			if(nNodeSize != sizeof(MTD_ReplierNode))
			{
				return false;
			}
			
			if(nNodeCount <= 0 || nNodeCount > MAX_PROPOSAL_REPLIER_COUNT)
			{
				return false;
			}
			
			for(int i = 0; i < nNodeCount; i++)
			{
				MTD_ReplierNode node;
				if(pRecvCmd->ReadData(&node, sizeof(MTD_ReplierNode)) == false) return false;
				
				node.szName[CHARNAME_LEN - 1] = '\0';
				
				vtResponders.push_back(node);
			}
			
			pRecvCmd->ReadInt(&nProposalMode);
			pRecvCmd->ReadInt(&nRequestID);
			pRecvCmd->ReadBool(&bAnswer);
			pRecvCmd->ReadBool(&bBusy);
			
			OnLadderProposalAgreement(uidProposer, uidResponder, szResponderName, vtResponders, nProposalMode, nRequestID, bAnswer, bBusy);
#else
			MUID uidProposer, uidResponder = pRecvCmd->GetOwnerUID();
			char szResponderName[CHARNAME_LEN];
			int nProposalMode;
			int nRequestID;
			bool bAnswer;
			
			pRecvCmd->ReadMUID(&uidProposer);
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadString(szResponderName, CHARNAME_LEN);
			pRecvCmd->ReadInt(&nProposalMode);
			pRecvCmd->ReadInt(&nRequestID);
			pRecvCmd->ReadBool(&bAnswer);
			
			OnLadderProposalAgreement(uidProposer, uidResponder, szResponderName, nProposalMode, nRequestID, bAnswer);
#endif
		}
		break;
		
		case MC_SPY_STAGE_BAN_MAP	:
		{
			int nMapID;
			bool bBanned;
			
			pRecvCmd->ReadInt(&nMapID);
			pRecvCmd->ReadBool(&bBanned);
			
			OnSpyActivateMap(pRecvCmd->GetOwnerUID(), nMapID, bBanned);
		}
		break;
		
		case MC_SPY_STAGE_REQUEST_START	:
		{
			int nNodeSize, nNodeCount;
			if(pRecvCmd->ReadBlobArray(&nNodeCount, &nNodeSize) == false) return false;
			
			if(nNodeSize != sizeof(int)) return false;
			if(nNodeCount <= 0 || nNodeCount > 30) return false;
			
			vector<int> vtMapList;
			
			for(int i = 0; i < nNodeCount; i++)
			{
				int nMapID;
				if(pRecvCmd->ReadData(&nMapID, sizeof(int)) == false) return false;
				
				vtMapList.push_back(nMapID);
			}
			
			OnSpyStageStart(pRecvCmd->GetOwnerUID(), vtMapList);
		}
		break;
		
		case MC_CASHSHOP_REQUEST_ITEMLIST	:
		{
			unsigned char nSex, nSortType;
			
			pRecvCmd->ReadUChar(&nSex);
			pRecvCmd->ReadUChar(&nSortType);
			
			OnCashShopItemList(pRecvCmd->GetOwnerUID(), (int)nSex, (int)nSortType);
		}
		break;
		
		case MC_CASHSHOP_REQUEST_ITEMINFO	:
		{
			unsigned char nSex, nSortType;
			int nStartIndex, nCount;
			
			pRecvCmd->ReadUChar(&nSex);
			pRecvCmd->ReadUChar(&nSortType);
			pRecvCmd->ReadInt(&nStartIndex);
			pRecvCmd->ReadInt(&nCount);
			
			OnCashShopItemInfo(pRecvCmd->GetOwnerUID(), (int)nSex, (int)nSortType, nStartIndex, nCount);
		}
		break;
		
		case MC_CASHSHOP_REQUEST_CASH_REMAINDER	:
		{
			OnSyncCashRemainder(pRecvCmd->GetOwnerUID());
		}
		break;
		
		case MC_MATCH_CHECK_GIFTITEM_LIST	:
		{
			OnCheckGiftItem(pRecvCmd->GetOwnerUID());
		}
		break;
		
		case MC_MATCH_READ_GIFTITEM_INFO	:
		{
			int nNodeSize, nNodeCount;
			if(pRecvCmd->ReadBlobArray(&nNodeCount, &nNodeSize) == false) return false;
			
			if(nNodeSize != sizeof(int)) return false;
			if(nNodeCount <= 0 || nNodeCount > 50) return false;
			
			vector<int> vtGiftID;
			
			for(int i = 0; i < nNodeCount; i++)
			{
				int nGiftID;
				if(pRecvCmd->ReadData(&nGiftID, sizeof(int)) == false) return false;
				
				vtGiftID.push_back(nGiftID);
			}
			
			OnReadGiftItem(pRecvCmd->GetOwnerUID(), vtGiftID);
		}
		break;
		
		case MC_MATCH_REQUEST_PEER_RELAY	:
		{
			MUID uidPlayer = pRecvCmd->GetOwnerUID(), uidPeer;
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadMUID(&uidPeer);
			
			OnPeerRelay(uidPlayer, uidPeer);
		}
		break;
		
		case MC_MATCH_REGISTERAGENT	:
		{
			char szIP[64];
			int nPort, nUDPPort;
			
			pRecvCmd->ReadString(szIP, SIZEOFA(szIP));
			pRecvCmd->ReadInt(&nPort);
			pRecvCmd->ReadInt(&nUDPPort);
			
			OnAgentRegister(pRecvCmd->GetOwnerUID(), szIP, nPort, nUDPPort);
		}
		break;
		
		case MC_MATCH_UNREGISTERAGENT	:
		{
			OnAgentUnregister(pRecvCmd->GetOwnerUID());
		}
		break;
		
		case MC_AGENT_STAGE_READY	:
		{
			MUID uidStage;
			pRecvCmd->ReadMUID(&uidStage);
			
			OnAgentStageReady(pRecvCmd->GetOwnerUID(), uidStage);
		}
		break;
		
		case MC_AGENT_PEER_READY	:
		{
			MUID uidPlayer, uidPeer;
			
			pRecvCmd->ReadMUID(&uidPlayer);
			pRecvCmd->ReadMUID(&uidPeer);
			
			OnAgentPeerReady(pRecvCmd->GetOwnerUID(), uidPlayer, uidPeer);
		}
		break;
		
		case MC_MATCH_AGENT_REQUEST_LIVECHECK	:
		{
			unsigned long nTimestamp;
			unsigned int nStageCount, nUserCount;
			
			pRecvCmd->ReadULong(&nTimestamp);
			pRecvCmd->ReadUInt(&nStageCount);
			pRecvCmd->ReadUInt(&nUserCount);
			
			OnLiveCheck(pRecvCmd->GetOwnerUID(), nTimestamp, nStageCount, nUserCount);
		}
		break;
		
		case MC_ADMIN_ANNOUNCE	:
		{
			MUID uidAdmin = pRecvCmd->GetOwnerUID();
			char szMsg[256];
			int nMsgType;
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadString(szMsg, SIZEOFA(szMsg));
			pRecvCmd->ReadInt(&nMsgType);
			
			OnAdminAnnounce(uidAdmin, szMsg, nMsgType);
		}
		break;
		
		case MC_ADMIN_SERVER_HALT	:
		{
			MUID uidAdmin = pRecvCmd->GetOwnerUID();
			pRecvCmd->ReadSkip(sizeof(MUID));
			
			OnAdminHalt(uidAdmin);
		}
		break;
		
		case MC_ADMIN_REQUEST_KICK_PLAYER	:
		{
			char szCharName[CHARNAME_LEN];
			pRecvCmd->ReadString(szCharName, CHARNAME_LEN);
			
			OnAdminKick(pRecvCmd->GetOwnerUID(), szCharName);
		}
		break;
		
		case MC_ADMIN_HIDE	:
		{
			OnAdminHide(pRecvCmd->GetOwnerUID());
		}
		break;
		
		case MC_ADMIN_ASSASSIN	:
		{
			OnAdminAssassin(pRecvCmd->GetOwnerUID());
		}
		break;
		
		default	:
		{
#ifdef _SHOW_RECVCMD_ID
			printf("[%lu:%lu] - Unknown Command ID %u.\n", pRecvCmd->GetOwnerUID().ulHighID, pRecvCmd->GetOwnerUID().ulLowID, pRecvCmd->GetCommandID());
#endif
		}
		break;
		
	}

	return true;
}

deque<MCommandReader *> g_CommandQueue;

void AddCommandToQueue(MCommandReader *pCmd)
{
	g_CommandQueue.push_back(pCmd);
}

void DeleteQueueCommand(MCommandReader *pCmd)
{
	DestroyReaderCommand(pCmd);
}

bool CheckCommandValidation(MCommandReader *pCmd)
{
	const MUID &uidSender = pCmd->GetOwnerUID();
	
	if(g_MatchAgent.IsAgentUID(uidSender) == true) return true;
	
	MMatchObject *pSenderObj = g_ObjectMgr.Get(uidSender);
	if(pSenderObj == NULL)
	{
		// guess not logged-in yet. expect login command (but only once).
		
		unsigned short nCmdID = pCmd->GetCommandID();
		
		if(nCmdID == MC_MATCH_LOGIN || 
			nCmdID == MC_MATCH_LOGIN_NETMARBLE || 
			nCmdID == MC_MATCH_LOGIN_NHNUSA || 
			nCmdID == MC_MATCH_LOGIN_FHL_PERU || 
			nCmdID == MC_MATCH_REGISTERAGENT)
		{
			bool *pIsLoggedIn = g_ClientAcceptor.GetClientLoginState(pCmd->GetOwnerUID());
			if(pIsLoggedIn == NULL) return false;	// is it disconnected object?
			
			if(*pIsLoggedIn == false)
			{
				*pIsLoggedIn = true;
				return true;
			}
		}
	}
	else
	{
		// already logged-in. doing flood check is better than blocking multiple-login.
		
		// returning flood check result.
		return pSenderObj->UpdateCmdRecvTime();
	}
	
	return false;
}

void MCommandProcess()
{
	while(g_CommandQueue.empty() == false)
	{
		MCommandReader *pCmd = g_CommandQueue.front();
		
		if(CheckCommandValidation(pCmd) == true)
		{
			MMatchServer_OnCommand(pCmd);
		}
		else
		{
			// flooding client will be d/c'd by updater thread.
			
			// if exists, invalidate acceptor client.
			g_ClientAcceptor.InvalidateClient(pCmd->GetOwnerUID());
				
			#ifdef _SHOW_RECVCMD_ID
			printf("[%lu:%lu] - Command ID %u is ignored.\n", pCmd->GetOwnerUID().ulHighID, pCmd->GetOwnerUID().ulLowID, pCmd->GetCommandID());
			#endif
		}
		
		DeleteQueueCommand(pCmd);
		
		g_CommandQueue.pop_front();
	}
}

bool MCommandAdd(vector<MCommandReader *> *pCmdList)	// built separate command from list.
{
	for(vector<MCommandReader *>::iterator i = pCmdList->begin(); i != pCmdList->end(); i++)
	{
		AddCommandToQueue(*i);
	}
	
	pCmdList->clear();
	return true;
}