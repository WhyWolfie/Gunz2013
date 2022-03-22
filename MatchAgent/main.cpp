#include "pch.h"

#include "main.h"
#include "MatchAgent.h"

#include "../MatchServer/MMatchCryptKey.h"

MServerCommInfo g_ServerCommInfo;

MLastPeerInfo g_LastPeerInfo;

Socket::socket_type g_ClientSocket;
Socket::socket_type g_PeerSocket;

// global setting.
MAgentConfig g_AgentConfig;

bool LoadConfig()
{
	XMLDocument doc;
	
	if(doc.LoadFile("matchagent.xml") != XML_SUCCESS)
	{
		return false;
	}
	
	XMLElement *elem;
	XMLHandle han(&doc);
	
	elem = han.FirstChildElement("XML").FirstChildElement("AGENT").FirstChildElement("WANIP").ToElement();
	if(elem != NULL && elem->GetText() != NULL)
	{
		strcpy(g_AgentConfig.Agent.szIP, elem->GetText());
	}
	else return false;
	
	
	elem = han.FirstChildElement("XML").FirstChildElement("AGENT").FirstChildElement("TCPPORT").ToElement();
	if(elem != NULL && elem->GetText() != NULL)
	{
		unsigned int temp;
		if(elem->QueryUnsignedText(&temp) != XML_SUCCESS) return false;
		
		g_AgentConfig.Agent.nTCPPort = (unsigned short)temp;
	}
	else return false;
	
	elem = han.FirstChildElement("XML").FirstChildElement("AGENT").FirstChildElement("UDPPORT").ToElement();
	if(elem != NULL && elem->GetText() != NULL)
	{
		unsigned int temp;
		if(elem->QueryUnsignedText(&temp) != XML_SUCCESS) return false;
		
		g_AgentConfig.Agent.nUDPPort = (unsigned short)temp;
	}
	else return false;
	
	elem = han.FirstChildElement("XML").FirstChildElement("SERVER").FirstChildElement("IP").ToElement();
	if(elem != NULL && elem->GetText() != NULL)
	{
		strcpy(g_AgentConfig.Server.szIP, elem->GetText());
	}
	else return false;
	
	elem = han.FirstChildElement("XML").FirstChildElement("SERVER").FirstChildElement("PORT").ToElement();
	if(elem != NULL && elem->GetText() != NULL)
	{
		unsigned int temp;
		if(elem->QueryUnsignedText(&temp) != XML_SUCCESS) return false;
		
		g_AgentConfig.Server.nPort = (unsigned short)temp;
	}
	else return false;
	
	return true;
}

unsigned short GetPacketMessageType(const unsigned char *p)
{
	unsigned short n;
	memcpy(&n, &p[0], 2);
	return n;
}

void CheckHandShakeData(const unsigned char *pData, MUID *pOutServerUID, MUID *pOutClientUID, unsigned int *pOutTimeStamp)
{
	/*
	unsigned short nMsg;			+ 0
	unsigned short nSize;			+ 2
	unsigned short nCheckSum;		+ 4
	------------------------------------
	unsigned int	nHostHigh;		+ 6
	unsigned int	nHostLow;		+ 10
	unsigned int	nAllocHigh;		+ 14
	unsigned int	nAllocLow;		+ 18
	unsigned int	nTimeStamp;		+ 22
	
	(Total Size = 26.)
	*/
	
	if(pOutServerUID != NULL)
	{
		MUID uidServer;
		
		memcpy(&uidServer.ulHighID, &pData[6], 4);
		memcpy(&uidServer.ulLowID, &pData[10], 4);
		
		*pOutServerUID = uidServer;
	}
	
	if(pOutClientUID != NULL)
	{
		MUID uidClient;
		
		memcpy(&uidClient.ulHighID, &pData[14], 4);
		memcpy(&uidClient.ulLowID, &pData[18], 4);
		
		*pOutClientUID = uidClient;
	}
	
	if(pOutTimeStamp != NULL)
	{
		unsigned int nTimeStamp;
		
		memcpy(&nTimeStamp, &pData[22], 4);
		
		*pOutTimeStamp = nTimeStamp;
	}
}

// queue data manager.
void AddQueueData(vector<unsigned char> *pTo, const unsigned char *pFrom, const int nFromLen)
{
	pTo->reserve((unsigned int)pTo->size() + (unsigned int)nFromLen);
	
	for(int i = 0; i < nFromLen; i++)
		pTo->push_back(pFrom[i]);
}

void ClearQueueData(vector<unsigned char> *pDest)
{
	pDest->clear();
}

