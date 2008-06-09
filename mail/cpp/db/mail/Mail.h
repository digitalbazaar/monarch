/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_mail_Mail_H
#define db_mail_Mail_H

#include "db/rt/DynamicObject.h"
#include "db/rt/DynamicObjectIterator.h"

namespace db
{
namespace mail
{

// typedefs for Address and AddressList
typedef db::rt::DynamicObject Address;
typedef db::rt::DynamicObject AddressList;
typedef db::rt::DynamicObjectIterator AddressIterator;
typedef db::rt::DynamicObject Message;

/**
 * A Mail is a class that represents an email message.
 * 
 * @author Dave Longley
 */
class Mail
{
protected:
   /**
    * The sender of this Mail.
    */
   Address mSender;
   
   /**
    * The list of recipients of this Mail.
    */
   AddressList mRecipients;
   
   /**
    * The message of this Mail.
    */
   Message mMessage;
   
   /**
    * Sets the passed address, updating its properties including its
    * SMTP-encoding of the passed email address.
    * 
    * @param a the Address to update.
    * @param address the email address to update it with.
    * 
    * @return true if the email address was valid, false if not.
    */
   virtual bool setAddress(Address& a, const char* address);
   
   /**
    * Adds a recipient to this Mail.
    * 
    * @param header the header to file the mail under.
    * @param address the recipient's email address to add to this Mail.
    * 
    * @return true if the email address was valid, false if not.
    */
   virtual bool addRecipient(const char* header, const char* address);
   
public:
   /**
    * Creates a new Mail.
    */
   Mail();
   
   /**
    * Destructs this Mail.
    */
   virtual ~Mail();
   
   /**
    * Clears this Mail.
    */
   virtual void clear();
   
   /**
    * Sets the sender of this Mail.
    * 
    * @param address the email address of the sender.
    * 
    * @return true if the email address was valid, false if not.
    */
   virtual bool setSender(const char* address);
   
   /**
    * Gets the sender of this Mail.
    * 
    * @return the address of the sender.
    */
   virtual Address& getSender();
   
   /**
    * Adds a recipient of this Mail using the "To" header.
    * 
    * @param address the recipient's email address.
    * 
    * @return true if the email address was valid, false if not.
    */
   virtual bool addTo(const char* address);
   
   /**
    * Adds a recipient of this Mail using the "CC" header.
    * 
    * @param address the recipient's email address.
    * 
    * @return true if the email address was valid, false if not.
    */
   virtual bool addCc(const char* address);
   
   /**
    * Adds a recipient of this Mail using no header.
    * 
    * @param address the recipient's email address.
    * 
    * @return true if the email address was valid, false if not.
    */
   virtual bool addBcc(const char* address);
   
   /**
    * Gets a list of the recipients of this Mail.
    * 
    * @return a list of recipients of this Mail.
    */
   virtual AddressList& getRecipients();
   
   /**
    * Sets a header in this Mail.
    * 
    * @param header the header name.
    * @param value the header value.
    */
   virtual void setHeader(const char* header, const char* value);
   
   /**
    * Sets the subject of this Mail's message.
    * 
    * @param subject the subject of this Mail's message.
    */
   virtual void setSubject(const char* subject);
   
   /**
    * Sets the body of this Mail's message.
    * 
    * @param body the body of this Mail's message.
    */
   virtual void setBody(const char* body);
   
   /**
    * Appends the passed line to the body of this Mail's message.
    * 
    * @param line the line to append to this Mail's message.
    */
   virtual void appendBodyLine(const char* line);
   
   /**
    * Gets the Message of this Mail.
    * 
    * @return the Message of this Mail.
    */
   virtual Message& getMessage();
   
   /**
    * Converts this Mail into a template that can be re-parsed to create
    * this Mail again. This is useful for spooling.
    * 
    * @return the Mail template.
    */
   virtual std::string toTemplate();
   
   /**
    * Encodes a string for smtp message transfer.
    * 
    * @param str the message to encode.
    * 
    * @return the encoded string.
    */
   static std::string& smtpMessageEncode(std::string& str);
};

} // end namespace mail
} // end namespace db
#endif
