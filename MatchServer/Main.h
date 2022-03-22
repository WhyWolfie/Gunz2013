#ifndef __MAIN_H__
#define __MAIN_H__

#include "MUID.h"

extern SFMT_T g_sfmt;
extern mutex g_Mutex;
extern wxFile g_File;

void MLog(const char *pszFormat, ...);
#define mlog MLog

#define SIZEOFA(v)	( sizeof(v) / sizeof(v[0]) )

#define ToInt(s)	(int)strtol(s, NULL, 10)

// zero memset() function, like ZeroMemory macro.
inline void *ZeroInit(void *p, size_t n)
{
	return memset(p, 0, n);
}

// for MUID zero set.
inline void ZeroUID(MUID *p, int n)
{
	for(int i = 0; i < n; i++)
	{
		*p = MUID(0, 0);
	}
}

// random number (int).
inline int RandNum(int nMin, int nMax)
{
	return (int)((uint32_t)nMin + (sfmt_genrand_uint32(&g_sfmt) % ((uint32_t)nMax - (uint32_t)nMin + 1)));
}

// random number (float). - http://stackoverflow.com/questions/686353/c-random-float
inline float RandNum(float fMin, float fMax)
{
	return (float)(fMin + ((float)sfmt_genrand_uint32(&g_sfmt) / ((float)UINT_MAX / (fMax - fMin))));
}

// random : 0 ~ UINT_MAX number (unsigned int).
inline unsigned int RandNum()
{
	return (unsigned int)sfmt_genrand_uint32(&g_sfmt);
}

// check added value is overflows. if normal, returns add result. if gone minus, returns value before add.
inline int CheckPlusOver(int v, int plus)
{
	int nRes = v + plus;
	return nRes >= 0 ? nRes : v;
}

inline int CheckMinusOver(int v, int minus)
{
	int nRes = v - minus;
	return nRes >= 0 ? nRes : v;
}

// elapsed time.
inline unsigned long GetTime()
{
	return (unsigned long)wxGetLocalTimeMillis().GetValue();
}

/*
// global mutex wrapper. use this if you want to access global variables or consoles.
inline void Lock()
{
	g_Mutex.lock();
}

inline void Unlock()
{
	g_Mutex.unlock();
}
*/

bool CheckGameVersion(int year);

#endif
