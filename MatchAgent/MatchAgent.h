#ifndef __MATCHAGENT_H__
#define __MATCHAGENT_H__

enum MAgentClientPeerType
{
	MACPT_IDLE, 
	MACPT_TCP, 
	MACPT_UDP
};

class MAgentClient
{
public:
	MAgentClient(const MUID &uidClient, const MUID &uidStage, const char *pszIP, unsigned short nPort);
	
	const MUID &GetUID()	{ return m_uidClient; }
	const MUID &GetStageUID()	{ return m_uidStage; }
	
	MAgentClientPeerType GetPeerType()	{ return m_nPeerType; }
	void SetPeerType(MAgentClientPeerType type)	{ m_nPeerType = type; }
	
	const char *GetIPStr()	{ return m_szIP; }
	unsigned long GetIP()	{ return m_nIP; }
	
	unsigned short GetPort()	{ return m_nPort; }
	
	void SetAddress(const char *pszIP, unsigned short nPort);
	
private:
	MUID m_uidClient;
	MUID m_uidStage;

	MAgentClientPeerType m_nPeerType;
	
	char m_szIP[64];
	unsigned long m_nIP;
	
	unsigned short m_nPort;
};

class MStageAgent
{
public:
	MStageAgent(const MUID &uidStage);
	
	const MUID &GetUID()	{ return m_uidStage; }
	
	void Enter(MAgentClient *pClient);
	void Leave(MAgentClient *pClient);
	
	MAgentClient *GetClient(MAgentClient *pClient);
	
	list<MAgentClient *>::iterator ObjBegin()	{ return m_Users.begin(); }
	list<MAgentClient *>::iterator ObjEnd()		{ return m_Users.end(); }
	
	bool IsEmpty()	{ return m_Users.empty(); }
	
private:
	MUID m_uidStage;
	
	list<MAgentClient *> m_Users;
};

// -----------------------------------.

class MAgentClientManager
{
public:
	MAgentClientManager();
	~MAgentClientManager();
	
	void Remove(const MUID &uidClient);
	MAgentClient *Add(const MUID &uidClient, const MUID &uidStage, const char *pszIP, unsigned short nPort);
	MAgentClient *Get(const MUID &uidClient);
	
	MUID FindUID(unsigned long nIP, unsigned short nPort);
	
	list<MAgentClient *>::iterator Begin()	{ return m_Clients.begin(); }
	list<MAgentClient *>::iterator End()	{ return m_Clients.end(); }
	
private:
	list<MAgentClient *> m_Clients;
};

class MStageAgentManager
{
public:
	MStageAgentManager();
	~MStageAgentManager();
	
	void Remove(const MUID &uidStage);
	MStageAgent *Add(const MUID &uidStage);
	MStageAgent *Get(const MUID &uidStage);
	
private:
	list<MStageAgent *> m_Stages;
};

extern MAgentClientManager g_ClientMgr;
extern MStageAgentManager g_StageMgr;

// -----------------------------------.

struct MAgentTunnelingData
{
	unsigned char nData[1024];
	int nSize;
};

void SendToServer(MCommandWriter *pCmd);

bool MMatchAgent_OnCommand(MCommandReader *pRecvCmd);
bool MMatchAgent_OnPeerCommand(MCommandReader *pRecvCmd);

#endif