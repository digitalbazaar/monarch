/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_xml_DomReader_H
#define db_data_xml_DomReader_H

#include "db/data/xml/XmlReader.h"
#include "db/data/xml/DomTypes.h"

namespace db
{
namespace data
{
namespace xml
{

/**
 * A DomReader can parse XML (eXtensible Markup Language) into a
 * DOM (Document Object Model).
 * 
 * @author Dave Longley
 */
class DomReader : public XmlReader
{
protected:
   /**
    * True once the root element has been started.
    */
   bool mRootStarted;
   
   /**
    * Handles start elements for this reader.
    * 
    * @param name the name of the element (namespace-uri|element-name).
    * @param attrs the attributes of the element.
    */
   virtual void startElement(const XML_Char* name, const XML_Char** attrs);
   
   /**
    * Handles end elements for this reader.
    * 
    * @param name the name of the element (namespace-uri|element-name). 
    */
   virtual void endElement(const XML_Char* name);
   
   /**
    * Handles character data for this reader.
    * 
    * @param data the read data.
    * @param length the length of the data.
    */
   virtual void appendData(const XML_Char* data, int length);
   
public:
   /**
    * Creates a new DomReader.
    */
   DomReader();
   
   /**
    * Destructs this DomReader.
    */
   virtual ~DomReader();
   
   /**
    * Starts deserializing a Document from xml. This DomReader can be re-used
    * by calling start() with the same or a new object. Calling start() before
    * a previous deserialization has finished will abort the previous state.
    * 
    * Using a non-empty object can be used to merge in new values. This is
    * only defined for similar object types (i.e., merging an array into
    * a map will overwrite the map).
    * 
    * @param root the DynamicObject to use as the root element for the Document.
    */
   using XmlReader::start;
   
   /**
    * Finishes deserializing a document from xml. This method must be called
    * to complete deserialization.
    * 
    * @return true if the finish succeeded, false if an IOException occurred.
    */
   virtual bool finish();
};

} // end namespace xml
} // end namespace data
} // end namespace db
#endif
