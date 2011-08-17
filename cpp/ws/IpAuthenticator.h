/*
 * Copyright (c) 2011 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_ws_IpAuthenticator_h
#define monarch_ws_IpAuthenticator_h

#include "monarch/util/Pattern.h"
#include "monarch/ws/RequestAuthenticator.h"

#include <vector>

namespace monarch
{
namespace ws
{

/**
 * An IpAuthenticator determines if the IP that makes a request is acceptable.
 * If patterns are set with addAllowRegex, then the IP must match one of the
 * patterns otherwise the request will be denied. If the IP matches any of the
 * patterns set with addDenyRegex the request will be denied.
 *
 * @author David I. Lehn
 */
class IpAuthenticator : public monarch::ws::RequestAuthenticator
{
protected:
   /**
    * Vector of PatternRefs.
    */
   typedef std::vector<monarch::util::PatternRef> PatternRefList;

   /**
    * List of IP patterns that are allowed.
    */
   PatternRefList mAllowPatterns;

   /**
    * List of IP patterns that are denied.
    */
   PatternRefList mDenyPatterns;

public:
   /**
    * Creates a new IpAuthenticator.
    */
   IpAuthenticator();

   /**
    * Destructs this IpAuthenticator.
    */
   virtual ~IpAuthenticator();

   /**
    * {@inheritDoc}
    */
   virtual monarch::ws::RequestAuthenticator::Result
      checkAuthentication(monarch::ws::ServiceChannel* ch);

   /**
    * Add a regex that request IPs must match.
    *
    * @param regex IP regex to allow.
    *
    * @return true if regex was added, false if an exception occurred.
    */
   virtual bool addAllowRegex(const char* regex);

   /**
    * Add a regex that request IPs are not allowed to match.
    *
    * @param regex IP regex to allow.
    *
    * @return true if regex was added, false if an exception occurred.
    */
   virtual bool addDenyRegex(const char* regex);
};

} // end namespace ws
} // end namespace monarch
#endif
