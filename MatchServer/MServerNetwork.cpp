#include "pch.h"

#include "MServerNetwork.h"
#include "MClientAcceptor.h"

#include "MMatchConstant.h"

#include "MMatchServer_OnCommand.h"
#include "MMatchObject.h"

#include "MMatchAgent.h"

/*
	Remaining data queues.
*/

struct MRemainData
{
	Socket::socket_type s;
	vector<unsigned char> vtData;
};

class MRemainDataQueue
{
	#define MAX_REMAINDATA_QUEUE_LENGTH	 4096
	
public:
	~MRemainDataQueue();
	
	vector<unsigned char> *PushData(Socket::socket_type s, const unsigned char *p, const int n);
	void DeleteData(Socket::socket_type s);
	
private:
	vector<unsigned char> *FindData(Socket::socket_type s);
	void ConcatData(vector<unsigned char> *pDest, const unsigned char *p, const int n);
	
	list<MRemainData *> m_DataList;
};

MRemainDataQueue::~MRemainDataQueue()
{
	for(list<MRemainData *>::iterator i = m_DataList.begin(); i != m_DataList.end(); i++)
	{
		delete (*i);
	}
	m_DataList.clear();
}

vector<unsigned char> *MRemainDataQueue::PushData(Socket::socket_type s, const unsigned char *p, const int n)
{
	vector<unsigned char> *pvtData = FindData(s);
	
	if(pvtData != NULL)
	{
		ConcatData(pvtData, p, n);
		return pvtData;
	}
	
	MRemainData *pNew = new MRemainData;
	pNew->s = s;
	ConcatData(&pNew->vtData, p, n);
	m_DataList.push_back(pNew);
	
	return &pNew->vtData;
}

void MRemainDataQueue::DeleteData(Socket::socket_type s)
{
	for(list<MRemainData *>::iterator i = m_DataList.begin(); i != m_DataList.end(); i++)
	{
		MRemainData *pCurr = (*i);
		
		if(pCurr->s == s)
		{
			delete pCurr;
			m_DataList.erase(i);
			break;
		}
	}
}

vector<unsigned char> *MRemainDataQueue::FindData(Socket::socket_type s)
{
	for(list<MRemainData *>::iterator i = m_DataList.begin(); i != m_DataList.end(); i++)
	{
		MRemainData *pCurr = (*i);
		if(pCurr->s == s) return &pCurr->vtData;
	}
	return NULL;
}

void MRemainDataQueue::ConcatData(vector<unsigned char> *pDest, const unsigned char *p, const int n)
{
	pDest->reserve((unsigned int)pDest->size() + (unsigned int)n);
	
	for(int i = 0; i < n; i++)
		pDest->push_back(p[i]);
}

MRemainDataQueue g_RemainDataQueue;
// --------------------------------------------------

void OnClientConnect(Socket::socket_type s)
{
#ifdef _SHOW_RECVCMD_ID
	// Display connected client info.
	// ---------------------------------------- .
	Socket::address_type addr;
	Socket::GetPeerName(s, &addr);
	
	char szIP[64];
	strcpy(szIP, Socket::InetNtoa(&addr));
	
	printf("%s:%u client connected.\n", szIP, Socket::Ntohs(addr.sin_port));
	// ---------------------------------------- .
#endif

	if(g_ClientAcceptor.AddClient(s) == false)
	{
		ReserveDisconnect(s);
		return;
	}
}

bool OnClientRecv(Socket::socket_type s, const char *data, const int size)
{
	MUID uidSender = MUID(0, 0);
	unsigned char nSenderKey[ENCRYPTIONKEY_LENGTH];
	
	// try to get from acceptor list.
	if(g_ClientAcceptor.GetClientInfo(s, &uidSender, nSenderKey) == false)
	{
		// not found in acceptor list, try to get from object list.
		if(g_MatchAgent.IsAgentClient(s) == true)
		{
			// this is match agent.
			g_MatchAgent.GetAgentBasicInfo(&uidSender, nSenderKey);
		}
		else
		{
			// this is player.
			MMatchObject *pObj = g_ObjectMgr.Get(s);
			if(pObj != NULL)
			{
				uidSender = pObj->GetUID();
				memcpy(nSenderKey, pObj->GetCryptKey(), sizeof(unsigned char) * ENCRYPTIONKEY_LENGTH);
			}
		}
	}
	
	if(uidSender == MUID(0, 0))
	{
		return false;
	}
	
	// process packet.
	vector<unsigned char> *pvtData = g_RemainDataQueue.PushData(s, (const unsigned char *)data, size);
	int nDataSize = (int)pvtData->size();
	
	if(nDataSize >= MAX_REMAINDATA_QUEUE_LENGTH)
	{
		return false;
	}
	
	unsigned char *pData = new unsigned char[nDataSize];
	for(int i = 0; i < nDataSize; i++) pData[i] = (*pvtData)[i];
	
	int nRes = PacketDecrypter(pData, nDataSize, nSenderKey);
	if(nRes == -1)
	{
		delete[] pData;
		return false;
	}
	else if(nRes == nDataSize)
	{
		vector<MCommandReader *> vtCmdList;
		
		if(BuildReaderCommand(pData, nRes, uidSender, &vtCmdList) == false)
		{
			delete[] pData;
			return false;
		}
		
		MCommandAdd(&vtCmdList);
		
		pvtData->clear();
	}
	
	delete[] pData;
	return true;
}

void OnClientDisconnect(Socket::socket_type s)
{
#ifdef _SHOW_RECVCMD_ID
	Socket::address_type addr;
	Socket::GetPeerName(s, &addr);
	
	char szIP[64];
	strcpy(szIP, Socket::InetNtoa(&addr));
	
	printf("%s:%u client disconnected.\n", szIP, Socket::Ntohs(addr.sin_port));
#endif

	g_ClientAcceptor.RemoveClient(s);
	if(g_MatchAgent.IsAgentClient(s) == true) 
	{
		MUID uidAgent = g_MatchAgent.GetAgentObject()->GetUID();
		printf("Un-setting Agent server... (UID %lu:%lu)\n", uidAgent.ulHighID, uidAgent.ulLowID);
		
		g_MatchAgent.UnsetAgentObject();
	}
	else g_ObjectMgr.Erase(s);
	g_RemainDataQueue.DeleteData(s);
}

// --------------------------------------------- .

vector<Socket::socket_type> g_vtCloseSocket;

void ReserveDisconnect(Socket::socket_type s)
{
	Socket::Shutdown(s);
	g_vtCloseSocket.push_back(s);
}

bool CheckDisconnectReserved(Socket::socket_type s)
{
	for(vector<Socket::socket_type>::iterator i = g_vtCloseSocket.begin(); i != g_vtCloseSocket.end(); i++)
	{
		Socket::socket_type curr = (*i);
		if(curr == s) return true;
	}
	return false;
}

void ClearDisconnectReservedSocket()
{
	g_vtCloseSocket.clear();
}
