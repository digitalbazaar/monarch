/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "FileNotFoundException.h"

using namespace std;
using namespace db::io;

FileNotFoundException::FileNotFoundException(
   const string& message, const string& code) : IOException(message, code)
{
}

FileNotFoundException::~FileNotFoundException()
{
}
