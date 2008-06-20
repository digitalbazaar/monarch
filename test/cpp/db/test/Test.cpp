/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/test/Test.h"

#include <iostream>
#include <sstream>

#include "db/sql/SqlException.h"
#include "db/data/json/JsonWriter.h"
#include "db/io/OStreamOutputStream.h"

using namespace std;
using namespace db::data::json;
using namespace db::io;
using namespace db::rt;
using namespace db::test;

bool db::test::dumpException(ExceptionRef& e)
{
   bool rval;
   
   db::sql::SqlException* sqlEx = NULL;
   if(!e.isNull())
   {
      sqlEx = dynamic_cast<db::sql::SqlException*>(&(*e));
   }
   
   if(sqlEx != NULL)
   {
      cout << "SqlException occurred!" << endl;
      cout << "sqlstate: " << sqlEx->getSqlState() << endl;
   }
   else
   {
      cout << "Exception occurred!" << endl;
   }
   
   cout << "message: " << e->getMessage() << endl;
   cout << "type: " << e->getType() << endl;
   cout << "code: " << e->getCode() << endl;
   cout << "details: ";
   rval = dynamicObjectToStream(e->getDetails(), cout, false);
   cout << endl;         

   if(rval && !e.isNull() && !e->getCause().isNull())
   {
      cout << "CAUSE:" << endl;
      rval = dumpException(e->getCause());         
   }
   
   return rval;
}

bool db::test::dumpException()
{
   bool rval = true;
   
   if(Exception::hasLast())
   {
      ExceptionRef e = Exception::getLast();
      rval = dumpException(e);
   }
   
   return rval;
}

void db::test::dumpDynamicObjectText_(
   DynamicObject& dyno, DynamicObjectIterator doi, int indent)
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

void db::test::dumpDynamicObjectText(DynamicObject& dyno)
{
   dumpDynamicObjectText_(dyno, NULL, 0);
}

bool db::test::dynamicObjectToStream(
   DynamicObject& dyno, ostream& stream, bool compact)
{
   bool rval;
   
   OStreamOutputStream os(&stream);
   JsonWriter jw;
   jw.setCompact(compact);
   if(!compact)
   {
      jw.setIndentation(0, 3);
   }
   rval = jw.write(dyno, &os);
   
   return rval;
}

bool db::test::dynamicObjectToString(
   DynamicObject& dyno, string& str, bool compact)
{
   bool rval;
   
   ostringstream oss;
   if((rval = dynamicObjectToStream(dyno, oss, compact)))
   {
      str = oss.str();
   }
   else
   {
      str.clear();
   }
   
   return rval;
}

bool db::test::dumpDynamicObject(DynamicObject& dyno, bool compact)
{
   bool rval;
   
   rval = dynamicObjectToStream(dyno, cout, compact);
   cout << endl;
   cout.flush();
   
   return rval;
}
