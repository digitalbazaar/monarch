/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
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
    * Stores the namespace declaration information before each call to
    * startElement, where it gets added to the current element and cleared.
    */
   db::rt::DynamicObject mNamespacePrefixMap;

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
   virtual void start(Element& root);

   /**
    * Finishes deserializing a document from xml. This method must be called
    * to complete deserialization.
    *
    * @return true if the finish succeeded, false if an Exception occurred.
    */
   virtual bool finish();

protected:
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

   /**
    * Called before the start element handler for each namespace declared
    * in that start element.
    *
    * @param prefix the prefix associated with the namespace-uri.
    * @param uri the namespace-uri.
    */
   virtual void startNamespaceDeclaration(
      const XML_Char* prefix, const XML_Char* uri);
};

} // end namespace xml
} // end namespace data
} // end namespace db
#endif
