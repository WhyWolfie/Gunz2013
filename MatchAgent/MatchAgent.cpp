#include "pch.h"

#include "MatchAgent.h"
#include "main.h"

#define SIZEOFA(v)	( sizeof(v) / sizeof(v[0]) )

MAgentClientManager g_ClientMgr;
MStageAgentManager g_StageMgr;

// class.  ------------------------------
MAgentClient::MAgentClient(const MUID &uidClient, const MUID &uidStage, const char *pszIP, unsigned short nPort)
{
	m_uidClient = uidClient;
	m_uidStage = uidStage;
	
	SetAddress(pszIP, nPort);
	
	m_nPeerType = MACPT_IDLE;
}

void MAgentClient::SetAddress(const char *pszIP, unsigned short nPort)
{
	strcpy(m_szIP, pszIP);
	m_nIP = Socket::InetAddr(pszIP);
	m_nPort = nPort;
}

MStageAgent::MStageAgent(const MUID &uidStage)
{
	m_uidStage = uidStage;
}

void MStageAgent::Enter(MAgentClient *pClient)
{
	Leave(pClient);
	m_Users.push_back(pClient);
}

void MStageAgent::Leave(MAgentClient *pClient)
{
	for(list<MAgentClient *>::iterator i = m_Users.begin(); i != m_Users.end(); i++)
	{
		if((*i)->GetUID() == pClient->GetUID())
		{
			m_Users.erase(i);
			break;
		}
	}
}

MAgentClient *MStageAgent::GetClient(MAgentClient *pClient)
{
	for(list<MAgentClient *>::iterator i = m_Users.begin(); i != m_Users.end(); i++)
	{
		MAgentClient *pCurr = (*i);
		
		if(pCurr->GetUID() == pClient->GetUID())
		{
			return pCurr;
		}
	}
	
	return NULL;
}
// ------------------------------

// managers. ------------------------------
// stage.
MStageAgentManager::MStageAgentManager()
{
}

MStageAgentManager::~MStageAgentManager()
{
	for(list<MStageAgent *>::iterator i = m_Stages.begin(); i != m_Stages.end(); i++)
	{
		delete (*i);
	}
	m_Stages.clear();
}

void MStageAgentManager::Remove(const MUID &uidStage)
{
	for(list<MStageAgent *>::iterator i = m_Stages.begin(); i != m_Stages.end(); i++)
	{
		MStageAgent *pCurr = (*i);
		
		if(pCurr->GetUID() == uidStage)
		{
			delete pCurr;
			m_Stages.erase(i);
			break;
		}
	}
}

MStageAgent *MStageAgentManager::Add(const MUID &uidStage)
{
	Remove(uidStage);
	
	MStageAgent *pNew = new MStageAgent(uidStage);
	m_Stages.push_back(pNew);
	return pNew;
}

MStageAgent *MStageAgentManager::Get(const MUID &uidStage)
{
	for(list<MStageAgent *>::iterator i = m_Stages.begin(); i != m_Stages.end(); i++)
	{
		MStageAgent *pCurr = (*i);
		
		if(pCurr->GetUID() == uidStage)
		{
			return pCurr;
		}
	}
	
	return NULL;
}

// client.
MAgentClientManager::MAgentClientManager()
{
}

MAgentClientManager::~MAgentClientManager()
{
	for(list<MAgentClient *>::iterator i = m_Clients.begin(); i != m_Clients.end(); i++)
	{
		delete (*i);
	}
	m_Clients.clear();
}

void MAgentClientManager::Remove(const MUID &uidClient)
{
	for(list<MAgentClient *>::iterator i = m_Clients.begin(); i != m_Clients.end(); i++)
	{
		MAgentClient *pCurr = (*i);
		
		if(pCurr->GetUID() == uidClient)
		{
			delete pCurr;
			m_Clients.erase(i);
			break;
		}
	}
}

