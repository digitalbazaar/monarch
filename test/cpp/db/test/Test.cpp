/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */

#include <assert.h>
#include <iostream>

#include "db/test/Test.h"
#include "db/rt/Exception.h"
#include "db/sql/SqlException.h"

using namespace std;
using namespace db::rt;
using namespace db::test;

void db::test::dumpException(Exception* e)
{
   if(dynamic_cast<db::sql::SqlException*>(e) != NULL)
   {
      db::sql::SqlException* dbe =
         (db::sql::SqlException*)e;
      
      cout << "SqlException occurred!" << endl;
      cout << "message: " << dbe->getMessage() << endl;
      cout << "type: " << dbe->getType() << endl;
      cout << "code: " << dbe->getCode() << endl;
      cout << "sqlstate: " << dbe->getSqlState() << endl;
   }
   else
   {
      cout << "Exception occurred!" << endl;
      cout << "message: " << e->getMessage() << endl;
      cout << "type: " << e->getType() << endl;
      cout << "code: " << e->getCode() << endl;
      if(e->getCause() != NULL)
      {
         cout << "CAUSE:" << endl;
         cout << "message: " << e->getCause()->getMessage() << endl;
         cout << "type: " << e->getCause()->getType() << endl;
         cout << "code: " << e->getCause()->getCode() << endl;
      }
   }
}

void db::test::assertNoException()
{
   if(Exception::hasLast())
   {
      Exception* e = Exception::getLast();
      dumpException(e);
      assert(!Exception::hasLast());
   }
}


