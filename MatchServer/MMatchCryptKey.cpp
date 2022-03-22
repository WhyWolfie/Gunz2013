#include "pch.h"
#include "MMatchConstant.h"

void MakeCryptKey(const MUID &uidClient, const MUID &uidServer, unsigned int nTimeStamp, unsigned char *pOutCryptKey)
{
	// --------------- generate crypt key ---------------.
	static const unsigned char nKeyForInit1[] =
	{
		0x37,
		0x04,
		0x5D,
		0x2E,
		0x43,
		0x3A,	// MCommand version.
		0x49,
		0x53,
		0x50,
		0x05,
		0x13,
		0xC9,
		0x28,
		0xA4,
		0x4D,
		0x05
	};

	static const unsigned char nKeyForInit2[] =
	{
		0x57,
		0x02,
		0x5B,
		0x04,
		0x34,
		0x06,
		0x01,
		0x08,
		0x37,
		0x0A,
		0x12,
		0x69,
		0x41,
		0x38,
		0x0F,
		0x78
	};

	unsigned char nMadeKey[ENCRYPTIONKEY_LENGTH];
	memset(nMadeKey, 0, sizeof(nMadeKey));
	
	unsigned long nTime = (unsigned long)nTimeStamp;
	
	memcpy(&nMadeKey[0], &nTime, 4);
	memcpy(&nMadeKey[4], &uidServer.ulLowID, 4);
	memcpy(&nMadeKey[8], &uidClient.ulHighID, 4);
	memcpy(&nMadeKey[12], &uidClient.ulLowID, 4);
	memcpy(&nMadeKey[16], &nKeyForInit1[0], 16);

	unsigned int nA, nB, nRes;

	for(int i = 0; i < 4; i++)
	{
		memcpy(&nA, &nKeyForInit2[i * 4], 4);
		memcpy(&nB, &nMadeKey[i * 4], 4);
		nRes = nA ^ nB;
		memcpy(&nMadeKey[i * 4], &nRes, 4);
	}

	// copy from temp buffer.
	memcpy(&pOutCryptKey[0], &nMadeKey[0], sizeof(unsigned char) * ENCRYPTIONKEY_LENGTH);
}