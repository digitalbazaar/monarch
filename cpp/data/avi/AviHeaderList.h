/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_data_avi_AviHeaderList_H
#define monarch_data_avi_AviHeaderList_H

#include "monarch/data/avi/AviHeader.h"
#include "monarch/data/avi/AviStreamHeaderList.h"
#include "monarch/data/riff/RiffListHeader.h"
#include "monarch/io/OutputStream.h"
#include "monarch/logging/Logging.h"

#include <vector>

namespace monarch
{
namespace data
{
namespace avi
{

/**
 * An AVI Header List. This list contains an AVI main header and stream
 * header lists.
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
class AviHeaderList
{
public:
   /**
    * Chunk fourcc id "hrdl".
    */
   static const fourcc_t CHUNK_ID = MO_FOURCC_FROM_CHARS('h','d','r','l');

protected:
   /**
    * The Header List RIFF header.
    */
   monarch::data::riff::RiffListHeader mRiffHeader;

   /**
    * The main AviHeader for this header list.
    */
   AviHeader mMainHeader;

   /**
    * The AviStreamHeaderLists in this header list.
    */
   typedef std::vector<AviStreamHeaderList*> StreamHeaderLists;
   StreamHeaderLists mStreamHeaderLists;

public:
   /**
    * Constructs a new AviHeaderList.
    */
   AviHeaderList();

   /**
    * Destructs a AviHeaderList.
    */
   virtual ~AviHeaderList();

   /**
    * Writes this AviHeaderList, including the RIFF header, to an OutputStream.
    *
    * @param os the OutputStream to write to.
    *
    * @return true on success, false on an Exception.
    */
   virtual bool writeTo(monarch::io::OutputStream& os);

   /**
    * Converts from a byte array with at least enough bytes for the chunk
    * header and the main AviHeader. If more bytes are available, then
    * an attempt will be made to parse AviStreamHeaderLists.
    *
    * @param b the byte array to convert from.
    * @param length the number of valid bytes in the buffer.
    *
    * @return true if successful, false if not.
    */
   virtual bool convertFromBytes(const char* b, int length);

   /**
    * Returns whether or not this AviHeaderList is valid.
    *
    * @return true if valid, false if not.
    */
   virtual bool isValid();

   /**
    * Gets the size of this AviHeaderList, including its RIFF header.
    *
    * @return the size of this AviHeaderList.
    */
   virtual int getSize();

   /**
    * Gets the main AviHeader.
    *
    * @return the main AviHeader.
    */
   virtual AviHeader& getMainHeader();

   /**
    * Gets the list of AviStreamHeaderLists.
    *
    * @return the list of AviStreamHeaderLists.
    */
   std::vector<AviStreamHeaderList*>& getStreamHeaderLists();

protected:
   /**
    * Frees AviStreamHeaderLists.
    */
   virtual void freeStreamHeaderLists();
};

} // end namespace avi
} // end namespace data
} // end namespace monarch
#endif
