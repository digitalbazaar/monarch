/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/mail/MailTemplateParser.h"
#include "db/util/StringTools.h"

using namespace std;
using namespace db::io;
using namespace db::mail;
using namespace db::rt;
using namespace db::util;

MailTemplateParser::MailTemplateParser()
{
}

MailTemplateParser::~MailTemplateParser()
{
}

Exception* MailTemplateParser::parseLine(
   Mail* mail, DynamicObject& vars, const char* line, bool& headers)
{
   Exception* rval = NULL;
   
   // keep a string for appending message data to
   string msg;
   
   // replace all variables in the line
   const char* var = NULL;
   const char* start = line;
   while(start != NULL && (var = strchr(start, '$')) != NULL)
   {
      // ensure the '$' isn't escaped
      bool escaped = false;
      if(var != line)
      {
         // check the preceeding characters for a non-escape character
         for(const char* v = var - 1; !escaped && v != line; v--)
         {
            if(*v != '\\')
            {
               // '$' is not escaped, no need to check further
               break;
            }
            else if((v - 1) == line)
            {
               // '$' is escaped
               escaped = true;
            }
            else if(*(v - 1) == '\\')
            {
               // an escaped '\' has been found, decrement v
               v--;
            }
            else
            {
               // '$' is escaped
               escaped = true;
            }
         }
      }
      
      if(escaped)
      {
         // append line data before the escape character
         // the '$' is escaped, so just append the dollar sign
         msg.append(start, var - start - 1);
         msg.push_back('$');
         
         // update start
         start = var + 1;
      }
      else
      {
         // append all line data before the variable
         msg.append(start, var - start);
         
         // pass '$' character
         var++;
         
         // find the next non-alphanumeric (non-variable character) and use
         // it as the next starting point for parsing variables
         start = strpbrk(var, " \t\\`~!@#$%^&*()-+=[]{}|;':\",./<>?");
         
         // append the variable value
         if(start != NULL)
         {
            // variable name ends before the line is terminated
            char key[start - var + 1];
            strncpy(key, var, start - var);
            key[start - var] = 0;
            msg.append(vars[key]->getString());
         }
         else
         {
            // variable name goes to the end of the line
            msg.append(vars[var]->getString());
         }
      }
   }
   
   // append remainder of the line
   if(var == NULL && start != NULL)
   {
      msg.append(start);
   }
   
   // unescape all '\' characters
   StringTools::replaceAll(msg, "\\\\", "\\");
   
   // point line at the parsed line
   line = msg.c_str();
   
   if(headers)
   {
      // get the header name
      const char* header = strstr(line, ": ");
      
      // ensure there is a header name and that no white-space occurs in it
      if(header == NULL || (int)strcspn(line, " \t") < (header - line))
      {
         rval = new Exception(
            "Parse error while parsing mail template! Mail header "
            "is malformed, non-existant, or Subject header was not "
            "found.");
         Exception::setLast(rval);
      }
      else
      {
         // see if the header is the subject header
         if(strncasecmp(line, "subject", 7) == 0)
         {
            // subject line found, no longer adding headers
            mail->setSubject(header + 2);
            headers = false;
         }
         else
         {
            // set the header
            char hdr[header - line + 1];
            strncpy(hdr, line, header - line);
            hdr[header - line] = 0;
            mail->setHeader(hdr, header + 2);
         }
      }
   }
   else
   {
      // append the line to the mail's body
      mail->appendBodyLine(line);
   }
   
   return rval;
}

Exception* MailTemplateParser::parse(
   Mail* mail, DynamicObject& vars, InputStream* is)
{
   Exception* rval = NULL;
   
   // SMTP RFC requires lines be no longer than 998 bytes (+2 for CRLF = 1000)
   // so read in a maximum of 1000 bytes at a time
   bool headers = true;
   char b[1000];
   int numBytes;
   int length = 0;
   char* start = 0;
   char* end = 0;
   bool cr = false;
   
   // read as much as 1000 bytes at a time, then check the read buffer
   while(rval == NULL && (numBytes = is->read(b, 1000 - length)) > 0)
   {
      // increment length
      length += numBytes;
      
      // ensure line is null-terminated
      b[length] = 0;
      start = b;
      
      // parse lines according to line breaks
      while(rval == NULL && (end = strpbrk(start, "\r\n")) != NULL)
      {
         // take note of CR, then insert null-terminator
         cr = (end[0] == '\r');
         end[0] = 0;
         
         // update length
         length -= (end - start);
         
         // parse line and increment start, skipping LF as appropriate
         rval = parseLine(mail, vars, start, headers);
         start = (cr && end[1] == '\n') ? end + 2 : end + 1;
      }
      
      if(end == NULL && length == 1000)
      {
         // invalid line detected
         numBytes = -1;
         rval = new Exception(
            "Message line too long! SMTP requires that lines be no longer "
            "than 1000 bytes, including the terminating CRLF.");
         Exception::setLast(rval);
      }
      else if(start > b)
      {
         // shift buffer contents as necessary
         memmove(b, start, length);
      }
   }
   
   if(numBytes < 0)
   {
      rval = Exception::getLast();
   }
   else if(start != NULL)
   {
      // parse the last line
      rval = parseLine(mail, vars, start, headers);
   }
   
   return rval;
}
