#include "pch.h"

#include <stdarg.h>
#include <time.h>

#include "MServerSetting.h"
#include "MServerNetwork.h"

#include "MClientAcceptor.h"

#include "MMatchDBMgr.h"
#include "MAsyncDBProcess.h"

#include "MMatchServer_OnCommand.h"
#include "MPeerServer.h"

#include "MServerUpdater.h"

#include "MMatchDuelTournament.h"
#include "MMatchBlitzKrieg.h"

#include "MMatchAdmin.h"

SFMT_T g_sfmt;
mutex g_Mutex;
wxFile g_File;

void MLog(const char *pszFormat, ...)
{
	va_list args;
	va_start(args, pszFormat);

	// wxString a;
	// a.PrintfV(pszFormat, args);
	
	char a[1024];
	vsprintf(a, pszFormat, args);
	
	va_end(args);

	// --------------------

	wxDateTime dt = wxDateTime::Now();

	char d[32];
	strcpy(d, dt.FormatISODate().mb_str());
	char t[32];
	strcpy(t, dt.FormatISOTime().mb_str());

	wxString s;
	s.Printf(wxString::FromAscii("[%s %s] %s\n"), d, t, a /*.c_str()*/);
	
	g_File.Write(s);
}

// check game version is newer.
bool CheckGameVersion(int year)
{
	int n = _GAME_VERSION;
	return n >= year ? true : false ;
}

int main()
{
	if(wxInitialize() == false)
	{
		printf("Initialization failed.\n");
		return -1;
	}

	if(g_File.Open(wxString::FromAscii("Server Log.txt"), wxFile::write_append) == false)
	{
		printf("Could not open the log file.\n");
		return -1;
	}

	printf("Build : " __DATE__ " " __TIME__ ". \n");

	static const char szLaunched[] = "Server started... (" __DATE__ " " __TIME__ ") ";
	MLog(szLaunched);
	
	sfmt_init_gen_rand(&g_sfmt, (uint32_t)time(NULL));

	printf("Parse server configuration file - ");

	// Load config.
	if(LoadServerXML() == false)
	{
		printf("failed.\n");
		return -1;
	}
	printf("succeed.\n");

	if(Db_Connect(g_ServerConfig.DatabaseSetting.szUsername,
	              g_ServerConfig.DatabaseSetting.szPassword,
	              g_ServerConfig.DatabaseSetting.szDSN) == false)
	{
		printf("Failed to connect to database.\n");
		return -1;
	}
	printf("Database server connected.\n");
	
	if(g_DTRankingMgr.FetchRanking() == false)
	{
		printf("Couldn't fetch initial DT ranking.\n");
		return -1;
	}
	
	if(g_BlitzShop.LoadFromDB() == false)
	{
		printf("Couldn't fetch blitz shop item from DB.\n");
	}
	
	// init network...
	if(Socket::Initialize() == false)
	{
		printf("Socket initialization failed.\n");
		mlog("Fail to init socket.");
		return -1;
	}
	
	// <TCP>
	Socket::socket_type server_socket = Socket::CreateStreamServerSocket(g_ServerConfig.ServerSetting.nPort);
	if(server_socket == INVALID_SOCKET_DESC)
	{
		printf("Create TCP server failed.\n");
		mlog("Fail to create TCP server.");
		
		Socket::UnInitialize();
		
		return -1;
	}
	
	// <UDP>
	Socket::socket_type peer_socket = Socket::CreateDatagramSocket(g_ServerConfig.ServerSetting.nUdpPort);
	if(peer_socket == INVALID_SOCKET_DESC)
	{
		printf("Create UDP server failed.\n");
		mlog("Fail to create UDP server.");
		
		Socket::Close(server_socket);
		Socket::UnInitialize();
		
		return -1;
	}
	// network init finish.
	
	thread AsyncDBProcessThread(MAsyncDBProcessThread, NULL);
	
	printf("MatchServer created on TCP port %u.\n", g_ServerConfig.ServerSetting.nPort);
	MLog("Initialization completed.");
	
	list<Socket::socket_type> SocketList;
	
	while(CheckServerHalt() == false)
	{
		g_Mutex.lock();
		
		// accept.
		if(Socket::IsReadable(server_socket) == true)
		{
			Socket::address_type addr;
			Socket::socket_type s = Socket::Accept(server_socket, &addr);
			
			OnClientConnect(s);
			
			SocketList.push_back(s);
		}
		
		// recv.
		list<Socket::socket_type>::iterator it = SocketList.begin();
		
		while(it != SocketList.end())
		{
			Socket::socket_type curr = (*it);
			
			if(CheckDisconnectReserved(curr) == true)
			{
				#define DISCONNECT					\
				{									\
					OnClientDisconnect(curr);		\
													\
					Socket::Close(curr);			\
					it = SocketList.erase(it);		\
													\
					continue;						\
				}
				
				DISCONNECT;
			}
			else
			{
				if(Socket::IsReadable(curr) == true)
				{
					#define MAX_RECVBUFF_SIZE	4096
					
					char nRecvData[MAX_RECVBUFF_SIZE];
					int nRecvSize;
					
					nRecvSize = Socket::Recv(curr, nRecvData, sizeof(nRecvData));
					
					if(nRecvSize <= 0)
					{
						DISCONNECT;
					}
					else
					{
						if(OnClientRecv(curr, nRecvData, nRecvSize) == false)
						{
							DISCONNECT;
						}
					}
				}
			}
			
			it++;
		}
		
		ClearDisconnectReservedSocket();
		
		// peer.
		while(Socket::IsReadable(peer_socket) == true)
		{
			char nRecvData[MAX_RECVBUFF_SIZE];
			int nRecvSize;
			
			Socket::address_type from;
			
			nRecvSize = Socket::RecvFrom(peer_socket, nRecvData, sizeof(nRecvData), &from);
			
			OnServerPeerRecv(peer_socket, &from, nRecvData, nRecvSize);
		}
		
		// update.
		unsigned long nNowTime = GetTime();
		UpdateServer(nNowTime);
		
		g_Mutex.unlock();
		
		this_thread::sleep_for(chrono::milliseconds(10));
	}
	
	for(list<Socket::socket_type>::iterator i = SocketList.begin(); i != SocketList.end(); i++)
	{
		OnClientDisconnect(*i);
		Socket::Close(*i);
	}
	SocketList.clear();
	
	printf("Waiting for finish async DB tasks...\n");
	AsyncDBProcessThread.join();
	
	printf("MatchServer shutting down...\n");
	mlog("Shutting down the server...");
	
	Socket::Close(peer_socket);
	Socket::Close(server_socket);
	Socket::UnInitialize();
	
	Db_Disconnect();
	
	g_File.Close();
	
	wxUninitialize();
	
	return 0;
}
