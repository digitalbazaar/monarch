/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/net/Datagram.h"

#include <cstdlib>
#include <cstring>

using namespace std;
using namespace monarch::io;
using namespace monarch::net;

Datagram::Datagram(int capacity) :
   mAddress(NULL),
   mBuffer(capacity)
{
}

Datagram::Datagram(InternetAddressRef& address, int capacity) :
   mAddress(address),
   mBuffer(capacity)
{
}

Datagram::~Datagram()
{
}

void Datagram::setAddress(InternetAddressRef& address)
{
   mAddress = address;
}

InternetAddressRef& Datagram::getAddress()
{
   return mAddress;
}

ByteBuffer* Datagram::getBuffer()
{
   return &mBuffer;
}

void Datagram::assignString(const string& str)
{
   mBuffer.reAllocate(str.length());
   mBuffer.put(str.c_str(), str.length(), false);
}

string Datagram::getString()
{
   return (mBuffer.length() > 0) ?
      string(mBuffer.data(), mBuffer.length()) : "";
}
