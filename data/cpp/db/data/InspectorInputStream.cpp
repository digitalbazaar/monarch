/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/data/InspectorInputStream.h"

using namespace std;
using namespace db::io;
using namespace db::data;

InspectorInputStream::InspectorInputStream(InputStream* is, bool cleanup) :
   FilterInputStream(is, cleanup),
   mReadBuffer(2048)
{
   // no bytes inspected and made available yet
   mAvailableBytes = 0;
}

InspectorInputStream::~InspectorInputStream()
{
   // clean up meta-data
   for(InspectorMap::iterator i = mInspectors.begin();
       i != mInspectors.end(); i++)
   {
      // clean up name
      free((char*)i->first);
      
      // clean up inspector if appropriate
      if(i->second.cleanupInspector)
      {
         delete i->second.inspector;
      }
   }
}

int InspectorInputStream::read(char* b, int length)
{
   int rval = 0;
   
   // if the read buffer is empty, populate it
   if(mReadBuffer.isEmpty())
   {
      mReadBuffer.put(mInputStream);
   }
   
   // if no bytes are available, run inspectors to release inspected bytes
   if(mAvailableBytes == 0)
   {
      // add all inspector meta-data to the waiting list
      for(InspectorMap::iterator i = mInspectors.begin();
          i != mInspectors.end(); i++)
      {
         // add meta-data to list
         mWaiting.push_back(&i->second);
      }      
      
      // keep inspecting while inspectors are waiting and not end of stream
      bool eos = false;
      int uninspected, inspected;
      while(!mWaiting.empty() && !mReadBuffer.isEmpty())
      {
         // run waiting inspectors
         for(InspectorList::iterator i = mWaiting.begin(); i != mWaiting.end();)
         {
            // get next meta data
            DataInspectorMetaData* metaData = *i;
            
            // determine the number of uninspected bytes
            uninspected = mReadBuffer.length() - metaData->inspectedBytes;
            if(uninspected > 0)
            {
               // inspect data using inspector
               inspected = metaData->inspector->inspectData(
                  mReadBuffer.data() + metaData->inspectedBytes, uninspected);
               
               // see if any data was inspected
               if(inspected > 0)
               {
                  // update number of inspected bytes, remove from wait list
                  metaData->inspectedBytes += inspected;
                  i = mWaiting.erase(i);
               }
               else
               {
                  i++;
               }
            }
            else
            {
               // remove inspector from list, all current data inspected
               i = mWaiting.erase(i);
            }
         }
         
         // remove all waiting inspectors if the read buffer is full or eos
         if(mReadBuffer.isFull() || eos)
         {
            mWaiting.clear();
         }
         else if(!mWaiting.empty())
         {
            // read more data into the read buffer
            if(mReadBuffer.put(mInputStream) == 0)
            {
               // end of stream
               eos = true;
            }
         }
      }
   }
   
   // reset the number of available bytes to the maximum
   mAvailableBytes = mReadBuffer.length();
   for(InspectorMap::iterator i = mInspectors.begin();
       i != mInspectors.end(); i++)
   {
      if(i->second.inspectedBytes > 0)
      {
         // update the amount of available bytes
         mAvailableBytes = (mAvailableBytes < i->second.inspectedBytes) ?
            mAvailableBytes : i->second.inspectedBytes;
      }
   }
   
   // if bytes are available, release them
   if(mAvailableBytes > 0)
   {
      // pull bytes from the read buffer
      rval = mReadBuffer.get(
         b, (length < mAvailableBytes) ? length : mAvailableBytes);
      mAvailableBytes -= rval;
      
      // update the number of inspected bytes in each inspector
      int count;
      for(InspectorMap::iterator i = mInspectors.begin();
          i != mInspectors.end(); i++)
      {
         count = i->second.inspectedBytes - rval;
         i->second.inspectedBytes = (count < 0) ? 0 : count;
      }
   }
   
   return rval;
}

bool InspectorInputStream::addInspector(
   const char* name, DataInspector* di, bool cleanup)
{
   bool rval = false;
   
   // find existing meta-data
   InspectorMap::iterator i = mInspectors.find(name);
   if(i == mInspectors.end())
   {
      // create meta-data
      DataInspectorMetaData metaData;
      metaData.inspector = di;
      metaData.inspectedBytes = 0;
      metaData.cleanupInspector = cleanup;
      
      // store meta-data in map
      mInspectors[strdup(name)] = metaData;
      
      rval = true;
   }
   
   return rval;
}

bool InspectorInputStream::removeInspector(const char* name)
{
   bool rval = false;
   
   // find existing meta-data
   InspectorMap::iterator i = mInspectors.find(name);
   if(i != mInspectors.end())
   {
      // clean up name
      free((char*)i->first);
      
      // clean up inspector if appropriate
      if(i->second.cleanupInspector)
      {
         delete i->second.inspector;
      }
      
      mInspectors.erase(i);
      rval = true;
   }
   
   return rval;
}

DataInspector* InspectorInputStream::getInspector(const char* name)
{
   DataInspector* rval = NULL;
   
   // find existing meta-data
   InspectorMap::iterator i = mInspectors.find(name);
   if(i != mInspectors.end())
   {
      rval = i->second.inspector;
   }
   
   return rval;
}

void InspectorInputStream::getInspectors(list<DataInspector*>& inspectors)
{
   for(InspectorMap::iterator i = mInspectors.begin();
       i != mInspectors.end(); i++)
   {
      // add inspector to list
      inspectors.push_back(i->second.inspector);
   }
}
