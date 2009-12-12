/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/io/IOException.h"

using namespace db::io;
using namespace db::rt;

IOException::IOException(
   const char* message, const char* type, int code) :
   Exception(message, type, code)
{
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
