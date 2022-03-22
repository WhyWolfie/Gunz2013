#include "pch.h"

#include "MMatchObject.h"

#include "MMatchChatRoom.h"
#include "MMatchAgent.h"

// send queue. --------------------
struct MSendData
{
	MUID uidClient;
	unsigned char *pData;
	int nDataSize;
};

vector<MSendData *> g_vtQueuedSendData;

void AddDataToQueue(const MUID &uidClient, unsigned char *pData, int nDataSize)
{
	MSendData *pNew = new MSendData;
	pNew->uidClient = uidClient;
	pNew->pData = pData;
	pNew->nDataSize = nDataSize;
	g_vtQueuedSendData.push_back(pNew);
}

void ClearQueuedData()
{
	for(vector<MSendData *>::iterator i = g_vtQueuedSendData.begin(); i != g_vtQueuedSendData.end(); i++)
	{
		MSendData *pCurr = (*i);
		
		delete[] pCurr->pData;
		delete pCurr;
	}
	g_vtQueuedSendData.clear();
}

void SendQueueDataToClient(MMatchObject *pObj)
{
	MUID uidClient = pObj->GetUID();
	
	vector<MSendData *> vtQueueData;
	int nSize = 0;
	
	for(vector<MSendData *>::iterator i = g_vtQueuedSendData.begin(); i != g_vtQueuedSendData.end(); i++)
	{
		MSendData *pInfo = (*i);
		
		if(pInfo->uidClient == uidClient)
		{
			vtQueueData.push_back(pInfo);
			nSize += pInfo->nDataSize;
		}
	}
	
	if(nSize <= 0) return;
	
	unsigned char *pNew = new unsigned char[nSize];
	
	int nCurrIndex = 0;
	for(vector<MSendData *>::iterator i = vtQueueData.begin(); i != vtQueueData.end(); i++)
	{
		MSendData *pInfo = (*i);
		
		memcpy(&pNew[nCurrIndex], pInfo->pData, pInfo->nDataSize);
		nCurrIndex += pInfo->nDataSize;
	}

	Socket::socket_type s = pObj->GetSocket();
	Socket::Send(s, (const char *)pNew, nSize);
	
	delete[] pNew;
}

// this function is must to used with thread mutex.
void SendQueuedData()
{
	for(list<MMatchObject *>::iterator i = g_ObjectMgr.Begin(); i != g_ObjectMgr.End(); i++)
		SendQueueDataToClient((*i));
		
	ClearQueuedData();
}

void RouteToQueue(MCommandWriter *pCmd, MMatchObject *pObj)
{
	if(pCmd->IsReadyToUse() == false)
		return;

	const int nIndex = pCmd->GetDataIndex();

	unsigned char *pNewData = new unsigned char[nIndex];
	PacketEncrypter(pCmd, &pNewData[0], &pObj->GetCryptKey()[0]);
	
	AddDataToQueue(pObj->GetUID(), pNewData, nIndex);
}

// send function. --------------------
void SendToClient(MCommandWriter *pCmd, Socket::socket_type s, const unsigned char *pCryptKey)
{
	if(pCmd->IsReadyToUse() == false)
		return;

	const int nIndex = pCmd->GetDataIndex();

	unsigned char *pNewData = new unsigned char[nIndex];

	if(PacketEncrypter(pCmd, &pNewData[0], &pCryptKey[0]) == false)
	{
		printf("::PacketEncrypter() failed, at ::SendToClient().\n");
		delete[] pNewData;
		return;
	}
	
	// static mutex SendMutex;
	
	// SendMutex.lock();
	Socket::Send(s, (const char *)&pNewData[0], nIndex);
	// SendMutex.unlock();

	delete[] pNewData;
}

void SendToClient(MCommandWriter *pCmd, const MUID &uidClient)
{
	if(pCmd->IsReadyToUse() == false)
		return;
		
	MMatchObject *pObj = g_ObjectMgr.Get(uidClient);
	if(pObj == NULL)
		return;
		
	// SendToClient(pCmd, pObj->GetSocket(), pObj->GetCryptKey());
	RouteToQueue(pCmd, pObj);
}

void SendToAll(MCommandWriter *pCmd)
{
	if(pCmd->IsReadyToUse() == false)
		return;
		
	for(list<MMatchObject *>::iterator i = g_ObjectMgr.Begin(); i != g_ObjectMgr.End(); i++)
	{
		MMatchObject *pCurrObj = (*i);
		// SendToClient(pCmd, pCurrObj->GetSocket(), pCurrObj->GetCryptKey());
		RouteToQueue(pCmd, pCurrObj);
	}
}

