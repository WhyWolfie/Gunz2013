#include "pch.h"
#include "MMatchAgent.h"

#include "MServerSetting.h"

MMatchObject_Agent::MMatchObject_Agent(Socket::socket_type s, const MUID &uid, const unsigned char *pCryptKey)
{
	m_Socket = s;
	m_uid = uid;
	memcpy(&m_nCryptKey[0], &pCryptKey[0], sizeof(unsigned char) * ENCRYPTIONKEY_LENGTH);
	
	// init socket address.
	Socket::address_type addr;
	Socket::GetPeerName(s, &addr);
	
	char szIP[64];
	strcpy(szIP, Socket::InetNtoa(&addr));
	
	strcpy(m_szIP, szIP);
	m_nIP = Socket::InetAddr(szIP);
	m_nPort = Socket::Ntohs(addr.sin_port);
	
	strcpy(m_szAgentIP, "");
	m_nAgentIP = 0;
	m_nUDPPort = 0;
}

MMatchObject_Agent::~MMatchObject_Agent()
{
}

void MMatchObject_Agent::SetAgentAddress(const char *pszIP, unsigned short nUDPPort)
{
	strcpy(m_szAgentIP, pszIP);
	m_nIP = Socket::InetAddr(pszIP);
	m_nUDPPort = nUDPPort;
}


MMatchAgent g_MatchAgent;

MMatchAgent::MMatchAgent()
{
	m_pAgentObject = NULL;
}

MMatchAgent::~MMatchAgent()
{
	UnsetAgentObject();
}

bool MMatchAgent::IsValidAgentIP(const char *pszIP)
{
	static const char *pszValidIP = g_ServerConfig.AgentSetting.szTrustedIP;
	return strcmp(pszValidIP, pszIP) == 0 ? true : false ;
}

void MMatchAgent::UnsetAgentObject()
{
	if(m_pAgentObject != NULL)
	{
		delete m_pAgentObject;
		m_pAgentObject = NULL;
	}
}

void MMatchAgent::SetAgentObject(Socket::socket_type s, const MUID &uid, const unsigned char *pCryptKey)
{
	UnsetAgentObject();
	m_pAgentObject = new MMatchObject_Agent(s, uid, pCryptKey);
}

void MMatchAgent::SetAgentAddressInfo(const char *pszIP, unsigned short nUDPPort)
{
	if(m_pAgentObject == NULL) return;
	m_pAgentObject->SetAgentAddress(pszIP, nUDPPort);
}

bool MMatchAgent::IsAgentClient(Socket::socket_type s)
{
	if(m_pAgentObject == NULL) return false;
	return m_pAgentObject->GetSocket() == s ? true : false ;
}

bool MMatchAgent::GetAgentBasicInfo(MUID *pOutUID, unsigned char *pOutCryptKey)
{
	if(m_pAgentObject == NULL) return false;
	
	(*pOutUID) = m_pAgentObject->GetUID();
	memcpy(pOutCryptKey, m_pAgentObject->GetCryptKey(), sizeof(unsigned char) * ENCRYPTIONKEY_LENGTH);
	
	return true;
}

bool MMatchAgent::IsAgentUID(const MUID &uid)
{
	if(m_pAgentObject == NULL) return false;
	return m_pAgentObject->GetUID() == uid ? true : false ;
}