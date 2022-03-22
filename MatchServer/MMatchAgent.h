#ifndef __MMATCHAGENT_H__
#define __MMATCHAGENT_H__

class MMatchObject_Agent
{
public:
	MMatchObject_Agent(Socket::socket_type s, const MUID &uid, const unsigned char *pCryptKey);
	~MMatchObject_Agent();

	Socket::socket_type GetSocket()	{ return m_Socket; }
	const MUID &GetUID()	{ return m_uid; }
	const unsigned char *GetCryptKey()	{ return m_nCryptKey; }
	
	const char *GetIPString()	{ return m_szIP; }
	unsigned long GetIP()	{ return m_nIP; }
	unsigned short GetPort()	{ return m_nPort; }
	
	void SetAgentAddress(const char *pszIP, unsigned short nUDPPort);
	
	unsigned long GetAgentIP()	{ return m_nAgentIP; }
	const char *GetAgentIPString()	{ return m_szAgentIP; }
	
	unsigned short GetUDPPort()	{ return m_nUDPPort; }

private:
	Socket::socket_type m_Socket;

	MUID m_uid;
	unsigned char m_nCryptKey[ENCRYPTIONKEY_LENGTH];
	
	char m_szIP[64];
	unsigned long m_nIP;
	unsigned short m_nPort;
	
	char m_szAgentIP[64];
	unsigned long m_nAgentIP;
	
	unsigned short m_nUDPPort;
};

class MMatchAgent
{
public:
	MMatchAgent();
	~MMatchAgent();
	
	bool IsValidAgentIP(const char *pszIP);
	void UnsetAgentObject();
	void SetAgentObject(Socket::socket_type s, const MUID &uid, const unsigned char *pCryptKey);
	void SetAgentAddressInfo(const char *pszIP, unsigned short nUDPPort);
	bool IsAgentClient(Socket::socket_type s);
	bool GetAgentBasicInfo(MUID *pOutUID, unsigned char *pOutCryptKey);
	bool IsAgentUID(const MUID &uid);
	
	MMatchObject_Agent *GetAgentObject()	{ return m_pAgentObject; }
	
private:
	MMatchObject_Agent *m_pAgentObject;
};

extern MMatchAgent g_MatchAgent;

#endif