/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_xml_XmlReader_H
#define db_data_xml_XmlReader_H

#include "DataBinding.h"
#include "InputStream.h"

#include <expat.h>
#include <list>

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
    * A stack of DataBindings.
    */
   std::list<DataBinding*> mDataBindingsStack;
   
   /**
    * Handles start elements for this reader.
    * 
    * @param name the name of the element (namespace-uri|element-name).
    * @param attrs the attributes of the element.
    */
   void startElement(const XML_Char* name, const XML_Char** attrs);
   
   /**
    * Handles end elements for this reader.
    * 
    * @param name the name of the element (namespace-uri|element-name). 
    */
   void endElement(const XML_Char* name);
   
   /**
    * The read size in bytes.
    */
   static unsigned int READ_SIZE;
   
   /**
    * Parses a namespace uri out of the given name and sets the passed
    * name pointer to the start of the local name and the passed namespace
    * pointer to the start of a null-terminated namespace string.
    * 
    * @param name the namespace|local-name string.
    * @param ns the pointer to point at the namespace string. 
    */
   static void parseNamespace(const char** name, char** ns);
   
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
   static void endElement(void* xr, const XML_Char* name);
   
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
