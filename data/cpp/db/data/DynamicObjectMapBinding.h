/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_DynamicObjectMapBinding_H
#define db_data_DynamicObjectMapBinding_H

#include "db/data/DynamicObjectBasicBinding.h"
#include "db/util/DynamicObjectIterator.h"

namespace db
{
namespace data
{

// forward declare DynamicObjectMapBinding, DynamicObjectArrayBinding
class DynamicObjectMapBinding;
class DynamicObjectArrayBinding;

/**
 * A DynamicObjectMemberBinding is a helper binding that binds object
 * members to a DynamicObject.
 * 
 * @author Dave Longley
 */
class DynamicObjectMemberBinding : public db::data::DataBinding
{
protected:
   /**
    * The DataMappings for this binding.
    */
   DataMappingFunctor<DynamicObjectMemberBinding> mNameMapping;
   DataMappingFunctor<DynamicObjectMemberBinding, db::util::DynamicObject>
      mStringMapping;
   DataMappingFunctor<DynamicObjectMemberBinding, db::util::DynamicObject>
      mBooleanMapping;
   DataMappingFunctor<DynamicObjectMemberBinding, db::util::DynamicObject>
      mNumberMapping;
   DataMappingFunctor<DynamicObjectMemberBinding, db::util::DynamicObject>
      mMapMapping;
   DataMappingFunctor<DynamicObjectMemberBinding, db::util::DynamicObject>
      mArrayMapping;
   
   /**
    * The DataBindings for this binding.
    */
   DynamicObjectBasicBinding mBasicBinding;
   DynamicObjectMapBinding* mMapBinding;
   DynamicObjectArrayBinding* mArrayBinding;
   
   /**
    * Stores the parent map binding.
    */
   DynamicObjectMapBinding* mParentBinding;
   
public:
   /**
    * Creates a new DynamicObjectMemberBinding.
    * 
    * @param dyno the DynamicObject this binding is for.
    * @param mb the parent DynamicObjectMapBinding.
    */
   DynamicObjectMemberBinding(
      db::util::DynamicObject* dyno, DynamicObjectMapBinding* mb);
   
   /**
    * Destructs this DynamicObjectMemberBinding.
    */
   virtual ~DynamicObjectMemberBinding();
   
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
    * Sets the current name.
    * 
    * @param name the name.
    */
   virtual void setName(const char* name);
   
   /**
    * Gets the current name.
    * 
    * @return the current name.
    */
   virtual const char* getName();
   
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
 * A DynamicObjectMapBinding is a helper binding that binds maps to
 * a DynamicObject. 
 * 
 * @author Dave Longley
 */
class DynamicObjectMapBinding : public db::data::DataBinding
{
protected:
   /**
    * The DataMappings for this binding.
    */
   db::data::DataMappingFunctor<DynamicObjectMapBinding> mMemberMapping;
   
   /**
    * The DataBindings for this binding.
    */
   db::data::DynamicObjectMemberBinding mMemberBinding;
   
   /**
    * An iterator with the next member name.
    */
   db::util::DynamicObjectIterator mIterator;
   
   /**
    * The current member name.
    */
   const char* mCurrentName;
   
public:
   /**
    * Creates a new DynamicObjectMapBinding.
    * 
    * @param dyno the DynamicObject this binding is for.
    */
   DynamicObjectMapBinding(db::util::DynamicObject* dyno = NULL);
   
   /**
    * Destructs this DynamicObjectBinding.
    */
   virtual ~DynamicObjectMapBinding();
   
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
    * Creates a member.
    * 
    * @return the created member.
    */
   virtual void* createMember();
   
   /**
    * Adds a member.
    * 
    * @param m the member to add.
    */
   virtual void addMember(void* m);
   
   /**
    * Sets the current member name.
    * 
    * @param name the member name.
    */
   virtual void setName(const char* name);
   
   /**
    * Gets the current member name.
    * 
    * @return the current member name.
    */
   virtual const char* getName();
   
   /**
    * Increments and then gets the current member name.
    * 
    * @return the current member name.
    */
   virtual const char* getNextName();
   
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
