/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_json_JsonWriter_H
#define db_data_json_JsonWriter_H

#include "db/data/DynamicObjectWriter.h"

namespace db
{
namespace data
{
namespace json
{

/**
 * A JsonWriter provides an interface for serializing objects to
 * JSON (JavaScript Object Notation) (RFC 4627).
 * 
 * A JsonWriter writes out a whole object at once and can be used again.
 * The compact setting should be used to minimize extra whitespace when not
 * needed.
 * 
 * @author David I. Lehn
 */
class JsonWriter : public DynamicObjectWriter
{
protected:
   /**
    * Compact mode to minimize whitespace.
    */
   bool mCompact;
   
   /**
    * The starting indentation level. 
    */
   int mIndentLevel;
   
   /**
    * The number of spaces per indentation level.
    */
   int mIndentSpaces;
   
   /**
    * Writes out indentation.  None if in compact mode.
    * 
    * @param os the OutputStream to write to.
    * @param level indentation level.
    * 
    * @return true if successful, false if an exception occurred. 
    */
   virtual bool writeIndentation(db::io::OutputStream* os, int level);
   
   /**
    * Recursively serializes an object to JSON using the passed DynamicObject.
    * 
    * @param dyno the DynamicObject to serialize.
    * @param os the OutputStream to write the JSON to.
    * @param level current level of indentation (-1 to initialize with default).
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool write(
      db::rt::DynamicObject& dyno, db::io::OutputStream* os, int level);
   
public:
   /**
    * Creates a new JsonWriter.
    */
   JsonWriter();
   
   /**
    * Destructs this JsonWriter.
    */
   virtual ~JsonWriter();
   
   /**
    * Serializes an object to JSON using the passed DynamicObject.
    * 
    * @param dyno the DynamicObject to serialize.
    * @param os the OutputStream to write the JSON to.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool write(
      db::rt::DynamicObject& dyno, db::io::OutputStream* os);
   
   /**
    * Sets the starting indentation level and the number of spaces
    * per indentation level.
    * 
    * @param level the starting indentation level.
    * @param spaces the number of spaces per indentation level.
    */
   virtual void setIndentation(int level, int spaces);

   /**
    * Sets the writer to use compact mode and not output unneeded whitespace.
    * 
    * @param compact true to minimize whitespace, false not to.
    */
   virtual void setCompact(bool compact);
};

} // end namespace json
} // end namespace data
} // end namespace db
#endif
