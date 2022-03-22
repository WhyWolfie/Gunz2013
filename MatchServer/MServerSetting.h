#ifndef __MSERVERSETTING_H__
#define __MSERVERSETTING_H__

// ---------- server config ----------

// server config file name.
#define MCONFIG_FILENAME	"matchserver.xml"//"serverconf.xml"

// Server mode.
#define MSM_NORMAL	0
#define MSM_CLAN	1
#define MSM_QUEST	4
#define MSM_BOTH	5

// Server mode string.
#define MSM_NORMAL_STR	"match"
#define MSM_CLAN_STR	"clan"
#define MSM_QUEST_STR	"quest"
#define MSM_BOTH_STR	"both"

// Locator server type.
#define MLOCATOR_SERVERTYPE_DEBUG	1
#define MLOCATOR_SERVERTYPE_MATCH	2
#define MLOCATOR_SERVERTYPE_CLAN	3
#define MLOCATOR_SERVERTYPE_QUEST	4
#define MLOCATOR_SERVERTYPE_EVENT	5
#define MLOCATOR_SERVERTYPE_UNKNOWN	7

// Locator server type string.
#define MLOCATOR_SERVERTYPE_DEBUG_STR	"debug"
#define MLOCATOR_SERVERTYPE_MATCH_STR	"match"
#define MLOCATOR_SERVERTYPE_CLAN_STR	"clan"
#define MLOCATOR_SERVERTYPE_QUEST_STR	"quest"
#define MLOCATOR_SERVERTYPE_EVENT_STR	"event"
#define MLOCATOR_SERVERTYPE_UNKNOWN_STR	"unknown"

// Default settings.
#define MCONFIG_DEFAULT_SERVER_TCPPORT			6000
#define MCONFIG_DEFAULT_SERVER_UDPPORT			7777
#define MCONFIG_DEFAULT_SERVER_MAXPLAYERS		500
#define MCONFIG_DEFAULT_SERVER_NAME				"MatchServer1"
#define MCONFIG_DEFAULT_SERVER_SURVIVAL			true
#define MCONFIG_DEFAULT_SERVER_DUELTOURNAMENT	true
#define MCONFIG_DEFAULT_SERVER_MODE				MSM_QUEST

#define MCONFIG_DEFAULT_DATABASE_USERNAME	"postgres"
#define MCONFIG_DEFAULT_DATABASE_PASSWORD	"password"
#define MCONFIG_DEFAULT_DATABASE_DSN		"gunzdb"

#define MCONFIG_DEFAULT_AGENT_TRUSTED_IP	"127.0.0.1"

#define MCONFIG_DEFAULT_LOCATOR_SERVERID	1
#define MCONFIG_DEFAULT_LOCATOR_SERVERIP	"127.0.0.1"
#define MCONFIG_DEFAULT_LOCATOR_SERVERTYPE	MLOCATOR_SERVERTYPE_QUEST

#define MCONFIG_DEFAULT_OTHER_IPBAN			false
#define MCONFIG_DEFAULT_OTHER_IPACCOUNT		false

struct MConfig
{

	struct
	{
		unsigned short nPort;
		unsigned short nUdpPort;
		unsigned short nMaxPlayers;
		char szName[128];
		bool bSurvival;
		bool bDuelTournament;
		unsigned char nServerMode;
	} 
	ServerSetting;

	struct
	{
		char szUsername[128];
		char szPassword[128];
		char szDSN[128];
	} 
	DatabaseSetting;

	struct
	{
		char szTrustedIP[64];
	}
	AgentSetting;

	struct
	{
		int nServerID;
		char szIP[64];
		unsigned char nType;
	} 
	LocatorSetting;

	struct
	{
		bool bIpBan;
		bool bIpAccount;
	} 
	OtherSetting;

};

extern MConfig g_ServerConfig;

// -----------------------------------------.

// Main of XML reader function.
bool LoadServerXML();

#endif
