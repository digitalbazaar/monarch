/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_DynamicObjectReader_H
#define db_data_DynamicObjectReader_H

#include "db/rt/DynamicObject.h"
#include "db/io/InputStream.h"

namespace db
{
namespace data
{

/**
 * A DynamicObjectReader reads a DynamicObject from an InputStream.
 * 
 * @author Dave Longley
 */
class DynamicObjectReader
{
public:
   /**
    * Creates a new DynamicObjectReader.
    */
   DynamicObjectReader() {};
   
   /**
    * Destructs this DynamicObjectReader.
    */
   virtual ~DynamicObjectReader() {};
   
   /**
    * Starts deserializing an object. This reader can be re-used by calling
    * start() with the same or a new object. Calling start() before a previous
    * deserialization has finished will abort the previous state.
    * 
    * Using a non-empty object can be used to merge in new values. This is
    * only defined for similar object types (i.e., merging an array into
    * a map will overwrite the map).
    * 
    * @param dyno the DynamicObject for the object to deserialize.
    */
   virtual void start(db::rt::DynamicObject& dyno) = 0;
   
   /**
    * This method reads from the passed InputStream until the end of the
    * stream, blocking if necessary.
    * 
    * The start() method must be called at least once before calling read(). As
    * the data is read, the DynamicObject provided in start() is used to
    * deserialize an object.
    * 
    * This method may be called multiple times if the input stream needs to
    * be populated in between calls or if multiple input streams are used.
    * 
    * The object is built incrementally and on error will be partially built.
    * 
    * The finish() method must be called to complete the deserialization.
    * 
    * @param is the InputStream to read the data from.
    * 
    * @return true if the read succeeded, false if an Exception occurred.
    */
   virtual bool read(db::io::InputStream* is) = 0;
   
   /**
    * Finishes deserializing an object. This method must be called to
    * complete deserialization.
    * 
    * @return true if the finish succeeded, false if an Exception occurred.
    */
   virtual bool finish() = 0;
};

} // end namespace data
} // end namespace db
#endif
