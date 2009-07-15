/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/crypto/BlockCipherInputStream.h"

using namespace db::crypto;
using namespace db::io;

BlockCipherInputStream::BlockCipherInputStream(
   BlockCipher* cipher, bool cleanupCipher,
   InputStream* os, bool cleanupStream) :
   FilterInputStream(os, cleanupStream),
   mReadBuffer(2048)
{
   mCipher = cipher;
   mCleanupCipher = cleanupCipher;
   mCipherFinished = false;
}

BlockCipherInputStream::~BlockCipherInputStream()
{
   if(mCleanupCipher && mCipher != NULL)
   {
      delete mCipher;
   }
}

int BlockCipherInputStream::read(char* b, int length)
{
   int rval = 0;
   
   // read from buffer if not empty
   if(!mReadBuffer.isEmpty())
   {
      rval = mReadBuffer.get(b, length);
   }
   else if(mCipher == NULL)
   {
      // read from underlying stream, no ciphering
      rval = FilterInputStream::read(b, length);
   }
   else
   {
      // while no data and cipher not finished, read and cipher data
      bool success;
      while(rval == 0 && !mCipherFinished)
      {
         // read from underlying stream
         rval = FilterInputStream::read(b, length);
         if(rval >= 0)
         {
            // update or finish cipher based on data left in underlying stream
            if(rval > 0)
            {
               // update cipher
               success = mCipher->update(b, rval, &mReadBuffer, true);
            }
            else
            {
               // finish cipher
               success = mCipher->finish(&mReadBuffer, true);
               mCipherFinished = true;
            }
            
            if(success)
            {
               // read from buffer
               rval = mReadBuffer.get(b, length);
            }
            else
            {
               // exception occurred
               rval = -1;
            }
         }
      }
   }
   
   return rval;
}

void BlockCipherInputStream::setCipher(BlockCipher* cipher, bool cleanup)
{
   if(mCleanupCipher && mCipher != NULL)
   {
      delete mCipher;
   }
   
   mCipher = cipher;
   mCleanupCipher = cleanup;
   mCipherFinished = false;
}

BlockCipher* BlockCipherInputStream::getCipher()
{
   return mCipher;
}
