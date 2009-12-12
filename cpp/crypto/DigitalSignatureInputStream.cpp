/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/crypto/DigitalSignatureInputStream.h"

using namespace monarch::crypto;
using namespace monarch::io;

DigitalSignatureInputStream::DigitalSignatureInputStream(
   DigitalSignature* ds, bool cleanupSignature,
   InputStream* os, bool cleanupStream) :
   FilterInputStream(os, cleanupStream)
{
   mSignature = ds;
   mCleanupSignature = cleanupSignature;
}

DigitalSignatureInputStream::~DigitalSignatureInputStream()
{
   if(mCleanupSignature && mSignature != NULL)
   {
      delete mSignature;
   }
}

int DigitalSignatureInputStream::read(char* b, int length)
{
   // read from underlying stream
   int rval = FilterInputStream::read(b, length);

   if(rval > 0 && mSignature != NULL)
   {
      // update digital signature
      mSignature->update(b, rval);
   }

   return rval;
}

void DigitalSignatureInputStream::setSignature(
   DigitalSignature* ds, bool cleanup)
{
   if(mCleanupSignature && mSignature != NULL)
   {
      delete mSignature;
   }

   mSignature = ds;
   mCleanupSignature = cleanup;
}

DigitalSignature* DigitalSignatureInputStream::getSignature()
{
   return mSignature;
}
