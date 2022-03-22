#include "pch.h"

#include "MClientAcceptor.h"
#include "MMatchConstant.h"

#include "MMatchCryptKey.h"

#include "MServerNetwork.h"

MClientAcceptor g_ClientAcceptor;

void InitCryptKey(Socket::socket_type s, const MUID &uidClient, unsigned char *pOutKey)
{
	unsigned long nTime = GetTime();
	MakeCryptKey(uidClient, SERVERUID, (unsigned int)nTime, pOutKey);

	// --------------- send key info ---------------.
	unsigned char nKeyInfo[26];
	
	int IntTemp;
	short ShortTemp;
	
	ShortTemp = 0xA;
	memcpy(&nKeyInfo[0], &ShortTemp, 2);
	ShortTemp = 26;
	memcpy(&nKeyInfo[2], &ShortTemp, 2);
	ShortTemp = 0;
	memcpy(&nKeyInfo[4], &ShortTemp, 2);
	IntTemp = SERVERUID_HIGHID;
	memcpy(&nKeyInfo[6], &IntTemp, 4);
	IntTemp = SERVERUID_LOWID;
	memcpy(&nKeyInfo[10], &IntTemp, 4);
	memcpy(&nKeyInfo[14], &uidClient.ulHighID, 4);
	memcpy(&nKeyInfo[18], &uidClient.ulLowID, 4);
	memcpy(&nKeyInfo[22], &nTime, 4);
	
	Socket::Send(s, (const char *)nKeyInfo, 26);
}

MClientAcceptor::MClientAcceptor()
{
}

MClientAcceptor::~MClientAcceptor()
{
	for(vector<MClientAcceptInfo *>::iterator i = m_Clients.begin(); i != m_Clients.end(); i++)
	{
		MClientAcceptInfo *pCurr = (*i);
		
		delete[] pCurr->pCryptKey;
		delete pCurr;
	}
	m_Clients.clear();
}

bool MClientAcceptor::AddClient(Socket::socket_type s)
{
	// get client address & block existing ip.
	Socket::address_type address;
	Socket::GetPeerName(s, &address);
	
	char szIP[64];
	strcpy(szIP, Socket::InetNtoa(&address));
	
	unsigned long nIP = Socket::InetAddr(szIP);
	
	if(IsExistingClientIP(nIP) == true) 
		return false;
	
	// MUID related.
	MUID uidAssigned = MUID::Assign();

	// Crypt-key related.
	unsigned char *pNewKey = new unsigned char[ENCRYPTIONKEY_LENGTH];
	InitCryptKey(s, uidAssigned, &pNewKey[0]);

	// General.
	MClientAcceptInfo *pNewInfo = new MClientAcceptInfo;

	pNewInfo->s = s;
	pNewInfo->uid = uidAssigned;
	pNewInfo->pCryptKey = pNewKey;
	pNewInfo->bLoggedIn = false;
	pNewInfo->nIP = nIP;
	pNewInfo->bInvalid = false;
	
	// Add to list.
	m_Clients.push_back(pNewInfo);
	
	return true;
}

bool MClientAcceptor::GetClientInfo(Socket::socket_type s, MUID *pOutUid, unsigned char *pOutKey)
{
	if(pOutUid != NULL) *pOutUid = MUID(0, 0);
	if(pOutKey != NULL) ZeroInit(&pOutKey[0], sizeof(unsigned char) * ENCRYPTIONKEY_LENGTH);
	
	for(vector<MClientAcceptInfo *>::iterator it = m_Clients.begin();
	        it != m_Clients.end(); it++)
	{
		MClientAcceptInfo *pCurr = (*it);
		
		if(pCurr->s == s)
		{
			if(pOutUid != NULL) *pOutUid = pCurr->uid;
			if(pOutKey != NULL) memcpy(&pOutKey[0], &pCurr->pCryptKey[0], sizeof(unsigned char) * ENCRYPTIONKEY_LENGTH);
			return true;
		}
	}

	return false;
}

bool MClientAcceptor::GetClientInfo(const MUID &uid, Socket::socket_type *pOutSocket, unsigned char *pOutKey)
{
	if(pOutSocket != NULL) *pOutSocket = INVALID_SOCKET_DESC;
	if(pOutKey != NULL) ZeroInit(&pOutKey[0], sizeof(unsigned char) * ENCRYPTIONKEY_LENGTH);
	
	for(vector<MClientAcceptInfo *>::iterator it = m_Clients.begin();
	        it != m_Clients.end(); it++)
	{
		MClientAcceptInfo *pCurr = (*it);
		
		if(pCurr->uid == uid)
		{
			if(pOutSocket != NULL) *pOutSocket = pCurr->s;
			if(pOutKey != NULL) memcpy(&pOutKey[0], &pCurr->pCryptKey[0], sizeof(unsigned char) * ENCRYPTIONKEY_LENGTH);
			return true;
		}
	}

	return false;
}

bool *MClientAcceptor::GetClientLoginState(const MUID &uid)
{
	for(vector<MClientAcceptInfo *>::iterator it = m_Clients.begin();
	        it != m_Clients.end(); it++)
	{
		MClientAcceptInfo *pCurr = (*it);
		
		if(pCurr->uid == uid)
		{
			return &pCurr->bLoggedIn;
		}
	}
	
	return NULL;
}

bool MClientAcceptor::IsExistingClientIP(unsigned long nIP)
{
	for(vector<MClientAcceptInfo *>::iterator it = m_Clients.begin();
	        it != m_Clients.end(); it++)
	{
		MClientAcceptInfo *pCurr = (*it);
		
		if(pCurr->nIP == nIP)
		{
			return true;
		}
	}
	
	return false;
}

void MClientAcceptor::InvalidateClient(const MUID &uid)
{
	for(vector<MClientAcceptInfo *>::iterator it = m_Clients.begin();
	        it != m_Clients.end(); it++)
	{
		MClientAcceptInfo *pCurr = (*it);
		
		if(pCurr->uid == uid)
		{
			pCurr->bInvalid = true;
			break;
		}
	}
}

void MClientAcceptor::RemoveClient(Socket::socket_type s)
{
	for(vector<MClientAcceptInfo *>::iterator it = m_Clients.begin();
	        it != m_Clients.end(); it++)
	{
		MClientAcceptInfo *pCurr = (*it);
		
		if(pCurr->s == s)
		{
			delete[] pCurr->pCryptKey;
			delete pCurr;
			m_Clients.erase(it);
			
			break;
		}
	}
}

void MClientAcceptor::KickInvalidClient()
{
	vector<MClientAcceptInfo *>::iterator it = m_Clients.begin();
	
	while(it != m_Clients.end())
	{
		MClientAcceptInfo *pCurr = (*it);
		
		if(pCurr->bInvalid == false)
		{
			it++;
			continue;
		}
		
		ReserveDisconnect(pCurr->s);
		
		delete[] pCurr->pCryptKey;
		delete pCurr;
		
		it = m_Clients.erase(it);
	}
}