/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_json_JsonOutputStream_H
#define db_data_json_JsonOutputStream_H

#include "db/io/OutputStream.h"
#include "db/io/ByteArrayInputStream.h"
#include "db/data/json/JsonReader.h"

namespace db
{
namespace data
{
namespace json
{

/**
 * An JsonOutputStream is used to write a DynamicObject to JSON.
 *
 * @author David I. Lehn 
 * @author Dave Longley
 */
class JsonOutputStream : public db::io::OutputStream
{
protected:
   /**
    * The JsonReader to parse the data with.
    */
   JsonReader mReader;
   
   /**
    * A ByteArrayInputStream for reading the output to this stream.
    */
   db::io::ByteArrayInputStream mInputStream;
   
public:
   /**
    * Creates a new JsonOutputStream that writes to the passed
    * DataBinding.
    * 
    * @param db the DataBinding to write to.
    */
   JsonOutputStream(db::util::DynamicObject dyno);
   
   /**
    * Destructs this JsonOutputStream.
    */
   virtual ~JsonOutputStream();
   
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

} // end namespace json
} // end namespace data
} // end namespace db
#endif
