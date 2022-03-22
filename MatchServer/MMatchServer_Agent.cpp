#include "pch.h"
#include "MMatchAgent.h"

#include "MMatchObject.h"

#include "MClientAcceptor.h"

void OnPeerRelay(const MUID &uidPlayer, const MUID &uidPeer)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	if(pObj->m_uidStage == MUID(0, 0)) return;
	if(pObj->CheckGameFlag(MMOGF_ENTERED) == false) return;
	
	MMatchObject *pPeerObj = g_ObjectMgr.Get(uidPeer);
	if(pPeerObj == NULL) return;
	
	if(pPeerObj->m_bCharInfoExist == false) return;
	if(pPeerObj->m_uidStage == MUID(0, 0)) return;
	if(pPeerObj->CheckGameFlag(MMOGF_ENTERED) == false) return;
	
	if(pObj->m_uidStage != pPeerObj->m_uidStage) return;
	
	if(g_MatchAgent.GetAgentObject() == NULL)
	{
		MCmdWriter Cmd;
		
		Cmd.WriteInt(0);
		Cmd.Finalize(MC_AGENT_ERROR, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		
		return;
	}
	
	pObj->m_bAgentUser = true;
	pPeerObj->m_bAgentUser = true;
	
	printf("Player (%lu:%lu) is requesting relay with Peer (%lu:%lu). Telling about this to the Agent Server...\n", 
			uidPlayer.ulHighID, uidPlayer.ulLowID, uidPeer.ulHighID, uidPeer.ulLowID);
	
	// send relay request to agent.
	MCmdWriter Cmd2Agent;
	Cmd2Agent.WriteMUID(uidPlayer);
	Cmd2Agent.WriteMUID(uidPeer);
	Cmd2Agent.WriteMUID(pObj->m_uidStage);
	Cmd2Agent.Finalize(MC_AGENT_RELAY_PEER, MCFT_END);
	SendToAgent(&Cmd2Agent);
}

// param 3 : nPort is not used.
void OnAgentRegister(const MUID &uidAgent, const char *pszIP, int nPort, int nUDPPort)
{
	if(g_MatchAgent.GetAgentObject() != NULL)	// don't replace agent.
	{
		printf("Got Agent register request, but Agent already exists. - Rejected (%lu:%lu).\n", 
				uidAgent.ulHighID, uidAgent.ulLowID);
		return;
	}
	
	Socket::socket_type s;
	unsigned char nCryptKey[ENCRYPTIONKEY_LENGTH];

	if(g_ClientAcceptor.GetClientInfo(uidAgent, &s, nCryptKey) == false) return;
	
	Socket::address_type addr;
	Socket::GetPeerName(s, &addr);
	
	char szSocketIP[64];
	strcpy(szSocketIP, Socket::InetNtoa(&addr));
	
	if(g_MatchAgent.IsValidAgentIP(szSocketIP) == false) 
	{
		printf("Unreliable Agent add request, not a valid IP. - Rejected (%lu:%lu).\n", 
				uidAgent.ulHighID, uidAgent.ulLowID);
		return;
	}
	
	g_MatchAgent.SetAgentObject(s, uidAgent, nCryptKey);
	g_MatchAgent.SetAgentAddressInfo(pszIP, (unsigned short)nUDPPort);
	
	g_ClientAcceptor.RemoveClient(s);
	
	printf("<%s:%u - %lu:%lu> Agent registration done. peer IP : %s - peer port : %d - peer UDP port : %d.\n", 
			szSocketIP, Socket::Ntohs(addr.sin_port), uidAgent.ulHighID, uidAgent.ulLowID, pszIP, nPort, nUDPPort);
			
	mlog("<%s:%u - %lu:%lu> Agent added. peer IP : %s - peer port : %d - peer UDP port : %d.", 
			szSocketIP, Socket::Ntohs(addr.sin_port), uidAgent.ulHighID, uidAgent.ulLowID, pszIP, nPort, nUDPPort);
}

void OnAgentUnregister(const MUID &uidAgent)
{
	if(g_MatchAgent.IsAgentUID(uidAgent) == false) return;
	g_MatchAgent.UnsetAgentObject();
	
	printf("Agent unregistered. (%lu:%lu)\n", uidAgent.ulHighID, uidAgent.ulLowID);
	mlog("Agent removed. (%lu:%lu)", uidAgent.ulHighID, uidAgent.ulLowID);
}

void OnAgentStageReady(const MUID &uidAgent, const MUID &uidStage)
{
	if(g_MatchAgent.IsAgentUID(uidAgent) == false) return;
	
	// do nothing.
}

void OnAgentPeerReady(const MUID &uidAgent, const MUID &uidPlayer, const MUID &uidPeer)
{
	if(g_MatchAgent.IsAgentUID(uidAgent) == false) return;
	
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	if(pObj->m_uidStage == MUID(0, 0)) return;
	if(pObj->CheckGameFlag(MMOGF_ENTERED) == false) return;
	
	MCmdWriter Cmd;
	
	Cmd.WriteMUID(uidAgent);
	Cmd.WriteString(g_MatchAgent.GetAgentObject()->GetAgentIPString());
	Cmd.WriteInt(0);	// tcp port.
	Cmd.WriteInt((int)g_MatchAgent.GetAgentObject()->GetUDPPort());
	Cmd.Finalize(MC_AGENT_LOCATETO_CLIENT);
	
	Cmd.WriteMUID(uidPeer);
	Cmd.Finalize(MC_MATCH_RESPONSE_PEER_RELAY, MCFT_END);
	
	SendToClient(&Cmd, uidPlayer);
	
	printf("Agent (%lu:%lu) located to player (%lu:%lu).\n", 
			uidAgent.ulHighID, uidAgent.ulLowID, uidPlayer.ulHighID, uidPlayer.ulLowID);
}

void OnLiveCheck(const MUID &uidAgent, unsigned long nTimestamp, unsigned int nStageCount, unsigned int nUserCount)
{
	if(g_MatchAgent.IsAgentUID(uidAgent) == false) return;
	
	MCmdWriter Cmd;
	Cmd.WriteULong(nTimestamp);
	Cmd.Finalize(MC_MATCH_AGENT_RESPONSE_LIVECHECK, MCFT_END);
	SendToAgent(&Cmd);
}