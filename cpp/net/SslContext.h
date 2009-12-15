/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_net_SslContext_H
#define monarch_net_SslContext_H

#include <openssl/ssl.h>

#include "monarch/io/File.h"
#include "monarch/net/TcpSocket.h"
#include "monarch/rt/ExclusiveLock.h"

namespace monarch
{
namespace net
{

/**
 * An SslContext uses on SSL context to produce SslSockets.
 *
 * @author Dave Longley
 */
class SslContext
{
protected:
   /**
    * The SSL context object.
    */
   SSL_CTX* mContext;

   /**
    * A lock for generating new SSLs.
    */
   monarch::rt::ExclusiveLock mLock;

public:
   /**
    * Creates a new SslContext. Peer authentication will default to
    * setPeerAuthentication(client).
    *
    * @param protocol the protocol to be used for this context. ("SSLv2",
    *        "SSLv3", "SSLv23", "TLS", "ALL", or NULL for default of ALL).
    * @param client true for a client context, false for a server context.
    */
   SslContext(const char* protocol, bool client);

   /**
    * Destructs this SslContext.
    */
   virtual ~SslContext();

   /**
    * Creates a new openssl "SSL" object for a TcpSocket.
    *
    * @param socket the TcpSocket to create the SSL object for.
    * @param true if the socket is a client socket, false if it is a server
    *        socket.
    *
    * @return the created SSL object.
    */
   virtual SSL* createSSL(TcpSocket* socket, bool client);

   /**
    * Sets the PEM-formatted certificate for this SSL context to use.
    *
    * @param certFile the file with the PEM-formatted certificate to use.
    *
    * @return true if the certificate loaded, false if an Exception occurred.
    */
   virtual bool setCertificate(monarch::io::File& certFile);

   /**
    * Sets the PEM-formatted private key for this SSL context to use.
    *
    * @param pkeyFile the file with the PEM-formatted private key to use.
    *
    * @return true if the private key loaded, false if an Exception occurred.
    */
   virtual bool setPrivateKey(monarch::io::File& pkeyFile);

   /**
    * Sets the peer authentication mode for this SSL context. If peer
    * authentication is turned on, then any server connections created
    * by this context will request a client certificate and any client
    * connections created by this context will check a server certificate.
    *
    * The default is to turn this on for clients, but not for servers. This
    * means that servers will need to be authenticated by clients but clients
    * will not need to be authenticated by servers (this is the most common
    * usage of SSL).
    *
    * @param on true to require peer authentication, false not to.
    */
   virtual void setPeerAuthentication(bool on);

   /**
    * Sets the main verification CA (Vertificate Authority) file and backup
    * CA directory. The main CA file should contain a list of PEM-formatted
    * CA certificates that are trusted and used to verify certificates received
    * from peers. The backup CA directory contains N CA certificates (one
    * per file) that are also trusted.
    *
    * When verifying a peer's certificate, the main CA file will be checked
    * first, followed by the backup CA directory.
    *
    * @param caFile the main CA file (can be NULL if caDir is not).
    * @param caDir the backup CA directory (can be NULL is caFile is not).
    *
    * @return true if the CAs were set properly, false if an Exception occurred.
    */
   virtual bool setVerifyCAs(
      monarch::io::File* caFile, monarch::io::File* caDir);

   /**
    * Repeatedly call ERR_get_error and build an Array of error messages with
    * ERR_error_string.  May be an empty array if no errors are present.
    *
    * @return an Array of error strings.  May be empty.
    */
   static monarch::rt::DynamicObject getSslErrorStrings();
};

// type definition for reference counted SslContext
typedef monarch::rt::Collectable<SslContext> SslContextRef;

} // end namespace net
} // end namespace monarch
#endif
