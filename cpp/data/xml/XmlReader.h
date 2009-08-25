/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_data_xml_XmlReader_H
#define db_data_xml_XmlReader_H

#include "db/data/DynamicObjectReader.h"
#include "db/rt/Exception.h"

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
class XmlReader : public DynamicObjectReader
{
protected:
   /**
    * The xml parser for this reader.
    */
   XML_Parser mParser;

   /**
    * True if this xml parser has started, false if not.
    */
   bool mStarted;

   /**
    * An exception, if one occurred during parsing.
    */
   db::rt::ExceptionRef mException;

   /**
    * A stack of DynamicObjects.
    */
   std::list<db::rt::DynamicObject*> mDynoStack;

   /**
    * A stack of DynamicObjectTypes.
    */
   std::list<db::rt::DynamicObjectType> mTypeStack;

   /**
    * The character encoding.
    */
   static const char* CHAR_ENCODING;

   /**
    * The read size in bytes.
    */
   static unsigned int READ_SIZE;

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
    * Starts deserializing an object from xml. This XmlReader can be re-used
    * by calling start() with the same or a new object. Calling start() before
    * a previous deserialization has finished will abort the previous state.
    *
    * Using a non-empty object can be used to merge in new values. This is
    * only defined for similar object types (i.e., merging an array into
    * a map will overwrite the map).
    *
    * @param dyno the DynamicObject for the object to deserialize.
    */
   virtual void start(db::rt::DynamicObject& dyno);

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
    * @return true if the read succeeded, false if an Exception occurred.
    */
   virtual bool read(db::io::InputStream* is);

   /**
    * Finishes deserializing an object from xml. This method must be called
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

   /**
    * Called after the end element handler for the element the namespace
    * is associated with.
    *
    * @param prefix the prefix associated with the namespace-uri.
    * @param uri the namespace-uri.
    */
   virtual void endNamespaceDeclaration(const XML_Char* prefix);

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

   /**
    * Handles character data.
    *
    * @param xr the XmlReader that read in the data.
    * @param data the data that was read.
    * @param length the length of the data.
    */
   static void appendData(void* xr, const XML_Char* data, int length);

   /**
    * Called before the start element handler for each namespace declared
    * in that start element.
    *
    * @param xr the XmlReader that read in the data.
    * @param prefix the prefix associated with the namespace-uri.
    * @param uri the namespace-uri.
    */
   static void startNamespaceDeclaration(
      void* xr, const XML_Char* prefix, const XML_Char* uri);

   /**
    * Called after the end element handler for the element the namespace
    * is associated with.
    *
    * @param xr the XmlReader that read in the data.
    * @param prefix the prefix associated with the namespace-uri.
    * @param uri the namespace-uri.
    */
   static void endNamespaceDeclaration(void* xr, const XML_Char* prefix);

   /**
    * Gets a DynamicObjectType from the passed element tag name.
    *
    * @param tagName the element tag name.
    *
    * @return the associated DynamicObjectType.
    */
   static db::rt::DynamicObjectType tagNameToType(const char* name);

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
};

} // end namespace xml
} // end namespace data
} // end namespace db
#endif
