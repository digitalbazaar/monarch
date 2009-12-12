/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_net_WindowsSupport_H
#define db_net_WindowsSupport_H

#include "monarch/net/SocketDefinitions.h"

#ifndef WIN32

// use standard functions for non-windows
#define SOCKET_MACRO_socket(domain, type, protocol) \
   ::socket(domain, type, protocol)
#define SOCKET_MACRO_connect(fd, addr, addrlen) \
   ::connect(fd, addr, addrlen)
#define SOCKET_MACRO_accept(fd, addr, addrlen) \
   ::accept(fd, addr, addrlen)
#define SOCKET_MACRO_bind(fd, addr, addrlen) \
   ::bind(fd, addr, addrlen);
#define SOCKET_MACRO_listen(fd, backlog) \
   ::listen(fd, backlog)
#define SOCKET_MACRO_shutdown(fd, mode) \
   ::shutdown(fd, mode)
#define SOCKET_MACRO_close(fd) \
   ::close(fd)
#define SOCKET_MACRO_getsockname(fd, name, namelen) \
   ::getsockname(fd, name, namelen)
#define SOCKET_MACRO_getpeername(fd, name, namelen) \
   ::getpeername(fd, name, namelen)
#define SOCKET_MACRO_recv(fd, buf, len, flags) \
   ::recv(fd, buf, len, flags)
#define SOCKET_MACRO_send(fd, buf, len, flags) \
   ::send(fd, buf, len, flags)
#define SOCKET_MACRO_recvfrom(fd, buf, len, flags, from, fromlen) \
   ::recvfrom(fd, buf, len, flags, from, fromlen)
#define SOCKET_MACRO_sendto(fd, buf, len, flags, to, tolen) \
   ::sendto(fd, buf, len, flags, to, tolen)
#define SOCKET_MACRO_fcntl(fd, cmd, arg) \
   ::fcntl(fd, cmd, arg)

// WIN32
#else
/**
 * Create simple wrappers to set errno as windows does not do this.
 *
 * These wrappers will use WSAGetLastError() to retrieve the socket errors
 * and then set them to errno. Also, where windows sets strange error values,
 * errno will be mapped to the correct POSIX value. For example when windows
 * sets a socket error of WSAEWOULDBLOCK after a non-blocking connect(), it
 * will be mapped to EINPROGRESS. This allows socket code outside of this file
 * to be written consistently regardless of windows vs. unix-based platforms.
 */
#define SOCKET_MACRO_socket(domain, type, protocol) \
   win_compat_socket(domain, type, protocol)
#define SOCKET_MACRO_connect(fd, addr, addrlen) \
   win_compat_connect(fd, addr, addrlen)
#define SOCKET_MACRO_accept(fd, addr, addrlen) \
   win_compat_accept(fd, addr, addrlen)
#define SOCKET_MACRO_bind(fd, addr, addrlen) \
   win_compat_bind(fd, addr, addrlen);
#define SOCKET_MACRO_listen(fd, backlog) \
   win_compat_listen(fd, backlog)
#define SOCKET_MACRO_shutdown(fd, mode) \
   win_compat_shutdown(fd, mode)
#define SOCKET_MACRO_close(fd) \
   win_compat_close(fd)
#define SOCKET_MACRO_getsockname(fd, name, namelen) \
   win_compat_getsockname(fd, name, namelen)
#define SOCKET_MACRO_getpeername(fd, name, namelen) \
   win_compat_getpeername(fd, name, namelen)
#define SOCKET_MACRO_recv(fd, buf, len, flags) \
   win_compat_recv(fd, buf, len, flags)
#define SOCKET_MACRO_send(fd, buf, len, flags) \
   win_compat_send(fd, buf, len, flags)
#define SOCKET_MACRO_recvfrom(fd, buf, len, flags, from, fromlen) \
   win_compat_recvfrom(fd, buf, len, flags, from, fromlen)
#define SOCKET_MACRO_sendto(fd, buf, len, flags, to, tolen) \
   win_compat_sendto(fd, buf, len, flags, to, tolen)
#define SOCKET_MACRO_fcntl(fd, cmd, arg) \
   win_compat_fcntl(fd, cmd, arg)

/**
 * A wrapper that calls winsock's socket() and sets errno appropriately.
 *
 * @param domain the communication domain.
 * @param type the communication semantics.
 * @param protocol the protocol family.
 *
 * @return -1 on error with errno set, otherwise the new file descriptor.
 */
int win_compat_socket(int domain, int type, int protocol);

/**
 * A wrapper that calls winsock's connect() and sets errno appropriately.
 *
 * @param fd the socket's file descriptor.
 * @param addr the address to connect to.
 * @param addrlen the length of the address.
 *
 * @return -1 on error with errno set, 0 on success.
 */
