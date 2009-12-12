/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_data_xml_XmlWriter_H
#define db_data_xml_XmlWriter_H

#include "monarch/data/DynamicObjectWriter.h"

#include <string>

namespace db
{
namespace data
{
namespace xml
{

/**
 * An XmlWriter provides an interface for serializing objects to
 * XML (eXtensible Markup Language).
 *
 * A XmlWriter writes out a whole object at once and can be used again.
 * The compact setting should be used to minimize extra whitespace when not
 * needed.
 *
 * @author Dave Longley
 */
class XmlWriter : public DynamicObjectWriter
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
    * Xml-encodes the special characters in the passed data.
    *
    * @param data the data to xml-encode.
    *
    * @return the xml-encoded data.
    */
   virtual std::string encode(const char* data);

   /**
    * Writes out indentation. None if in compact mode.
    *
    * @param os the OutputStream to write to.
    * @param level indentation level.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool writeIndentation(db::io::OutputStream* os, int level);

   /**
    * Recursively serializes the passed DynamicObject to XML.
    *
    * @param dyno the DynamicObject to serialize.
    * @param os the OutputStream to write the XML to.
    * @param level current level of indentation (-1 to initialize with default).
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool write(
      db::rt::DynamicObject& dyno, db::io::OutputStream* os, int level);

public:
   /**
    * Creates a new XmlWriter.
    */
   XmlWriter();

   /**
    * Destructs this XmlWriter.
    */
   virtual ~XmlWriter();

   /**
    * Serializes the passed DynamicObject to XML.
    *
    * @param dyno the DynamicObject to serialize.
    * @param os the OutputStream to write the XML to.
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

} // end namespace xml
} // end namespace data
} // end namespace db
#endif
