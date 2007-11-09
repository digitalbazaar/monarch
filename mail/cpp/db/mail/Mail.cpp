/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/mail/Mail.h"
#include "db/util/StringTools.h"

using namespace std;
using namespace db::mail;
using namespace db::util;

Mail::Mail()
{
   mRecipients->setType(Array);
   mMessage["headers"]["To"]->setType(Array);
}

Mail::~Mail()
{
}

string& Mail::smtpMessageEncode(string& str)
{
   return StringTools::replaceAll(str, "\r\n.\r\n", "\r\n..\r\n");
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
   }
   
   return rval;
}

bool Mail::addRecipient(const char* header, const char* address)
{
   bool rval = false;
   
   Address a;
   if(rval = setAddress(a, address))
   {
      // add to list of recipients
      mRecipients[mRecipients->length()] = a;
      
      // add header if not NULL
      if(header != NULL)
      {
         string str = address;
         smtpMessageEncode(str);
         mMessage["headers"][header]->setType(Array);
         mMessage["headers"][header][mMessage["headers"][header]->length()] =
            str.c_str();
      }
   }
   
   return rval;
}

bool Mail::setSender(const char* address)
{
   bool rval = false;
   
   if(rval = setAddress(mSender, address))
   {
      // set "From" header
      string str = address;
      smtpMessageEncode(str);
      mMessage["headers"]["From"] = str.c_str();
   }
   
   return rval;
}

Address Mail::getSender()
{
   return mSender;
}

bool Mail::addTo(const char* address)
{
   return addRecipient("To", address);
}

bool Mail::addCC(const char* address)
{
   return addRecipient("CC", address);
}

bool Mail::addBCC(const char* address)
{
   return addRecipient(NULL, address);
}

AddressList Mail::getRecipients()
{
   return mRecipients;
}

void Mail::setHeader(const char* header, const char* value)
{
   string str = value;
   smtpMessageEncode(str);
   mMessage["headers"][header] = str.c_str();
}

void Mail::setSubject(const char* subject)
{
   string str = subject;
   smtpMessageEncode(str);
   mMessage["headers"]["Subject"] = str.c_str();
}

void Mail::setBody(const char* body)
{
   string str = body;
   smtpMessageEncode(str);
   mMessage["body"] = str.c_str();
}

Message Mail::getMessage()
{
   return mMessage;
}
