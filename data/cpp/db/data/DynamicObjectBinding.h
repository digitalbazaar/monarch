/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_DynamicObjectBinding_H
#define db_data_DynamicObjectBinding_H

#include "db/data/DynamicObjectBasicBinding.h"
#include "db/data/DynamicObjectMapBinding.h"
#include "db/data/DynamicObjectArrayBinding.h"

namespace db
{
namespace data
{

/**
 * A DynamicObjectBinding provides the data bindings for a DynamicObject so
 * that it can be serialized and/or deserialized.
 * 
 * Note: There is a lot of commonality amongst the various helper data
 * binding classes for a DynamicObject -- it could be consolidated. 
 * 
 * @author Dave Longley
 */
class DynamicObjectBinding : public db::data::DataBinding
{
protected:
   /**
    * The DataMappings for this binding.
    */
   DataMappingFunctor<DynamicObjectBinding, db::util::DynamicObject>
      mStringMapping;
   DataMappingFunctor<DynamicObjectBinding, db::util::DynamicObject>
      mBooleanMapping;
   DataMappingFunctor<DynamicObjectBinding, db::util::DynamicObject>
      mNumberMapping;
   DataMappingFunctor<DynamicObjectBinding, db::util::DynamicObject>
      mMapMapping;
   DataMappingFunctor<DynamicObjectBinding, db::util::DynamicObject>
      mArrayMapping;
   
   /**
    * The DataBindings for this binding.
    */
   DynamicObjectBasicBinding mBasicBinding;
   DynamicObjectMapBinding mMapBinding;
   DynamicObjectArrayBinding mArrayBinding;
   
public:
   /**
    * Creates a new DynamicObjectBinding.
    * 
    * @param dyno the DynamicObject this binding is for.
    */
   DynamicObjectBinding(db::util::DynamicObject* dyno);
   
   /**
    * Destructs this DynamicObjectBinding.
    */
   virtual ~DynamicObjectBinding();
   
   /**
    * Returns the object to use with a create/add DataMapping. The default
    * implementation returns the bound object.
    * 
    * This can be overloaded if a special data translation is to be used
    * by a DataBinding for a given bound object. In other words, if the
    * bound object doesn't have the methods necessary to create/add a child,
    * another object (like the extended DataBinding) can provide the
    * create/add methods to update the bound object.
    * 
    * @return the object to use with a create/add DataMapping.
    */
   virtual void* getCreateAddObject(db::data::DataName* dn);
   
   /**
    * Called to indicate that this binding is now being used in a
    * serialization process.
    * 
    * This callback is useful to dynamically altering mappings/bindings.
    */
   virtual void serializationStarted();
   
   /**
    * Called to indicate that this binding is now being used in a
    * deserialization process.
    * 
    * This callback is useful to dynamically altering mappings/bindings.
    */
   virtual void deserializationStarted();
   
   /**
    * Creates a DynamicObject.
    * 
    * @return the heap-allocated DynamicObject.
    */
   db::util::DynamicObject* createDynamicObject();
   
   /**
    * Adds a DynamicObject to the underlying DynamicObject.
    * 
    * @param dyno the DynamicObject to add.
    */
   virtual void addDynamicObject(db::util::DynamicObject* dyno);
   
   /**
    * Populates a list of child objects for the given DataName. This method
    * must be overloaded to populate child objects with data using this binding.
    *
    * @param dn the DataName to get the child objects for.
    * @param children the list to store child objects in.
    */
   virtual void getChildren(db::data::DataName* dn, std::list<void*>& children);
};

} // end namespace data
} // end namespace db
#endif