MAgentClient *MAgentClientManager::Add(const MUID &uidClient, const MUID &uidStage, const char *pszIP, unsigned short nPort)
{
	Remove(uidClient);
	
	MAgentClient *pNew = new MAgentClient(uidClient, uidStage, pszIP, nPort);
	m_Clients.push_back(pNew);
	return pNew;
}

MAgentClient *MAgentClientManager::Get(const MUID &uidClient)
{
	for(list<MAgentClient *>::iterator i = m_Clients.begin(); i != m_Clients.end(); i++)
	{
		MAgentClient *pCurr = (*i);
		
		if(pCurr->GetUID() == uidClient)
		{
			return pCurr;
		}
	}
	
	return NULL;
}

MUID MAgentClientManager::FindUID(unsigned long nIP, unsigned short nPort)
{
	for(list<MAgentClient *>::iterator i = m_Clients.begin(); i != m_Clients.end(); i++)
	{
		MAgentClient *pCurr = (*i);
		if(pCurr->GetIP() == nIP && pCurr->GetPort() == nPort) return pCurr->GetUID();
	}
	
	return MUID(0, 0);
}

// misc. ------------------------------
//   - to server.
void SendToServer(MCommandWriter *pCmd)
{
	if(pCmd->IsReadyToUse() == false) return;
	
	const int nIndex = pCmd->GetDataIndex();
	unsigned char *pNewData = new unsigned char[nIndex];
	
	PacketEncrypter(pCmd, pNewData, g_ServerCommInfo.nCryptKey);
	
	Socket::Send(g_ClientSocket, (const char *)pNewData, nIndex);
	
	delete[] pNewData;
}

//   - to peer.
void SendToAgentStage(const unsigned char *pData, const int nSize, const MUID &uidStage)
{
	unsigned char *pNewData = new unsigned char[nSize];
	
	// success only raw data.
	if(PacketEncrypter(pData, nSize, pNewData) == false) return;
	
	for(list<MAgentClient *>::iterator i = g_ClientMgr.Begin(); i != g_ClientMgr.End(); i++)
	{
		MAgentClient *pCurr = (*i);
		
		if(pCurr->GetStageUID() == uidStage)
		{
			if(pCurr->GetPeerType() != MACPT_UDP)
			{
				// the player is not ready yet.
				continue;
			}
			
			Socket::address_type addr;
			Socket::MakeAddress(&addr, pCurr->GetIPStr(), pCurr->GetPort());
			
			Socket::SendTo(g_PeerSocket, (const char *)pNewData, nSize, &addr);
		}
	}
	
	delete[] pNewData;
}

void SendToAgentStage(MCommandWriter *pCmd, const MUID &uidStage)
{
	if(pCmd->IsReadyToUse() == false) return;
	SendToAgentStage(pCmd->GetData(), pCmd->GetDataIndex(), uidStage);
}

//   - to peer with receiver.
void SendToAgentUser(const unsigned char *pData, const int nSize, const MUID &uidReceiver)
{
	MAgentClient *pClient = g_ClientMgr.Get(uidReceiver);
	if(pClient == NULL) return;
	
	unsigned char *pNewData = new unsigned char[nSize];
	
	// success only raw data.
	if(PacketEncrypter(pData, nSize, pNewData) == false) return;
	
	if(pClient->GetPeerType() == MACPT_UDP)	// is player ready?
	{
		Socket::address_type addr;
		Socket::MakeAddress(&addr, pClient->GetIPStr(), pClient->GetPort());
		
		Socket::SendTo(g_PeerSocket, (const char *)pNewData, nSize, &addr);
	}
	
	delete[] pNewData;
}

void SendToAgentUser(MCommandWriter *pCmd, const MUID &uidReceiver)
{
	if(pCmd->IsReadyToUse() == false) return;
	SendToAgentUser(pCmd->GetData(), pCmd->GetDataIndex(), uidReceiver);
}
// ------------------------------

