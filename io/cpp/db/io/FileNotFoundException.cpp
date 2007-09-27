/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/io/FileNotFoundException.h"

using namespace std;
using namespace db::io;

FileNotFoundException::FileNotFoundException(
   const char* message, const char* type, int code) :
   IOException(message, type, code)
{
}

FileNotFoundException::~FileNotFoundException()
{
}
