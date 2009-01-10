/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/mail/MailTemplateParser.h"

#include "db/data/TemplateInputStream.h"

using namespace db::data;
using namespace db::io;
using namespace db::mail;
using namespace db::rt;

MailTemplateParser::MailTemplateParser()
{
}

MailTemplateParser::~MailTemplateParser()
{
}

/**
 * Parses a single line from the template and adds its contents to the
 * passed Mail either as a header or as a line of the message body.
 * Template variables are replaced according to the passed "vars"
 * DynamicObject and the headers flag is cleared once a blank line
 * has been parsed.
 * 
 * @param mail the Mail to populate.
 * @param vars the key-value variables in the template.
 * @param line the line to parse.
 * @param headers the flag to clear once the headers have been parsed.
 * 
 * @return true if successful, false if an exception occurred.
 */
static bool parseLine(Mail* mail, const char* line, bool& headers)
{
   bool rval = true;
   
   if(headers)
   {
      if(strlen(line) == 0)
      {
         // empty line means last header found
         headers = false;
      }
      else
      {
         // get the header name
         const char* header = strstr(line, ": ");
         
         // ensure there is a header name and that no white-space occurs in it
         if(header == NULL || (int)strcspn(line, " \t") < (header - line))
         {
            ExceptionRef e = new Exception(
               "Parse error while parsing mail template! Mail header "
               "is malformed or non-existant.");
            Exception::setLast(e, false);
            rval = false;
         }
         else
         {
            // see if the header is the subject header
            if(strncasecmp(line, "subject", 7) == 0)
            {
               // subject line found
               mail->setSubject(header + 2);
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
   }
   else
   {
      // append the line to the mail's body
      mail->appendBodyLine(line);
   }
   
   return rval;
}

bool MailTemplateParser::parse(
   Mail* mail, DynamicObject& vars, bool strict, InputStream* is)
{
   bool rval = true;
   
   // clear mail
   mail->clear();
   
   // add template input stream to passed input stream
   TemplateInputStream tis(vars, strict, is, false);
   
   // SMTP RFC requires lines be no longer than 998 bytes (+2 for CRLF = 1000)
   // so read in a maximum of 1000 bytes at a time
   bool headers = true;
   char b[1001];
   int numBytes;
   int length = 0;
   char* start = 0;
   char* end = 0;
   bool cr = false;
   
   // read as much as 1000 bytes at a time, then check the read buffer
   while(rval && (numBytes = tis.read(b + length, 1000 - length)) > 0)
   {
      // increment length (number of valid bytes in 'b')
      length += numBytes;
      
      // ensure line is null-terminated
      b[length] = 0;
      start = b;
      
      // parse lines according to line breaks
      while(rval && (end = strpbrk(start, "\r\n")) != NULL)
      {
         // take note of CR, then insert null-terminator
         cr = (end[0] == '\r');
         end[0] = 0;
         
         // parse line
         rval = parseLine(mail, start, headers);
         
         // decrement length and increment start skipping LF as appropriate
         // Note: 'b' always ends in 0, so end[1] must always be a valid byte
         // or the null-terminator of b
         int skip = (cr && end[1] == '\n' ? 2 : 1);
         length -= (end - start) + skip;
         start = end + skip;
      }
      
      if(end == NULL && length > 998)
      {
         // invalid line detected
         numBytes = -1;
         ExceptionRef e = new Exception(
            "Message line too long! SMTP requires that lines be no longer "
            "than 1000 bytes, including the terminating CRLF.");
         Exception::setLast(e, false);
         rval = false;
      }
      else if(start > b)
      {
         // shift buffer contents as necessary
         memmove(b, start, length + 1);
      }
   }
   
   if(numBytes < 0)
   {
      rval = false;
   }
   else if(length > 0)
   {
      // parse the last line
      rval = parseLine(mail, b, headers);
   }
   
   return rval;
}
