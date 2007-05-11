/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef XmlSerializer_H
#define XmlSerializer_H

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
    * @exception IOException thrown if an IO error occurs.
    */
   virtual serialize(db::rt::Object* object, db::io::OutputStream* os)
   throw(db::io::IOException) = 0;
   
   /**
    * Deserializes the passed Object from xml.
    * 
    * @param object the Object to deserialize.
    * @param is the InputStream to read from.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   virtual deserialize(db::rt::Object* object, db::io::InputStream* is)
   throw(db::io::IOException) = 0;
};

} // end namespace xml
} // end namespace db
#endif
