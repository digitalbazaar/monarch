/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_data_DynamicObjectInputStream_H
#define db_data_DynamicObjectInputStream_H

#include "monarch/data/DynamicObjectWriter.h"
#include "monarch/io/ByteArrayInputStream.h"

namespace db
{
namespace data
{

/**
 * A DynamicObjectInputStream is used to read data from a DynamicObject
 * according to the data format output by a DynamicObjectWriter.
 *
 * @author David I. Lehn
 * @author Dave Longley
 */
class DynamicObjectInputStream : public db::io::InputStream
{
protected:
   /**
    * True if data has been written to the serialization buffer and is
    * ready to be read.
    */
   bool mHasData;

   /**
    * The DynamicObject to read from.
    */
   db::rt::DynamicObject mDyno;

   /**
    * The DynamicObjectWriter for writing out data.
    */
   DynamicObjectWriter* mWriter;

   /**
    * True to clean up the writer when destructing, false not to.
    */
   bool mCleanupWriter;

   /**
    * A buffer for storing temporary data to be read out.
    */
   db::io::ByteBuffer mBuffer;

   /**
    * A ByteArrayInputStream for reading data from the read buffer.
    */
   db::io::ByteArrayInputStream mBufferInputStream;

public:
   /**
    * Creates a new DynamicObjectInputStream.
    *
    * @param dyno the DynamicObject to read from.
    * @param writer the DynamicObjectWriter to produce data to read.
    * @param cleanup true to clean up the writer when destructing, false to
    *                leave it alone.
    */
   DynamicObjectInputStream(
      db::rt::DynamicObject& dyno, DynamicObjectWriter* writer, bool cleanup);

   /**
    * Destructs this DynamicObject.
    */
   virtual ~DynamicObjectInputStream();

   /**
    * Reads some bytes from the stream. This method will block until at least
    * one byte can be read or until the end of the stream is reached. A
    * value of 0 will be returned if the end of the stream has been reached,
    * a value of -1 will be returned if an IO exception occurred, otherwise
    * the number of bytes read will be returned.
    *
    * @param b the array of bytes to fill.
    * @param length the maximum number of bytes to read into the buffer.
    *
    * @return the number of bytes read from the stream or 0 if the end of the
    *         stream has been reached or -1 if an IO exception occurred.
    */
   virtual int read(char* b, int length);

   /**
    * Gets the DynamicObjectWriter in use by this stream.
    *
    * @return the DynamicObjectWriter in use by this stream.
    */
   virtual DynamicObjectWriter* getWriter();
};

} // end namespace data
} // end namespace db
#endif
