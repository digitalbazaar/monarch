/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_data_json_JsonWriter_H
#define monarch_data_json_JsonWriter_H

#include "monarch/data/DynamicObjectWriter.h"

#include <ostream>

namespace monarch
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
    * True if JSON must start with an object or array.
    */
   bool mStrict;

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
   virtual bool writeIndentation(monarch::io::OutputStream* os, int level);

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
      monarch::rt::DynamicObject& dyno, monarch::io::OutputStream* os, int level);

public:
   /**
    * Creates a new JsonWriter.
    *
    * In strict mode the JSON stream must start with an object or array.  In
    * non-strict mode any valid JSON value can be serialized.
    *
    * @param strict the JSON stream must start with an object or array.
    */
   JsonWriter(bool strict = true);

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
      monarch::rt::DynamicObject& dyno, monarch::io::OutputStream* os);

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

   /**
    * Writes a DynamicObject as JSON to an ostream.
    *
    * @param dyno the DynamicObject to write out.
    * @param stream the ostream to write to.
    * @param compact true to use compact syntax, false not to.
    * @param strict the JSON stream must start with an object or array.
    *
    * @return true on success, false with exception set on failure.
    */
   static bool writeToOStream(
      monarch::rt::DynamicObject dyno, std::ostream& stream, bool compact = false,
      bool strict = true);

   /**
    * Writes a DynamicObject as JSON to a string.
    *
    * @param dyno the DynamicObject to write out.
    * @param compact true to use compact syntax, false not to.
    * @param strict the JSON stream must start with an object or array.
    *
    * @return the string with JSON data on success, a blank string with
    *         exception set on failure.
    */
   static std::string writeToString(
      monarch::rt::DynamicObject dyno, bool compact = false, bool strict = true);

   /**
    * Writes a DynamicObject as JSON to standard out.
    *
    * @param dyno the DynamicObject to write out.
    * @param compact true to use compact syntax, false not to.
    * @param strict the JSON stream must start with an object or array.
    *
    * @return true on success, false with exception set on failure.
    */
   static bool writeToStdOut(
      monarch::rt::DynamicObject dyno, bool compact = false, bool strict = true);
};

} // end namespace json
} // end namespace data
} // end namespace monarch
#endif
