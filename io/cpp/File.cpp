/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "File.h"

using namespace std;
using namespace db::io;

File::File()
{
}

File::File(const string& name)
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
