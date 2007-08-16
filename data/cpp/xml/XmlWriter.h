/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_xml_XmlWriter_H
#define db_data_xml_XmlWriter_H

#include "DataBinding.h"
#include "OutputStream.h"

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
 * @author Dave Longley
 */
class XmlWriter
{
protected:
   /**
    * Writes the beginning of a start element.
    * 
    * @param dn the DataName to use.
    * @param os the OutputStream to write the xml to.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool openStartElement(
      db::data::DataName* dn, db::io::OutputStream* os);
   
   /**
    * Writes the end of a start element.
    * 
    * @param empty true if the start element is empty, false if not.
    * @param os the OutputStream to write the xml to.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool closeStartElement(bool empty, db::io::OutputStream* os);
   
   /**
    * Writes an end element.
    * 
    * @param dn the DataName to use.
    * @param os the OutputStream to write the xml to.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool writeEndElement(
      db::data::DataName* dn, db::io::OutputStream* os);
   
   /**
    * Writes out an element with the given DataName and data to the given
    * output stream.
    * 
    * @param dn the DataName for the element.
    * @param data the data for the element.
    * @param os the OutputStream to write the xml to.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool writeElement(
      db::data::DataName* dn, const char* data, db::io::OutputStream* os);
   
   /**
    * Writes out an attribute with the given DataName and data to the given
    * output stream.
    * 
    * @param dn the DataName for the attribute.
    * @param data the data for the attribute.
    * @param os the OutputStream to write the xml to.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool writeAttribute(
      db::data::DataName* dn, const char* data, db::io::OutputStream* os);
   
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
    * Serializes an object to xml using the passed DataBinding.
    * 
    * @param db the DataBinding for the object to serialize.
    * @param os the OutputStream to write the xml to.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool write(
      db::data::DataBinding* db, db::io::OutputStream* os);
};

} // end namespace xml
} // end namespace data
} // end namespace db
#endif
