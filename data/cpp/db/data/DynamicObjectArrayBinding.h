/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_DynamicObjectArrayBinding_H
#define db_data_DynamicObjectArrayBinding_H

#include "db/data/DynamicObjectBasicBinding.h"

namespace db
{
namespace data
{

// forward declare DynamicObjectMapBinding, DynamicObjectArrayBinding
class DynamicObjectMapBinding;
class DynamicObjectArrayBinding;

/**
 * A DynamicObjectElementBinding is a helper binding that binds array
 * elements to a DynamicObject.
 * 
 * @author Dave Longley
 */
class DynamicObjectElementBinding : public db::data::DataBinding
{
protected:
   /**
    * The DataMappings for this binding.
    */
   DataMappingFunctor<DynamicObjectElementBinding> mIndexMapping;
   DataMappingFunctor<DynamicObjectElementBinding, db::util::DynamicObject>
      mStringMapping;
   DataMappingFunctor<DynamicObjectElementBinding, db::util::DynamicObject>
      mBooleanMapping;
   DataMappingFunctor<DynamicObjectElementBinding, db::util::DynamicObject>
      mNumberMapping;
   DataMappingFunctor<DynamicObjectElementBinding, db::util::DynamicObject>
      mMapMapping;
   DataMappingFunctor<DynamicObjectElementBinding, db::util::DynamicObject>
      mArrayMapping;
   
   /**
    * The DataBindings for this binding.
    */
   DynamicObjectBasicBinding mBasicBinding;
   DynamicObjectMapBinding* mMapBinding;
   DynamicObjectArrayBinding* mArrayBinding;
   
   /**
    * Stores the parent array binding.
    */
   DynamicObjectArrayBinding* mParentBinding;
   
public:
   /**
    * Creates a new DynamicObjectElementBinding.
    * 
    * @param dyno the DynamicObject this binding is for.
    * @param ab the parent DynamicObjectArrayBinding.
    */
   DynamicObjectElementBinding(
      db::util::DynamicObject* dyno, DynamicObjectArrayBinding* ab);
   
   /**
    * Destructs this DynamicObjectElementBinding.
    */
   virtual ~DynamicObjectElementBinding();
   
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
    * Returns the object to use with a set/get DataMapping. The default
    * implementation returns the bound object.
    * 
    * This can be overloaded if a special data translation is to be used
    * by a DataBinding for a given bound object. In other words, if the
    * bound object doesn't have the methods necessary to set/get data,
    * another object (like the extended DataBinding) can provide the
    * set/get methods to update the bound object.
    * 
    * @return the object to use with a set/get DataMapping.
    */
   virtual void* getSetGetObject(DataName* dn);
   
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
    * Sets the current index.
    * 
    * @param index the index.
    */
   virtual void setIndex(int index);
   
   /**
    * Gets the current index.
    * 
    * @return the current index.
    */
   virtual int getIndex();
   
   /**
    * Creates a dynamic object.
    * 
    * @return the created dynamic object.
    */
   virtual db::util::DynamicObject* createDynamicObject();
   
   /**
    * Adds a dynamic object.
    * 
    * @param dyno the dynamic object to add.
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

/**
 * A DynamicObjectArrayBinding is a helper binding that binds arrays to
 * a DynamicObject. 
 * 
 * @author Dave Longley
 */
class DynamicObjectArrayBinding : public db::data::DataBinding
{
protected:
   /**
    * The DataMappings for this binding.
    */
   db::data::DataMappingFunctor<DynamicObjectArrayBinding> mElementMapping;
   
   /**
    * The DataBindings for this binding.
    */
   db::data::DynamicObjectElementBinding mElementBinding;
   
   /**
    * Stores the current index.
    */
   int mCurrentIndex;
   
public:
   /**
    * Creates a new DynamicObjectArrayBinding.
    * 
    * @param dyno the DynamicObject this binding is for.
    */
   DynamicObjectArrayBinding(db::util::DynamicObject* dyno = NULL);
   
   /**
    * Destructs this DynamicObjectBinding.
    */
   virtual ~DynamicObjectArrayBinding();
   
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
    * Creates an element.
    * 
    * @return the created element.
    */
   virtual void* createElement();
   
   /**
    * Adds an element.
    * 
    * @param e the element to add.
    */
   virtual void addElement(void* e);
   
   /**
    * Sets the current index.
    * 
    * @param index the index.
    */
   virtual void setIndex(int index);
   
   /**
    * Gets the current index.
    * 
    * @return the current index.
    */
   virtual int getIndex();
   
   /**
    * Increments and then gets the current index.
    * 
    * @return the current index.
    */
   virtual int getNextIndex();
   
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
