/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */

#include <string>

#include "db/database/Connection.h"

using namespace db::database;

Connection::Connection(const char *params)
{
   std::string paramsString(params);
   mInitParams = new db::net::Url(paramsString);
}

Connection::~Connection()
{
   delete mInitParams;
}

void Connection::commit()
{
}

void Connection::rollback()
{
}
