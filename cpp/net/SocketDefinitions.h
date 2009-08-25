/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_net_SocketDefinitions_H
#define db_net_SocketDefinitions_H

#define SOCKET_EXCEPTION_TYPE         "db.net.Socket"
#define SOCKET_TIMEOUT_EXCEPTION_TYPE "db.net.SocketTimeout"
#define SSL_EXCEPTION_TYPE            "db.net.SSL"

#ifdef WIN32
   // windows socket library
   #include <winsock2.h>
   #include <ws2tcpip.h>
#else
   // data types
   #include <sys/types.h>
   // linux socket library
   #include <sys/socket.h>
   // closes file descriptors with close()
   #include <unistd.h>
   // includes gethostbyname() method
   #include <netdb.h>
   // includes sockaddr_in structure for internet addresses and
   // methods for converting numbers to network byte order (big-endian)
   #include <netinet/in.h>
   // include inet_pton() and inet_ntop()
   #include <arpa/inet.h>
   // include fcntl
   #include <sys/fcntl.h>
#endif

// for errors
#include <errno.h>

//// define IP protocols as necessary
//#ifndef IPPROTO_IP
//   #define IPPROTO_IP     0
//#endif
//#ifndef IPPROTO_ICMP
//   #define IPPROTO_ICMP   1
//#endif
//#ifndef IPPROTO_GGP
//   #define IPPROTO_GGP    2
//#endif
//#ifndef IPPROTO_TCP
//   #define IPPROTO_TCP    6
//#endif
//#ifndef IPPROTO_PUP
//   #define IPPROTO_PUP    12
//#endif
//#ifndef IPPROTO_UDP
//   #define IPPROTO_UDP    17
//#endif
//#ifndef IPPROTO_IDP
//   #define IPPROTO_IDP    22
//#endif
//#ifndef IPPROTO_ND
//   #define IPPROTO_ND     77
//#endif
//
//#ifndef IPPROTO_RAW
//   #define IPPROTO_RAW    255
//#endif
//#ifndef IPPROTO_MAX
//   #define IPPROTO_MAX    256
//#endif

#ifdef WIN32

// define socklen_t
typedef int socklen_t;

// define multicast values
#ifndef IP_MULTICAST_IF
   #define IP_MULTICAST_IF           9
#endif
#ifndef IP_MULTICAST_TTL
   #define IP_MULTICAST_TTL          10
#endif
#ifndef IP_MULTICAST_LOOP
   #define IP_MULTICAST_LOOP         11
#endif
#ifndef IP_ADD_MEMBERSHIP
   #define IP_ADD_MEMBERSHIP         12
#endif
#ifndef IP_DROP_MEMBERSHIP
   #define IP_DROP_MEMBERSHIP        13
#endif
#ifndef IP_DEFAULT_MULTICAST_TTL
   #define IP_DEFAULT_MULTICAST_TTL  1
#endif
#ifndef IP_DEFAULT_MULTICAST_LOOP
   #define IP_DEFAULT_MULTICAST_LOOP 1
#endif
#ifndef IP_MAX_MEMBERSHIPS
   #define IP_MAX_MEMBERSHIPS        20
#endif
#ifndef DESTINATION_MCAST
   #define DESTINATION_MCAST         "234.5.6.7"
#endif
#ifndef DESTINATION_PORT
   #define DESTINATION_PORT          4567
#endif

// define socket blocking options
#ifndef F_SETFL
   #define F_SETFL           FIONBIO
#endif
#ifndef O_NONBLOCK
   #define O_NONBLOCK        1
#endif

// define getnameinfo()/getaddrinfo() constants
/*
#ifndef NI_NUMERICHOST
   #define NI_NUMERICHOST    1
#endif
#ifndef NI_NUMERICSERV
   #define NI_NUMERICSERV    2
#endif
*/

/*
#define MSG_OOB         1
#define MSG_PEEK        2
#define MSG_DONTROUTE   4
#define MSG_TRYHARD     4       // Synonym for MSG_DONTROUTE for DECnet
#define MSG_CTRUNC      8
#define MSG_PROBE       0x10    // Do not send. Only probe path f.e. for MTU
#define MSG_TRUNC       0x20
#define MSG_DONTWAIT    0x40    // Nonblocking io
#define MSG_EOR         0x80    // End of record
#define MSG_WAITALL     0x100   // Wait for a full request
#define MSG_FIN         0x200
#define MSG_SYN         0x400
#define MSG_CONFIRM     0x800   // Confirm path validity
#define MSG_RST         0x1000
#define MSG_ERRQUEUE    0x2000  // Fetch message from error queue
#define MSG_NOSIGNAL    0x4000  // Do not generate SIGPIPE
#define MSG_MORE        0x8000  // Sender will send more
#define MSG_EOF         MSG_FIN
*/

#ifndef EAI_ADDRFAMILY
   #define EAI_ADDRFAMILY    -6
#endif
#ifndef EAI_SYSTEM
   #define EAI_SYSTEM        -11
#endif
#ifndef EAI_OVERFLOW
   #define EAI_OVERFLOW      -12
#endif

// define standard errors according to winsock errors
#ifndef EWOULDBLOCK
   #define EWOULDBLOCK       WSAEWOULDBLOCK