// from matchserver commands.
void OnStageReserve(const MUID &uidStage)
{
	// just send-back command to matchserver.
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidStage);
	Cmd.Finalize(MC_AGENT_STAGE_READY, MCFT_END);
	SendToServer(&Cmd);
}

void OnRelayPeer(const MUID &uidPlayer, const MUID &uidPeer, const MUID &uidStage)
{
	// add both client.
	MAgentClient *pClient = g_ClientMgr.Get(uidPlayer);
	if(pClient == NULL)
	{
		pClient = g_ClientMgr.Add(uidPlayer, uidStage, "", 0);
	}
	
	MAgentClient *pPeer = g_ClientMgr.Get(uidPeer);
	if(pPeer == NULL)
	{
		pPeer = g_ClientMgr.Add(uidPeer, uidStage, "", 0);
	}
	
	// add stage.
	MStageAgent *pStage = g_StageMgr.Get(uidStage);
	if(pStage == NULL)
	{
		pStage = g_StageMgr.Add(uidStage);
	}
	
	// enter to the stage.
	if(pStage->GetClient(pClient) == NULL)
	{
		pStage->Enter(pClient);
	}
	
	if(pStage->GetClient(pPeer) == NULL)
	{
		pStage->Enter(pPeer);
	}
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidPlayer);
	Cmd.WriteMUID(uidPeer);
	Cmd.Finalize(MC_AGENT_PEER_READY, MCFT_END);
	SendToServer(&Cmd);
	
	printf("Player (%lu:%lu) relaying request for Peer (%lu:%lu) is accepted. Telling this result to Match Server...\n", 
			uidPlayer.ulHighID, uidPlayer.ulLowID, uidPeer.ulHighID, uidPeer.ulLowID);
}

void OnPeerUnbind(const MUID &uidPlayer)
{
	MAgentClient *pClient = g_ClientMgr.Get(uidPlayer);
	if(pClient == NULL) return;
	
	MStageAgent *pStage = g_StageMgr.Get(pClient->GetStageUID());
	if(pStage != NULL)
	{
		pStage->Leave(pClient);
		
		if(pStage->IsEmpty() == true)
		{
			g_StageMgr.Remove(pClient->GetStageUID());
		}
	}
	
	g_ClientMgr.Remove(uidPlayer);
	
	printf("Player (%lu:%lu) relaying is stopped.\n", 
			uidPlayer.ulHighID, uidPlayer.ulLowID);
}

void OnLiveCheck(unsigned long nTimestamp)
{
	// do nothing.
}

// from matchclient commands.
void OnPeerBindUDP(const MUID &uidPlayer, const char *pszLocalIP, unsigned int nLocalPort, const char *pszIP, unsigned int nPort)
{
	MAgentClient *pClient = g_ClientMgr.Get(uidPlayer);
	if(pClient == NULL) return;
	
	pClient->SetAddress(pszIP, (unsigned short)nPort);
	pClient->SetPeerType(MACPT_UDP);
	
	MCmdWriter Cmd;
	Cmd.Finalize(MC_AGENT_ALLOW_TUNNELING_UDP, MCFT_END | MCFT_RAW);
	SendToAgentUser(&Cmd, uidPlayer);
}

void OnTunnelingUDP(const MUID &uidSender, const MUID &uidReceiver, const MAgentTunnelingData *pData)
{
	MAgentClient *pClient = g_ClientMgr.Get(uidSender);
	if(pClient == NULL) return;
	
	if(pClient->GetPeerType() != MACPT_UDP) return;
	
	MCmdWriter Cmd;
	
	Cmd.WriteMUID(uidSender);
	Cmd.WriteMUID(uidReceiver);
	
	Cmd.StartBlob(pData->nSize);
	Cmd.WriteData(pData->nData, pData->nSize);
	Cmd.EndBlob();
	
	Cmd.Finalize(MC_AGENT_TUNNELING_UDP, MCFT_END | MCFT_RAW);
	
	if(uidReceiver == MUID(0, 0))
	{
		SendToAgentStage(&Cmd, pClient->GetStageUID());
	}
	else
	{
		SendToAgentUser(&Cmd, uidReceiver);
	}
}

