/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/net/Datagram.h"

#include <cstdlib>
#include <cstring>

using namespace std;
using namespace db::io;
using namespace db::net;

Datagram::Datagram(InternetAddress* address, int capacity) :
   mAddress(address),
   mAddressRef(NULL),
   mBuffer(capacity)
{
}

Datagram::Datagram(InternetAddressRef& address, int capacity) :
   mAddress(&(*address)),
   mAddressRef(address),
   mBuffer(capacity)
{
}

Datagram::~Datagram()
{
}

void Datagram::setAddress(InternetAddress* address)
{
   mAddress = address;
   mAddressRef.setNull();
}

void Datagram::setAddress(InternetAddressRef& address)
{
   mAddress = &(*address);
   mAddressRef = address;
}

InternetAddress* Datagram::getAddress()
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
