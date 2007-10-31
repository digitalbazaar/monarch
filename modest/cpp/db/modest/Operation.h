/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_modest_Operation_H
#define db_modest_Operation_H

#include "db/modest/OperationImpl.h"

namespace db
{
namespace modest
{

/**
 * An Operation is some set of instructions that can be executed by an
 * Engine provided that its current State is compatible with this Operation's
 * OperationGuard. An Operation may change the current State of the
 * Engine that executes it.
 * 
 * Operations running on the same Engine share its State information and can
 * therefore specify the conditions underwhich they can execute. This provides
 * developers with an easy yet powerful way to protect resources and restrict
 * the flow of their code such that it only executes in the specific manner
 * desired.
 * 
 * The class functions as a reference-counting pointer for a heap-allocated
 * Operation implementation (a OperationImpl). When no more references to
 * a given heap-allocated OperationImpl exist, it will be freed.
 * 
 * @author Dave Longley
 */
class Operation
{
protected:
   /**
    * An Operation implementation reference.
    */
   typedef struct ImplReference
   {
      /**
       * A pointer to an Operation implementation.
       */
      OperationImpl* impl;
      
      /**
       * A reference count for the implementation.
       */
      unsigned int count;
   };
   
   /**
    * A reference to an Operation implementation.
    */
   ImplReference* mReference;
   
   /**
    * Acquires the passed implementation reference.
    * 
    * @param ref the ImplReference to acquire.
    */
   virtual void acquire(ImplReference* ref);
   
   /**
    * Releases the current implementation reference.
    */
   virtual void release();
   
public:
   /**
    * Creates a new Operation that points to the given OperationImpl.
    * 
    * @param impl the OperationImpl to point at.
    */
   Operation(OperationImpl* impl = NULL);
   
   /**
    * Creates a new Operation by copying an existing one.
    * 
    * @param copy the Operation to copy.
    */
   Operation(const Operation& copy);
   
   /**
    * Destructs this Operation.
    */
   virtual ~Operation();
   
   /**
    * Sets this Operation equal to another one.
    * 
    * @param rhs the Operation to set this one equal to.
    * 
    * @return a reference to this Operation.
    */
   virtual Operation& operator=(const Operation& rhs);
   
   /**
    * Compares this Operation against another one for equality. If the
    * passed Operation has a reference to the same OperationImpl as this
    * one, then they are equal.
    * 
    * @param rhs the Operation to compare this one against.
    * 
    * @return true if this Operation is equal the another one, false if not.
    */
   virtual bool operator==(const Operation& rhs);
   
   /**
    * Compares this Operation against another one for inequality.
    * 
    * @param rhs the Operation to compare this one against.
    * 
    * @return true if this Operation is not equal the another one, false if not.
    */
   virtual bool operator!=(const Operation& rhs);
   
   /**
    * Returns a reference to this Operation's implementation.
    * 
    * @return a reference to this Operation's implementation.
    */
   virtual OperationImpl& operator*();
   
   /**
    * Returns a pointer to this Operation's implementation.
    * 
    * @return a pointer to this Operation's implementation.
    */
   virtual OperationImpl* operator->();
};

} // end namespace modest
} // end namespace db
#endif
