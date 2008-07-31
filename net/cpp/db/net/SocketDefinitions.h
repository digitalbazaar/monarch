/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
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
   
   // define fcntl() for ioctlsocket():
   
   /**
    * Changes some control aspect of a file descriptor.
    * 
    * @param fd the file descriptor to change.
    * @param cmd the command identifying the kind of change to be made.
    * @param arg the argument for the command.
    * 
    * @return -1 for an error and errno will be set, otherwise something else
    *         that is dependent on the specific command.
    */
   inline static int fcntl(int fd, long cmd, unsigned long arg)
   {
      return ioctlsocket(fd, cmd, &arg);
   }
   
   // define close() for closesocket():
   
   /**
    * Closes the passed file descriptor.
    * 
    * @param fd the file descriptor to close.
    */
   inline static int close(int fd)
   {
      return closesocket(fd);
   }
   
   // define inet_ntop():
   
   /**
    * Converts an address from network byte order format to a presentation
    * numerical formatted address.
    * 
    * @param af the address family (AF_INET for IPv4, AF_INET6 for IPv6).
    * @param src the source address to convert (an in_addr or in6_addr).
    * @param dst the buffer to store the converted numerical address in.
    * @param size the size of dst in bytes.
    * 
    * @return a pointer to dst on success, NULL on failure (errno is set).
    */
   inline static const char *inet_ntop(
      int af, const void* src, char* dst, unsigned int size)
   {
      const char* rval = NULL;
      
      // this method uses getnameinfo() which translates a socket address
      // to a node name and service location -- it has this prototype:
      //
      // getnameinfo(const struct sockaddr* sa, socklen_t salen,
      //    char* node, coklen_t nodelen, char* service, socklen_t servicelen,
      //    int flags);
      
      // check address family
      if(af == AF_INET)
      {
         // get a zero'd-out IPv4 address structure
         struct sockaddr_in sa;
         memset(&sa, '\0', sizeof(sa));
         
         // set the address family to IPv4
         sa.sin_family = AF_INET;
         
         // copy the source in_addr into the structure
         memcpy(&sa.sin_addr, src, sizeof(in_addr));
         
         // NULL specifies that we don't care about getting a "service" name
         // NI_NUMERICHOST ensures that the numeric form of the address
         // given in sockaddr_in will be returned
         int error = getnameinfo(
            (sockaddr*)&sa, sizeof(sa), dst, size, NULL, 0, NI_NUMERICHOST);
         if(error == 0)
         {
            // point at dst
            rval = dst;
         }
         else
         {
            // set error
            if(error == EAI_FAMILY)
            {
               errno = EAFNOSUPPORT;
            }
            else if(error == EAI_OVERFLOW)
            {
               errno = ENOSPC;
            }
            else if(error != EAI_SYSTEM)
            {
               errno = error;
            }
         }
      }
      else if(af == AF_INET6)
      {
         // get a zero'd-out IPv6 address structure
         struct sockaddr_in6 sa;
         memset(&sa, '\0', sizeof(sa));
         
         // set the address family to IPv6
         sa.sin6_family = AF_INET6;
         
         // copy the source in_addr into the structure
         memcpy(&sa.sin6_addr, src, sizeof(in6_addr));
         
         // NULL specifies that we don't care about getting a "service" name
         // NI_NUMERICHOST ensures that the numeric form of the address
         // given in sockaddr_in will be returned
         int error = getnameinfo(
            (sockaddr*)&sa, sizeof(sa), dst, size, NULL, 0, NI_NUMERICHOST);
         if(error == 0)
         {
            // point at dst
            rval = dst;
         }
         else
         {
            // set error
            if(error == EAI_FAMILY)
            {
               errno = EAFNOSUPPORT;
            }
            else if(error == EAI_OVERFLOW)
            {
               errno = ENOSPC;
            }
            else if(error != EAI_SYSTEM)
            {
               errno = error;
            }
         }
      }
      
      return rval;
   }
   
   /**
    * Converts a presentation (numerical or hostname) formatted address to a
    * network byte order format address.
    * 
    * @param af the address family (AF_INET for IPv4, AF_INET6 for IPv6).
    * @param src the buffer with the presentation address.
    * @param dst the buffer to store the converted address
    *            (an in_addr or in6_addr).
    * 
    * @return >= 1 on success, 0 for an unparseable address, and -1 for an
    *         error with errno set.
    */
   inline static int inet_pton(int af, const char* src, void* dst)
   {
      int rval = -1;
      
      // this method uses getaddrinfo() which obtains address information,
      // it has this prototype:
      //
      // getaddrinfo(const char* nodename, const char* servicename,
      //    const struct addrinfo* hints, struct addrinfo** res);
      
      // create hints address structure
      struct addrinfo hints;
      memset(&hints, '\0', sizeof(hints));
      hints.ai_family = af;      
      
      // create pointer for storing allocated resolved address
      struct addrinfo* res = NULL;
      
      // get address information
      int error = getaddrinfo(src, NULL, &hints, &res);
      if(error == 0)
      {
         rval = 1;
      }
      
      if(res != NULL)
      {
         if(af == AF_INET)
         {
            // copy the IPv4 result
            struct sockaddr_in* addr = (sockaddr_in*)res->ai_addr;
            memcpy(dst, &addr->sin_addr, sizeof(in_addr));
         }
         else if(af == AF_INET6)
         {
            // copy the IPv6 result
            struct sockaddr_in6* addr = (sockaddr_in6*)res->ai_addr;
            memcpy(dst, &addr->sin6_addr, sizeof(in6_addr));
         }
         
         // free the result
         freeaddrinfo(res);
      }
      
      return rval;
   }
// end #ifdef WIN32
#endif

#endif
