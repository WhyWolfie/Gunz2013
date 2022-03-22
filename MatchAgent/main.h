#ifndef __MATCHAGENT_MAIN_H__
#define __MATCHAGENT_MAIN_H__

struct MServerCommInfo
{
	MUID uidClient;	// means this Agent UID.
	MUID uidServer;
	
	unsigned char nCryptKey[ENCRYPTIONKEY_LENGTH];
};

extern MServerCommInfo g_ServerCommInfo;

struct MLastPeerInfo
{
	char szIP[64];
	unsigned long nIP;
	
	unsigned short nPort;
};

extern MLastPeerInfo g_LastPeerInfo;

extern Socket::socket_type g_ClientSocket;
extern Socket::socket_type g_PeerSocket;

struct MAgentConfig
{
	struct
	{
		char szIP[64];
		unsigned short nTCPPort;
		unsigned short nUDPPort;
	} Agent;
	
	struct
	{
		char szIP[64];
		unsigned short nPort;
	} Server;
};

extern MAgentConfig g_AgentConfig;

#endif