void CreateQueueByte(const vector<unsigned char> *pFrom, unsigned char **ppOut, int *pOutLen)
{
	int nSize = (int)pFrom->size();
	unsigned char *pNew = new unsigned char[nSize];
	
	for(int i = 0; i < nSize; i++)
	{
		pNew[i] = (*pFrom)[i];
	}
	
	*ppOut = pNew;
	*pOutLen = nSize;
}

void DeleteQueueByte(unsigned char *p)
{
	delete[] p;
}
// end of manager.

// matchserver -> matchagent.
void OnServerRecv(const char *data, const int size)
{
	static vector<unsigned char> vtQueued;
	AddQueueData(&vtQueued, (const unsigned char *)data, size);
	
	unsigned char *pQueued;
	int nQueuedSize;
	CreateQueueByte(&vtQueued, &pQueued, &nQueuedSize);
	
	if(nQueuedSize < 6)	// 6 = packet header's size.
	{
		// guess probably there is next data, just wait for it.
		DeleteQueueByte(pQueued);
		return;
	}
	
	if(GetPacketMessageType(pQueued) == 0xA)	// 0xA = ReplyConnect message.
	{
		// 26 = connect reply msg's data size. see CheckHandShakeData() for more details.
		if(nQueuedSize < 26)
		{
			DeleteQueueByte(pQueued);
			return;
		}
		else if(nQueuedSize > 26)
		{
			// error invalid data.
			printf("Hand-shaking failed. This MatchAgent may not work. Please restart the Agent Server.\n");
			
			DeleteQueueByte(pQueued);
			ClearQueueData(&vtQueued);
			return;
		}
		
		MUID uidClient, uidServer;
		unsigned int nTimeStamp;
		CheckHandShakeData(pQueued, &uidServer, &uidClient, &nTimeStamp);
		
		unsigned char nCryptKey[ENCRYPTIONKEY_LENGTH];
		MakeCryptKey(uidClient, uidServer, (unsigned long)nTimeStamp, nCryptKey);
		
		g_ServerCommInfo.uidClient = uidClient;
		memcpy(g_ServerCommInfo.nCryptKey, nCryptKey, sizeof(nCryptKey));
		
		g_ServerCommInfo.uidServer = uidServer;
		
		// request register agent to match server,
		MCmdWriter Cmd;
		Cmd.WriteString(g_AgentConfig.Agent.szIP);
		Cmd.WriteInt(0);	// NOTE : TCP port is not used, always 0.
		Cmd.WriteInt((int)g_AgentConfig.Agent.nUDPPort);
		Cmd.Finalize(MC_MATCH_REGISTERAGENT, MCFT_END);
		SendToServer(&Cmd);
		
		DeleteQueueByte(pQueued);
		ClearQueueData(&vtQueued);
		
		return;
	}
	
	unsigned char *pDecData = new unsigned char[nQueuedSize];
	memcpy(pDecData, pQueued, nQueuedSize);
	
	int nDecRes = PacketDecrypter(pDecData, nQueuedSize, g_ServerCommInfo.nCryptKey);
	
	if(nDecRes == -1)
	{
		printf("::PacketDecrypter() - decryption failed.\n");
		
		delete[] pDecData;
		DeleteQueueByte(pQueued);
		ClearQueueData(&vtQueued);
		
		return;
	}
	else if(nDecRes != nQueuedSize)
	{
		// packet still remaining...
		
		delete[] pDecData;
		DeleteQueueByte(pQueued);
		
		return;
	}
	
	vector<MCommandReader *> vtRecvCmd;
	if(BuildReaderCommand(pDecData, nQueuedSize, g_ServerCommInfo.uidServer, &vtRecvCmd) == false)
	{
		printf("::BuildReaderCommand() - can't build commands.\n");
		
		delete[] pDecData;
		DeleteQueueByte(pQueued);
		ClearQueueData(&vtQueued);
		
		return;
	}
	
	for(vector<MCommandReader *>::iterator i = vtRecvCmd.begin(); i != vtRecvCmd.end(); i++)
	{
		MCommandReader *pCmd = (*i);
		MMatchAgent_OnCommand(pCmd);
		
		DestroyReaderCommand(pCmd);
	}
	vtRecvCmd.clear();
	
	delete[] pDecData;
	DeleteQueueByte(pQueued);
	ClearQueueData(&vtQueued);
}

