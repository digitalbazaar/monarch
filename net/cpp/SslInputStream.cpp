/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "SslInputStream.h"

using namespace db::io;
using namespace db::net;

SslInputStream::SslInputStream(SslSocket* s) :
   FilterInputStream(s->getSocket()->getInputStream(), false)
{
   mSocket = s;
}

SslInputStream::~SslInputStream()
{
}

int SslInputStream::read(
   char* b, unsigned int offset, unsigned int length) throw(IOException)
{
   int rval = -1;
   
   // FIXME:
   
   return rval;
}
