/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_xml_XmlReader_H
#define db_data_xml_XmlReader_H

#include "DataBinding.h"
#include "InputStream.h"

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
