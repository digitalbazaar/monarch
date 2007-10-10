/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/data/AbstractDataFormatInspector.h"

using namespace db::data;

AbstractDataFormatInspector::AbstractDataFormatInspector()
{
   mDataSatisfied = false;
   mFormatRecognized = false;
   mInspectAfterSatisfied = false;
   mBytesInspected = 0;
   mSkipBytes = 0;
}

AbstractDataFormatInspector::~AbstractDataFormatInspector()
{
}

void AbstractDataFormatInspector::setFormatRecognized(bool recognized)
{
   mFormatRecognized = recognized;
   mDataSatisfied = true;
}

void AbstractDataFormatInspector::setSkipBytes(long long count)
{
   mSkipBytes = count;
}

int AbstractDataFormatInspector::inspectData(const char* b, int length)
{
   int rval = 0;
   
   // inspect data if not satisfied or if keep-inspecting enabled
   if(!isDataSatisfied() || (keepInspecting() && isFormatRecognized()))
   {
      // skip bytes as appropriate
      if(mSkipBytes > 0)
      {
         rval = ((int)mSkipBytes < length) ? (int)mSkipBytes : length;
         mSkipBytes -= rval;
      }
      else
      {
         // try to detect format
         rval = detectFormat(b, length);
      }
      
      // increase number of inspected bytes
      mBytesInspected += rval;
   }
   else
   {
      // consider all data inspected
      rval = length;
   }
   
   return rval;
}

bool AbstractDataFormatInspector::isDataSatisfied()
{
   return mDataSatisfied;
}

bool AbstractDataFormatInspector::isFormatRecognized()
{
   return mFormatRecognized;
}

void AbstractDataFormatInspector::setKeepInspecting(bool inspect)
{
   mInspectAfterSatisfied = inspect;
}

bool AbstractDataFormatInspector::keepInspecting()
{
   return mInspectAfterSatisfied;
}

unsigned long long AbstractDataFormatInspector::getBytesInspected()
{
   return mBytesInspected;
}
