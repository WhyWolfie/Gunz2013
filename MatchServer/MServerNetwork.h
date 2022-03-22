#ifndef __MSERVERNETWORK_H__
#define __MSERVERNETWORK_H__

void OnClientConnect(Socket::socket_type s);
bool OnClientRecv(Socket::socket_type s, const char *data, const int size);
void OnClientDisconnect(Socket::socket_type s);

// --------------------------------------------- .

// d/c reserved socket array.
void ReserveDisconnect(Socket::socket_type s);
bool CheckDisconnectReserved(Socket::socket_type s);
void ClearDisconnectReservedSocket();

#endif