// oncommand.
bool MMatchAgent_OnCommand(MCommandReader *pRecvCmd)
{
	switch(pRecvCmd->GetCommandID())
	{
		case MC_AGENT_STAGE_RESERVE	:
		{
			MUID uidStage;
			pRecvCmd->ReadMUID(&uidStage);
			
			OnStageReserve(uidStage);
		}
		break;
		
		case MC_AGENT_RELAY_PEER	:
		{
			MUID uidPlayer, uidPeer;
			MUID uidStage;
			
			pRecvCmd->ReadMUID(&uidPlayer);
			pRecvCmd->ReadMUID(&uidPeer);
			pRecvCmd->ReadMUID(&uidStage);
			
			OnRelayPeer(uidPlayer, uidPeer, uidStage);
		}
		break;
		
		case MC_AGENT_PEER_UNBIND	:
		{
			MUID uidPlayer;
			pRecvCmd->ReadMUID(&uidPlayer);
			
			OnPeerUnbind(uidPlayer);
		}
		break;
		
		case MC_MATCH_AGENT_RESPONSE_LIVECHECK	:
		{
			unsigned long nTimestamp;
			pRecvCmd->ReadULong(&nTimestamp);
			
			OnLiveCheck(nTimestamp);
		}
		break;
		
		default	:
		{
			printf("Unknown command ID : %u is received.\n", pRecvCmd->GetCommandID());
		}
		break;
	}
	
	return true;
}

bool MMatchAgent_OnPeerCommand(MCommandReader *pRecvCmd)
{
	switch(pRecvCmd->GetCommandID())
	{
		case MC_AGENT_PEER_BINDUDP	:
		{
			// MUID uidPlayer = pRecvCmd->GetOwnerUID();
			MUID uidPlayer;
			char szLocalIP[64];
			unsigned int nLocalPort;
			char szIP[64];
			unsigned int nPort;
			
			// pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadMUID(&uidPlayer);
			pRecvCmd->ReadString(szLocalIP, SIZEOFA(szLocalIP));
			pRecvCmd->ReadUInt(&nLocalPort);
			pRecvCmd->ReadString(szIP, SIZEOFA(szIP));
			pRecvCmd->ReadUInt(&nPort);
			
			// OnPeerBindUDP(uidPlayer, szLocalIP, nLocalPort, szIP, nPort);
			OnPeerBindUDP(uidPlayer, szLocalIP, nLocalPort, g_LastPeerInfo.szIP, (unsigned int)g_LastPeerInfo.nPort);
		}
		break;
		
		case MC_AGENT_TUNNELING_UDP	:
		{
			if(pRecvCmd->GetOwnerUID() == MUID(0, 0)) break;	// owner uid shouldn't be invalid.
			
			MUID uidSender = pRecvCmd->GetOwnerUID(), uidReceiver;
			MAgentTunnelingData data;
			
			pRecvCmd->ReadSkip(sizeof(MUID));
			pRecvCmd->ReadMUID(&uidReceiver);
			
			int nNodeCount, nNodeSize;
			pRecvCmd->ReadBlobArray(&nNodeCount, &nNodeSize);
			
			if(nNodeCount != 1) break;
			if(nNodeSize >= 1024) break;
			
			data.nSize = nNodeSize;
			if(pRecvCmd->ReadData(data.nData, nNodeSize) == false) break;
			
			OnTunnelingUDP(uidSender, uidReceiver, &data);
		}
		break;
		
		default	:
		{
			printf("Unknown Peer command ID : %u is received.\n", pRecvCmd->GetCommandID());
		}
		break;
	}
	
	return true;
}