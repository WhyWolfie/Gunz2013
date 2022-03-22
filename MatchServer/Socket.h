#ifndef __SOCKET_H__
#define __SOCKET_H__

// Simple BSD-style socket wrapper.
// Tested only on Windows (WinSock) and Debian GNU/Linux.

#include <string.h>

#ifdef _WIN_BUILD
#include <WinSock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

namespace Socket
{
	#ifdef _WIN_BUILD
	// WinSock.
	typedef SOCKET socket_type;
	typedef sockaddr_in address_type;
	typedef int socklen_type;
	
	#define INVALID_SOCKET_DESC		INVALID_SOCKET
	
	#define SHUTDOWN_READ	SD_RECEIVE
	#define SHUTDOWN_WRITE	SD_SEND
	#define SHUTDOWN_BOTH	SD_BOTH
	#else
	// Linux.
	typedef int socket_type;
	typedef sockaddr_in address_type;
	typedef socklen_t socklen_type;
	
	#define INVALID_SOCKET_DESC		-1
	
	#define SHUTDOWN_READ	SHUT_RD
	#define SHUTDOWN_WRITE	SHUT_WR
	#define SHUTDOWN_BOTH	SHUT_RDWR
	#endif
	
	inline bool Initialize()
	{
		#ifdef _WIN_BUILD
		WSADATA wsaData;
		if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
			return false;
		#endif
			
		return true;
	}
	
	inline void UnInitialize()
	{
		#ifdef _WIN_BUILD
		WSACleanup();
		#endif
	}
	
	inline socket_type Accept(socket_type s, address_type *addr)
	{
		socklen_type addrlen = sizeof(address_type);
		return accept(s, (sockaddr *)addr, &addrlen);
	}
	
	inline bool Bind(socket_type s, const address_type *name)
	{
		socklen_type namelen = sizeof(address_type);
		return bind(s, (const sockaddr *)name, namelen) == 0 ? true : false ;
	}
	
	inline bool Close(socket_type s)
	{
		#ifdef _WIN_BUILD
		return closesocket(s) == 0 ? true : false ;
		#else
		return close(s) == 0 ? true : false ;
		#endif
	}
	
	inline bool Connect(socket_type s, const address_type *name)
	{
		socklen_type namelen = sizeof(address_type);
		return connect(s, (const sockaddr *)name, namelen) == 0 ? true : false ;
	}
	
	inline bool GetPeerName(socket_type s, address_type *name)
	{
		socklen_type namelen = sizeof(address_type);
		return getpeername(s, (sockaddr *)name, &namelen) == 0 ? true : false ;
	}
	
	inline unsigned long Htonl(unsigned long hostlong)
	{
		return htonl(hostlong);
	}
	
	inline unsigned short Htons(unsigned short hostshort)
	{
		return htons(hostshort);
	}
	
	inline unsigned long InetAddr(const char *cp)
	{
		unsigned long n[4];
		if(sscanf(cp, "%lu.%lu.%lu.%lu", &n[0], &n[1], &n[2], &n[3]) != 4) return INADDR_NONE;
	
		unsigned long nRes = 0;
	
		for(int i = 0; i < 4; i++)
		{
			if(n[i] > 255) return 0;
			nRes |= n[i] << (i * 8);
		}
	
		return nRes;
	}
	
	inline char *InetNtoa(address_type *addr)
	{
		return inet_ntoa(addr->sin_addr);
	}
	
	inline bool Listen(socket_type s, int backlog = 5)
	{
		return listen(s, backlog) == 0 ? true : false ;
	}
	
	inline unsigned long Htohl(unsigned short netlong)
	{
		return ntohl(netlong);
	}
	
	inline unsigned short Ntohs(unsigned short netshort)
	{
		return ntohs(netshort);
	}
	
	inline int Recv(socket_type s, char *buf, int len)
	{
		return recv(s, buf, len, 0);
	}
	
	inline int RecvFrom(socket_type s, char *buf, int len, address_type *from)
	{
		socklen_type fromlen = sizeof(address_type);
		return recvfrom(s, buf, len, 0, (sockaddr *)from, &fromlen);
	}
	
	// ::select(), just readable or not.
	inline bool IsReadable(socket_type s)
	{
		fd_set rfds;
		FD_ZERO(&rfds);
		
		FD_SET(s, &rfds);

		timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 0;
		
		if(select(1, &rfds, NULL, NULL, &tv) <= 0)
			return false;
		
		return FD_ISSET(s, &rfds) != 0 ? true : false ;
	}
	
	inline int Send(socket_type s, const char *buf, int len)
	{
		return send(s, buf, len, 0);
	}
	
	inline int SendTo(socket_type s, const char *buf, int len, const address_type *to)
	{
		socklen_type tolen = sizeof(address_type);
		return sendto(s, buf, len, 0, (const sockaddr *)to, tolen);
	}
	
	inline bool Shutdown(socket_type s, int how = SHUTDOWN_BOTH)
	{
		return shutdown(s, how) == 0 ? true : false ;
	}
	
	// ::socket(), named Create().
	inline socket_type Create(int af, int type, int protocol)
	{
		// INVALID_SOCKET on error.
		return socket(af, type, protocol);
	}
	
	// address utils.
	inline void MakeAddress(address_type *addr, const char *ip, unsigned short port)
	{
		memset(addr, 0, sizeof(address_type));
		
		addr->sin_family = AF_INET;
		#ifdef _WIN_BUILD
		addr->sin_addr.S_un.S_addr = InetAddr(ip);
		#else
		addr->sin_addr.s_addr = InetAddr(ip);
		#endif
		addr->sin_port = Htons(port);
	}
	
	inline void MakeServerAddress(address_type *addr, unsigned short port)
	{
		MakeAddress(addr, "0.0.0.0", port);
	}
	
	inline unsigned long ExtractInetAddr(const address_type &addr)
	{
		#ifdef _WIN_BUILD
		return addr.sin_addr.S_un.S_addr;
		#else
		return addr.sin_addr.s_addr;
		#endif
	}
	
	// socket utils.
	inline socket_type CreateStreamClientSocket(const char *ip, unsigned short port)
	{
		socket_type s = Create(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(s == INVALID_SOCKET_DESC) return INVALID_SOCKET_DESC;
		
		address_type addr;
		MakeAddress(&addr, ip, port);
		
		if(Connect(s, &addr) == false)
		{
			Close(s);
			return INVALID_SOCKET_DESC;
		}
		
		return s;
	}
	
	inline socket_type CreateStreamServerSocket(unsigned short port)
	{
		socket_type s = Create(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(s == INVALID_SOCKET_DESC) return INVALID_SOCKET_DESC;
		
		address_type addr;
		MakeServerAddress(&addr, port);
		
		if(Bind(s, &addr) == false)
		{
			Close(s);
			return INVALID_SOCKET_DESC;
		}
		
		if(Listen(s) == false)
		{
			Close(s);
			return INVALID_SOCKET_DESC;
		}
		
		return s;
	}
	
	inline socket_type CreateDatagramSocket(unsigned short port)
	{
		socket_type s = Create(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if(s == INVALID_SOCKET_DESC) return INVALID_SOCKET_DESC;
		
		address_type addr;
		MakeServerAddress(&addr, port);
		
		if(Bind(s, &addr) == false)
		{
			Close(s);
			return INVALID_SOCKET_DESC;
		}
		
		return s;
	}
};

#endif