int win_compat_connect(int fd, const struct sockaddr* addr, socklen_t addrlen);

/**
 * A wrapper that calls winsock's bind() and sets errno appropriately.
 *
 * @param fd the socket's file descriptor.
 * @param addr the address to bind to.
 * @param addrlen the length of the address.
 *
 * @return -1 on error with errno set, 0 on success.
 */
int win_compat_bind(int fd, const struct sockaddr* addr, socklen_t addrlen);

/**
 * A wrapper that calls winsock's listen() and sets errno appropriately.
 *
 * @param fd the socket's file descriptor.
 * @param backlog the number of connections to backlog before rejecting.
 *
 * @return -1 on error with errno set, 0 on success.
 */
int win_compat_listen(int fd, int backlog);

/**
 * A wrapper that calls winsock's accept() and sets errno appropriately.
 *
 * @param fd the socket's file descriptor.
 * @param addr the address to populate with the peer's address.
 * @param addrlen the length of the peer's address.
 *
 * @return -1 on error with errno set, 0 on success.
 */
int win_compat_accept(int fd, struct sockaddr* addr, socklen_t* addrlen);

/**
 * A wrapper that calls winsock's shutdown() and sets errno appropriately.
 *
 * @param fd the socket's file descriptor.
 * @param mode the shutdown mode.
 *
 * @return -1 on error with errno set, 0 on success.
 */
int win_compat_shutdown(int fd, int mode);

/**
 * A wrapper that calls winsock's closesocket() and sets errno appropriately.
 *
 * @param fd the file descriptor to close.
 *
 * @return -1 on error with errno set, 0 on success.
 */
int win_compat_close(int fd);

/**
 * A wrapper that calls winsock's getsockname() and sets errno appropriately.
 *
 * @param fd the socket's file descriptor.
 * @param name the address to populate with the local address.
 * @param namelen the length of the local address.
 *
 * @return -1 on error with errno set, 0 on success.
 */
int win_compat_getsockname(int fd, struct sockaddr* name, socklen_t* namelen);

/**
 * A wrapper that calls winsock's getpeername() and sets errno appropriately.
 *
 * @param fd the socket's file descriptor.
 * @param name the address to populate with the peer's address.
 * @param namelen the length of the peer's address.
 *
 * @return -1 on error with errno set, 0 on success.
 */
int win_compat_getpeername(int fd, struct sockaddr* name, socklen_t* namelen);

/**
 * A wrapper that calls winsock's recv() and sets errno appropriately.
 *
 * @param fd the socket's file descriptor.
 * @param buf the buffer to write data to.
 * @param len the length of the buffer.
 * @param flags any receive flags.
 *
 * @return -1 on error with errno set, 0 on an orderly shutdown by the peer,
 *         otherwise the number of bytes received.
 */
int win_compat_recv(int fd, void* buf, size_t len, int flags);

/**
 * A wrapper that calls winsock's send() and sets errno appropriately.
 *
 * @param fd the socket's file descriptor.
 * @param buf the buffer of data to send.
 * @param len the length of the buffer.
 * @param flags any send flags.
 *
 * @return -1 on error with errno set, otherwise the number of bytes sent.
 */
int win_compat_send(int fd, const void* buf, size_t len, int flags);

/**
 * A wrapper that calls winsock's recvfrom() and sets errno appropriately.
 *
 * @param fd the socket's file descriptor.
 * @param buf the buffer to write data to.
 * @param len the length of the buffer.
 * @param flags any receive flags.
 * @param from to store the address the message was sent from.
 * @param fromlen to store the length of the from address.
 *
 * @return -1 on error with errno set, 0 on an orderly shutdown by the peer,
 *         otherwise the number of bytes received.
 */
int win_compat_recvfrom(
   int fd, void* buf, size_t len, int flags,
   struct sockaddr* from, socklen_t* fromlen);

/**
 * A wrapper that calls winsock's sendto() and sets errno appropriately.
 *
 * @param fd the socket's file descriptor.
 * @param buf the buffer of data to send.
 * @param len the length of the buffer.
 * @param flags any send flags.
 * @param to the address to send the message to.
 * @param tolen the length of the send address.
 *
 * @return -1 on error with errno set, otherwise the number of bytes sent.
 */
int win_compat_sendto(
   int fd, const void* buf, size_t len, int flags,
   const struct sockaddr* to, socklen_t tolen);

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
int win_compat_fcntl(int fd, long cmd, unsigned long arg);

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
const char *inet_ntop(int af, const void* src, char* dst, unsigned int size);

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
int inet_pton(int af, const char* src, void* dst);

#endif // WIN32

#endif
