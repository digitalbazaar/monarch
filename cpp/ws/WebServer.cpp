/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_FORMAT_MACROS
#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

#include "monarch/ws/WebServer.h"

#include "monarch/crypto/AsymmetricKeyFactory.h"
#include "monarch/data/json/JsonWriter.h"
#include "monarch/logging/Logging.h"
#include "monarch/net/NullSocketDataPresenter.h"
#include "monarch/net/SslSocketDataPresenter.h"

using namespace std;
using namespace monarch::config;
using namespace monarch::crypto;
using namespace monarch::data::json;
using namespace monarch::http;
using namespace monarch::io;
using namespace monarch::net;
using namespace monarch::rt;
using namespace monarch::util;
using namespace monarch::ws;

WebServer::WebServer() :
   mServer(NULL),
   mHostAddress(NULL),
   mSslContext(NULL),
   mSocketDataPresenterList(NULL),
   mServiceId(Server::sInvalidServiceId)
{
}

WebServer::~WebServer()
{
}

bool WebServer::initialize(Config& cfg)
{
   bool rval = true;

   MO_CAT_DEBUG(MO_WS_CAT, "WebServer initializing...");

   // create default container if one is not set
   if(mContainer.isNull())
   {
      mContainer = new WebServiceContainer();
   }

   // get basic config
   // security: on/off/both
   bool secure;
   bool nonSecure;
   const char* security = cfg["security"]->getString();
   if(strcasecmp(security, "on") == 0)
   {
      secure = true;
      nonSecure = false;
   }
   else if(strcasecmp(security, "off") == 0)
   {
      secure = false;
      nonSecure = true;
   }
   else
   {
      secure = nonSecure = true;
   }

   if(secure)
   {
      MO_CAT_DEBUG(MO_WS_CAT, "WebServer enabling SSL...");

      // FIXME: make configurable
      /* Create SSL server context. "TLS" is most secure and recent SSL but we
         must use "ALL" to handle browsers that use SSL 3.0. */
      mSslContext = new SslContext("ALL", false);

      // setup certificate file and private key
      File certFile(cfg["certificate"]->getString());
      File pkeyFile(cfg["privateKey"]->getString());

      MO_CAT_DEBUG(MO_WS_CAT,
         "WebServer setting SSL certificate and private key...");

      // set certificate and private key for SSL context
      rval =
         mSslContext->setCertificate(certFile) &&
         mSslContext->setPrivateKey(pkeyFile);
      if(rval)
      {
         MO_CAT_DEBUG(MO_WS_CAT, "WebServer reading SSL certificate...");

         // set default virtual host based on certificate common name
         ByteBuffer b(certFile->getLength());
         rval = certFile.readBytes(&b);
         if(rval)
         {
            MO_CAT_DEBUG(MO_WS_CAT,
               "WebServer loading SSL certificate from PEM...");

            AsymmetricKeyFactory afk;
            X509CertificateRef cert = afk.loadCertificateFromPem(
               b.data(), b.length());
            rval = !cert.isNull();
            if(rval)
            {
               DynamicObject subject = cert->getSubject();
               string commonName = cert->getField(subject, "CN");
               MO_CAT_DEBUG(MO_WS_CAT,
                  "Setting default virtual host to common name: %s",
                  commonName.c_str());
               mSslContext->setVirtualHost(commonName.c_str());
            }
         }
      }

      MO_CAT_DEBUG(MO_WS_CAT, "WebServer SSL setup complete.");
   }

   if(rval)
   {
      // setup host address
      const char* host = cfg["host"]->getString();
      uint32_t port = cfg["port"]->getUInt32();
      mHostAddress = new InternetAddress(host, port);

      // handle socket presentation layer
      mSocketDataPresenterList = new SocketDataPresenterList(true);
      if(secure)
      {
         SslSocketDataPresenter* ssdp =
            new SslSocketDataPresenter(&(*mSslContext));
         mSocketDataPresenterList->add(ssdp);
      }
      if(nonSecure)
      {
         NullSocketDataPresenter* nsdp = new NullSocketDataPresenter();
         mSocketDataPresenterList->add(nsdp);
      }

      // get the list of default domains
      DynamicObject domains(NULL);
      if(cfg->hasMember("domains"))
      {
         domains = cfg["domains"].clone();
         if(domains->length() == 0)
         {
            // add wildcard if no domains specified
            domains->append() = "*";
         }
      }
      else
      {
         // no specified default domains, so use "*"
         domains = DynamicObject();
         domains->append() = "*";
      }
      mContainer->setDefaultDomains(domains);

      MO_CAT_INFO(MO_WS_CAT, "WebServer running web services on domains: %s",
         JsonWriter::writeToString(domains, false, false).c_str());
   }

   if(rval)
   {
      MO_CAT_DEBUG(MO_WS_CAT, "WebServer initialized.");
   }

   return rval;
}

void WebServer::cleanup()
{
   // reset container
   mContainer->clear();
   DynamicObject domains;
   domains->append() = "*";
   mContainer->setDefaultDomains(domains);

   // clean up
   mHostAddress.setNull();
   mSslContext.setNull();
   mSocketDataPresenterList.setNull();
}

bool WebServer::enable(Server* server, const char* name)
{
   bool rval = true;

   // add http connection service
   mServiceId = server->addConnectionService(
      &(*mHostAddress), mContainer->getServicer(),
      &(*mSocketDataPresenterList), name);
   if(mServiceId == Server::sInvalidServiceId)
   {
      // could not add connection service
      rval = false;
   }
   else
   {
      mServer = server;
      MO_CAT_INFO(MO_WS_CAT, "WebServer %s serving on %s",
         name, mHostAddress->toString(false).c_str());
   }

   return rval;
}

void WebServer::disable()
{
   // remove http connection service
   if(mServiceId != Server::sInvalidServiceId)
   {
      mServer->removePortService(mServiceId);
      mServiceId = Server::sInvalidServiceId;
      mServer = NULL;
   }
}

void WebServer::setContainer(WebServiceContainerRef& c)
{
   mContainer = c;
}

WebServiceContainerRef& WebServer::getContainer()
{
   return mContainer;
}

InternetAddressRef WebServer::getHostAddress()
{
   return mHostAddress;
}

SslContextRef WebServer::getSslContext()
{
   return mSslContext;
}
