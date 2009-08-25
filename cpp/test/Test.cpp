/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/test/Test.h"

// FIXME: replace iostream with cstdlib and printfs
#include <iostream>
#include <sstream>

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

   printf("EXCEPTION:\n");
   DynamicObject d = Exception::convertToDynamicObject(e);
   rval = JsonWriter::writeToStdOut(d);
   printf("\n");

   return rval;
}

bool db::test::dumpException()
{
   bool rval = true;

   if(Exception::isSet())
   {
      ExceptionRef e = Exception::get();
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

bool db::test::dynamicObjectToOStream(
   DynamicObject& dyno, ostream& stream, bool compact)
{
   return JsonWriter::writeToOStream(dyno, stream, compact);
}

bool db::test::dynamicObjectToString(
   DynamicObject& dyno, string& str, bool compact)
{
   str = JsonWriter::writeToString(dyno, compact);
   return str.length() > 0;
}

bool db::test::dumpDynamicObject(DynamicObject& dyno, bool compact)
{
   return JsonWriter::writeToStdOut(dyno, compact);
}
