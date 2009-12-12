/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_mail_MailSpool_H
#define db_mail_MailSpool_H

#include "monarch/mail/Mail.h"
#include "monarch/io/File.h"
#include "monarch/sql/DatabaseClient.h"

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
    * Gets the first mail in the spool. Note: If you are using getCount() to
    * iterate over the mails in the spool, either make sure you do so in a
    * single thread or you properly synchronize access to the spool.
    * Alternatively, if you use unwind() with the "mail" and "unwound"
    * parameters instead of getCount() and getFirst(), you do not need any
    * special synchronization if you access the spool from multiple threads.
    *
    * @param mail the Mail to populate.
    *
    * @return true if successful, false if there was an exception.
    */
   virtual bool getFirst(Mail* mail);

   /**
    * Deletes the first mail in the spool, if there is one. If the spool is
    * empty this method will still return true. To find out if a mail was
    * unwound from the spool, you must provide the "unwound" parameter. To
    * save the mail that was unwound, you must provide the "mail" parameter.
    *
    * Unwinding a mail will be done synchronously so that if two threads call
    * unwind() concurrently, they will not both unwind the same mail.
    *
    * Note: If you are using getCount() to iterate over the mails in the spool
    * as you unwind them, make sure you do so in a single thread or you properly
    * synchronize access to the spool. Alternatively you can call unwind()
    * without any special synchronizing if you use the "mail" and "unwound"
    * parameters and do not rely on getCount().
    *
    * @param mail the Mail to save an unwound Mail into, NULL to drop it.
    * @param unwound will be set to true if a mail was unwound from the spool,
    *                false if not.
    *
    * @return true if successful, false if there was an exception.
    */
   virtual bool unwind(Mail* mail = NULL, bool* unwound = NULL);

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
