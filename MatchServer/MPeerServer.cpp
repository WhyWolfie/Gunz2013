#include "pch.h"

#include "MServerSetting.h"
#include "MMatchObject.h"

void OnBridgePeer(const MUID &uidPlayer, unsigned long nIP, unsigned int nUDPPort)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->GetIP() != nIP) return;
	
	pObj->SetUDPPort((unsigned short)nUDPPort);
	
	MCmdWriter Cmd;
	Cmd.WriteMUID(uidPlayer);
	Cmd.WriteInt(0);	// ok.
	Cmd.Finalize(MC_MATCH_BRIDGEPEER_ACK, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
	
	pObj->m_nPlayerFlag |= (unsigned long)MTD_PlayerFlags_BridgePeer;
}

// -------------------------------------------------------

void OnServerPeerRecv(Socket::socket_type s, Socket::address_type *from, const char *data, const int size)
{
	#define MAX_PEER_RECVBUFF_LEN	4096
	
	if(size >= MAX_PEER_RECVBUFF_LEN) return;
	
	unsigned char nData[MAX_PEER_RECVBUFF_LEN];
	memcpy(nData, data, size);
	
	if(PacketDecrypter(nData, size) != size) return;
		
	MCmdReader r(nData, size);
	
	if(r.GetCommandID() == MC_MATCH_BRIDGEPEER)	
	{
		MUID uidPlayer;
		
		r.ReadMUID(&uidPlayer);
		r.ReadSkip(sizeof(unsigned long));	// ip.
		r.ReadSkip(sizeof(unsigned int));	// udp port.
		
		char szIP[64];
		strcpy(szIP, Socket::InetNtoa(from));
		
		OnBridgePeer(uidPlayer, Socket::InetAddr(szIP), (unsigned int)Socket::Ntohs(from->sin_port));
	}
}
