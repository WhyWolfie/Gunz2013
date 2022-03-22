#include "pch.h"

#ifdef _UNIX_BUILD
#define OTL_ODBC_UNIX
#else
#define OTL_ODBC
#endif
#define OTL_DEFAULT_NUMERIC_NULL_TO_VAL 0
#include "../MatchServer/otlv4.h"

// global settings.
struct MLocatorConfig
{
	struct
	{
		unsigned short nUDPPort;
	} Locator;
	
	struct
	{
		char szUsername[128];
		char szPassword[128];
		char szDSN[128];
	} Database;
} g_LocatorConfig;

bool LoadConfig()
{
	XMLDocument doc;
	
	if(doc.LoadFile("locator.xml") != XML_SUCCESS)
	{
		return false;
	}
	
	XMLElement *elem;
	XMLHandle han(&doc);
	
	elem = han.FirstChildElement("XML").FirstChildElement("LOCATOR").FirstChildElement("UDPPORT").ToElement();
	if(elem != NULL && elem->GetText() != NULL)
	{
		unsigned int temp;
		if(elem->QueryUnsignedText(&temp) != XML_SUCCESS) return false;
		
		g_LocatorConfig.Locator.nUDPPort = (unsigned short)temp;
	}
	else return false;
	
	elem = han.FirstChildElement("XML").FirstChildElement("DATABASE").FirstChildElement("USERNAME").ToElement();
	if(elem != NULL && elem->GetText() != NULL)
	{
		strcpy(g_LocatorConfig.Database.szUsername, elem->GetText());
	}
	else return false;
	
	elem = han.FirstChildElement("XML").FirstChildElement("DATABASE").FirstChildElement("PASSWORD").ToElement();
	if(elem != NULL && elem->GetText() != NULL)
	{
		strcpy(g_LocatorConfig.Database.szPassword, elem->GetText());
	}
	else return false;
	
	elem = han.FirstChildElement("XML").FirstChildElement("DATABASE").FirstChildElement("DSN").ToElement();
	if(elem != NULL && elem->GetText() != NULL)
	{
		strcpy(g_LocatorConfig.Database.szDSN, elem->GetText());
	}
	else return false;
	
	return true;
}

// db. ------------------------------
otl_connect db;

void Db_Error(otl_exception &e);
bool Db_Connect(const char *pszUsername, const char *pszPassword, const char *pszDSN);
void Db_Disconnect();

