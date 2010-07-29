/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_FORMAT_MACROS
#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

#include "monarch/ws/WebServiceContainer.h"

#include "monarch/logging/Logging.h"
#include "monarch/rt/DynamicObjectIterator.h"

using namespace std;
using namespace monarch::http;
using namespace monarch::rt;
using namespace monarch::util;
using namespace monarch::ws;

WebServiceContainer::WebServiceContainer()
{
   // set default domains to "*"
   mDefaultDomains->append() = "*";
}

WebServiceContainer::~WebServiceContainer()
{
   WebServiceContainer::clear();
}

bool WebServiceContainer::addService(
   WebServiceRef& service,
   WebServiceContainer::SecurityType st,
   bool initialize,
   const char* domain)
{
   bool rval = true;

   if(initialize)
   {
      rval = service->initialize();
   }

   if(rval)
   {
      // build list of domains to add service to
      DynamicObject domains(NULL);
      DynamicObject added;
      added->setType(Array);
      if(domain != NULL)
      {
         domains = DynamicObject();
         domains->append() = domain;
      }
      else
      {
         domains = mDefaultDomains;
      }

      HttpRequestServicer* hrs = &(*service);
      const char* path = hrs->getPath();

      // prevent other container access
      mContainerLock.lockExclusive();

      DynamicObjectIterator di = domains.getIterator();
      while(rval && di->hasNext())
      {
         const char* dom = di->next()->getString();
         rval = internalAddService(service, st, domain);
         if(rval)
         {
            added->append() = dom;
         }
      }

      di = rval ? domains.getIterator() : added.getIterator();
      while(di->hasNext())
      {
         const char* dom = di->next()->getString();

         // success, add domain to http servicer
         if(rval)
         {
            if(st == WebServiceContainer::Secure ||
               st == WebServiceContainer::Both)
            {
               mHttpConnectionServicer.addRequestServicer(hrs, true, dom);
               MO_CAT_DEBUG(MO_WS_CAT,
                  "Added secure web service: %s%s", dom, path);
            }
            if(st != WebServiceContainer::Secure)
            {
               mHttpConnectionServicer.addRequestServicer(hrs, false, dom);
               MO_CAT_DEBUG(MO_WS_CAT,
                  "Added non-secure web service: %s%s", dom, path);
            }
         }
         // could not add service to all domains, so remove it
         else
         {
            internalRemoveService(path, st, dom, NULL);
         }
      }

      // permit access again
      mContainerLock.unlockExclusive();
   }

   // failed to add service
   if(!rval)
   {
      // service was initialized, so clean it up
      if(initialize)
      {
         service->cleanup();
      }

      // set exception
      ExceptionRef e = new Exception(
         "Could not add web service.",
         "monarch.ws.AddWebServiceFailure");
      Exception::push(e);
   }

   return rval;
}

void WebServiceContainer::removeService(
   const char* path,
   WebServiceContainer::SecurityType st,
   bool cleanup,
   const char* domain)
{
   // build list of domains to remove service from
   DynamicObject domains(NULL);
   if(domain != NULL)
   {
      domains = DynamicObject();
      domains->append() = domain;
   }
   else
   {
      domains = mDefaultDomains;
   }

   // build a unique list of services to cleanup
   UniqueList<WebServiceRef> cleanupList;

   // prevent other container access
   mContainerLock.lockExclusive();

   DynamicObjectIterator di = domains.getIterator();
   while(di->hasNext())
   {
      const char* dom = di->next()->getString();
      if(st == WebServiceContainer::Both ||
         st == WebServiceContainer::Secure)
      {
         mHttpConnectionServicer.removeRequestServicer(path, true, dom);
         MO_CAT_DEBUG(MO_WS_CAT,
            "Removed secure web service: %s%s", dom, path);
      }
      if(st != WebServiceContainer::Secure)
      {
         mHttpConnectionServicer.removeRequestServicer(path, false, dom);
         MO_CAT_DEBUG(MO_WS_CAT,
            "Removed non-secure web service: %s%s", dom, path);
      }
      internalRemoveService(path, st, dom, &cleanupList);
   }

   // permit access again
   mContainerLock.unlockExclusive();

   // clean up services
   if(cleanup)
   {
      IteratorRef<WebServiceRef> i = cleanupList.getIterator();
      while(i->hasNext())
      {
         WebServiceRef& ws = i->next();
         ws->cleanup();
      }
   }
}

