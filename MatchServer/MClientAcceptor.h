#ifndef __MCLIENTACCEPTOR_H__
#define __MCLIENTACCEPTOR_H__

struct MClientAcceptInfo
{
	Socket::socket_type s;
	MUID uid;
	unsigned char *pCryptKey;
	bool bLoggedIn;
	unsigned long nIP;	// anti-connect flood : don't accept same ip at same time.
	bool bInvalid;	// anti-command flood : don't accept spam commands without logged-in (no object).
};

class MClientAcceptor
{
public:
	MClientAcceptor();
	~MClientAcceptor();
	
	bool AddClient(Socket::socket_type s);
	bool GetClientInfo(Socket::socket_type s, MUID *pOutUid, unsigned char *pOutKey);
	bool GetClientInfo(const MUID &uid, Socket::socket_type *pOutSocket, unsigned char *pOutKey);
	bool *GetClientLoginState(const MUID &uid);
	bool IsExistingClientIP(unsigned long nIP);
	void InvalidateClient(const MUID &uid);
	void RemoveClient(Socket::socket_type s);
	
	void KickInvalidClient();
	
private:
	vector<MClientAcceptInfo *> m_Clients;
};

extern MClientAcceptor g_ClientAcceptor;

#endif
