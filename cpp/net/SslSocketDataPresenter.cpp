/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/net/SslSocketDataPresenter.h"

#include "monarch/net/SslSocket.h"
#include "monarch/net/SocketDefinitions.h"

using namespace db::net;
using namespace db::rt;

SslSocketDataPresenter::SslSocketDataPresenter(SslContext* context)
{
   mContext = context;
}

SslSocketDataPresenter::~SslSocketDataPresenter()
{
}

bool SslSocketDataPresenter::detectSsl(Socket* s)
{
   bool rval = false;

   // save old receive timeout, set new one at 3 seconds
   uint32_t recvTimeout = s->getReceiveTimeout();
   s->setReceiveTimeout(1000 * 3);

   // at least 5 bytes are needed to detect an SSL/TLS packet
   char b[5];
   int numBytes = 0;

   // peek 5 bytes
   int count = 5;
   while(count > 0 &&
         (numBytes = s->getInputStream()->peek(b + (5 - count), count)) > 0)
   {
      count -= numBytes;
   }

   if(numBytes < 0)
   {
      // clear socket timeout exceptions
      ExceptionRef e = Exception::get();
      if(e->isType(SOCKET_TIMEOUT_EXCEPTION_TYPE))
      {
         Exception::clear();
      }
   }

   // reset old receive timeout
   s->setReceiveTimeout(recvTimeout);

   if(count == 0)
   {
      // check SSL record type
      // check for handshake (value of 22 == 0x16)
      if(b[0] == 0x16)
      {
         // offset + 1 is major version
         // offset + 2 is minor version
         // offset + 3, 4 is a short that is the length of
         // data in the record excluding the header (max of 16384)

         // SSL 3.0 / TLS 1.0 both have major version 3.0
         if(b[1] == 0x03)
         {
            // TLS v1.0 connection detected
            rval = true;
         }
      }

      // not TLS 1.0, may be SSL 2.0/3.0
      if(!rval)
      {
         // EXPECT SSL 2.0/3.0:
         // * means optional, escape and padding only exist if
         // the most significant bit is set for record-length
         //
         // FORMAT:
         // HEADER {record-length, *is-escape-record?, *padding}
         // MAC-DATA[MAC_SIZE]
         // ACTUAL-DATA[N]
         // PADDING-DATA[PADDING]

         // for storing when the actual data starts
         unsigned int actualDataOffset = 0;

         // for storing record length
         unsigned int recordLength = 0;

         // see if the most significant bit is set in the first byte
         if((b[0] & 0x80) != 0)
         {
            // total header length is 2 bytes
            actualDataOffset = 2;

            // get the record length
            recordLength = ((b[0] & 0x7F) << 8) | b[1];
         }
         else
         {
            // total header length is 3 bytes
            actualDataOffset = 3;

            // get the record length
            recordLength = ((b[0] & 0x3F) << 8) | b[1];
         }

         // get the client-hello
         unsigned int clientHello = b[actualDataOffset];

         // get version
         unsigned int version = b[actualDataOffset + 1];
         if(version == 0)
         {
            version = b[actualDataOffset + 2];
         }

         // ensure that we have a client hello
         if(clientHello == 1)
         {
            // check for version 2 or 3
            if(version == 2 || version == 3)
            {
               rval = true;
            }
         }
      }
   }

   return rval;
}

Socket* SslSocketDataPresenter::createPresentationWrapper(
   Socket* s, bool& secure)
{
   SslSocket* rval = NULL;

   // detect SSL record
   if(detectSsl(s))
   {
      // create an SSL socket, (false = use server mode, true = cleanup)
      rval = new SslSocket(mContext, (TcpSocket*)s, false, true);
      secure = true;
   }

   return rval;
}
