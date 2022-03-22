#ifndef __MLOCATORCOMMANDBLOB_H__
#define __MLOCATORCOMMANDBLOB_H__

#pragma pack(1)

#if defined(_AGENT_PING) //defined(LOCALE_NHNUSA)
struct MTD_ServerStatusInfo
{
	unsigned long		nIP;
	unsigned long		nAgentIP;
	int					nPort;
	unsigned char		nServerID;
	short				nMaxPlayer;
	short				nCurPlayer;
	char				nType;
	bool				bIsLive;
	char				szServerName[64];
};
#else
struct MTD_ServerStatusInfo
{
	unsigned long		nIP;
	int					nPort;
	unsigned char		nServerID;
	short				nMaxPlayer;
	short				nCurPlayer;
	char				nType;
	bool				bIsLive;
	char				szServerName[64];
};
#endif

#pragma pack()

#endif