/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/mail/MailSpool.h"

#include "db/io/ByteArrayInputStream.h"
#include "db/io/FileInputStream.h"
#include "db/io/FileOutputStream.h"
#include "db/data/Data.h"
#include "db/data/json/JsonReader.h"
#include "db/data/json/JsonWriter.h"
#include "db/mail/MailTemplateParser.h"

using namespace db::data::json;
using namespace db::io;
using namespace db::mail;
using namespace db::rt;

MailSpool::MailSpool() :
   mFile((FileImpl*)NULL)
{
   // spool empty thus far
   mHead = mTail = mCount = 0;
}

MailSpool::~MailSpool()
{
}

bool MailSpool::writeIndex()
{
   bool rval;
   
   // get associated index name
   std::string name = mFile->getAbsolutePath();
   name.append(".idx");
   File file(name.c_str());
   
   // write out index data
   FileOutputStream fos(file);
   DynamicObject config;
   config["head"] = mHead;
   config["tail"] = mTail;
   config["count"] = mCount;
   JsonWriter writer;
   rval = writer.write(config, &fos);
   fos.close();
   
   return rval;
}

void MailSpool::lock()
{
   mSpoolLock.lock();
}

void MailSpool::unlock()
{
   mSpoolLock.unlock();
}

bool MailSpool::setFile(File& file)
{
   bool rval;
   
   lock();
   {
      // set file
      mFile = file;
      
      // read associated index
      std::string name = mFile->getAbsolutePath();
      name.append(".idx");
      File file(name.c_str());
      if(!file->exists())
      {
         // create new spool index
         mHead = mTail = mCount = 0;
         rval = writeIndex();
      }
      else
      {
         // read existing spool index
         FileInputStream fis(file);
         JsonReader reader;
         DynamicObject config;
         reader.start(config);
         rval = reader.read(&fis) && reader.finish();
         fis.close();
         
         if(rval)
         {
            mHead = config["head"]->getUInt32();
            mTail = config["tail"]->getUInt32();
            mCount = config["count"]->getUInt32();
         }
      }
   }
   unlock();
   
   return rval;
}

bool MailSpool::spool(Mail* mail)
{
   bool rval;
   
   // convert mail to template
   std::string tpl = mail->toTemplate();
   
   lock();
   {
      // write out mail entry
      FileOutputStream fos(mFile, mCount > 0);
      
      // entry = index + size + tpl
      uint32_t idx = DB_UINT32_TO_LE(mTail);
      uint32_t size = DB_UINT32_TO_LE(tpl.length());
      rval =
         fos.write((char*)&idx, 4) &&
         fos.write((char*)&size, 4) &&
         fos.write(tpl.c_str(), tpl.length());
      fos.close();
      
      // increment tail, count
      mTail++;
      mCount++;
      
      // write out index
      rval = rval && writeIndex();
   }
   unlock();
   
   return rval;
}

bool MailSpool::getFirst(Mail* mail)
{
   bool rval = false;
   
   lock();
   {
      if(mCount == 0)
      {
         ExceptionRef e = new Exception(
            "Cannot get first mail from spool! Spool is empty!");
         Exception::setLast(e, false);
      }
      else
      {
         // read from spool file
         FileInputStream fis(mFile);
         char b[2048];
         int numBytes = 1;
         uint32_t idx;
         uint32_t size;
         
         // skip mail entries until head is reached
         while(numBytes > 0)
         {
            // read current index
            if((numBytes = fis.read(b, 4)) == 4)
            {
               memcpy(&idx, b, 4);
               idx = DB_UINT32_FROM_LE(idx);
            }
            
            // read current size
            if((numBytes = fis.read(b, 4) == 4))
            {
               memcpy(&size, b, 4);
               size = DB_UINT32_FROM_LE(size);
            }
            
            // read mail if current index is head
            if(idx == mHead)
            {
               // read mail data
               ByteBuffer bb(size);
               int read = (int)(size > 2048 ? 2048 : size);
               while(read > 0 && (numBytes = fis.read(b, read)) > 0)
               {
                  bb.put(b, numBytes, false);
                  size -= numBytes;
                  read = (int)(size > 2048 ? 2048 : size);
               }
               
               if(numBytes != -1)
               {
                  // parse mail data
                  ByteArrayInputStream bais(&bb, false);
                  MailTemplateParser parser;
                  DynamicObject vars;
                  rval = parser.parse(mail, vars, &bais);
               }
               break;
            }
            else
            {
               // skip mail data
               if(fis.skip(size) < 0)
               {
                  // error while skipping
                  numBytes = -1;
               }
            }
         }
         
         // close spool file
         fis.close();
      }
   }
   unlock();
   
   return rval;
}

bool MailSpool::unwind()
{
   bool rval;
   
   lock();
   {
      if(mCount == 0)
      {
         // nothing to do, spool is empty
         rval = true;
      }
      else
      {
         // increment head, decrement count
         mHead++;
         mCount--;
         
         if(mCount == 0)
         {
            // reset index, spool now empty
            mHead = mTail = 0;
            mFile->remove();
         }
         
         // write out index
         rval = writeIndex();
      }
   }
   unlock();
   
   return rval;
}

uint32_t MailSpool::getCount()
{
   return mCount;
}
