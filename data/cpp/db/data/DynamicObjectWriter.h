/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_DynamicObjectWriter_H
#define db_data_DynamicObjectWriter_H

#include "db/data/DataBinding.h"
#include "db/util/DynamicObject.h"

namespace db
{
namespace data
{

/**
 * A DynamicObjectWriter uses a DataBinding to convert a regular object into
 * a DynamicObject.
 * 
 * @author Dave Longley
 */
class DynamicObjectWriter
{
protected:
   
public:
   /**
    * Creates a new DynamicObjectWriter.
    */
   DynamicObjectWriter();
   
   /**
    * Destructs this DynamicObjectWriter.
    */
   virtual ~DynamicObjectWriter();
   
   /**
    * Writes a regular object out as a DynamicObject using the passed
    * DataBinding.
    *  
    * @param db the DataBinding for the object to write out as a DynamicObject.
    */
   db::util::DynamicObject write(DataBinding* db);
};

} // end namespace data
} // end namespace db
#endif
