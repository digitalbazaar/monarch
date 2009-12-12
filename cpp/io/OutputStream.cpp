/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/io/OutputStream.h"

#include "monarch/rt/Exception.h"

using namespace db::io;
using namespace db::rt;

OutputStream::OutputStream()
{
}

OutputStream::~OutputStream()
{
}

bool OutputStream::flush()
{
   return true;
}

bool OutputStream::finish()
{
   return true;
}

void OutputStream::close()
{
}