// matchclient -> matchagent via UDP.
void OnAgentPeerRecv(Socket::address_type *from, const char *data, const int size)
{
	// addresses.
	char szIP[64];
	strcpy(szIP, Socket::InetNtoa(from));
	
	unsigned long nIP = Socket::ExtractInetAddr(*from);
	unsigned short nPort = Socket::Ntohs(from->sin_port);
			
	// set last peer info.
	strcpy(g_LastPeerInfo.szIP, szIP);
	g_LastPeerInfo.nIP = nIP;
	g_LastPeerInfo.nPort = nPort;

	// ---------------------------------------- ...
			
	MUID uidSender = g_ClientMgr.FindUID(nIP, nPort);
	// if(uidSender == MUID(0, 0)) return false;
	
	unsigned char *pDecData = new unsigned char[size];
	memcpy(pDecData, data, size);
	
	int nDecRes = PacketDecrypter(pDecData, size);
	
	if(nDecRes != size)
	{
		printf("[::OnPeerRecv()] ::PacketDecrypter() - decryption failed.\n");
		delete[] pDecData;
		return;
	}
	
	vector<MCommandReader *> vtRecvCmd;
	if(BuildReaderCommand(pDecData, size, uidSender, &vtRecvCmd) == false)
	{
		printf("::BuildReaderCommand() - can't build commands.\n");
		delete[] pDecData;
		return;
	}
	
	for(vector<MCommandReader *>::iterator i = vtRecvCmd.begin(); i != vtRecvCmd.end(); i++)
	{
		MCommandReader *pCmd = (*i);
		MMatchAgent_OnPeerCommand(pCmd);
		
		DestroyReaderCommand(pCmd);
	}
	vtRecvCmd.clear();
	
	delete[] pDecData;
}

int main()
{
	if(wxInitialize() == false)
	{
		printf("::wxInitialize() failed.\n");
		return -1;
	}
	
	printf("Build : " __DATE__ " " __TIME__ ".\n");
	
	if(LoadConfig() == false)
	{
		printf("Agent configuration is invalid and couldn't be loaded.\n");
		return -1;
	}
	printf("Agent config loaded.\n");
	
	// init peer socket.
	if(Socket::Initialize() == false)
	{
		printf("Can't initialize socket.\n");
		return -1;
	}
	
	Socket::socket_type peer_socket = Socket::CreateDatagramSocket(g_AgentConfig.Agent.nUDPPort);
	if(peer_socket == INVALID_SOCKET_DESC)
	{
		printf("Can't create peer socket.\n");
		Socket::UnInitialize();
		return -1;
	}
	
	printf("Peer socket created.\n");
	// end of peer socket.
	
	// init of socket.
	Socket::socket_type client_socket = Socket::CreateStreamClientSocket(g_AgentConfig.Server.szIP, g_AgentConfig.Server.nPort);
	if(client_socket == INVALID_SOCKET_DESC)
	{
		printf("Can't create stream socket.\n");
		Socket::Close(peer_socket);
		Socket::UnInitialize();
		return -1;
	}
	
	printf("Connected to the Match Server. (%s:%u)\n", 
			g_AgentConfig.Server.szIP, 
			g_AgentConfig.Server.nPort);
	// end of socket.
	
	// init globals.
	g_ClientSocket = client_socket;
	g_PeerSocket = peer_socket;
	
	printf("Match Agent server created. (Global IP : %s - UDP port : %u)\n", 
			g_AgentConfig.Agent.szIP, 
			g_AgentConfig.Agent.nUDPPort);
	
	while(1)
	{
		if(Socket::IsReadable(client_socket) == true)
		{
			#define MAX_RECVBUFF_SIZE	4096
			
			char nRecvData[MAX_RECVBUFF_SIZE];
			int nRecvSize;
			
			nRecvSize = Socket::Recv(client_socket, nRecvData, sizeof(nRecvData));
			
			if(nRecvSize <= 0)
			{
				printf("Connection to the Match Server is terminated. Aborting Agent Server...\n");
				break;
			}
			
			OnServerRecv(nRecvData, nRecvSize);
		}
		
		while(Socket::IsReadable(peer_socket) == true)
		{
			char nRecvData[MAX_RECVBUFF_SIZE];
			int nRecvSize;
			
			Socket::address_type from;
			
			nRecvSize = Socket::RecvFrom(peer_socket, nRecvData, sizeof(nRecvData), &from);
			
			OnAgentPeerRecv(&from, nRecvData, nRecvSize);
		}
		
		this_thread::sleep_for(chrono::milliseconds(10));
	}
	
	Socket::Close(client_socket);
	Socket::Close(peer_socket);
	Socket::UnInitialize();
	
	wxUninitialize();
	
	return 0;
}
