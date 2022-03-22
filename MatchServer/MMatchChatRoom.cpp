#include "pch.h"
#include "MMatchChatRoom.h"

#include "MMatchObject.h"

MMatchChatRoom::MMatchChatRoom(const char *pszName)
{
	static unsigned long nAssignID = 1;
	m_nID = nAssignID;
	nAssignID++;
	
	// do not set to 0 = an invalid id.
	if(nAssignID == 0) nAssignID = 1;
	
	strcpy(m_szName, pszName);
}

MMatchChatRoom::~MMatchChatRoom()
{
}

bool MMatchChatRoom::Join(MMatchObject *pObj)
{
	if(m_Users.find(pObj->GetUID()) != m_Users.end()) return false;
	
	m_Users.insert(pair<MUID, MMatchObject *>(pObj->GetUID(), pObj));
	return true;
}

bool MMatchChatRoom::Leave(MMatchObject *pObj)
{
	m_Users.erase(pObj->GetUID());
	return true;
}

// ------------------------------------------------------- .

MMatchChatRoomManager g_ChatRoomMgr;

MMatchChatRoomManager::MMatchChatRoomManager()
{
}

MMatchChatRoomManager::~MMatchChatRoomManager()
{
	Clear();
}

MMatchChatRoom *MMatchChatRoomManager::Get(const char *pszRoomName)
{
	for(list<MMatchChatRoom *>::iterator i = m_ChatRoomList.begin(); i != m_ChatRoomList.end(); i++)
	{
		MMatchChatRoom *pCurr = (*i);
		
		if(MStricmp(pCurr->GetName(), pszRoomName) == 0)
		{
			return pCurr;
		}
	}
	
	return NULL;
}

MMatchChatRoom *MMatchChatRoomManager::Get(unsigned long nID)
{
	for(list<MMatchChatRoom *>::iterator i = m_ChatRoomList.begin(); i != m_ChatRoomList.end(); i++)
	{
		MMatchChatRoom *pCurr = (*i);
		
		if(pCurr->GetID() == nID)
		{
			return pCurr;
		}
	}
	
	return NULL;
}

MMatchChatRoom *MMatchChatRoomManager::Create(const char *pszRoomName)
{
	MMatchChatRoom *pChatRoom = Get(pszRoomName);
	if(pChatRoom != NULL) return pChatRoom;	// return existing chat room.
	
	MMatchChatRoom *pNew = new MMatchChatRoom(pszRoomName);
	m_ChatRoomList.push_back(pNew);
	return pNew;	// return created chat room.
}

void MMatchChatRoomManager::Remove(const char *pszRoomName)
{
	for(list<MMatchChatRoom *>::iterator i = m_ChatRoomList.begin(); i != m_ChatRoomList.end(); i++)
	{
		MMatchChatRoom *pCurr = (*i);
		
		if(MStricmp(pCurr->GetName(), pszRoomName) == 0)
		{
			delete pCurr;
			m_ChatRoomList.erase(i);
			break;
		}
	}
}

void MMatchChatRoomManager::Remove(unsigned long nID)
{
	for(list<MMatchChatRoom *>::iterator i = m_ChatRoomList.begin(); i != m_ChatRoomList.end(); i++)
	{
		MMatchChatRoom *pCurr = (*i);
		
		if(pCurr->GetID() == nID)
		{
			delete pCurr;
			m_ChatRoomList.erase(i);
			break;
		}
	}
}

void MMatchChatRoomManager::Clear()
{
	for(list<MMatchChatRoom *>::iterator i = m_ChatRoomList.begin(); i != m_ChatRoomList.end(); i++)
	{
		delete (*i);
	}
	m_ChatRoomList.clear();
}