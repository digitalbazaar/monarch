/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Date.h"
#include "StringTools.h"

using namespace std;
using namespace db::util;

Date::Date()
{
}

Date::~Date()
{
}

string& Date::format(string& str, const string& format)
{
   string f = format;
   StringTools::replaceAll(f, "EEE", "%a");
   StringTools::replaceAll(f, "d", "%d");
   StringTools::replaceAll(f, "MMM", "%b");
   StringTools::replaceAll(f, "yyyy", "%Y");
   StringTools::replaceAll(f, "HH", "%H");
   StringTools::replaceAll(f, "hh", "%I");
   StringTools::replaceAll(f, "mm", "%M");
   StringTools::replaceAll(f, "ss", "%S");
   
   //gmtime(&mTimeData);
   
//   // add date
//   SimpleDateFormat df = new SimpleDateFormat("EEE, d MMM yyyy HH:mm:ss");
//   TimeZone tz = TimeZone.getTimeZone("GMT");
//   df.setTimeZone(tz);
}

Date& Date::parse(const string& str, const string& format)     
{
}
