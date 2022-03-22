#include "pch.h"

#include "MMatchObject.h"
#include "MMatchChannel.h"

#include "MMatchConstant.h"
#include "MMatchObject_Constant.h"

#include "MMessageID.h"

#include "MMatchClan.h"

void SendChannelPlayerList(const MUID &uidChannel);
void SendChannelList();

void OnStageList(const MUID &uidPlayer, const MUID &uidChannel, int nStageCursor, bool bChecksum = true);	// from stage command.

void OnAppropriateChannel(const MUID &uidPlayer)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	MUID uidResult = MUID(0, 0);
	
	for(list<MMatchChannel *>::iterator i = g_ChannelMgr.Begin(); i != g_ChannelMgr.End(); i++)
	{
		MMatchChannel *pChannel = (*i);
		
		if(pChannel->GetCurrPlayers() == 0)
		{
			uidResult = pChannel->GetUID();
			break;
		}
		else
		{
			float fRatio = (float)pChannel->GetMaxPlayers() / (float)pChannel->GetCurrPlayers();
			if(fRatio > 1.1f)
			{
				uidResult = pChannel->GetUID();
				break;
			}
		}
	}
	
	if(uidResult != MUID(0, 0))
	{
		MCmdWriter Cmd;
		Cmd.WriteMUID(uidResult);
		Cmd.Finalize(MC_MATCH_RESPONSE_APPROPRIATE_CHANNEL, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
	}
}

