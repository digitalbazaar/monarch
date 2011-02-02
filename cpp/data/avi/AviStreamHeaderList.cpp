/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/data/avi/AviStreamHeaderList.h"

using namespace monarch::data;
using namespace monarch::data::avi;
using namespace monarch::data::riff;
using namespace monarch::io;

AviStreamHeaderList::AviStreamHeaderList() :
   mRiffHeader(CHUNK_ID),
   mStreamHeader(NULL),
   mStreamFormat(NULL)
{
}

AviStreamHeaderList::~AviStreamHeaderList()
{
}

bool AviStreamHeaderList::writeTo(OutputStream& os)
{
   bool rval;

   // write RIFF header
   // write stream header, if exists
   // write stream format, if exists
   rval =
      mRiffHeader.writeTo(os) &&
      (mStreamHeader == NULL || mStreamHeader->writeTo(os)) &&
      (mStreamFormat == NULL || mStreamFormat->writeTo(os));

   return rval;
}

bool AviStreamHeaderList::convertFromBytes(const char* b, int length)
{
   bool rval = false;

   // clean up stream header and format
   setStreamHeader(NULL);
   setStreamFormat(NULL);

   // convert the RIFF header
   if(mRiffHeader.convertFromBytes(b, length) &&
      mRiffHeader.getIdentifier() == CHUNK_ID)
   {
      // if there is enough data, convert the components
      // FIXME: this includes 'strd' ... which we ignore
      if(length < getSize())
      {
         // do not convert components
         rval = true;
      }
      else
      {
         // step forward past RIFF header
         b += RiffListHeader::HEADER_SIZE;

         // set length to list size
         length = (int)mRiffHeader.getListSize();

         // convert header
         setStreamHeader(new AviStreamHeader());
         if(mStreamHeader->convertFromBytes(b, length))
         {
            // step forward past header
            b += mStreamHeader->getSize();
            length -= mStreamHeader->getSize();

            // convert format
            setStreamFormat(new AviStreamFormat());
            if(mStreamFormat->convertFromBytes(b, length))
            {
               // header list converted, 'strd' is ignored
               rval = true;
            }
         }
      }
   }

   return rval;
}

bool AviStreamHeaderList::isValid()
{
   return mRiffHeader.isValid() && mRiffHeader.getIdentifier() == CHUNK_ID;
}

int AviStreamHeaderList::getListSize()
{
   // AVI stream header list is expected to be much smaller than 32-bits
   return (int)mRiffHeader.getListSize();
}

int AviStreamHeaderList::getSize()
{
   return getListSize() + RiffListHeader::HEADER_SIZE;
}

void AviStreamHeaderList::setStreamHeader(AviStreamHeader* h)
{
   delete mStreamHeader;
   mStreamHeader = h;
}

AviStreamHeader* AviStreamHeaderList::getStreamHeader()
{
   return mStreamHeader;
}

void AviStreamHeaderList::setStreamFormat(AviStreamFormat* f)
{
   delete mStreamFormat;
   mStreamFormat = f;
}

AviStreamFormat* AviStreamHeaderList::getStreamFormat()
{
   return mStreamFormat;
}
