/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include <iostream>

#include "db/config/ConfigManager.h"
#include "db/data/json/JsonWriter.h"
#include "db/io/FileOutputStream.h"

using namespace std;
using namespace db::config;
using namespace db::data::json;
using namespace db::io;
using namespace db::rt;

int main()
{
   cout << "Testing core ConfigManager..." << endl;
   
   ConfigManager cm;
   
   if(Exception::hasLast())
   {
      ExceptionRef e = Exception::getLast();
      DynamicObject dyno = Exception::convertToDynamicObject(e);
      JsonWriter::writeToStdOut(dyno);
      exit(1);
   }
   
   return 0;
}
