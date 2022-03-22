#ifndef __MMATCHCHATROOM_H__
#define __MMATCHCHATROOM_H__

#include "MUID.h"
#include "MMatchObject.h"

// for MMatchObject, max joinable rooms.
#define MAX_CHATROOM_JOINABLE_COUNT	3

// for this MMatchChatRoom.
#define MAX_CHATROOM_NAME_LENGTH	64
#define MAX_CHATROOM_PLAYER_COUNT	16

class MMatchChatRoom
{
public:
	MMatchChatRoom(const char *pszName);
	~MMatchChatRoom();
	
	unsigned long GetID()	{ return m_nID; }
	const char *GetName()	{ return m_szName; }
	
	bool Join(MMatchObject *pObj);
	bool Leave(MMatchObject *pObj);
	
	map<MUID, MMatchObject *>::iterator Begin()	{ return m_Users.begin(); }
	map<MUID, MMatchObject *>::iterator End()	{ return m_Users.end(); }
	
	bool IsMax()	{ return m_Users.size() >= MAX_CHATROOM_PLAYER_COUNT ? true : false ; }
	bool IsEmpty()	{ return m_Users.empty(); }
	
private:
	unsigned long m_nID;
	char m_szName[MAX_CHATROOM_NAME_LENGTH];
	
	map<MUID, MMatchObject *> m_Users;
};

class MMatchChatRoomManager
{
public:
	MMatchChatRoomManager();
	~MMatchChatRoomManager();

	MMatchChatRoom *Get(const char *pszRoomName);
	MMatchChatRoom *Get(unsigned long nID);
	MMatchChatRoom *Create(const char *pszRoomName);
	void Remove(const char *pszRoomName);
	void Remove(unsigned long nID);
	void Clear();
	
	list<MMatchChatRoom *>::iterator Begin()	{ return m_ChatRoomList.begin(); }
	list<MMatchChatRoom *>::iterator End()		{ return m_ChatRoomList.end(); }
	
private:
	list<MMatchChatRoom *> m_ChatRoomList;
};

extern MMatchChatRoomManager g_ChatRoomMgr;

#endif