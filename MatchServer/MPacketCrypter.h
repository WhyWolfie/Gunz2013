#ifndef __MPACKETCRYPTER_H__
#define __MPACKETCRYPTER_H__

unsigned short CreatePacketChecksum(const unsigned char *pPacket, int nLen);
unsigned char _Enc(unsigned char n, unsigned char k);
unsigned char _Dec(unsigned char n, unsigned char k);
bool PacketEncrypter(const unsigned char *pSrcData, const int nSrcDataSize, unsigned char *pOutData, const unsigned char *pCryptKey = NULL);
bool PacketEncrypter(MCommandWriter *pSrcCmd, unsigned char *pOutData, const unsigned char *pCryptKey = NULL);
int PacketDecrypter(unsigned char *pInOutData, int nLen, const unsigned char *pCryptKey = NULL);

#endif