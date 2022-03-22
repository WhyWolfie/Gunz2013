#ifndef __MUTIL__
#define __MUTIL__

#include <stdio.h>
#include <limits.h>

#include <string.h>

#include <ctype.h>

// moved to namespace Socket::InetAddr().
/*
inline unsigned long MInetAddr(const char *pszIPAddr)
{
	unsigned long n[4];
	if(sscanf(pszIPAddr, "%lu.%lu.%lu.%lu", &n[0], &n[1], &n[2], &n[3]) != 4) return 0;
	
	unsigned long nRes = 0;
	
	for(int i = 0; i < 4; i++)
	{
		if(n[i] > 255) return 0;
		nRes |= n[i] << (i * 8);
	}
	
	return nRes;
}
*/

inline int MStrcmp(const char *s1, const char *s2)
{
	return strcmp(s1, s2);
}

inline int MStricmp(const char *s1, const char *s2)
{
	#ifdef _UNIX_BUILD
	return strcasecmp(s1, s2);
	#else
	return stricmp(s1, s2);
	#endif
}

inline void MStrlwr(char *pszInOut)
{
	for(int i = 0; pszInOut[i] != '\0'; i++)
	{
		pszInOut[i] = tolower(pszInOut[i]);
	}
}

#endif