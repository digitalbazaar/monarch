/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/sql/util/DatabaseClient.h"

using namespace db::sql;
using namespace db::sql::util;

DatabaseClient::DatabaseClient(ConnectionPool* cp)
{
   mConnectionPool = cp;
}

DatabaseClient::~DatabaseClient()
{
}

Connection* DatabaseClient::getConnection()
{
   return mConnectionPool->getConnection();
}
