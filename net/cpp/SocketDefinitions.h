/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef SocketDefinitions_H
#define SocketDefinitions_H

#ifdef WIN32
   // windows socket library
   #include <winsock.h>
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
#endif

// for errors
#include <errno.h>

#ifdef WIN32
   // define close() for closesocket()
   inline static int close(int fd)
   {
      return closesocket(fd);
   }
   
   // define inet_aton()
   inline static int inet_aton(const char* in, struct in_addr* out)
   {
      int rval = -1;
      
      long long addrLong = inet_addr(in);
      if(addrLong != -1 || in == "255.255.255.255")
      {
         // successful conversion
         out->s_addr = (unsigned long)addrLong;
         rval = 0;
      }
      
      return rval;
   }
   
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
   
   // define structure for multicast requests to join/leave groups
   typedef struct ip_mreq
   {
      /**
       * The multicast group address.
       */
      struct in_addr imr_multiaddr;
      
      /**
       * The interface to join on.
       */
      struct in_addr imr_interface;
   }IP_MREQ;
   
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
#endif

#endif