void SendToChannel(MCommandWriter *pCmd, const MUID &uidChannel)
{
	if(uidChannel == MUID(0, 0))
		return;
		
	if(pCmd->IsReadyToUse() == false)
		return;
		
	for(list<MMatchObject *>::iterator i = g_ObjectMgr.Begin(); i != g_ObjectMgr.End(); i++)
	{
		MMatchObject *pCurrObj = (*i);
		
		if(pCurrObj->m_uidChannel == uidChannel)
			// SendToClient(pCmd, pCurrObj->GetSocket(), pCurrObj->GetCryptKey());
			RouteToQueue(pCmd, pCurrObj);
	}
}

void SendToStage(MCommandWriter *pCmd, const MUID &uidStage, bool bSkipInGame)
{
	if(uidStage == MUID(0, 0))
		return;
		
	if(pCmd->IsReadyToUse() == false)
		return;
		
	for(list<MMatchObject *>::iterator i = g_ObjectMgr.Begin(); i != g_ObjectMgr.End(); i++)
	{
		MMatchObject *pCurrObj = (*i);
		
		if(pCurrObj->m_uidStage == uidStage)
			if(bSkipInGame == false || pCurrObj->CheckGameFlag(MMOGF_INGAME) == false)
				// SendToClient(pCmd, pCurrObj->GetSocket(), pCurrObj->GetCryptKey());
				RouteToQueue(pCmd, pCurrObj);
	}
}

void SendToBattle(MCommandWriter *pCmd, const MUID &uidStage)
{
	if(uidStage == MUID(0, 0))
		return;
		
	if(pCmd->IsReadyToUse() == false)
		return;
		
	for(list<MMatchObject *>::iterator i = g_ObjectMgr.Begin(); i != g_ObjectMgr.End(); i++)
	{
		MMatchObject *pCurrObj = (*i);
		
		if(pCurrObj->m_uidStage == uidStage)
			if(pCurrObj->CheckGameFlag(MMOGF_ENTERED) == true)
				// SendToClient(pCmd, pCurrObj->GetSocket(), pCurrObj->GetCryptKey());
				RouteToQueue(pCmd, pCurrObj);
	}
}

void SendToClan(MCommandWriter *pCmd, int nCLID)
{
	if(nCLID == 0)
		return;
		
	if(pCmd->IsReadyToUse() == false)
		return;
		
	for(list<MMatchObject *>::iterator i = g_ObjectMgr.Begin(); i != g_ObjectMgr.End(); i++)
	{
		MMatchObject *pCurrObj = (*i);
		
		if(pCurrObj->m_Clan.nCLID == nCLID)
			// SendToClient(pCmd, pCurrObj->GetSocket(), pCurrObj->GetCryptKey());
			RouteToQueue(pCmd, pCurrObj);
	}
}

void SendToChatRoom(MCommandWriter *pCmd, unsigned long nRoomID)
{
	if(nRoomID == 0)
		return;
		
	if(pCmd->IsReadyToUse() == false)
		return;
		
	for(list<MMatchChatRoom *>::iterator i = g_ChatRoomMgr.Begin(); i != g_ChatRoomMgr.End(); i++)
	{
		// find chatroom by ID.
		MMatchChatRoom *pChatRoom = (*i);
		
		if(pChatRoom->GetID() == nRoomID)
		{
			// send to chatroom client.
			for(map<MUID, MMatchObject *>::iterator j = pChatRoom->Begin(); j != pChatRoom->End(); j++)
			{
				MMatchObject *pObj = (*j).second;
				// SendToClient(pCmd, pObj->GetSocket(), pObj->GetCryptKey());
				RouteToQueue(pCmd, pObj);
			}
		}
	}
}

void SendToChannelListRequester(MCommandWriter *pCmd, int nChannelType)
{
	if(pCmd->IsReadyToUse() == false)
		return;
		
	for(list<MMatchObject *>::iterator i = g_ObjectMgr.Begin(); i != g_ObjectMgr.End(); i++)
	{
		MMatchObject *pCurrObj = (*i);
		
		if(pCurrObj->m_nRequestedChannelListType == nChannelType)
			// SendToClient(pCmd, pCurrObj->GetSocket(), pCurrObj->GetCryptKey());
			RouteToQueue(pCmd, pCurrObj);
	}
}

void SendToAgent(MCommandWriter *pCmd)
{
	if(g_MatchAgent.GetAgentObject() == NULL) return;
	
	SendToClient(pCmd, g_MatchAgent.GetAgentObject()->GetSocket(), g_MatchAgent.GetAgentObject()->GetCryptKey());
}
