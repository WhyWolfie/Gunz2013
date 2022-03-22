#include "pch.h"
#include "MMatchConstant.h"
#include "MCommand.h"

unsigned short CreatePacketChecksum(const unsigned char *pPacket, int nLen)
{
	if(nLen > 0xFFFF)
		return 0;

	unsigned int nResult = 0, nTemp = 0;

	for(int i = 6; i < nLen; i++)
		nResult += (unsigned int)pPacket[i];
		
	for(int i = 0; i < 4; i++)
		nTemp += (unsigned int)pPacket[i];
		
	nResult -= nTemp;
	nTemp = nResult >> 0x10;

	return (unsigned short)(nResult + nTemp);
}

unsigned char _Enc(unsigned char n, unsigned char k)
{
	unsigned short a;
	unsigned char b;
	
	a = (unsigned short)(n ^ k);
	a <<= 5;
	
	b = (unsigned char)(a >> 8) | (unsigned char)(a & 0xFF);
	b ^= 0xF0;
	
	return b;
}

unsigned char _Dec(unsigned char n, unsigned char k)
{
	unsigned char a, b;
	
	a = n ^ 0xF0;
	b = a & 0x1F;
	
	a >>= 5;
	b <<= 3;
	b |= a;
	
	a = b ^ k;
	
	return a;
}

bool PacketEncrypter(const unsigned char *pSrcData, const int nSrcDataSize, unsigned char *pOutData, const unsigned char *pCryptKey)
{
	unsigned short a;

	int nReadIndex = 0;
	short nSize;

	const int nTotalIndex = nSrcDataSize;
	const unsigned char *pData = pSrcData;

	while(nReadIndex < nTotalIndex)
	{
		memcpy(&a, &pData[nReadIndex], 2);	// using a for packet header.

		if(a == 0x65)	 	// 0x65 (101) = encrypted.
		{
			if(pCryptKey == NULL) return false;
			
			memcpy(&nSize, &pData[nReadIndex + 2], 2);

			if(nSize < MPACKET_HEADER_LENGTH)
				return false;    // invalid size error.

			for(int i = 0; i < (int)nSize; i++)	 	// copy data.
				pOutData[nReadIndex + i] = pData[nReadIndex + i];
				
			// start encryption.
			for(int i = 0; i < 2; i++)
				pOutData[nReadIndex + i + 2] = _Enc(pOutData[nReadIndex + i + 2], pCryptKey[i % ENCRYPTIONKEY_LENGTH]);
				
			for(int i = 0; i < (int)(nSize - 6); i++)
				pOutData[nReadIndex + i + 6] = _Enc(pOutData[nReadIndex + i + 6], pCryptKey[i % ENCRYPTIONKEY_LENGTH]);
			// end encryption.

			a = CreatePacketChecksum(&pOutData[nReadIndex], (int)nSize);	// using a for checksum.
			memcpy(&pOutData[nReadIndex + 4], &a, 2);

			nReadIndex += (int)nSize;
		}
		else if(a == 0x64)	 	// 0x64 (100) = decrypted.
		{
			memcpy(&nSize, &pData[nReadIndex + 2], 2);

			if(nSize < MPACKET_HEADER_LENGTH)
				return false;    // invalid size error.

			for(int i = 0; i < (int)nSize; i++)	 	// copy data.
				pOutData[nReadIndex + i] = pData[nReadIndex + i];
				
			a = CreatePacketChecksum(&pOutData[nReadIndex], (int)nSize);	// using a for checksum.
			memcpy(&pOutData[nReadIndex + 4], &a, 2);

			nReadIndex += (int)nSize;
		}
		else return false;	// error.
	}
	
	return true;
}

bool PacketEncrypter(MCommandWriter *pSrcCmd, unsigned char *pOutData, const unsigned char *pCryptKey)
{
	return PacketEncrypter(pSrcCmd->GetData(), pSrcCmd->GetDataIndex(), pOutData, pCryptKey);
}

