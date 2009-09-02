/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_mail_MailSpool_H
#define db_mail_MailSpool_H

#include "db/mail/Mail.h"
#include "db/io/File.h"
#include "db/sql/DatabaseClient.h"

namespace db
{
namespace mail
{

/**
 * A MailSpool is used to save mail on disk to a single file.
 *
 * @author Dave Longley
 */
class MailSpool
{
protected:
   /**
    * The database client for the spool database.
    */
   db::sql::DatabaseClientRef mDbClient;

public:
   /**
    * Creates a new MailSpool.
    */
   MailSpool();

   /**
    * Destructs this MailSpool.
    */
   virtual ~MailSpool();

   /**
    * Initializes the spool for use.
    *
    * @param url the sqlite3 or file URL to the spool's database.
    *
    * @return true if successful, false if not.
    */
   virtual bool initialize(const char* url);

   /**
    * Sets whether or not debug logging will be used when writing
    * to the pool. Must be called after initialize.
    *
    * @param on true to turn on debug logging, false not to.
    */
   virtual void setDebugLogging(bool on);

   /**
    * Spools the passed mail, adding it to the existing spool file.
    *
    * @param mail the Mail to spool.
    * @param reason a dynamic object that explains the reason for the spooling,
    *               NULL for none.
    *
    * @return true if successful, false if there was an exception.
    */
   virtual bool spool(Mail* mail, db::rt::DynamicObject* reason);

   /**
    * Gets the first mail in the spool. Note: Make sure the spool is
    * locked when performing this operation inconjunction with an unwind().
    *
    * @param mail the Mail to populate.
    *
    * @return true if successful, false if there was an exception.
    */
   virtual bool getFirst(Mail* mail);

   /**
    * Deletes the first mail in the spool. Note: Make sure the spool is
    * locked when performing this operation.
    *
    * @return true if successful, false if there was an exception.
    */
   virtual bool unwind();

   /**
    * Returns the number of mails in the spool file.
    *
    * @return the number of mails.
    */
   virtual uint32_t getCount();

   /**
    * Clears the mail spool.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool clear();
};

} // end namespace mail
} // end namespace db
#endif
