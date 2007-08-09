/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_xml_XmlSerializer_H
#define db_data_xml_XmlSerializer_H

#include "DataBinding.h"
#include "OutputStream.h"

namespace db
{
namespace data
{
namespace xml
{

/**
 * An XmlSerializer provides an interface for serializing objects to
 * XML (eXtensible Markup Language).
 * 
 * @author Dave Longley
 */
class XmlSerializer
{
public:
   /**
    * Creates a new XmlSerializer.
    */
   XmlSerializer();
   
   /**
    * Destructs this XmlSerializer.
    */
   virtual ~XmlSerializer();
   
   /**
    * Serializes an object to xml using the passed DataBinding.
    * 
    * @param db the DataBinding for the object to serialize.
    * @param os the OutputStream to write the xml to.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool write(
      db::data::DataBinding* db, db::io::OutputStream* os);
};

} // end namespace xml
} // end namespace data
} // end namespace db
#endif
