/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/net/WindowsSupport.h"

#ifdef WIN32

/**
 * A helper function to set errno properly after a non-connect() winsock
 * call, mapping WSAEWOULDBLOCK to EAGAIN.
 */
static void setErrno()
{
   int e = WSAGetLastError();
   switch(e)
   {
     case WSAEWOULDBLOCK:
        errno = EAGAIN;
        break;
     default:
        errno = e;
        break;
   }
}

/**
 * A helper function to set errno after a connect() winsock call, mapping
 * WSAEWOULDBLOCK to EAGAIN.
 */
static void setConnectErrno()
{
   int e = WSAGetLastError();
   switch(e)
   {
      case WSAEINVAL:
      case WSAEALREADY:
      case WSAEWOULDBLOCK:
         errno = EINPROGRESS;
         break;
      default:
         errno = e;
         break;
   }
}

int win_compat_socket(int domain, int type, int protocol)
{
   int rval;

   SOCKET s = socket(domain, type, protocol);
   if(s == INVALID_SOCKET)
   {
      setErrno();
      rval = -1;
   }
   else
   {
      rval = (int)s;
   }

   return rval;
}

int win_compat_connect(int fd, const struct sockaddr* addr, socklen_t addrlen)
{
   int rval = connect(fd, addr, addrlen);
   if(rval == SOCKET_ERROR)
   {
      setConnectErrno();
      rval = -1;
   }

   return rval;
}

int win_compat_bind(int fd, const struct sockaddr* addr, socklen_t addrlen)
{
   int rval = bind(fd, addr, addrlen);
   if(rval == SOCKET_ERROR)
   {
      setErrno();
      rval = -1;
   }

   return rval;
}

int win_compat_listen(int fd, int backlog)
{
   int rval = listen(fd, backlog);
   if(rval == SOCKET_ERROR)
   {
      setErrno();
      rval = -1;
   }

   return rval;
}

int win_compat_accept(int fd, struct sockaddr* addr, socklen_t* addrlen)
{
   int rval;

   SOCKET s = accept(fd, addr, addrlen);
   if(s == INVALID_SOCKET)
   {
      setErrno();
      rval = -1;
   }
   else
   {
      rval = (int)s;
   }

   return rval;
}

int win_compat_shutdown(int fd, int mode)
{
   int rval = shutdown(fd, mode);
   if(rval == SOCKET_ERROR)
   {
      setErrno();
      rval = -1;
   }

   return rval;
}

int win_compat_close(int fd)
{
   int rval = closesocket(fd);
   if(rval == SOCKET_ERROR)
   {
      setErrno();
      rval = -1;
   }

   return rval;
}

int win_compat_getsockname(int fd, struct sockaddr* name, socklen_t* namelen)
{
   int rval = getsockname(fd, name, namelen);
   if(rval == SOCKET_ERROR)
   {
      setErrno();
      rval = -1;
   }

   return rval;
}

int win_compat_getpeername(int fd, struct sockaddr* name, socklen_t* namelen)
{
   int rval = getpeername(fd, name, namelen);
   if(rval == SOCKET_ERROR)
   {
      setErrno();
      rval = -1;
   }

   return rval;
}

int win_compat_recv(int fd, void* buf, size_t len, int flags)
{
   int rval = recv(fd, (char*)buf, len, flags);
   if(rval == SOCKET_ERROR)
   {
      setErrno();
      rval = -1;
   }

   return rval;
}

int win_compat_send(int fd, const void* buf, size_t len, int flags)
{
   int rval = send(fd, (const char*)buf, len, flags);
   if(rval == SOCKET_ERROR)
   {
      setErrno();
      rval = -1;
   }

   return rval;
}

int win_compat_recvfrom(
   int fd, void* buf, size_t len, int flags,
   struct sockaddr* from, socklen_t* fromlen)
{
   int rval = recvfrom(fd, (char*)buf, len, flags, from, fromlen);
   if(rval == SOCKET_ERROR)
   {
      setErrno();
      rval = -1;
   }

   return rval;
}

int win_compat_sendto(
   int fd, const void* buf, size_t len, int flags,
   const struct sockaddr* to, socklen_t tolen)
{
   int rval = sendto(fd, (const char*)buf, len, flags, to, tolen);
   if(rval == SOCKET_ERROR)
   {
      setErrno();
      rval = -1;
   }

   return rval;
}

int win_compat_fcntl(int fd, long cmd, unsigned long arg)
{
   int rval = ioctlsocket(fd, cmd, &arg);
   if(rval == SOCKET_ERROR)
   {
      setErrno();
      rval = -1;
   }

   return rval;
}

const char *inet_ntop(
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

int inet_pton(int af, const char* src, void* dst)
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
#endif // WIN32
