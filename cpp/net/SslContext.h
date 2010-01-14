/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_net_SslContext_H
#define monarch_net_SslContext_H

#include <openssl/ssl.h>

#include "monarch/crypto/PrivateKey.h"
#include "monarch/crypto/X509Certificate.h"
#include "monarch/io/File.h"
#include "monarch/net/TcpSocket.h"
#include "monarch/rt/ExclusiveLock.h"
#include "monarch/rt/SharedLock.h"
#include "monarch/util/StringTools.h"

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
    * A private key, if not provided via a file.
    */
   monarch::crypto::PrivateKeyRef mPrivateKey;

   /**
    * An X.509 Certificate, if not provided via a file.
    */
   monarch::crypto::X509CertificateRef mCertificate;

   /**
    * A lock for generating new SSLs.
    */
   monarch::rt::ExclusiveLock mContextLock;

   /**
    * A shared lock for reading/writing virtual hosts.
    */
   monarch::rt::SharedLock mVirtualHostLock;

   /**
    * A virtual host has a name and alternative SslContext.
    */
   struct VirtualHost
   {
      char* name;
      monarch::rt::Collectable<SslContext> ctx;
   };

   /**
    * Storage for virtual hosts.
    */
   typedef std::map<
      const char*, VirtualHost*, monarch::util::StringComparator>
      VirtualHostMap;
   VirtualHostMap mVirtualHosts;

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
    * Sets the default PEM-formatted certificate for this SSL context to use.
    *
    * If a server name is added to this context and it matches the server name
    * provided in a TLS SNI extension, it will be used instead. If there is
    * no match, this private key will be used.
    *
    * @param certFile the file with the PEM-formatted certificate to use.
    *
    * @return true if the certificate loaded, false if an Exception occurred.
    */
   virtual bool setCertificate(monarch::io::File& certFile);

   /**
    * Sets the default certificate for this SSL context to use.
    *
    * If a server name is added to this context and it matches the server name
    * provided in a TLS SNI extension, it will be used instead. If there is
    * no match, this private key will be used.
    *
    * @param cert the certificate to use.
    *
    * @return true if the certificate loaded, false if an Exception occurred.
    */
   virtual bool setCertificate(monarch::crypto::X509CertificateRef& cert);

   /**
    * Sets the default PEM-formatted private key for this SSL context to use.
    *
    * If a server name is added to this context and it matches the server name
    * provided in a TLS SNI extension, it will be used instead. If there is
    * no match, this certificate will be used.
    *
    * @param pkeyFile the file with the PEM-formatted private key to use.
    *
    * @return true if the private key loaded, false if an Exception occurred.
    */
   virtual bool setPrivateKey(monarch::io::File& pkeyFile);

   /**
    * Sets the default private key for this SSL context to use.
    *
    * If a server name is added to this context and it matches the server name
    * provided in a TLS SNI extension, it will be used instead. If there is
    * no match, this certificate will be used.
    *
    * @param pkeyFile the private key to use.
    *
    * @return true if the private key loaded, false if an Exception occurred.
    */
   virtual bool setPrivateKey(monarch::crypto::PrivateKeyRef& pkey);

   /**
    * Adds a virtual host to this context. If a client uses a TLS SNI
    * extension to specifically request a server with the given name, then
    * the given alternate SslContext will be used. Otherwise, the certificate
    * and private key from this SslContext will be used.
    *
    * @param name the name of the virtual host server.
    * @param ctx the SslContext to use.
    *
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool addVirtualHost(
      const char* name,
      monarch::rt::Collectable<SslContext>& ctx);

   /**
    * Removes a virtual host from this context. No special SslContext will be
    * used if a client requests the given server name via a TLS SNI extension.
    *
    * @param name the name of the virtual host to remove.
    * @param ctx set to the virtual host's SslContext, if NULL does nothing.
    *
    * @return true if removed, false if not.
    */
   virtual bool removeVirtualHost(
      const char* name, monarch::rt::Collectable<SslContext>* ctx = NULL);

   /**
    * Called internally when a Server Name Indication (SNI) TLS extension is
    * detected. This method will choose the correct SSL context to use. If a
    * matching virtual host cannot be found for the given server name, then
    * the default SSL context will be used.
    *
    * @param s the SSL for the connection.
    *
    * @return SSL_TLSEXT_ERR_OK if a matching virtual host was found,
    *         SSL_TLSEXT_ERR_ALERT_WARNING if the default host will be used,
    *         SSL_TLSEXT_ERR_NOACK if no server name could be read.
    */
   virtual int handleSni(SSL* s);

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
