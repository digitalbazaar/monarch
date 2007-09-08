/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_xml_XmlWriter_H
#define db_data_xml_XmlWriter_H

#include "db/data/DataBinding.h"
#include "db/io/OutputStream.h"

namespace db
{
namespace data
{
namespace xml
{

/**
 * An XmlWriter provides an interface for serializing objects to
 * XML (eXtensible Markup Language).
 * 
 * An XmlWriter maintains element state information as it is writing out
 * xml. If it has been used to write a chunk of xml, it should be reset() to
 * write another chunk of xml that is part of a different document.   
 * 
 * @author Dave Longley
 */
class XmlWriter
{
protected:
   /**
    * Stores information about an xml element that is being written out.
    */
   typedef struct ElementState
   {
      /**
       * The data name for the element.
       */
      DataName* dn;
      
      /**
       * Whether or not the start element is still open.
       */
      bool open;
   };
   
   /**
    * A stack of the current element states.
    */
   std::list<ElementState> mElementStack;
   
public:
   /**
    * Creates a new XmlWriter.
    */
   XmlWriter();
   
   /**
    * Destructs this XmlWriter.
    */
   virtual ~XmlWriter();
   
   /**
    * Resets this XmlWriter for another use.
    */
   virtual void reset();
   
   /**
    * Writes a start element.
    * 
    * @param dn the DataName to use.
    * @param os the OutputStream to write to.
    */
   virtual bool writeStartElement(
      db::data::DataName* dn, db::io::OutputStream* os);
   
   /**
    * Writes an end element.
    * 
    * @param os the OutputStream to write to.
    */
   virtual bool writeEndElement(db::io::OutputStream* os);
   
   /**
    * Writes an attribute.
    * 
    * @param dn the DataName to use.
    * @param data the data for the attribute.
    * @param length the length of the data for the attribute.
    * @param os the OutputStream to write to.
    */
   virtual bool writeAttribute(
      db::data::DataName* dn, const char* data, int length,
      db::io::OutputStream* os);
   
   /**
    * Writes an attribute.
    * 
    * @param dn the DataName to use.
    * @param dm the DataMapping to use.
    * @param obj the object with the data.
    * @param os the OutputStream to write to.
    */
   virtual bool writeAttribute(
      db::data::DataName* dn, db::data::DataMapping* dm, void* obj,
      db::io::OutputStream* os);
   
   /**
    * Writes element data.
    * 
    * @param data the data to write.
    * @param length the length of the data.
    * @param os the OutputStream to write to.
    */
   virtual bool writeElementData(
      const char* data, int length, db::io::OutputStream* os);
   
   /**
    * Writes element data using the passed DataMapping with the given object.
    * 
    * @param dm the DataMapping to use.
    * @param obj the object with the data.
    * @param os the OutputStream to write to.
    */
   virtual bool writeElementData(
      db::data::DataMapping* dm, void* obj, db::io::OutputStream* os);
   
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
