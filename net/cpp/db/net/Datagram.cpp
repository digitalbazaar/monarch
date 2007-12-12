/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/Datagram.h"

using namespace std;
using namespace db::net;

Datagram::Datagram(InternetAddress* address, int length)
{
   // set address
   mAddress = address;
   
   if(length > 0)
   {
      mData = (char*)malloc(length);
      mLength = length;
   }
   else
   {
      // no data yet
      mData = NULL;
      mLength = 0;
   }
   
   // cleanup by default
   mCleanup = true;
}

Datagram::~Datagram()
{
   // free data
   freeData();
}

void Datagram::freeData()
{
   if(mCleanup && mData != NULL)
   {
      free(mData);
   }
   
   mData = NULL;
   mLength = 0;
}

void Datagram::setAddress(InternetAddress* address)
{
   mAddress = address;
}

InternetAddress* Datagram::getAddress()
{
   return mAddress;
}

void Datagram::setData(char* data, int length, bool cleanup)
{
   // free existing data
   freeData();
   
   // set data buffer
   mData = data;
   mLength = length;
   mCleanup = cleanup;
}

void Datagram::assignData(const char* data, int length)
{
   // overwrite existing data if possible
   if(mCleanup && mData != NULL && mLength >= length)
   {
      // copy data
      memcpy(mData, data, length);
   }
   else
   {
      // free existing data
      freeData();
      
      // copy data into a new buffer
      mData = (char*)malloc(length);
      memcpy(mData, data, length);
   }
   
   // set length and cleanup
   mLength = length;
   mCleanup = true;
}

void Datagram::setLength(int length)
{
   // cannot set larger than existing length
   mLength = (length > mLength) ? mLength : length;
}

char* Datagram::getData(int& length)
{
   // set length
   length = mLength;
   
   // return data
   return mData;
}

void Datagram::assignString(const string& str)
{
   assignData(str.c_str(), str.length());
}

string Datagram::getString()
{
   return string(mData, mLength);
}
