/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/crypto/DigitalSignatureOutputStream.h"

using namespace db::crypto;
using namespace db::io;

DigitalSignatureOutputStream::DigitalSignatureOutputStream(
   DigitalSignature* ds, OutputStream* os, bool cleanup) :
   FilterOutputStream(os, cleanup)
{
   mSignature = ds;
}

DigitalSignatureOutputStream::~DigitalSignatureOutputStream()
{
}

bool DigitalSignatureOutputStream::write(const char* b, int length)
{
   // update digital signature
   mSignature->update(b, length);
   
   // write to underlying stream
   return FilterOutputStream::write(b, length);
}

DigitalSignature* DigitalSignatureOutputStream::getDigitalSignature()
{
   return mSignature;
}
