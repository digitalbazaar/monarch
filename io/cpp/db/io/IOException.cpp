/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/io/IOException.h"

using namespace db::io;
using namespace db::rt;

IOException::IOException(
   const char* message, const char* type, int code) :
   Exception(message, type, code)
{
   mUsedBytes = 0;
   mUnusedBytes = 0;
}

IOException::~IOException()
{
}

void IOException::setUsedBytes(int used)
{
   mUsedBytes = used;
}

int IOException::getUsedBytes()
{
   return mUsedBytes;
}

void IOException::setUnusedBytes(int unused)
{
   mUnusedBytes = unused;
}

int IOException::getUnusedBytes()
{
   return mUnusedBytes;
}
