/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/crypto/DigitalSignatureInputStream.h"

using namespace db::crypto;
using namespace db::io;

DigitalSignatureInputStream::DigitalSignatureInputStream(
   DigitalSignature* ds, InputStream* os, bool cleanup) :
   FilterInputStream(os, cleanup)
{
   mSignature = ds;
}

DigitalSignatureInputStream::~DigitalSignatureInputStream()
{
}

int DigitalSignatureInputStream::read(char* b, unsigned int length)
{
   // read from underlying stream
   int rval = FilterInputStream::read(b, length);
   
   if(rval > 0)
   {
      // update digital signature
      mSignature->update(b, rval);
   }
   
   return rval;
}

DigitalSignature* DigitalSignatureInputStream::getDigitalSignature()
{
   return mSignature;
}
