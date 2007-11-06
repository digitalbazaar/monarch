/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_DynamicObjectReader_H
#define db_data_DynamicObjectReader_H

#include "db/data/DataBinding.h"
#include "db/util/DynamicObject.h"

namespace db
{
namespace data
{

/**
 * A DynamicObjectReader uses a DataBinding to populate a regular object from
 * a DynamicObject.
 * 
 * @author Dave Longley
 */
class DynamicObjectReader
{
protected:
   
public:
   /**
    * Creates a new DynamicObjectReader.
    */
   DynamicObjectReader();
   
   /**
    * Destructs this DynamicObjectReader.
    */
   virtual ~DynamicObjectReader();
   
   /**
    * Populates a regular object from a DynamicObject using the passed
    * DataBinding.
    *  
    * @param dyno the DynamicObject to read from.
    * @param db the DataBinding for populating the regular object.
    */
   virtual void read(db::util::DynamicObject dyno, DataBinding* db);
};

} // end namespace data
} // end namespace db
#endif