void OnChannelJoin(const MUID &uidPlayer, const MUID &uidChannel)
{
	if(uidChannel == MUID(0, 0)) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;	// error, tried to join a channel without char info initialized.
	
	// join to a new channel.
	MMatchChannel *pChannel = g_ChannelMgr.Get(uidChannel);
	if(pChannel == NULL) return;	// error, channel not exist.
	
	MCmdWriter Cmd;
	
	if(pChannel->IsMax() == true)
	{
		// error, max players reached.
		Cmd.WriteInt(MERR_CHANNEL_MAXPLAYERS_REACHED);
		Cmd.Finalize(MC_MATCH_RESPONSE_RESULT, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	// same channel, = old channel and new channel is same.
	bool bSameChannel = pObj->m_uidChannel == uidChannel ? true : false;
	
	if(bSameChannel == false)
	{
		// leave from a previous channel.
		MMatchChannel *pPrevChannel = g_ChannelMgr.Get(pObj->m_uidChannel);
		if(pPrevChannel != NULL)
		{
			pPrevChannel->Leave(pObj);
			// SendChannelPlayerList(pObj->m_uidChannel);
			
			if(pPrevChannel->GetCurrPlayers() == 0)
			{
				if(pPrevChannel->GetType() == MCHANNEL_TYPE_USER || pPrevChannel->GetType() == MCHANNEL_TYPE_CLAN)
				{
					g_ChannelMgr.Remove(pPrevChannel);
				}
			}
			else
			{
				SendChannelPlayerList(pObj->m_uidChannel);
			}
		}

		// enter channel.
		pObj->m_uidChannel = uidChannel;
		pChannel->Enter(pObj);
	}
	
	Cmd.WriteMUID(uidChannel);
	Cmd.WriteInt(pChannel->GetType());
	Cmd.WriteString(pChannel->GetName());
	Cmd.WriteBool(true);	// interface enabled.
	Cmd.Finalize(MC_MATCH_CHANNEL_RESPONSE_JOIN);
	
	if(CheckGameVersion(2012) == true)
	{
		Cmd.WriteMUID(uidChannel);
		Cmd.WriteInt(5);	// TODO : for 2012, this is ID from ChannelRule.xml.
		Cmd.Finalize(MC_MATCH_CHANNEL_RESPONSE_RULE);
	}
	else
	{
		Cmd.WriteMUID(uidChannel);
		Cmd.WriteString(pChannel->GetRule());
		Cmd.Finalize(MC_MATCH_CHANNEL_RESPONSE_RULE);
	}
	
	Cmd.Finalize();
	SendToClient(&Cmd, uidPlayer);
	
	pObj->m_nPlace = MMP_LOBBY;
	
	if(bSameChannel == false)
	{
		SendChannelPlayerList(uidChannel);
		SendChannelList();
	}
	
	OnStageList(uidPlayer, uidChannel, pObj->m_nLastStageListPage, false);
}

void OnPrivateChannelJoin(const MUID &uidPlayer, int nChannelType, const char *pszChannelName)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	MUID uidChannel = MUID(0, 0);
	
	if(nChannelType == MCHANNEL_TYPE_BLITZKRIEG)
	{
		// transfar new blitz user to the blitzkrieg channel.
		vector<MMatchChannel *> vtBlitzChannels;
		g_ChannelMgr.Enum(MCHANNEL_ENUMTYPE_BLITZKRIEG, &vtBlitzChannels);
		
		for(vector<MMatchChannel *>::iterator i = vtBlitzChannels.begin(); i != vtBlitzChannels.end(); i++)
		{
			MMatchChannel *pCurr = (*i);
			
			if(pCurr->IsMax() == false)
			{
				uidChannel = pCurr->GetUID();
				break;
			}
		}
	}
	else
	{
		g_ChannelMgr.Add(pszChannelName, nChannelType, &uidChannel);
	}
	
	if(uidChannel == MUID(0, 0)) return;
	
	OnChannelJoin(uidPlayer, uidChannel);
}

void OnChannelPlayerList(const MUID &uidPlayer, const MUID &uidChannel, int nPage, bool bChecksum)
{
	if(uidChannel == MUID(0, 0)) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	if(pObj->m_uidChannel != uidChannel) return;
	
	MMatchChannel *pChannel = g_ChannelMgr.Get(uidChannel);
	if(pChannel == NULL) return;
	
	if(nPage < 0 || (nPage * MAX_CHANNEL_PLAYERLIST_COUNT) > pChannel->GetMaxPlayers()) nPage = 0;
	
	MChannelPlayerList list;
	pChannel->RetrievePlayerList(&list, nPage);
	
	if(bChecksum == true)
	{
		if(list.nChecksum == pObj->m_nLastChannelPlayerListChecksum) return;
	}
	pObj->m_nLastChannelPlayerListChecksum = list.nChecksum;
	
	MCmdWriter Cmd;
	Cmd.WriteUChar((unsigned char)pChannel->GetCurrPlayers());
	Cmd.WriteUChar((unsigned char)nPage);
	
	Cmd.StartBlob(sizeof(MTD_ChannelPlayerListNode));
	for(int i = 0; i < list.nTotalCount; i++)
	{
		MMatchObject *pListObj = list.pObject[i];
		
		char szClanName[DB_CLANNAME_LEN] = "";
		unsigned int nEmblemChecksum = 0;
		
		MMatchClan *pClan = g_ClanMgr.Get(pListObj->m_Clan.nCLID);
		if(pClan != NULL)
		{
			strcpy(szClanName, pClan->GetName());
			nEmblemChecksum = pClan->GetEmblemChecksum();
		}
		
		MTD_ChannelPlayerListNode node;
		ZeroInit(&node, sizeof(MTD_ChannelPlayerListNode));
		
		node.uidPlayer = pListObj->GetUID();
		strcpy(node.szName, pListObj->m_Char.szName);
		strcpy(node.szClanName, szClanName);
		node.nLevel = (char)pListObj->m_Exp.nLevel;
		node.nDTLastWeekGrade = (char)pListObj->m_DuelTournament.nClass;
		node.nPlace = pListObj->m_nPlace;
		node.nGrade = (unsigned char)pListObj->m_Account.nUGradeID;
		node.nPlayerFlags = (unsigned char)pListObj->m_nPlayerFlag;
		node.nCLID = (unsigned int)pListObj->m_Clan.nCLID;
		node.nEmblemChecksum = (unsigned int)nEmblemChecksum;
		
		Cmd.WriteData(&node, sizeof(MTD_ChannelPlayerListNode));
	}
	Cmd.EndBlob();
	
	Cmd.Finalize(MC_MATCH_CHANNEL_RESPONSE_PLAYER_LIST, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
	
	pObj->m_nLastChannelPlayerListPage = nPage;
}

void OnChannelChat(const MUID &uidPlayer, const MUID &uidChannel, const char *pszChat)
{
	if(uidChannel == MUID(0, 0)) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	if(pObj->m_uidChannel != uidChannel) return;
	
	MCmdWriter Cmd;
	
	if(CheckGameVersion(2012) == true)
	{
		unsigned int nUGradeID = (unsigned int)pObj->m_Account.nUGradeID;
		if(nUGradeID == 0)
		{
			nUGradeID |= (unsigned int)(pObj->m_BlitzKrieg.nPoint << 16);
		}
		
		Cmd.WriteMUID(uidChannel);
		Cmd.WriteString(pObj->m_Char.szName);
		Cmd.WriteString(pszChat);
		Cmd.WriteUInt(nUGradeID);
		Cmd.WriteMUID(MUID(0, 0));
		Cmd.Finalize(MC_MATCH_CHANNEL_CHAT);
	}
	else
	{
		Cmd.WriteMUID(uidChannel);
		Cmd.WriteString(pObj->m_Char.szName);
		Cmd.WriteString(pszChat);
		Cmd.WriteInt(pObj->m_Account.nUGradeID);
		Cmd.Finalize(MC_MATCH_CHANNEL_CHAT);
	}
	
	Cmd.Finalize();
	SendToChannel(&Cmd, uidChannel);
}

void OnChannelListStart(const MUID &uidPlayer, int nChannelType)
{
	if(IsValidChannelType(nChannelType) == false) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	// classic and blitz channel is in preset.
	if(nChannelType == MCHANNEL_TYPE_CLASSIC || nChannelType == MCHANNEL_TYPE_BLITZKRIEG)
	{
		nChannelType = MCHANNEL_TYPE_PRESET;
	}
	
	pObj->m_nRequestedChannelListType = nChannelType;
	
	SendChannelList();
}

void OnChannelListStop(const MUID &uidPlayer)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	pObj->m_nRequestedChannelListType = -1;
}


void SendChannelPlayerList(const MUID &uidChannel)
{
	for(list<MMatchObject *>::iterator i = g_ObjectMgr.Begin(); i != g_ObjectMgr.End(); i++)
	{
		MMatchObject *pCurrObj = (*i);
		
		if(pCurrObj->m_uidChannel == uidChannel)
			if(pCurrObj->m_nPlace == MMP_LOBBY)
				OnChannelPlayerList(pCurrObj->GetUID(), uidChannel, pCurrObj->m_nLastChannelPlayerListPage, true);
	}
}

void CreateChannelListCommand(unsigned long nChannelFlag, MCmdWriter *pOut)
{
	vector<MMatchChannel *> ChannelList;
	g_ChannelMgr.Enum(nChannelFlag, &ChannelList);
	
	pOut->ResetIndex();
	
	pOut->StartBlob(sizeof(MCHANNELLISTNODE));
	
	int nIndex = 1;
	for(vector<MMatchChannel *>::iterator i = ChannelList.begin(); i != ChannelList.end(); i++)
	{
		MMatchChannel *pChannel = (*i);
		
		MCHANNELLISTNODE node;
		ZeroInit(&node, sizeof(MCHANNELLISTNODE));
		
		node.uidChannel = pChannel->GetUID();
		node.nNo = (short)nIndex;
		node.nPlayers = (unsigned char)pChannel->GetCurrPlayers();
		node.nMaxPlayers = (short)pChannel->GetMaxPlayers();
		node.nLevelMin = 0;
		node.nLevelMax = 0;
		node.nChannelType = (char)pChannel->GetType();
		strcpy(node.szChannelName, pChannel->GetName());
		strcpy(node.szChannelNameStrResId, "");
		node.bIsUseTicket = false;
		node.nTicketID = 0;
		
		pOut->WriteData(&node, sizeof(MCHANNELLISTNODE));
		
		nIndex++;
	}
	
	pOut->EndBlob();
	pOut->Finalize(MC_MATCH_CHANNEL_LIST, MCFT_END);
}

void SendChannelList()
{
	MCmdWriter Cmd;
	
	// preset. (includes classic and blitz channel.)
	CreateChannelListCommand(MCHANNEL_ENUMTYPE_PRESET | MCHANNEL_ENUMTYPE_CLASSIC | MCHANNEL_ENUMTYPE_BLITZKRIEG, &Cmd);
	SendToChannelListRequester(&Cmd, MCHANNEL_TYPE_PRESET);
	
	// user.
	CreateChannelListCommand(MCHANNEL_ENUMTYPE_USER, &Cmd);
	SendToChannelListRequester(&Cmd, MCHANNEL_TYPE_USER);
	
	// clan.
	CreateChannelListCommand(MCHANNEL_ENUMTYPE_CLAN, &Cmd);
	SendToChannelListRequester(&Cmd, MCHANNEL_TYPE_CLAN);
	
	// duel tournament.
	CreateChannelListCommand(MCHANNEL_ENUMTYPE_DUELTOURNAMENT, &Cmd);
	SendToChannelListRequester(&Cmd, MCHANNEL_TYPE_DUELTOURNAMENT);
}
