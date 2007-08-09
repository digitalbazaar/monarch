/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_xml_XmlParser_H
#define db_data_xml_XmlParser_H

#include "DataBinding.h"
#include "InputStream.h"

namespace db
{
namespace data
{
namespace xml
{

/**
 * An XmlParser provides an interface for deserializing objects from
 * XML (eXtensible Markup Language).
 * 
 * @author Dave Longley
 */
class XmlParser
{
public:
   /**
    * Creates a new XmlParser.
    */
   XmlParser();
   
   /**
    * Destructs this XmlParser.
    */
   virtual ~XmlParser();
   
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
