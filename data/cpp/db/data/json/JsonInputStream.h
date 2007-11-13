/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_json_JsonInputStream_H
#define db_data_json_JsonInputStream_H

#include "db/data/json/JsonWriter.h"
#include "db/io/ByteArrayInputStream.h"
#include "db/io/ByteBuffer.h"
#include "db/io/InputStream.h"
#include "db/util/DynamicObject.h"

namespace db
{
namespace data
{
namespace json
{

/**
 * A JsonInputStream is used to read data in JSON format from a DynamicObject.
 * 
 * @author David I. Lehn
 * @author Dave Longley
 */
class JsonInputStream : public db::io::InputStream
{
protected:
   /**
    * Flag if JSON written to buffer.
    */
   bool mHasJSON;

   /**
    * DynamicObject to read from.
    */
   db::util::DynamicObject mDyno;
   
   /**
    * An JsonWriter for writing out JSON.
    */
   JsonWriter mJsonWriter;
   
   /**
    * A buffer for storing temporary data to be read out.
    */
   db::io::ByteBuffer mBuffer;
   
   /**
    * A ByteArrayInputStream for reading JSON from the read buffer.
    */
   db::io::ByteArrayInputStream mBufferInputStream;
   
public:
   /**
    * Creates a new JsonInputStream.
    * 
    * @param dyno the DynamicObject to read from.
    */
   JsonInputStream(db::util::DynamicObject dyno);
   
   /**
    * Destructs this JsonInputStream.
    */
   virtual ~JsonInputStream();
   
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
    * Sets the JSON writer to use compact mode and not output unneeded
    * whitespace.
    * 
    * @param bool the compact mode.
    */
   virtual void setCompact(bool compact);

   /**
    * Sets the starting indentation level and the number of spaces
    * per indentation level.
    * 
    * @param level the starting indentation level.
    * @param spaces the number of spaces per indentation level.
    */
   virtual void setIndentation(int level, int spaces);
};

} // end namespace
} // end namespace net
} // end namespace db
#endif
