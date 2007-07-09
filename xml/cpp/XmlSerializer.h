/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_xml_XmlSerializer_H
#define db_xml_XmlSerializer_H

#include "InputStream.h"
#include "OutputStream.h"

namespace db
{
namespace xml
{

/**
 * An XmlSerializer provides an interface for serializing objects to
 * XML (eXtensible Markup Language) and deserializing them from XML.
 * 
 * @author Dave Longley
 */
class XmlSerializer
{
public:
   /**
    * Creates a new XmlSerializer.
    */
   XmlSerializer() {};
   
   /**
    * Destructs this XmlSerializer.
    */
   virtual ~XmlSerializer() {};
   
   /**
    * Serializes the passed Object to xml.
    * 
    * @param object the Object to serialize.
    * @param os the OutputStream to write to.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool serialize(
      db::rt::Object* object, db::io::OutputStream* os) = 0;
   
   /**
    * Deserializes the passed Object from xml.
    * 
    * @param object the Object to deserialize.
    * @param is the InputStream to read from.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool deserialize(
      db::rt::Object* object, db::io::InputStream* is) = 0;
};

} // end namespace xml
} // end namespace db
#endif
