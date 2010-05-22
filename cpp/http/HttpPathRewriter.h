/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_http_HttpPathRewriter_H
#define monarch_http_HttpPathRewriter_H

#include "monarch/http/HttpRequestModifier.h"
#include "monarch/util/Pattern.h"

#include <vector>

namespace monarch
{
namespace http
{

/**
 * An HttpPathRewriter is used to rewrite paths in HttpRequest headers.
 *
 * @author Dave Longley
 */
class HttpPathRewriter : public HttpRequestModifier
{
protected:
   /**
    * A rewrite rule.
    */
   struct Rule
   {
      monarch::util::PatternRef regex;
      std::string format;
   };

   /**
    * The rewrite rules.
    */
   typedef std::vector<Rule> Rules;
   Rules mRules;

   /**
    * Set to true to apply all rules, false to apply only the first.
    */
   bool mApplyAllRules;

public:
   /**
    * Creates a new HttpPathRewriter.
    */
   HttpPathRewriter();

   /**
    * Destructs this HttpPathRewriter.
    */
   virtual ~HttpPathRewriter();

   /**
    * Sets whether or not all rules will be applied or just the first.
    *
    * @param on true to apply all rules, false to apply only the first.
    */
   virtual void setApplyAllRules(bool on);

   /**
    * Adds a rewrite rule. See StringTools::regexRewrite() for details on the
    * format string.
    *
    * @param regex the regex for matching a path (or components of it).
    * @param format the replacement format to use.
    *
    * @return true on success, false on failure with exception set.
    */
   virtual bool addRule(const char* regex, const char* format);

   /**
    * Modifies the passed HttpRequest. The header for the request has already
    * been received, but the body has not.
    *
    * @param request the HttpRequest to modify.
    */
   virtual void modifyRequest(HttpRequest* request);
};

} // end namespace http
} // end namespace monarch
#endif
