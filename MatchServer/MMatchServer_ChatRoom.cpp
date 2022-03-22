#include "pch.h"
#include "MMatchChatRoom.h"

#include "MMatchObject.h"
#include "MMessageID.h"

void OnChatRoomCreate(const MUID &uidPlayer, const char *pszChatRoomName)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	MCmdWriter Cmd;
	
	if(pObj->m_vtChatRoomID.size() >= MAX_CHATROOM_JOINABLE_COUNT)
	{
		Cmd.WriteInt(MATCHNOTIFY_CHATROOM_CREATE_FAILED);
		Cmd.Finalize(MC_MATCH_NOTIFY, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	MMatchChatRoom *pChatRoom = g_ChatRoomMgr.Get(pszChatRoomName);
	if(pChatRoom != NULL)
	{
		Cmd.WriteInt(MATCHNOTIFY_CHATROOM_ALREADY_EXIST);
		Cmd.Finalize(MC_MATCH_NOTIFY, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	MMatchChatRoom *pNewChatRoom = g_ChatRoomMgr.Create(pszChatRoomName);
	pNewChatRoom->Join(pObj);
	
	pObj->AttachChatRoom(pNewChatRoom->GetID());
	pObj->m_nChatRoomID = pNewChatRoom->GetID();
	
	Cmd.WriteInt(MATCHNOTIFY_CHATROOM_CREATE_SUCCEED);
	Cmd.Finalize(MC_MATCH_NOTIFY, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
}

void OnChatRoomJoin(const MUID &uidPlayer, const char *pszChatRoomName)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	MCmdWriter Cmd;
	
	if(pObj->m_vtChatRoomID.size() >= MAX_CHATROOM_JOINABLE_COUNT)
	{
		Cmd.WriteInt(MATCHNOTIFY_CHATROOM_JOIN_FAILED);
		Cmd.Finalize(MC_MATCH_NOTIFY, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	MMatchChatRoom *pChatRoom = g_ChatRoomMgr.Get(pszChatRoomName);
	if(pChatRoom == NULL)
	{
		Cmd.WriteInt(MATCHNOTIFY_CHATROOM_NOT_EXIST);
		Cmd.Finalize(MC_MATCH_NOTIFY, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	if(pObj->CheckChatRoomAttached(pChatRoom->GetID()) == true)
	{
		Cmd.WriteInt(MATCHNOTIFY_CHATROOM_JOIN_FAILED);
		Cmd.Finalize(MC_MATCH_NOTIFY, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	if(pChatRoom->IsMax() == true)
	{
		Cmd.WriteInt(MATCHNOTIFY_CHATROOM_USER_FULL);
		Cmd.Finalize(MC_MATCH_NOTIFY, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	pChatRoom->Join(pObj);
	
	pObj->AttachChatRoom(pChatRoom->GetID());
	pObj->m_nChatRoomID = pChatRoom->GetID();
	
	Cmd.WriteString(pObj->m_Char.szName);
	Cmd.WriteString(pChatRoom->GetName());
	Cmd.Finalize(MC_MATCH_CHATROOM_JOIN, MCFT_END);
	SendToChatRoom(&Cmd, pChatRoom->GetID());
}

void OnChatRoomLeave(const MUID &uidPlayer, const char *pszChatRoomName)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	MCmdWriter Cmd;
	
	MMatchChatRoom *pChatRoom = g_ChatRoomMgr.Get(pszChatRoomName);
	if(pChatRoom == NULL)
	{
		Cmd.WriteInt(MATCHNOTIFY_CHATROOM_NOT_EXIST);
		Cmd.Finalize(MC_MATCH_NOTIFY, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	if(pObj->CheckChatRoomAttached(pChatRoom->GetID()) == false)
	{
		// not joined to chat room.
		return;
	}
	
	pChatRoom->Leave(pObj);
	
	pObj->DetachChatRoom(pChatRoom->GetID());
	pObj->m_nChatRoomID = 0;

	Cmd.WriteString(pObj->m_Char.szName);
	Cmd.WriteString(pChatRoom->GetName());
	Cmd.Finalize(MC_MATCH_CHATROOM_LEAVE, MCFT_END);
	SendToChatRoom(&Cmd, pChatRoom->GetID());	// to chatroom remained players.
		
	SendToClient(&Cmd, uidPlayer);	// to leaver.
	
	if(pChatRoom->IsEmpty() == true)
	{
		g_ChatRoomMgr.Remove(pChatRoom->GetID());
	}
}

void OnChatRoomSelect(const MUID &uidPlayer, const char *pszChatRoomName)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	MCmdWriter Cmd;
	
	MMatchChatRoom *pChatRoom = g_ChatRoomMgr.Get(pszChatRoomName);
	if(pChatRoom == NULL)
	{
		Cmd.WriteInt(MATCHNOTIFY_CHATROOM_NOT_EXIST);
		Cmd.Finalize(MC_MATCH_NOTIFY, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	if(pObj->CheckChatRoomAttached(pChatRoom->GetID()) == false)
	{
		// not joined to chat room.
		return;
	}
	
	pObj->m_nChatRoomID = pChatRoom->GetID();
	
	Cmd.WriteString(pChatRoom->GetName());
	Cmd.Finalize(MC_MATCH_CHATROOM_SELECT_WRITE, MCFT_END);
	SendToClient(&Cmd, uidPlayer);
}

void OnChatRoomInvite(const MUID &uidPlayer, const char *pszTargetName)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	MCmdWriter Cmd;
	
	if(pObj->m_nChatRoomID == 0)
	{
		Cmd.WriteInt(MATCHNOTIFY_CHATROOM_NOT_USING);
		Cmd.Finalize(MC_MATCH_NOTIFY, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	MMatchChatRoom *pChatRoom = g_ChatRoomMgr.Get(pObj->m_nChatRoomID);
	if(pChatRoom == NULL)
	{
		Cmd.WriteInt(MATCHNOTIFY_CHATROOM_NOT_EXIST);
		Cmd.Finalize(MC_MATCH_NOTIFY, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	MMatchObject *pTargetObj = g_ObjectMgr.Get(pszTargetName);
	if(pTargetObj == NULL)
	{
		Cmd.WriteInt(MATCHNOTIFY_GENERAL_USER_NOTFOUND);
		Cmd.Finalize(MC_MATCH_NOTIFY, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	if(pTargetObj->m_bCharInfoExist == false)
	{
		Cmd.WriteInt(MATCHNOTIFY_GENERAL_USER_NOTFOUND);
		Cmd.Finalize(MC_MATCH_NOTIFY, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	// TIP : if you need invite block check, then add here.
	
	Cmd.WriteString(pObj->m_Char.szName);
	Cmd.WriteString(pTargetObj->m_Char.szName);
	Cmd.WriteString(pChatRoom->GetName());
	Cmd.Finalize(MC_MATCH_CHATROOM_INVITE, MCFT_END);
	SendToClient(&Cmd, pTargetObj->GetUID());
}

void OnChatRoomChat(const MUID &uidPlayer, const char *pszMsg)
{
	MMatchObject *pObj = g_ObjectMgr.Get(uidPlayer);
	if(pObj == NULL) return;
	
	if(pObj->m_bCharInfoExist == false) return;
	
	MCmdWriter Cmd;
	
	if(pObj->m_nChatRoomID == 0)
	{
		Cmd.WriteInt(MATCHNOTIFY_CHATROOM_NOT_USING);
		Cmd.Finalize(MC_MATCH_NOTIFY, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	MMatchChatRoom *pChatRoom = g_ChatRoomMgr.Get(pObj->m_nChatRoomID);
	if(pChatRoom == NULL)
	{
		Cmd.WriteInt(MATCHNOTIFY_CHATROOM_NOT_EXIST);
		Cmd.Finalize(MC_MATCH_NOTIFY, MCFT_END);
		SendToClient(&Cmd, uidPlayer);
		return;
	}
	
	Cmd.WriteString(pChatRoom->GetName());
	Cmd.WriteString(pObj->m_Char.szName);
	Cmd.WriteString(pszMsg);
	Cmd.Finalize(MC_MATCH_CHATROOM_CHAT, MCFT_END);
	SendToChatRoom(&Cmd, pChatRoom->GetID());
}