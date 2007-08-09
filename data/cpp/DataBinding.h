/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_DataBinding_H
#define db_data_DataBinding_H

namespace db
{
namespace data
{

/**
 * A DataBinding provides a generic interface for converting formatted data
 * to or from an object.
 * 
 * @author Dave Longley
 */
class DataBinding
{
public:
   /**
    * Creates a new DataBinding.
    */
   DataBinding() {};
   
   /**
    * Destructs this DataBinding.
    */
   virtual ~DataBinding() {};
};

} // end namespace data
} // end namespace db
#endif
