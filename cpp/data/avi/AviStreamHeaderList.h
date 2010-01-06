/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_data_avi_AviStreamHeaderList_H
#define monarch_data_avi_AviStreamHeaderList_H

#include "monarch/data/avi/AviStreamFormat.h"
#include "monarch/data/avi/AviStreamHeader.h"
#include "monarch/data/riff/RiffListHeader.h"
#include "monarch/io/OutputStream.h"

namespace monarch
{
namespace data
{
namespace avi
{

/**
 * An AVI Stream Header List ('strl').
 *
 * AVI Format is as follows:
 *
 * AVI Form Header ('RIFF' size 'AVI ' data)
 *    Header List ('LIST' size 'hdrl' data)
 *       AVI Header ('avih' size data)
 *       Video Stream Header List ('LIST' size 'strl' data)
 *          Video Stream Header ('strh' size data)
 *          Video Stream Format ('strf' size data)
 *          Video Stream Data ('strd' size data) - IGNORED, for DRIVERS
 *       Audio Stream Header List ('LIST' size 'strl' data)
 *          Audio Stream Header ('strh' size data)
 *          Audio Stream Format ('strf' size data)
 *          Audio Stream Data ('strd' size data) - IGNORED, for DRIVERS
 *    Info List ('LIST' size 'INFO' data)
 *       Index Entry ({'ISBJ','IART','ICMT',...} size data)
 *    Movie List ('LIST' size 'movi' data)
 *       Movie Entry ({'00db','00dc','01wb'} size data)
 *    Index Chunk ('idx1' size data)
 *       Index Entry ({'00db','00dc','01wb',...})
 *
 * The Stream Header List 'strl' applies to the first stream in the 'movi'
 * LIST, the second applies to the second stream, etc.
 *
 * @author Dave Longley
 * @author David I. Lehn
 */
class AviStreamHeaderList
{
public:
   /**
    * Chunk fourcc id "strl".
    */
   static const fourcc_t CHUNK_ID = MO_FOURCC_FROM_CHARS('s','t','r','l');

protected:
   /**
    * The AVI stream header RIFF header.
    */
   monarch::data::riff::RiffListHeader mRiffHeader;

   /**
    * The Stream Header.
    */
   AviStreamHeader* mStreamHeader;

   /**
    * The Stream Format.
    */
   AviStreamFormat* mStreamFormat;

public:
   /**
    * Constructs a new AviStreamHeaderList.
    */
   AviStreamHeaderList();

   /**
    * Destructs a AviStreamHeaderList.
    */
   virtual ~AviStreamHeaderList();

   /**
    * Writes this AviStreamHeaderList, including the RIFF header, to an
    * OutputStream.
    *
    * @param os the OutputStream to write to.
    *
    * @exception true on success, false on an Exception.
    */
   virtual bool writeTo(monarch::io::OutputStream& os);

   /**
    * Converts this AviStreamHeaderList from a byte array.
    *
    * @param b the byte array to convert from.
    * @param length the number of valid bytes in the buffer.
    *
    * @return true if successful, false if not.
    */
   virtual bool convertFromBytes(const char* b, int length);

   /**
    * Returns whether or not this AviStreamHeaderList is valid.
    *
    * @return true if valid, false if not.
    */
   virtual bool isValid();

   /**
    * Gets the size of this AviStreamHeaderList, excluding its chunk header.
    *
    * @return the size of this AviStreamHeaderList chunk.
    */
   virtual int getListSize();

   /**
    * Gets the size of this AviStreamHeaderList, including its chunk header.
    *
    * @return the size of this AviStreamHeaderList.
    */
   virtual int getSize();

   /**
    * Sets the stream header for this list. The header will be deleted on
    * destruction or when a new header is assigned.
    *
    * @param h the heap-allocated stream header for this list.
    */
   virtual void setStreamHeader(AviStreamHeader* h);

   /**
    * Gets the stream header for this list.
    *
    * @return the stream header for this list.
    */
   virtual AviStreamHeader* getStreamHeader();

   /**
    * Sets the stream format for this list. The format will be deleted on
    * destruction or when a new format is assigned.
    *
    * @param f the heap-allocated stream format for this list.
    */
   virtual void setStreamFormat(AviStreamFormat* f);

   /**
    * Gets the stream format for this list.
    *
    * @return the stream format for this list.
    */
   virtual AviStreamFormat* getStreamFormat();
};

} // end namespace avi
} // end namespace data
} // end namespace monarch
#endif
