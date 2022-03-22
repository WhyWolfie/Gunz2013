#include "pch.h"

// thread-safe shutdown flag.
bool g_bShutdown = false;
mutex g_HaltMutex;

void ShutdownServer()
{
	g_HaltMutex.lock();
	g_bShutdown = true;
	g_HaltMutex.unlock();
}

bool CheckServerHalt()
{
	bool bRet = false;
	
	g_HaltMutex.lock();
	bRet = g_bShutdown;
	g_HaltMutex.unlock();
	
	return bRet;
}