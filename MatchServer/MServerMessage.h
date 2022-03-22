#ifndef __MSERVERMESSAGE_H__
#define __MSERVERMESSAGE_H__

// MServerMessage : 
//   Usually used to translate server-sided messages.

#include <string>
using namespace std;

// Message IDs. : 
#define MSVRSTR_REMAINED_TIME_SEC_STRING_FORMAT			1 // "%lu seconds remaining."
#define MSVRSTR_REMAINED_TIME_MIN_SEC_STRING_FORMAT		2 // "%lu min %lu sec remaining."

class MServerMessage
{
public:
	MServerMessage();
	~MServerMessage();
	
	bool Load();
	const char *Get(int nMsgID);
	const char *GetDefaultString(int nMsgID);
	
private:
	map<int, string> m_Messages;
};

extern MServerMessage g_ServerMessage;

// String replace macro.
#define GetServerMsg(id)	g_ServerMessage.Get(id)

#endif