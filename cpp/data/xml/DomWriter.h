/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_data_xml_DomWriter_H
#define db_data_xml_DomWriter_H

#include "monarch/data/xml/XmlWriter.h"
#include "monarch/data/xml/DomTypes.h"

namespace db
{
namespace data
{
namespace xml
{

/**
 * An DomWriter provides an interface for serializing a
 * DOM (Document Object Model) to XML (eXtensible Markup Language).
 *
 * A DomWriter writes out a whole object at once and can be used again.
 * The compact setting should be used to minimize extra whitespace when not
 * needed.
 *
 * @author Dave Longley
 */
class DomWriter : public XmlWriter
{
protected:
   /**
    * Calls writeWithNamespaceSupport() with a new, empty, namespace prefix map.
    *
    * @param e the Element to serialize.
    * @param os the OutputStream to write the XML to.
    * @param level current level of indentation (-1 to initialize with default).
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool write(Element& e, db::io::OutputStream* os, int level);

   /**
    * Recursively serializes the passed DynamicObject to XML.
    *
    * @param e the Element to serialize.
    * @param os the OutputStream to write the XML to.
    * @param level current level of indentation (-1 to initialize with default).
    * @param nsPrefixMap a namespace URI to prefix map that adds new entries
    *                    as they are found in each element.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool writeWithNamespaceSupport(
      Element& dyno, db::io::OutputStream* os, int level,
      db::rt::DynamicObject& nsPrefixMap);

public:
   /**
    * Creates a new DomWriter.
    */
   DomWriter();

   /**
    * Destructs this DomWriter.
    */
   virtual ~DomWriter();

   /**
    * Serializes the passed Document (by its root Element) to XML.
    *
    * @param e the Document's root Element to serialize.
    * @param os the OutputStream to write the XML to.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool write(
      db::rt::DynamicObject& root, db::io::OutputStream* os);
};

} // end namespace xml
} // end namespace data
} // end namespace db
#endif
