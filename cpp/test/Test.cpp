/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_FORMAT_MACROS

#include "monarch/test/Test.h"

#include <cstdio>

#include "monarch/data/json/JsonWriter.h"
#include "monarch/io/OStreamOutputStream.h"

using namespace std;
using namespace monarch::data::json;
using namespace monarch::io;
using namespace monarch::rt;
using namespace monarch::test;

bool monarch::test::dumpException(ExceptionRef& e)
{
   bool rval;

   printf("EXCEPTION:\n");
   DynamicObject d = Exception::convertToDynamicObject(e);
   rval = JsonWriter::writeToStdOut(d);
   printf("\n");

   return rval;
}

bool monarch::test::dumpException()
{
   bool rval = true;

   if(Exception::isSet())
   {
      ExceptionRef e = Exception::get();
      rval = dumpException(e);
   }

   return rval;
}

void monarch::test::dumpDynamicObjectText_(
   DynamicObject& dyno, DynamicObjectIterator doi, int indent)
{
   for(int i = 0; i < indent; ++i)
   {
      printf(" ");
   }

   if(doi == NULL)
   {
      doi = dyno.getIterator();
   }

   switch(dyno->getType())
   {
      case String:
         printf("Name=%s,Value=%s\n", doi->getName(), dyno->getString());
         break;
      case Boolean:
         printf("Name=%s,Value=%d\n", doi->getName(), dyno->getBoolean());
         break;
      case Int32:
         printf("Name=%s,Value=%" PRIi32 "\n", doi->getName(), dyno->getInt32());
         break;
      case UInt32:
         printf("Name=%s,Value=%" PRIu32 "\n", doi->getName(), dyno->getUInt32());
         break;
      case Int64:
         printf("Name=%s,Value=%" PRIi64 "\n", doi->getName(), dyno->getInt64());
         break;
      case UInt64:
         printf("Name=%s,Value=%" PRIu64 "\n", doi->getName(), dyno->getUInt64());
         break;
      case Double:
         printf("Name=%s,Value=%g\n", doi->getName(), dyno->getDouble());
         break;
      case Map:
      case Array:
         printf("Name=%s\n", doi->getName());
         DynamicObjectIterator i = dyno.getIterator();
         while(i->hasNext())
         {
            dumpDynamicObjectText_(i->next(), i, indent + 1);
         }
         break;
   }
}

void monarch::test::dumpDynamicObjectText(DynamicObject& dyno)
{
   dumpDynamicObjectText_(dyno, NULL, 0);
}

bool monarch::test::dynamicObjectToOStream(
   DynamicObject& dyno, ostream& stream, bool compact)
{
   return JsonWriter::writeToOStream(dyno, stream, compact);
}

bool monarch::test::dynamicObjectToString(
   DynamicObject& dyno, string& str, bool compact)
{
   str = JsonWriter::writeToString(dyno, compact);
   return str.length() > 0;
}

bool monarch::test::dumpDynamicObject(DynamicObject& dyno, bool compact)
{
   return JsonWriter::writeToStdOut(dyno, compact);
}
