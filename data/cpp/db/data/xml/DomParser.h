/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_xml_DomParser_H
#define db_data_xml_DomParser_H

#include "db/data/DynamicObjectReader.h"
#include "db/data/xml/DomTypes.h"

#include <expat.h>
#include <list>

namespace db
{
namespace data
{
namespace xml
{

/**
 * A DomParser can parse XML (eXtensible Markup Language) into a
 * DOM (Document Object Model).
 * 
 * @author Dave Longley
 */
class DomParser : public DynamicObjectReader
{
protected:
   /**
    * The xml parser for this DomParser.
    */
   XML_Parser mParser;
   
   /**
    * True if the xml parser has started, false if not.
    */
   bool mStarted;
   
   /**
    * True once the root element has been started.
    */
   bool mRootStarted;
   
   /**
    * An exception, if one occurred during parsing.
    */
   db::rt::ExceptionRef mException;
   
   /**
    * A stack of Elements.
    */
   std::list<Element*> mElementStack;
   
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
    * The character encoding.
    */
   static const char* CHAR_ENCODING;
   
   /**
    * The read size in bytes.
    */
   static unsigned int READ_SIZE;
   
   /**
    * Parses the local name from the passed fully qualified name. The passed
    * pointer will be incremented until it points at the start of the local
    * name.
    * 
    * @param fullName the fully qualified name to change to the local name.
    */
   static void parseLocalName(const char** fullName);
   
   /**
    * Parses a namespace uri out of the given name and sets the passed
    * name pointer to the start of the local name and the passed namespace
    * pointer to the start of a null-terminated namespace string.
    * 
    * @param fullName the namespace|local-name string.
    * @param ns the pointer to point at the namespace string. 
    */
   static void parseNamespace(const char** fullName, char** ns);
   
   /**
    * Handles start elements.
    * 
    * @param dp the DomParser that read in the element.
    * @param name the name of the element (namespace-uri|element-name).
    * @param attrs the attributes of the element.
    */
   static void startElement(
      void* dp, const XML_Char* name, const XML_Char** attrs);
   
   /**
    * Handles end elements.
    * 
    * @param dp the DomParser that read in the element.
    * @param name the name of the element (namespace-uri|element-name). 
    */
   static void endElement(void* dp, const XML_Char* name);
   
   /**
    * Handles character data.
    * 
    * @param dp the DomParser that read in the data.
    * @param data the data that was read.
    * @param length the length of the data.
    */
   static void appendData(void* dp, const XML_Char* data, int length);
   
public:
   /**
    * Creates a new DomParser.
    */
   DomParser();
   
   /**
    * Destructs this DomParser.
    */
   virtual ~DomParser();
   
   /**
    * Starts deserializing a Document from xml. This DomParser can be re-used
    * by calling start() with the same or a new object. Calling start() before
    * a previous deserialization has finished will abort the previous state.
    * 
    * Using a non-empty object can be used to merge in new values. This is
    * only defined for similar object types (i.e., merging an array into
    * a map will overwrite the map).
    * 
    * @param root the DynamicObject to use as the root element for the Document.
    */
   virtual void start(db::rt::DynamicObject& root);
   
   /**
    * This method reads xml from the passed InputStream until the end of
    * the stream, blocking if necessary.
    * 
    * The start() method must be called at least once before calling read(). As
    * the xml is read, the DynamicObject provided in start() is used to
    * deserialize an object.
    * 
    * This method may be called multiple times if the input stream needs to
    * be populated in between calls or if multiple input streams are used.
    * 
    * The object is built incrementally and on error will be partially built.
    * 
    * The finish() method must be called to complete the deserialization.
    * 
    * @param is the InputStream to read the xml from.
    * 
    * @return true if the read succeeded, false if an IOException occurred.
    */
   virtual bool read(db::io::InputStream* is);
   
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
