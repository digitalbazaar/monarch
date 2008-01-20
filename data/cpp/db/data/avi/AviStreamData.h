/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_avi_AviStreamData_H
#define db_data_avi_AviStreamData_H

#include "db/data/riff/RiffChunkHeader.h"
#include "db/io/OutputStream.h"

namespace db
{
namespace data
{
namespace avi
{

/**
 * An AVI Stream Data ('strd').
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
 * @author Dave Longley
 * @author David I. Lehn
 */
class AviStreamData
{
public:
   /**
    * Chunk fourcc id "strd".
    */
   static const fourcc_t CHUNK_ID = DB_FOURCC_FROM_CHARS('s','t','r','d');
   
protected:
   /**
    * The AVI stream data RIFF header.
    */
   db::data::riff::RiffChunkHeader mRiffHeader;
   
   /**
    * The data for this chunk, not including its header.
    */
   char* mData;
   
public:
   /**
    * Constructs a new AviStreamData.
    */
   AviStreamData();
   
   /**
    * Destructs a AviStreamData.
    */
   virtual ~AviStreamData();
   
   /**
    * Writes this AviStreamData, including the RIFF header, to an
    * OutputStream.
    * 
    * @param os the OutputStream to write to.
    * 
    * @exception true on success, false on an IOException.
    */
   virtual bool writeTo(db::io::OutputStream& os);
   
   /**
    * Converts this AviHeader from a byte array.
    * 
    * @param b the byte array to convert from.
    * @param length the number of valid bytes in the buffer.
    * 
    * @return true if successful, false if not.
    */
   virtual bool convertFromBytes(const char* b, int length);
   
   /**
    * Returns whether or not this AviStreamData is valid.
    * 
    * @return true if valid, false if not.
    */
   virtual bool isValid();
   
   /**
    * Gets the size of this AviStreamData, excluding its chunk header.
    * 
    * @return the size of this AviStreamData chunk.
    */
   virtual int getChunkSize();
   
   /**
    * Gets the size of this AviStreamData, including its chunk header.
    * 
    * @return the size of this AviStreamData.
    */
   virtual int getSize();
};

} // end namespace avi
} // end namespace data
} // end namespace db
#endif
