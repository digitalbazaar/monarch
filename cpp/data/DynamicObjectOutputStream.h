/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_data_DynamicObjectOutputStream_H
#define db_data_DynamicObjectOutputStream_H

#include "monarch/io/OutputStream.h"
#include "monarch/io/ByteArrayInputStream.h"
#include "monarch/data/DynamicObjectReader.h"

namespace db
{
namespace data
{

/**
 * A DynamicObjectOutputStream is used to deserialize data to a DynamicObject.
 *
 * It uses a DynamicObjectReader to parse the incoming data and write it out
 * to a DynamicObject.
 *
 * @author David I. Lehn
 * @author Dave Longley
 */
class DynamicObjectOutputStream : public monarch::io::OutputStream
{
protected:
   /**
    * The DynamicObjectReader to parse the data with.
    */
   DynamicObjectReader* mReader;

   /**
    * True to cleam up the reader when destructing, false not to.
    */
   bool mCleanupReader;

   /**
    * A ByteArrayInputStream for reading the output to this stream.
    */
   monarch::io::ByteArrayInputStream mInputStream;

public:
   /**
    * Creates a new DynamicObjectOutputStream that writes to the passed
    * DynamicObject using the passed DynamicObjectReader to parse the
    * incoming data.
    *
    * @param dyno the DynamicObject to write to.
    * @param reader the DynamicObjectReader to parse the data with.
    * @param cleanup true to clean up the reader when destructing, false
    *                to leave it alone.
    */
   DynamicObjectOutputStream(
      monarch::rt::DynamicObject& dyno, DynamicObjectReader* reader, bool cleanup);

   /**
    * Destructs this DynamicObjectOutputStream.
    */
   virtual ~DynamicObjectOutputStream();

   /**
    * Writes some bytes to the stream.
    *
    * @param b the array of bytes to write.
    * @param length the number of bytes to write to the stream.
    *
    * @return true if the write was successful, false if an IO exception
    *         occurred.
    */
   virtual bool write(const char* b, int length);

   /**
    * Closes the stream.
    */
   virtual void close();
};

} // end namespace data
} // end namespace db
#endif