WebServiceRef WebServiceContainer::getService(
   const char* path,
   WebServiceContainer::SecurityType st,
   const char* domain)
{
   WebServiceRef rval(NULL);

   mContainerLock.lockShared();
   {
      DomainMap::iterator di = mServices.find(domain);
      if(di != mServices.end())
      {
         WebServiceMaps* wsm = di->second;
         WebServiceMap::iterator i;
         if(st == WebServiceContainer::NonSecure ||
            st == WebServiceContainer::Both)
         {
            i = wsm->nonSecure.find(path);
            if(i != wsm->nonSecure.end())
            {
               rval = i->second;
            }
         }
         if(rval.isNull() && st != WebServiceContainer::NonSecure)
         {
            i = wsm->secure.find(path);
            if(i != wsm->secure.end())
            {
               rval = i->second;
            }
         }
      }
   }
   mContainerLock.unlockShared();

   return rval;
}

void WebServiceContainer::clear()
{
   // clean up domains and maps
   for(DomainMap::iterator i = mServices.begin(); i != mServices.end(); ++i)
   {
      free((char*)i->first);
      delete i->second;
   }
}

HttpConnectionServicer* WebServiceContainer::getServicer()
{
   return &mHttpConnectionServicer;
}

void WebServiceContainer::setDefaultDomains(DynamicObject& domains)
{
   mDefaultDomains = domains;
}

DynamicObject& WebServiceContainer::getDefaultDomains()
{
   return mDefaultDomains;
}

bool WebServiceContainer::internalAddService(
   WebServiceRef& service,
   WebServiceContainer::SecurityType st,
   const char* domain)
{
   bool rval = true;

   // get service path
   const char* path = service->getPath();

   // see if service can be added
   WebServiceMaps* wsm;
   DomainMap::iterator di = mServices.find(domain);
   if(di == mServices.end())
   {
      // new domain, add a new maps entry
      wsm = new WebServiceMaps;
      mServices[strdup(domain)] = wsm;

      // add service, no other checks necessary
      if(st == WebServiceContainer::Secure ||
         st == WebServiceContainer::Both)
      {
         wsm->secure[path] = service;
      }
      if(st != WebServiceContainer::Secure)
      {
         wsm->nonSecure[path] = service;
      }
   }
   else
   {
      // use existing maps entry
      wsm = di->second;

      // ensure the service can be added
      if(st == WebServiceContainer::Both &&
         wsm->secure.find(path) == wsm->secure.end() &&
         wsm->nonSecure.find(path) == wsm->nonSecure.end())
      {
         wsm->secure[path] = service;
         wsm->nonSecure[path] = service;
      }
      else if(
         st == WebServiceContainer::Secure &&
         wsm->secure.find(path) == wsm->secure.end())
      {
         wsm->secure[path] = service;
      }
      else if(
         st == WebServiceContainer::NonSecure &&
         wsm->nonSecure.find(path) == wsm->nonSecure.end())
      {
         wsm->nonSecure[path] = service;
      }
      else
      {
         ExceptionRef e = new Exception(
            "Could not add WebService to container. Duplicate service.",
            "monarch.ws.DuplicateService");
         e->getDetails()["path"] = path;
         e->getDetails()["domain"] = domain;
         Exception::set(e);
         rval = false;
      }
   }

   return rval;
}

void WebServiceContainer::internalRemoveService(
   const char* path,
   WebServiceContainer::SecurityType st,
   const char* domain,
   UniqueList<WebServiceRef>* cleanupList)
{
   // find the domain
   DomainMap::iterator di = mServices.find(domain);
   if(di != mServices.end())
   {
      // find the path
      WebServiceMaps* wsm = di->second;
      WebServiceMap::iterator i;
      if(st == WebServiceContainer::Both ||
         st == WebServiceContainer::Secure)
      {
         i = wsm->secure.find(path);
         if(i != wsm->secure.end())
         {
            cleanupList->add(i->second);
            wsm->secure.erase(i);
         }
      }
      if(st != WebServiceContainer::Secure)
      {
         i = wsm->nonSecure.find(path);
         if(i != wsm->nonSecure.end())
         {
            cleanupList->add(i->second);
            wsm->nonSecure.erase(i);
         }
      }

      // clean up domain if it has no more services
      if(wsm->secure.empty() && wsm->nonSecure.empty())
      {
         free((char*)di->first);
         delete di->second;
         mServices.erase(di);
      }
   }
}
