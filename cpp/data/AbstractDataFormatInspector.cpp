/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_LIMIT_MACROS

#include "monarch/data/AbstractDataFormatInspector.h"

#include "monarch/util/Math.h"

using namespace monarch::data;
using namespace monarch::util;

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

void AbstractDataFormatInspector::setSkipBytes(int64_t count)
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
         int sb = (mSkipBytes > (uint64_t)INT32_MAX ? INT32_MAX : mSkipBytes);
         rval = (sb < length ? sb : length);
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

uint64_t AbstractDataFormatInspector::getBytesInspected()
{
   return mBytesInspected;
}
