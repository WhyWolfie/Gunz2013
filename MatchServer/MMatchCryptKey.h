#ifndef __MMATCHCRYPTKEY_H__
#define __MMATCHCRYPTKEY_H__

void MakeCryptKey(const MUID &uidClient, const MUID &uidServer, unsigned int nTimeStamp, unsigned char *pOutCryptKey);

#endif