void Db_Error(otl_exception &e)
{
	printf("%s\n", e.msg);
	printf("%s\n", e.stm_text);
	printf("%s\n", e.var_info);
	
	printf("Some error in SQL process.\n");
	
	if(e.code == 26)	// 26 = connection dead.
	{
		printf("Trying to restore dead DB connection...\n");
		
		Db_Disconnect();
		
		if(Db_Connect(g_LocatorConfig.Database.szUsername, 
						g_LocatorConfig.Database.szPassword, 
						g_LocatorConfig.Database.szDSN) == false)
		{
			printf("Couldn't be restored DB connection.\n");
			return;
		}
		
		printf("DB connection revived.\n");
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
// ------------------------------

vector<MTD_ServerStatusInfo> g_ServerList;

bool Db_GetServerStatus(vector<MTD_ServerStatusInfo> *pOut)
{
	try
	{
		#ifdef _AGENT_PING
		otl_stream a(10, "SELECT ip, agentip, port, id, maxplayer, currplayer, type, name FROM getserverstatus();", db, otl_implicit_select);
		#else
		otl_stream a(10, "SELECT ip, port, id, maxplayer, currplayer, type, name FROM getserverstatus();", db, otl_implicit_select);
		#endif
		
		while(a.eof() == 0)
		{
			char szIP[64] = "";
			#ifdef _AGENT_PING
			char szAgentIP[64] = "";
			#endif
			int nPort = 0;
			int nServerID = 0;
			int nMaxPlayer = 0;
			int nCurrPlayer = 0;
			int nType = 0;
			char szServerName[64] = "";
			
			#ifdef _AGENT_PING
			a >> szIP >> szAgentIP >> nPort >> nServerID >> nMaxPlayer >> nCurrPlayer >> nType >> szServerName;
			#else
			a >> szIP >> nPort >> nServerID >> nMaxPlayer >> nCurrPlayer >> nType >> szServerName;
			#endif
			
			MTD_ServerStatusInfo info;
			memset(&info, 0, sizeof(MTD_ServerStatusInfo));
			
			info.nIP = Socket::InetAddr(szIP);
			#ifdef _AGENT_PING
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

bool UpdateGlobalServerList()
{
	g_ServerList.clear();
	
	return Db_GetServerStatus(&g_ServerList);
}

void GenerateServerListInfoCommand(unsigned char *pOutData, int *pOutSize)
{
	MCmdWriter Cmd;
	
	Cmd.StartBlob(sizeof(MTD_ServerStatusInfo));
	
	for(vector<MTD_ServerStatusInfo>::iterator i = g_ServerList.begin(); i != g_ServerList.end(); i++)
	{
		const MTD_ServerStatusInfo *pInfo = &(*i);
		Cmd.WriteData(pInfo, sizeof(MTD_ServerStatusInfo));
	}
	
	Cmd.EndBlob();
	Cmd.Finalize(MC_RESPONSE_SERVER_LIST_INFO, MCFT_END | MCFT_RAW);
	
	int nSize = Cmd.GetDataIndex();
	memcpy(pOutData, Cmd.GetData(), nSize);
	
	unsigned short nChecksum = CreatePacketChecksum(pOutData, nSize);
	memcpy(&pOutData[4], &nChecksum, 2);
	
	(*pOutSize) = nSize;
}

bool CheckRecvDataValidation(const unsigned char *p, int n)
{
	// size check.
	if(n < MPACKET_HEADER_LENGTH) return false;
	
	// header check.
	unsigned short nHeader;
	memcpy(&nHeader, &p[0], 2);
	
	if(nHeader != 0x64) return false;
	
	// size check.
	unsigned short nDataSize;
	unsigned short nDataCmdSize;
	unsigned short nValidSize = (unsigned short)n;
	
	memcpy(&nDataSize, &p[2], 2);
	memcpy(&nDataCmdSize, &p[6], 2);
	
	if(nDataSize != nValidSize) return false;
	if(nDataCmdSize != nValidSize - 6) return false;
	
	// checksum check.
	unsigned short nDataChecksum;
	unsigned short nValidChecksum = CreatePacketChecksum(p, n);
	
	memcpy(&nDataChecksum, &p[4], 2);
	if(nDataChecksum != nValidChecksum) return false;
	
	// packet id check.
	unsigned short nDataCommandID;
	unsigned short nValidCommandID = MC_REQUEST_SERVER_LIST_INFO;
	
	memcpy(&nDataCommandID, &p[8], 2);
	if(nDataCommandID != nValidCommandID) return false;
	
	return true;
}

void OnLocatorRecv(Socket::socket_type s, Socket::address_type *addr, const char *data, const int size)
{
	if(CheckRecvDataValidation((const unsigned char *)data, size) == true)
	{
		int nSendDataSize;
		unsigned char nSendData[4096];
		
		GenerateServerListInfoCommand(nSendData, &nSendDataSize);
		Socket::SendTo(s, (const char *)nSendData, nSendDataSize, addr);
	}
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
		printf("Invalid config found.\n");
		return -1;
	}
	printf("Load configuration done.\n");
	
	// connect to db.
	if(Db_Connect(g_LocatorConfig.Database.szUsername, 
					g_LocatorConfig.Database.szPassword, 
					g_LocatorConfig.Database.szDSN) == false)
	{
		printf("Error while connecting DB.\n");
		return -1;
	}
	
	printf("Connect DB done.\n");
	// end of db.
	
	// init socket.
	if(Socket::Initialize() == false)
	{
		printf("Can't initialize socket.\n");
		return -1;
	}
	
	Socket::socket_type udp_socket = Socket::CreateDatagramSocket(g_LocatorConfig.Locator.nUDPPort);
	if(udp_socket == INVALID_SOCKET_DESC)
	{
		printf("Can't create socket.");
		Socket::UnInitialize();
		return -1;
	}
	
	printf("Init UDP socket done.\n");
	// end of init socket.
	
	printf("Locator server created on UDP port %u.\n", 
			g_LocatorConfig.Locator.nUDPPort);
	
	while(1)
	{
		const unsigned long nNowTime = (unsigned long)wxGetLocalTimeMillis().GetValue();
		
		static unsigned long nNextInfoListUpdate = 0;
		if(nNextInfoListUpdate <= nNowTime)
		{
			UpdateGlobalServerList();
			nNextInfoListUpdate = nNowTime + 20000;
		}
		
		if(Socket::IsReadable(udp_socket) == true)
		{
			#define MAX_RECVBUFF_SIZE	4096
			
			char nRecvData[MAX_RECVBUFF_SIZE];
			int nRecvSize;
			
			Socket::address_type from;
			
			nRecvSize = Socket::RecvFrom(udp_socket, nRecvData, sizeof(nRecvData), &from);
			
			OnLocatorRecv(udp_socket, &from, nRecvData, nRecvSize);
		}
		
		this_thread::sleep_for(chrono::milliseconds(10));
	}
	
	Socket::Close(udp_socket);
	Socket::UnInitialize();
	
	Db_Disconnect();
	
	wxUninitialize();
	
	return 0;
}
