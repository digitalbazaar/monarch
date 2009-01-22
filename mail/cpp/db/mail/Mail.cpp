/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/mail/Mail.h"

#include "db/net/http/HttpHeader.h"
#include "db/util/Base64Codec.h"
#include "db/util/StringTools.h"

using namespace std;
using namespace db::mail;
using namespace db::rt;
using namespace db::util;
using namespace db::net::http;

Mail::Mail()
{
   mRecipients->setType(Array);
   mMessage["headers"]["To"]->setType(Array);
   mMessage["headers"]["Subject"] = "";
   mMessage["body"]->setType(String);
}

Mail::~Mail()
{
}

void Mail::clear()
{
   mRecipients->setType(Array);
   mRecipients->clear();
   mMessage["headers"]["To"]->setType(Array);
   mMessage["headers"]["To"]->clear();
   mMessage["headers"]["Subject"] = "";
   mMessage["body"]->setType(String);
   mMessage["body"]->clear();
}

bool Mail::setAddress(Address& a, const char* address)
{
   bool rval = false;
   
   // FIXME: obviously this needs work to be robust
   
   // set address
   a["address"] = address;
   
   // check for domain
   const char* at = strchr(address, '@');
   if(at != NULL)
   {
      // set domain
      a["domain"] = (at + 1);
      
      // set smtp encoding of address
      char temp[strlen(address) + 3];
      sprintf(temp, "<%s>", address);
      a["smtpEncoding"] = temp;
      
      rval = true;
   }
   
   return rval;
}

bool Mail::addRecipient(const char* header, const char* address)
{
   bool rval = false;
   
   Address a;
   if((rval = setAddress(a, address)))
   {
      // add to list of recipients
      mRecipients[mRecipients->length()] = a;
      
      // add header if not NULL
      if(header != NULL)
      {
         mMessage["headers"][header]->setType(Array);
         mMessage["headers"][header]->append() = address;
      }
   }
   
   return rval;
}

bool Mail::setSender(const char* address)
{
   bool rval = false;
   
   if((rval = setAddress(mSender, address)))
   {
      // set "From" header
      mMessage["headers"]["From"] = address;
   }
   
   return rval;
}

Address& Mail::getSender()
{
   return mSender;
}

bool Mail::addTo(const char* address)
{
   return addRecipient("To", address);
}

bool Mail::addCc(const char* address)
{
   return addRecipient("CC", address);
}

bool Mail::addBcc(const char* address)
{
   return addRecipient(NULL, address);
}

AddressList& Mail::getRecipients()
{
   return mRecipients;
}

void Mail::setHeader(const char* header, const char* value)
{
   if(strcasecmp(header, "from") == 0)
   {
      setSender(value);
   }
   else if(strcasecmp(header, "to") == 0)
   {
      addTo(value);
   }
   else if(strcasecmp(header, "cc") == 0)
   {
      addCc(value);
   }
   else if(strcasecmp(header, "bcc") == 0)
   {
      addBcc(value);
   }
   else if(strcasecmp(header, "subject") == 0)
   {
      setSubject(value);
   }
   else
   {
      char tmp[strlen(header) + 1];
      strcpy(tmp, header);
      HttpHeader::biCapitalize(tmp);
      mMessage["headers"][tmp] = value;
   }
}

void Mail::setSubject(const char* subject)
{
   mMessage["headers"]["Subject"] = subject;
}

void Mail::setBody(const char* body)
{
   mMessage["body"] = body;
}

void Mail::appendBodyLine(const char* line)
{
   string str = mMessage["body"]->getString();
   str.append(line);
   str.push_back('\r');
   str.push_back('\n');
   mMessage["body"] = str.c_str();
}

Message& Mail::getMessage()
{
   return mMessage;
}

bool Mail::shouldTransferEncodeBody()
{
   bool rval = false;
   
   if(mMessage["headers"]->hasMember("Content-Transfer-Encoding"))
   {
      const char* encoding =
         mMessage["headers"]["Content-Transfer-Encoding"]->getString();
      if(strcasecmp(encoding, "base64") == 0)
      {
         rval = true;
      }
   }
   
   return rval;
}

string Mail::getTransferEncodedBody()
{
   string rval = mMessage["body"]->getString();
   
   bool encoded = false;
   if(mMessage["headers"]->hasMember("Content-Transfer-Encoding"))
   {
      const char* encoding =
         mMessage["headers"]["Content-Transfer-Encoding"]->getString();
      if(strcasecmp(encoding, "base64") == 0)
      {
         // base64 encode message
         rval = Base64Codec::encode(rval.c_str(), rval.length());
         encoded = true;
      }
   }
   
   if(!encoded)
   {
      // use default smtp-encoding
      smtpMessageEncode(rval);
   }
   
   return rval;
}

string Mail::toTemplate()
{
   string str;
   
   // add headers
   DynamicObjectIterator i = mMessage["headers"].getIterator();
   while(i->hasNext())
   {
      DynamicObject& header = i->next();
      
      if(header->getType() == Array)
      {
         DynamicObjectIterator ii = header.getIterator();
         while(ii->hasNext())
         {
            DynamicObject& each = ii->next();
            str.append(i->getName());
            str.append(": ");
            str.append(each->getString());
            str.append("\r\n");
         }
      }
      else
      {
         str.append(i->getName());
         str.append(": ");
         str.append(header->getString());
         str.append("\r\n");
      }
   }
   
   // terminate headers
   str.append("\r\n");
   
   // add body
   str.append(mMessage["body"]->getString());
   
   // escape all '{', '}', and '\'
   StringTools::replaceAll(str, "\\", "\\\\");
   StringTools::replaceAll(str, "{", "\\{");
   StringTools::replaceAll(str, "}", "\\}");
   
   return str;
}

string& Mail::smtpMessageEncode(string& str)
{
   // insert second dot for any line that starts with a dot
   return StringTools::replaceAll(str, "\r\n.", "\r\n..");
}
