/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/io/File.h"

using namespace std;
using namespace db::io;

File::File()
{
}

File::File(const char* name)
{
   mName = name;
}

File::~File()
{
}

const string& File::getName()
{
   return mName;
}
