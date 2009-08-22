/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_mail_MailSpool_H
#define db_mail_MailSpool_H

#include "db/mail/Mail.h"
#include "db/io/File.h"

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
    * A lock for modifying/reading from the spool.
    */
   db::rt::ExclusiveLock mSpoolLock;

   /**
    * The index of the first valid mail in the spool.
    */
   uint32_t mHead;

   /**
    * The index of the last valid mail in the spool.
    */
   uint32_t mTail;

   /**
    * The number of mails in the spool.
    */
   uint32_t mCount;

   /**
    * The spool file to use.
    */
   db::io::File mFile;

   /**
    * Creates or overwrites the index associated with the spool file.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool writeIndex();

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
    * Locks the spool prohibiting other threads from modifying it.
    */
   virtual void lock();

   /**
    * Unlocks the spool allowing other threads to modify it.
    */
   virtual void unlock();

   /**
    * Sets the spool file to use and reads in its associated index.
    *
    * @param file the spool file to use.
    *
    * @return true if successful, false if there was an exception.
    */
   virtual bool setFile(db::io::File& file);

   /**
    * Spools the passed mail, adding it to the existing spool file.
    *
    * @param mail the Mail to spool.
    *
    * @return true if successful, false if there was an exception.
    */
   virtual bool spool(Mail* mail);

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
};

} // end namespace mail
} // end namespace db
#endif
