/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/http/HttpPathRewriter.h"

#include "monarch/rt/Exception.h"

using namespace std;
using namespace monarch::http;
using namespace monarch::rt;
using namespace monarch::util;

HttpPathRewriter::HttpPathRewriter() :
   mApplyAllRules(false)
{
}

HttpPathRewriter::~HttpPathRewriter()
{
}

void HttpPathRewriter::setApplyAllRules(bool on)
{
   mApplyAllRules = on;
}

bool HttpPathRewriter::addRule(const char* regex, const char* format)
{
   bool rval = true;

   // compile regex, match case, allow submatches
   PatternRef p = Pattern::compile(regex, true, true);
   if(p.isNull())
   {
      ExceptionRef e = new Exception(
         "Could not add rewrite rule. Invalid regex.",
         "monarch.net.http.InvalidRewriteRule");
      e->getDetails()["regex"] = regex;
      e->getDetails()["format"] = format;
      Exception::push(e);
      rval = false;
   }
   else
   {
      // compile success, add rule
      Rule rule;
      rule.regex = p;
      rule.format = format;
      mRules.push_back(rule);
   }

   return rval;
}

void HttpPathRewriter::modifyRequest(HttpRequest* request)
{
   if(!mRules.empty())
   {
      // iterate over rules applying one or all
      string path;
      bool matched = false;
      for(Rules::iterator i = mRules.begin();
          !matched && i != mRules.end(); i++)
      {
         path = request->getHeader()->getPath();
         StringTools::regexRewrite(path, i->regex, i->format.c_str(), &matched);
         if(matched && !mApplyAllRules)
         {
            request->getHeader()->setPath(path.c_str());
         }
      }
   }
}
