/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_node_WebServiceContainer_H
#define monarch_node_WebServiceContainer_H

#include "monarch/http/HttpConnectionServicer.h"
#include "monarch/util/StringTools.h"
#include "monarch/util/UniqueList.h"
#include "monarch/ws/WebService.h"

namespace monarch
{
namespace ws
{

/**
 * A WebServiceContainer is a container for WebServices.
 *
 * @author Dave Longley
 */
class WebServiceContainer
{
protected:
   /**
    * A map of path to WebService.
    */
   typedef std::map<
      const char*, WebServiceRef,
      monarch::util::StringComparator> WebServiceMap;

   /**
    * A set of secure and non-secure WebServiceMaps.
    */
   struct WebServiceMaps
   {
      WebServiceMap secure;
      WebServiceMap nonSecure;
   };

   /**
    * A map of domain to WebServiceMaps.
    */
   typedef std::map<
      const char*, WebServiceMaps*, monarch::util::StringComparator>
      DomainMap;
   DomainMap mServices;

   /**
    * The HttpConnectionServicer to register the services with.
    */
   monarch::http::HttpConnectionServicer mHttpConnectionServicer;

   /**
    * A list of default domains to add WebServices to.
    */
   monarch::rt::DynamicObject mDefaultDomains;

   /**
    * A lock for adding/removing WebServices.
    */
   monarch::rt::SharedLock mContainerLock;

public:
   /**
    * Creates a new WebServiceContainer.
    */
   WebServiceContainer();

   /**
    * Destructs this WebServiceContainer.
    */
   virtual ~WebServiceContainer();

   /**
    * Adds a WebService to this container. If initialize is true, then
    * initialize() will be called on the service, and the service will only
    * be added if initialize() returns true. If initialize is false, the
    * service will be added without any call to initalize().
    *
    * SecurityType option details:
    *
    * WebService::Secure: service accessible with secure connection.
    * WebService::NonSecure: service accessible with non-secure.
    * WebService::Both: service accessible with either.
    *
    * @param service the WebService to add.
    * @param st the security type (ie: SSL) to use with the service.
    * @param initialize true to initialize the service, false not to.
    * @param domain the domain to add the service to, NULL to use the default
    *           domains for this container.
    *
    * @return true if the service was added, false if not.
    */
   virtual bool addService(
      WebServiceRef& service,
      WebService::SecurityType st,
      bool initialize = true,
      const char* domain = NULL);

   /**
    * Removes a WebService by its path.
    *
    * SecurityType option details:
    *
    * WebService::Secure: remove secure access to the service.
    * WebService::NonSecure: remove non-secure access to the service.
    * WebService::Both: moreve secure and non-secure access.
    *
    * @param path the path for the WebService to remove.
    * @param st the security type to remove the service from.
    * @param cleanup true to call cleanup() on the service.
    * @param domain the domain to remove the service from, NULL to use the
    *           default domains for this container.
    */
   virtual void removeService(
      const char* path,
      WebService::SecurityType st,
      bool cleanup = true,
      const char* domain = NULL);

   /**
    * Gets a WebService by its path.
    *
    * SecurityType option details:
    *
    * WebService::Secure: get secure service.
    * WebService::NonSecure: get non-secure service.
    * WebService::Both: get either, non-secure first, then secure.
    *
    * @param path the path for the WebService to get.
    * @param st the security type to get the service from.
    * @param domain the domain to get the service from, NULL to use the default
    *           domains for this container.
    *
    * @return the WebService that was fetched or NULL if none found.
    */
   virtual WebServiceRef getService(
      const char* path,
      WebService::SecurityType st,
      const char* domain = NULL);

   /**
    * Clears all WebServices from this container.
    */
   virtual void clear();

   /**
    * Gets the HttpConnectionServicer for this container.
    *
    * @return the HttpConnectionServicer for this container.
    */
   virtual monarch::http::HttpConnectionServicer* getServicer();

   /**
    * Sets the default domains for this container.
    *
    * @param domains an array of domains to use with this container.
    */
   virtual void setDefaultDomains(monarch::rt::DynamicObject& domains);

   /**
    * Gets the default domains for this container.
    *
    * @return an array of domains to use with this container.
    */
   virtual monarch::rt::DynamicObject& getDefaultDomains();

protected:
   /**
    * Adds a WebService to the domain map of services. Called within the
    * container lock and before adding the service to the
    * HttpConnectionServicer.
    *
    * @param service the WebService to add.
    * @param st the security type.
    * @param domain the domain.
    *
    * @return true if successful, false if not.
    */
   virtual bool internalAddService(
      WebServiceRef& service,
      WebService::SecurityType st,
      const char* domain);

   /**
    * Removes a WebService from the domain map of services. Called within the
    * container lock.
    *
    * @param path the path to the WebService to remove.
    * @param st the security type.
    * @param domain the domain.
    * @param cleanupList an optional list to add the WebService to.
    */
   virtual void internalRemoveService(
      const char* path,
      WebService::SecurityType st,
      const char* domain,
      monarch::util::UniqueList<WebServiceRef>* cleanupList);
};

// type definition for a reference counted WebServiceContainer
typedef monarch::rt::Collectable<WebServiceContainer> WebServiceContainerRef;

} // end namespace ws
} // end namespace monarch
#endif
