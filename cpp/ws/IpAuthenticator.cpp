/*
 * Copyright (c) 2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/validation/Validation.h"
#include "monarch/ws/IpAuthenticator.h"

using namespace std;
using namespace monarch::config;
using namespace monarch::rt;
using namespace monarch::util;
using namespace monarch::ws;
namespace v = monarch::validation;

// FIXME: add CIDR support

IpAuthenticator::IpAuthenticator()
{
}

IpAuthenticator::~IpAuthenticator()
{
}

bool IpAuthenticator::initializeFromConfig(
   Config& config, Config* privateConfig)
{
   bool rval;

   v::ValidatorRef v = new v::Map(
      "public", new v::Optional(new v::Type(Boolean)),
      "allow", new v::Optional(new v::Each(new v::Type(String))),
      "deny", new v::Optional(new v::Each(new v::Type(String))),
      NULL);

   rval = v->isValid(config);

   if(rval)
   {
      if(config->hasMember("public") && !config["public"]->getBoolean())
      {
         // fail if config is non-public but missing privateConfig
         if(privateConfig == NULL)
         {
            ExceptionRef e = new Exception(
               "Missing private IP config.",
               "monarch.ws.InvalidConfig");
            Exception::set(e);
            rval = false;
         }
         else
         {
            rval = initializeFromConfig(*privateConfig);
         }
      }
      else
      {
         if(config->hasMember("allow"))
         {
            DynamicObjectIterator i = config["allow"].getIterator();
            while(rval && i->hasNext())
            {
               rval = addAllowRegex(i->next()->getString());
            }
         }
         if(config->hasMember("deny"))
         {
            DynamicObjectIterator i = config["deny"].getIterator();
            while(rval && i->hasNext())
            {
               rval = addDenyRegex(i->next()->getString());
            }
         }
      }
   }

   return rval;
}

RequestAuthenticator::Result
   IpAuthenticator::checkAuthentication(ServiceChannel* ch)
{
   // default to client did not attempt to use an authentication mechanism
   RequestAuthenticator::Result rval = Success;

   const char* remoteAddress =
      ch->getConnection()->getRemoteAddress()->getAddress();

   // FIXME: issues with IPv4 vs IPv6 patterns?

   // check deny patterns
   if(mDenyPatterns.size() != 0)
   {
      for(
         PatternRefList::iterator i = mDenyPatterns.begin();
         rval == Success && i != mDenyPatterns.end();
         i++)
      {
         if((*i)->match(remoteAddress))
         {
            rval = Deny;
         }
      }
   }

   // check allow patterns if not yet denied and allow patterns are set
   if(rval != Deny && mAllowPatterns.size() != 0)
   {
      bool allow = false;

      for(
         PatternRefList::iterator i = mAllowPatterns.begin();
         !allow && i != mAllowPatterns.end();
         i++)
      {
         allow = (*i)->match(remoteAddress);
      }

      // deny if not found
      rval = allow ? Success : Deny;
   }

   if(rval == Deny)
   {
      ExceptionRef e = new Exception(
         "This IP is not authorized to access this resource.",
         "monarch.ws.InvalidIpAddress");
      e->getDetails()["httpStatusCode"] = 403;
      e->getDetails()["public"] = true;
      Exception::push(e);

      // client failed to pass authenticator
      ch->setAuthenticationException("ip", e);
   }

   return rval;
}

static bool _addRegex(
   std::vector<monarch::util::PatternRef>& patterns, const char* regex)
{
   bool rval;

   PatternRef pattern = Pattern::compile(regex);
   rval = !pattern.isNull();

   if(rval)
   {
      patterns.push_back(pattern);
   }

   return rval;
}

bool IpAuthenticator::addAllowRegex(const char* regex)
{
   return _addRegex(mAllowPatterns, regex);
}

bool IpAuthenticator::addDenyRegex(const char* regex)
{
   return _addRegex(mDenyPatterns, regex);
}
