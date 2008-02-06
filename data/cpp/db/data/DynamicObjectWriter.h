/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_DynamicObjectWriter_H
#define db_data_DynamicObjectWriter_H

#include "db/io/OutputStream.h"
#include "db/rt/DynamicObject.h"

namespace db
{
namespace data
{

/**
 * A DynamicObjectWriter writes a DynamicObject to an OutputStream in a
 * particular data format.
 * 
 * @author Dave Longley
 */
class DynamicObjectWriter
{
public:
   /**
    * Creates a new DynamicObjectWriter.
    */
   DynamicObjectWriter() {};
   
   /**
    * Destructs this DynamicObjectWriter.
    */
   virtual ~DynamicObjectWriter() {};
   
   /**
    * Serializes the passed DynamicObject.
    * 
    * @param dyno the DynamicObject to serialize.
    * @param os the OutputStream to write the serialized data to.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool write(
      db::rt::DynamicObject& dyno, db::io::OutputStream* os) = 0;
   
   /**
    * Sets the starting indentation level and the number of spaces
    * per indentation level.
    * 
    * @param level the starting indentation level.
    * @param spaces the number of spaces per indentation level.
    */
   virtual void setIndentation(int level, int spaces) = 0;
   
   /**
    * Sets option to minimize whitespace.
    * 
    * @param compact minimize whitespace.
    */
   virtual void setCompact(bool compact) = 0;
};

} // end namespace data
} // end namespace db
#endif
