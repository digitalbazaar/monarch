/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/mail/SmtpClient.h"
#include "db/net/InternetAddress.h"

using namespace std;
using namespace db::io;
using namespace db::mail;
using namespace db::net;
using namespace db::rt;
using namespace db::util;

SmtpClient::SmtpClient()
{
   mSslContext = NULL;
}

SmtpClient::~SmtpClient()
{
   if(mSslContext != NULL)
   {
      delete mSslContext;
   }
}

void SmtpClient::activateSsl(Connection* c, bool client)
{
   mSslContext = new SslContext();
   
   // switch underlying socket with an SSL socket
   Socket* s = new SslSocket(
      mSslContext, (TcpSocket*)c->getSocket(), client,
      c->mustCleanupSocket());
   c->setSocket(s, true);
   c->setSecure(true);
}

int SmtpClient::getResponseCode(Connection* c)
{
   int rval = -1;
   
   // get response line
   string line;
   if(c->getInputStream()->readCrlf(line))
   {
      // parse code from line
      rval = strtoul(line.c_str(), NULL, 10);
   }
   
   return rval;
}

bool SmtpClient::helo(Connection* c, const char* domain)
{
   bool rval = false;
   
   // send "HELO" verb
   if(c->getOutputStream()->write("HELO", 4))
   {
      // send space
      if(c->getOutputStream()->write(" ", 1))
      {
         // send domain
         rval = c->getOutputStream()->write(domain, strlen(domain));
      }
   }
   
   return rval;
}

bool SmtpClient::mailFrom(Connection* c, const char* address)
{
   bool rval = false;
   
   // send "MAIL FROM:" verb
   if(c->getOutputStream()->write("MAIL FROM:", 10))
   {
      // send SMTP-encoded address
      rval = c->getOutputStream()->write(address, strlen(address));
   }
   
   return rval;
}

bool SmtpClient::rcptTo(Connection* c, const char* address)
{
   bool rval = false;
   
   // send "RCPT TO:" verb
   if(c->getOutputStream()->write("RCTP TO:", 8))
   {
      // send SMTP-encoded address
      rval = c->getOutputStream()->write(address, strlen(address));
   }
   
   return rval;
}

bool SmtpClient::startData(Connection* c)
{
   // send "DATA" verb
   return c->getOutputStream()->write("DATA", 4);
}

bool SmtpClient::sendMessage(Connection* c, Message msg)
{
   bool rval = true;
   
   // send headers
   DynamicObjectIterator i = msg["headers"].getIterator();
   while(rval && i->hasNext())
   {
      // iterate over each header
      DynamicObjectIterator hi = i->next().getIterator();
      bool comma = false;
      while(rval && hi->hasNext())
      {
         DynamicObject header = hi->next();
         
         if(rval && !comma)
         {
            // send header name
            rval = c->getOutputStream()->write(
               i->getName(), strlen(i->getName()));
         }
         
         if(rval)
         {
            // send colon
            rval = c->getOutputStream()->write(": ", 2);
         }
         
         if(rval && comma)
         {
            // send comma
            rval = c->getOutputStream()->write(", ", 2);
         }
         else
         {
            comma = true;
         }
         
         if(rval)
         {
            // send header value
            rval = c->getOutputStream()->write(
               header->getString(), header->length());
         }
      }
   }
   
   if(rval)
   {
      // send body
      rval = c->getOutputStream()->write(
         msg["body"]->getString(), msg["body"]->length());
   }
   
   return rval;   
}

bool SmtpClient::endData(Connection* c)
{
   // end with .CRLF
   return c->getOutputStream()->write("./r/n", 3);
}

bool SmtpClient::quit(Connection* c)
{
   // send "QUIT" verb
   return c->getOutputStream()->write("QUIT", 4);
}

bool SmtpClient::sendMail(Connection* c, Mail* mail)
{
   bool rval = true;
   
   // FIXME: this is the simplest implementation to get this thing to
   // send mail to our server, it will have to be filled out later if
   // we so desire
   
   // for storing server response codes
   int code;
   
   // receive response from server
   if((code = getResponseCode(c)) != 220)
   {
      rval = false;
      Exception::setLast(
         new IOException("Bad SMTP server response code!"));
   }
   
   // do helo
   if(rval)
   {
      // say helo from sender's domain
      if(rval = helo(c, mail->getSender()["domain"]->getString()))
      {
         // receive response
         if((code = getResponseCode(c)) != 250)
         {
            rval = false;
            Exception::setLast(
               new IOException("Bad SMTP server response code!"));
         }
      }
   }
   
   // do mail from
   if(rval)
   {
      // send sender's address
      if(rval = mailFrom(
         c, mail->getSender()["smtpEncoding"]->getString()))
      {
         // receive response
         if((code = getResponseCode(c)) != 250)
         {
            rval = false;
            Exception::setLast(
               new IOException("Bad SMTP server response code!"));
         }
      }
   }
   
   // do rcpt to
   AddressIterator i = mail->getRecipients().getIterator();
   while(rval && i->hasNext())
   {
      // send recipient's address
      if(rval = rcptTo(c, i->next()["smtpEncoding"]->getString()))
      {
         // receive response
         if((code = getResponseCode(c)) != 250)
         {
            rval = false;
            Exception::setLast(
               new IOException("Bad SMTP server response code!"));
         }
      }
   }
   
   // start data
   if(rval)
   {
      // send DATA verb
      if(rval = startData(c))
      {
         // receive response
         if((code = getResponseCode(c)) != 354)
         {
            rval = false;
            Exception::setLast(
               new IOException("Bad SMTP server response code!"));
         }
      }
   }
   
   // send data
   if(rval)
   {
      // send message
      rval = sendMessage(c, mail->getMessage());
   }
   
   // end data
   if(rval)
   {
      // end data
      if(rval = endData(c))
      {
         // receive response
         if((code = getResponseCode(c)) != 250)
         {
            rval = false;
            Exception::setLast(
               new IOException("Bad SMTP server response code!"));
         }
      }
   }
   
   // quit
   if(rval)
   {
      // send QUIT verb
      if(rval = quit(c))
      {
         // receive response
         if((code = getResponseCode(c)) != 221)
         {
            rval = false;
            Exception::setLast(
               new IOException("Bad SMTP server response code!"));
         }
      }
   }
   
   return rval;
}

bool SmtpClient::sendMail(Url* url, Mail* mail)
{
   bool rval = false;
   
   // connect with 30 second timeout
   TcpSocket s;
   InternetAddress address(url->getHost(), url->getPort());
   if(s.connect(&address, 30))
   {
      // create smtp connection
      Connection c(&s, false);
      
      // send mail
      rval = sendMail(&c, mail);
      
      // disconnect
      c.close();
   }
   
   return rval;
}
