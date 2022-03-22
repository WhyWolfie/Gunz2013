#ifndef __MMATCHCHANNEL_H__
#define __MMATCHCHANNEL_H__

#include "MMatchObject.h"

#define MXML_FILENAME_CHANNEL		"channel.xml"

#define MCHANNEL_DEFAULT_RULENAME	"novice"
#define MCHANNEL_DEFAULT_RULEID		0

#define MAX_CHANNEL_PLAYERLIST_COUNT	6
#define DEFAULT_CHANNEL_MAXPLAYERS		200

// channel type.
#define MCHANNEL_TYPE_PRESET			0
#define MCHANNEL_TYPE_USER				1
#define MCHANNEL_TYPE_CLAN				3
#define MCHANNEL_TYPE_DUELTOURNAMENT	5
#define MCHANNEL_TYPE_CLASSIC			6
#define MCHANNEL_TYPE_BLITZKRIEG		7

// channel type flags for enum.
#define MCHANNEL_ENUMTYPE_PRESET			1
#define MCHANNEL_ENUMTYPE_USER				2
#define MCHANNEL_ENUMTYPE_CLAN				4
#define MCHANNEL_ENUMTYPE_DUELTOURNAMENT	8
#define MCHANNEL_ENUMTYPE_CLASSIC			16
#define MCHANNEL_ENUMTYPE_BLITZKRIEG		32

struct MChannelPlayerList
{
	int nTotalCount;
	MMatchObject *pObject[MAX_CHANNEL_PLAYERLIST_COUNT];
	
	unsigned int nChecksum;
};

class MMatchChannel
{
public:
	// dtor.
	~MMatchChannel()
	{
		m_Users.clear();
	}
	
public:
	// uid.
	void SetUID(const MUID &uid)
	{
		m_uidChannel = uid;
	}
	const MUID &GetUID()
	{
		return m_uidChannel;
	}
	
	// channel name.
	void SetName(const char *pszName)
	{
		strcpy(m_szName, pszName);
	}
	const char *GetName()
	{
		return m_szName;
	}
	
	// channel type.
	void SetType(int nType)
	{
		m_nType = (unsigned char)nType;
	}
	int GetType()
	{
		return (int)m_nType;
	}
	
	// rule name.
	void SetRule(const char *pszName)
	{
		strcpy(m_szRule, pszName);
	}
	const char *GetRule()
	{
		return m_szRule;
	}
	
	// rule id.
	void SetRuleID(int nID)
	{
		m_nRuleID = nID;
	}
	int GetRuleID()
	{
		return m_nRuleID;
	}
	
	// allowed players.
	void SetMaxPlayers(int nMaxPlayers)
	{
		m_nMaxPlayers = (unsigned char)nMaxPlayers;
	}
	int GetMaxPlayers()
	{
		return (int)m_nMaxPlayers;
	}
	
	// duel tournament checker.
	bool IsDuelTournament()
	{
		return m_nType == MCHANNEL_TYPE_DUELTOURNAMENT ? true : false ;
	}
	
	// blitz krieg checker.
	bool IsBlitzKrieg()
	{
		return m_nType == MCHANNEL_TYPE_BLITZKRIEG ? true : false ;
	}
	
	// curr players.
	int GetCurrPlayers()
	{
		return (int)m_Users.size();
	}
	
	// max players check.
	bool IsMax()
	{
		return (int)m_Users.size() >= (int)m_nMaxPlayers ? true : false ;
	}
	
	// mmatchobject add/remove.
	bool Enter(MMatchObject *pObj);
	bool Leave(MMatchObject *pObj);
	
	// lobby list.
	void RetrievePlayerList(MChannelPlayerList *pOut, int nPage = 0);
	
	// (static) default rule.
	static void SetDefaultRule(const char *pszName)	
	{
		strcpy(m_szDefaultRule, pszName);
	}
	static const char *GetDefaultRule()	
	{
		return m_szDefaultRule;
	}
	
	// (static) default rule id.
	static void SetDefaultRuleID(int nID)
	{
		m_nDefaultRuleID = nID;
	}
	static int GetDefaultRuleID()
	{
		return m_nDefaultRuleID;
	}
	
private:
	MUID m_uidChannel;
	
	char m_szName[128];
	unsigned char m_nType;
	
	char m_szRule[32];
	int m_nRuleID;
	
	unsigned char m_nMaxPlayers;
	
	static char m_szDefaultRule[32];
	static int m_nDefaultRuleID;
	
	map<MUID, MMatchObject *> m_Users;
};


inline bool IsValidChannelType(int nType)
{
	return (nType == MCHANNEL_TYPE_PRESET || 
			nType == MCHANNEL_TYPE_USER || 
			nType == MCHANNEL_TYPE_CLAN || 
			nType == MCHANNEL_TYPE_DUELTOURNAMENT || 
			nType == MCHANNEL_TYPE_CLASSIC || 
			nType == MCHANNEL_TYPE_BLITZKRIEG)
			? true : false ;
}

class MMatchChannelManager
{
public:
	MMatchChannelManager();
	~MMatchChannelManager();
	
	bool LoadPreset();
	
	bool Add(MMatchChannel *pChannel, MUID *pOutUID = NULL);
	bool Add(const char *pszName, int nType, MUID *pOutUID = NULL);
	MMatchChannel *Get(const MUID &uidChannel);
	bool Remove(const MUID &uidChannel);
	bool Remove(MMatchChannel *pChannel);
	bool Enum(unsigned long nChannelFlag, vector<MMatchChannel *> *pOut);
	
	static int ConvertChannelTypeNameToID(const char *pszTypeName);
	
	list<MMatchChannel *>::iterator Begin()	{ return m_Channels.begin(); }
	list<MMatchChannel *>::iterator End()	{ return m_Channels.end(); }
	
private:
	list<MMatchChannel *> m_Channels;
};

extern MMatchChannelManager g_ChannelMgr;

#endif