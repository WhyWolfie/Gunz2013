#ifndef __MPEERSERVER_H__
#define __MPEERSERVER_H__

void OnServerPeerRecv(Socket::socket_type s, Socket::address_type *from, const char *data, const int size);

#endif