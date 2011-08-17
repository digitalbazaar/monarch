/*
 * Copyright (c) 2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/ws/IpAuthenticator.h"

using namespace std;
using namespace monarch::rt;
using namespace monarch::util;
using namespace monarch::ws;

IpAuthenticator::IpAuthenticator()
{
}

IpAuthenticator::~IpAuthenticator()
{
}

RequestAuthenticator::Result
   IpAuthenticator::checkAuthentication(ServiceChannel* ch)
{
   // default to client did not attempt to use oauth1
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

// FIXME: add CIDR support

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