#endif
#ifndef EINPROGRESS
   #define EINPROGRESS       WSAEINPROGRESS
#endif
#ifndef EALREADY
   #define EALREADY          WSAEALREADY
#endif
#ifndef ENOTSOCK
   #define ENOTSOCK          WSAENOTSOCK
#endif
#ifndef EDESTADDRREQ
   #define EDESTADDRREQ      WSAEDESTADDRREQ
#endif
#ifndef EMSGSIZE
   #define EMSGSIZE          WSAEMSGSIZE
#endif
#ifndef EPROTOTYPE
   #define EPROTOTYPE        WSAEPROTOTYPE
#endif
#ifndef ENOPROTOOPT
   #define ENOPROTOOPT       WSAENOPROTOOPT
#endif
#ifndef EPROTONOSUPPORT
   #define EPROTONOSUPPORT   WSAEPROTONOSUPPORT
#endif
#ifndef ESOCKTNOSUPPORT
   #define ESOCKTNOSUPPORT   WSAESOCKTNOSUPPORT
#endif
#ifndef EOPNOTSUPP
   #define EOPNOTSUPP        WSAEOPNOTSUPP
#endif
#ifndef EPFNOSUPPORT
   #define EPFNOSUPPORT      WSAEPFNOSUPPORT
#endif
#ifndef EAFNOSUPPORT
   #define EAFNOSUPPORT      WSAEAFNOSUPPORT
#endif
#ifndef EADDRINUSE
   #define EADDRINUSE        WSAEADDRINUSE
#endif
#ifndef EADDRNOTAVAIL
   #define EADDRNOTAVAIL     WSAEADDRNOTAVAIL
#endif
#ifndef ENETDOWN
   #define ENETDOWN          WSAENETDOWN
#endif
#ifndef ENETUNREACH
   #define ENETUNREACH       WSAENETUNREACH
#endif
#ifndef ENETRESET
   #define ENETRESET         WSAENETRESET
#endif
#ifndef ECONNABORTED
   #define ECONNABORTED      WSAECONNABORTED
#endif
#ifndef ECONNRESET
   #define ECONNRESET        WSAECONNRESET
#endif
#ifndef ENOBUFS
   #define ENOBUFS           WSAENOBUFS
#endif
#ifndef EISCONN
   #define EISCONN           WSAEISCONN
#endif
#ifndef ENOTCONN
   #define ENOTCONN          WSAENOTCONN
#endif
#ifndef ESHUTDOWN
   #define ESHUTDOWN         WSAESHUTDOWN
#endif
#ifndef ETOOMANYREFS
   #define ETOOMANYREFS      WSAETOOMANYREFS
#endif
#ifndef ETIMEDOUT
   #define ETIMEDOUT         WSAETIMEDOUT
#endif
#ifndef ECONNREFUSED
   #define ECONNREFUSED      WSAECONNREFUSED
#endif
#ifndef ELOOP
   #define ELOOP             WSAELOOP
#endif
#ifndef ENAMETOOLONG
   #define ENAMETOOLONG      WSAENAMETOOLONG
#endif
#ifndef EHOSTDOWN
   #define EHOSTDOWN         WSAEHOSTDOWN
#endif
#ifndef EHOSTUNREACH
   #define EHOSTUNREACH      WSAEHOSTUNREACH
#endif
#ifndef ENOTEMPTY
   #define ENOTEMPTY         WSAENOTEMPTY
#endif
#ifndef EPROCLIM
   #define EPROCLIM          WSAEUSERS
#endif
#ifndef EDQUOT
   #define EDQUOT            WSAEDQUOT
#endif
#ifndef ESTALE
   #define ESTALE            WSAESTALE
#endif
#ifndef EREMOTE
   #define EREMOTE           WSAEREMOTE
#endif
#ifndef EAI_AGAIN
   #define EAI_AGAIN         WSATRY_AGAIN
#endif
#ifndef EAI_BADFLAGS
   #define EAI_BADFLAGS      WSAEINVAL
#endif
#ifndef EAI_FAIL
   #define EAI_FAIL          WSANO_RECOVERY
#endif
#ifndef EAI_FAMILY
   #define EAI_FAMILY        WSAEAFNOSUPPORT
#endif
#ifndef EAI_MEMORY
   #define EAI_MEMORY        WSA_NOT_ENOUGH_MEMORY
#endif
#ifndef EAI_NODATA
   #define EAI_NODATA        WSANO_DATA
#endif
#ifndef EAI_NONAME
   #define EAI_NONAME        WSAHOST_NOT_FOUND
#endif
#ifndef EAI_SERVICE
   #define EAI_SERVICE       WSATYPE_NOT_FOUND
#endif
#ifndef EAI_SOCKTYPE
   #define EAI_SOCKTYPE      WSAESOCKTNOSUPPORT
#endif

#endif // WIN32

#ifdef MACOS

// define multicast values
#ifndef IPV6_ADD_MEMBERSHIP
   #define IPV6_ADD_MEMBERSHIP IPV6_JOIN_GROUP
#endif
#ifndef IPV6_DROP_MEMBERSHIP
   #define IPV6_DROP_MEMBERSHIP IPV6_LEAVE_GROUP
#endif

#endif // MACOS

#endif
