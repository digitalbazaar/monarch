/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_mail_SmtpClient_H
#define monarch_mail_SmtpClient_H

#include "monarch/net/Connection.h"
#include "monarch/net/SslSocket.h"
#include "monarch/net/Url.h"
#include "monarch/mail/Mail.h"

namespace monarch
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
   monarch::net::SslContext* mSslContext;

   /**
    * Turns on TLS for an smtp connection by wrapping the underlying
    * connection's socket with an SSL Socket.
    *
    * @param c the smtp connection.
    */
   virtual void activateSsl(monarch::net::Connection* c);

   /**
    * Receives the smtp server's response code for the last send verb.
    *
    * @param c the smtp connection.
    *
    * @return the received code or -1 if an exception occurred.
    */
   virtual int getResponseCode(monarch::net::Connection* c);

   /**
    * Sends a CRLF to end a sent verb.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool sendCrlf(monarch::net::Connection* c);

   /**
    * Sends the "HELO" verb to the server.
    *
    * @param c the smtp connection.
    * @param domain the domain to send with the verb.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool helo(monarch::net::Connection* c, const char* domain);

   /**
    * Sends the "MAIL FROM:" verb to the server.
    *
    * @param c the smtp connection.
    * @param address the address to send with the verb.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool mailFrom(monarch::net::Connection* c, const char* address);

   /**
    * Sends the "RCPT TO:" verb to the server.
    *
    * @param c the smtp connection.
    * @param address the address to send with the verb.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool rcptTo(monarch::net::Connection* c, const char* address);

   /**
    * Sends the "DATA" verb to the server.
    *
    * @param c the smtp connection.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool startData(monarch::net::Connection* c);

   /**
    * Sends the message to the server.
    *
    * @param c the smtp connection.
    * @param mail the mail with the message to send.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool sendMessage(monarch::net::Connection* c, Mail* mail);

   /**
    * Sends the data delimiter signaling the end of data to the server.
    *
    * @param c the smtp connection.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool endData(monarch::net::Connection* c);

   /**
    * Sends the "QUIT" verb to the server.
    *
    * @param c the smtp connection.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool quit(monarch::net::Connection* c);

   /**
    * Sends mail over the passed connection to an smtp server.
    *
    * @param c the smtp connection.
    * @param mail the mail to send.
    *
    * @return true if the mail was sent successfully, false if an exception
    *         occurred.
    */
   virtual bool sendMail(monarch::net::Connection* c, Mail* mail);

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
   virtual bool sendMail(monarch::net::Url* url, Mail* mail);
};

} // end namespace mail
} // end namespace monarch
#endif
