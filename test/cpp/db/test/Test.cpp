/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */

#include <assert.h>
#include <iostream>

#include "db/test/Test.h"
#include "db/rt/Exception.h"
#include "db/sql/SqlException.h"
#include "db/data/json/JsonWriter.h"
#include "db/io/OStreamOutputStream.h"

using namespace std;
using namespace db::data::json;
using namespace db::io;
using namespace db::rt;
using namespace db::test;
using namespace db::util;

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

void db::test::dumpDynamicObjectText_(
   DynamicObject dyno, DynamicObjectIterator doi, int indent)
{
   for(int i = 0; i < indent; i++)
   {
      cout << ' ';
   }
   
   if(doi == NULL)
   {
      doi = dyno.getIterator();
   }
   
   switch(dyno->getType())
   {
      case String:
         cout << "Name=" << doi->getName() << ",Value=" << dyno->getString();
         cout << endl;
         break;
      case Boolean:
         cout << "Name=" << doi->getName() << ",Value=" << dyno->getBoolean();
         cout << endl;
         break;
      case Int32:
         cout << "Name=" << doi->getName() << ",Value=" << dyno->getInt32();
         cout << endl;
         break;
      case UInt32:
         cout << "Name=" << doi->getName() << ",Value=" << dyno->getUInt32();
         cout << endl;
         break;
      case Int64:
         cout << "Name=" << doi->getName() << ",Value=" << dyno->getInt64();
         cout << endl;
         break;
      case UInt64:
         cout << "Name=" << doi->getName() << ",Value=" << dyno->getUInt64();
         cout << endl;
         break;
      case Double:
         cout << "Name=" << doi->getName() << ",Value=" << dyno->getDouble();
         cout << endl;
         break;
      case Map:
      case Array:
         cout << "Name=" << doi->getName() << endl;
         DynamicObjectIterator i = dyno.getIterator();
         while(i->hasNext())
         {
            dumpDynamicObjectText_(i->next(), i, indent + 1);
         }
         break;
   }
}

void db::test::dumpDynamicObjectText(DynamicObject dyno)
{
   dumpDynamicObjectText_(dyno, NULL, 0);
}

void db::test::dumpDynamicObject_(DynamicObject dyno, bool compact)
{
   OStreamOutputStream os(&cout);
   JsonWriter jw;
   jw.setCompact(compact);
   if(!compact)
   {
      jw.setIndentation(0, 3);
   }
   jw.write(dyno, &os);
   cout.flush();
}

void db::test::dumpDynamicObject(DynamicObject dyno)
{
   dumpDynamicObject_(dyno, false);
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

void db::test::assertException()
{
   if(!Exception::hasLast())
   {
      Exception* e = new Exception("Expected exception");
      dumpException(e);
      assert(Exception::hasLast());
   }
}