/*
void PacketEncrypter(MCommandWriter *pSrcCmd, unsigned char *pOutData, const unsigned char *pCryptKey)
{
	unsigned short a;

	int nReadIndex = 0;
	short nSize;

	const int nTotalIndex = pSrcCmd->GetDataIndex();
	const unsigned char *pData = pSrcCmd->GetData();

	while(nReadIndex < nTotalIndex)
	{
		memcpy(&a, &pData[nReadIndex], 2);	// using a for packet header.

		if(a == 0x65)	 	// 0x65 (101) = encrypted.
		{
			memcpy(&nSize, &pData[nReadIndex + 2], 2);

			if(nSize < MPACKET_HEADER_LENGTH)
				break;    // invalid size error.

			for(int i = 0; i < (int)nSize; i++)	 	// copy data.
				pOutData[nReadIndex + i] = pData[nReadIndex + i];
				
			// start encryption.
			for(int i = 0; i < 2; i++)
				pOutData[nReadIndex + i + 2] = _Enc(pOutData[nReadIndex + i + 2], pCryptKey[i % ENCRYPTIONKEY_LENGTH]);
				
			for(int i = 0; i < (int)(nSize - 6); i++)
				pOutData[nReadIndex + i + 6] = _Enc(pOutData[nReadIndex + i + 6], pCryptKey[i % ENCRYPTIONKEY_LENGTH]);
			// end encryption.

			a = CreatePacketChecksum(&pOutData[nReadIndex], (int)nSize);	// using a for checksum.
			memcpy(&pOutData[nReadIndex + 4], &a, 2);

			nReadIndex += (int)nSize;
		}
		else if(a == 0x64)	 	// 0x64 (100) = decrypted.
		{
			memcpy(&nSize, &pData[nReadIndex + 2], 2);

			if(nSize < MPACKET_HEADER_LENGTH)
				break;    // invalid size error.

			for(int i = 0; i < (int)nSize; i++)	 	// copy data.
				pOutData[nReadIndex + i] = pData[nReadIndex + i];
				
			a = CreatePacketChecksum(&pOutData[nReadIndex], (int)nSize);	// using a for checksum.
			memcpy(&pOutData[nReadIndex + 4], &a, 2);

			nReadIndex += (int)nSize;
		}
		else break;	// error.
	}
}
*/

int PacketDecrypter(unsigned char *pInOutData, int nLen, const unsigned char *pCryptKey)
{
	if(nLen == 0) return -1;	// no data. error.

	unsigned short a;
	unsigned char s[2];

	int nIndex = 0;

	short nSize;
	short nCmdSize;

	while(nIndex + MPACKET_HEADER_LENGTH <= nLen && nIndex != nLen)
	{
		memcpy(&a, &pInOutData[nIndex], 2);

		if(a == 0x65)	 	// 0x65 (101) = encrypted.
		{
			if(pCryptKey == NULL) return -1;
			
			for(int i = 0; i < 2; i++)	 	// decrypt packet size.
				s[i] = _Dec(pInOutData[nIndex + i + 2], pCryptKey[i % ENCRYPTIONKEY_LENGTH]);
			
			memcpy(&nSize, &s[0], 2);	// nSize = c (decrypted packet size).
			if(nSize < MPACKET_HEADER_LENGTH || (int)nSize > nLen - nIndex)
				// invalid size.
				return nIndex;

			memcpy(&a, &pInOutData[nIndex + 4], 2);	// a = checksum from data.
			if(a != CreatePacketChecksum(&pInOutData[nIndex], (int)nSize))
				// invalid checksum.
				return nIndex;
				
			// start decrypt and save to pInOutData.
			for(int i = 0; i < 2; i++)
				pInOutData[nIndex + i + 2] = _Dec(pInOutData[nIndex + i + 2], pCryptKey[i % ENCRYPTIONKEY_LENGTH]);
				
			for(int i = 0; i < (int)(nSize - 6); i++)
				pInOutData[nIndex + i + 6] = _Dec(pInOutData[nIndex + i + 6], pCryptKey[i % ENCRYPTIONKEY_LENGTH]);
			// end of decryption.
				
			memcpy(&nCmdSize, &pInOutData[nIndex + 6], 2);
			if(nSize != nCmdSize + 6)
				// should not be incorrect.
				return -1;

			nIndex += (int)nSize;
		}
		else if(a == 0x64)	 	// 0x64 (100) = decrypted.
		{
			memcpy(&nSize, &pInOutData[nIndex + 2], 2);	// nSize = (decrypted packet size).
			if(nSize < MPACKET_HEADER_LENGTH || (int)nSize > nLen - nIndex)
				// invalid size.
				return nIndex;

			memcpy(&a, &pInOutData[nIndex + 4], 2);	// a = checksum from data.
			if(a != CreatePacketChecksum(&pInOutData[nIndex], (int)nSize))
				// invalid checksum.
				return nIndex;
				
			memcpy(&nCmdSize, &pInOutData[nIndex + 6], 2);
			if(nSize != nCmdSize + 6)
				// should not be incorrect.
				return -1;

			nIndex += (int)nSize;
		}
		else break;	// error.
	}
	
	return nIndex;
}
