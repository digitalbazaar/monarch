/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_xml_XmlReader_H
#define db_data_xml_XmlReader_H

#include "DataBinding.h"
#include "InputStream.h"

#include <expat.h>

namespace db
{
namespace data
{
namespace xml
{

/**
 * An XmlReader provides an interface for deserializing objects from
 * XML (eXtensible Markup Language).
 * 
 * @author Dave Longley
 */
class XmlReader
{
protected:
   /**
    * The xml parser for this reader.
    */
   XML_Parser mParser;
   
   /**
    * Handles start elements.
    * 
    * @param xr the XmlReader that read in the element.
    * @param name the name of the element (namespace-uri|element-name).
    * @param attrs the attributes of the element.
    */
   static void startElement(
      void* xr, const XML_Char* name, const XML_Char** attrs);
   
   /**
    * Handles end elements.
    * 
    * @param xr the XmlReader that read in the element.
    * @param name the name of the element (namespace-uri|element-name). 
    */
   static void endElement(void* reader, const XML_Char* name);
   
public:
   /**
    * Creates a new XmlReader.
    */
   XmlReader();
   
   /**
    * Destructs this XmlReader.
    */
   virtual ~XmlReader();
   
   /**
    * Deserializes an object from xml using the passed DataBinding.
    * 
    * @param db the DataBinding for the object to deserialize.
    * @param is the InputStream to read the xml from.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool read(
      db::data::DataBinding* db, db::io::InputStream* is);
};

} // end namespace xml
} // end namespace data
} // end namespace db
#endif
