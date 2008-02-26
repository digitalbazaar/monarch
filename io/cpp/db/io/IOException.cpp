/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/io/IOException.h"

#include "db/rt/DynamicObject.h"

using namespace db::io;
using namespace db::rt;

IOException::IOException(
   const char* message, const char* type, int code) :
   Exception(message, type, code)
{
   getDetails()["usedBytes"] = 0;
   getDetails()["unusedBytes"] = 0;
}

IOException::~IOException()
{
}

void IOException::setUsedBytes(int used)
{
   getDetails()["usedBytes"] = used;
}

int IOException::getUsedBytes()
{
   return getDetails()["usedBytes"]->getInt32();
}

void IOException::setUnusedBytes(int unused)
{
   getDetails()["unusedBytes"] = unused;
}

int IOException::getUnusedBytes()
{
   return getDetails()["unusedBytes"]->getInt32();
}
