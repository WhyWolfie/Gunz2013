#ifndef __MMATCHSERVER_ETC_H__
#define __MMATCHSERVER_ETC_H__

void AnnounceToClient(const char *pszMsg, const MUID &uidPlayer);
void AnnounceToStage(const char *pszMsg, const MUID &uidStage);
void AnnounceToAll(const char *pszMsg);

#endif