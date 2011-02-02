/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/crypto/DigitalSignatureOutputStream.h"

using namespace monarch::crypto;
using namespace monarch::io;

DigitalSignatureOutputStream::DigitalSignatureOutputStream(
   DigitalSignature* ds, bool cleanupSignature,
   OutputStream* os, bool cleanupStream) :
   FilterOutputStream(os, cleanupStream),
   mSignature(ds),
   mCleanupSignature(cleanupSignature)
{
}

DigitalSignatureOutputStream::~DigitalSignatureOutputStream()
{
   if(mCleanupSignature)
   {
      delete mSignature;
   }
}

bool DigitalSignatureOutputStream::write(const char* b, int length)
{
   if(mSignature != NULL)
   {
      // update digital signature
      mSignature->update(b, length);
   }

   // write to underlying stream
   return FilterOutputStream::write(b, length);
}

void DigitalSignatureOutputStream::setSignature(
   DigitalSignature* ds, bool cleanup)
{
   if(mCleanupSignature)
   {
      delete mSignature;
   }

   mSignature = ds;
   mCleanupSignature = cleanup;
}

DigitalSignature* DigitalSignatureOutputStream::getSignature()
{
   return mSignature;
}
