#ifndef __MPACKETSENDER_H__
#define __MPACKETSENDER_H__

void SendQueuedData();

void SendToClient(MCommandWriter *pCmd, Socket::socket_type s, const unsigned char *pCryptKey);
void SendToClient(MCommandWriter *pCmd, const MUID &uidClient);
void SendToAll(MCommandWriter *pCmd);
void SendToChannel(MCommandWriter *pCmd, const MUID &uidChannel);
void SendToStage(MCommandWriter *pCmd, const MUID &uidStage, bool bSkipInGame = false);
void SendToBattle(MCommandWriter *pCmd, const MUID &uidStage);
void SendToClan(MCommandWriter *pCmd, int nCLID);
void SendToChatRoom(MCommandWriter *pCmd, unsigned long nRoomID);
void SendToChannelListRequester(MCommandWriter *pCmd, int nChannelType);

void SendToAgent(MCommandWriter *pCmd);

#endif