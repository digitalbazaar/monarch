/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_mail_SmtpClient_H
#define db_mail_SmtpClient_H

#include "db/net/Connection.h"
#include "db/net/SslSocket.h"
#include "db/net/Url.h"
#include "db/mail/Mail.h"

namespace db
{
namespace mail
{

/**
 * A SmtpClient provides a client interface that speaks the Simple Mail
 * Transfer Protocol to a server.
 * 
 * @author Dave Longley
 */
class SmtpClient
{
protected:
   /**
    * An SSL context for doing TLS communication.
    */
   db::net::SslContext* mSslContext;
   
   /**
    * Turns on TLS for an smtp connection by wrapping the underlying
    * connection's socket with an SSL Socket.
    * 
    * @param c the smtp connection.
    */
   virtual void activateSsl(db::net::Connection* c);
   
   /**
    * Receives the smtp server's response code for the last send verb.
    * 
    * @param c the smtp connection.
    * 
    * @return the received code or -1 if an exception occurred.
    */
   virtual int getResponseCode(db::net::Connection* c);
   
   /**
    * Sends a CRLF to end a sent verb.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool sendCrlf(db::net::Connection* c);
   
   /**
    * Sends the "HELO" verb to the server.
    * 
    * @param c the smtp connection.
    * @param domain the domain to send with the verb.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool helo(db::net::Connection* c, const char* domain);
   
   /**
    * Sends the "MAIL FROM:" verb to the server.
    * 
    * @param c the smtp connection.
    * @param address the address to send with the verb.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool mailFrom(db::net::Connection* c, const char* address);
   
   /**
    * Sends the "RCPT TO:" verb to the server.
    * 
    * @param c the smtp connection.
    * @param address the address to send with the verb.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool rcptTo(db::net::Connection* c, const char* address);
   
   /**
    * Sends the "DATA" verb to the server.
    * 
    * @param c the smtp connection.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool startData(db::net::Connection* c);
   
   /**
    * Sends the message to the server.
    * 
    * @param c the smtp connection.
    * @param mail the mail with the message to send.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool sendMessage(db::net::Connection* c, Mail* mail);
   
   /**
    * Sends the data delimiter signaling the end of data to the server.
    * 
    * @param c the smtp connection.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool endData(db::net::Connection* c);
   
   /**
    * Sends the "QUIT" verb to the server.
    * 
    * @param c the smtp connection.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool quit(db::net::Connection* c);
   
   /**
    * Sends mail over the passed connection to an smtp server.
    * 
    * @param c the smtp connection.
    * @param mail the mail to send.
    * 
    * @return true if the mail was sent successfully, false if an exception
    *         occurred.
    */
   virtual bool sendMail(db::net::Connection* c, Mail* mail);
   
public:
   /**
    * Creates a new SmtpClient.
    */
   SmtpClient();
   
   /**
    * Destructs this SmtpClient.
    */
   virtual ~SmtpClient();
   
   /**
    * Sends mail to the passed url (that points at an smtp server).
    * 
    * @param url the url to send the mail to.
    * @param mail the mail to send.
    * 
    * @return true if the mail was sent successfully, false if an exception
    *         occurred.
    */
   virtual bool sendMail(db::net::Url* url, Mail* mail);
};

} // end namespace mail
} // end namespace db
#endif
