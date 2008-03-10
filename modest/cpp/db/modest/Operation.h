/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_modest_Operation_H
#define db_modest_Operation_H

#include "db/rt/Collectable.h"
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
 * The Operation type functions as a reference-counting pointer for a
 * heap-allocated Operation implementation (a OperationImpl). When no more
 * references to a given heap-allocated OperationImpl exist, it will be freed.
 * 
 * @author Dave Longley
 */
class Operation : public db::rt::Collectable<OperationImpl>
{
public:
   /**
    * Creates a new Operation with the given Runnable.
    * 
    * @param r the Runnable to use.
    */
   Operation(db::rt::Runnable& r);
   Operation(db::rt::RunnableRef& r);
   
   /**
    * Creates a new Operation reference to the passed OperationImpl.
    * 
    * @param impl the OperationImpl to reference.
    */
   Operation(OperationImpl* impl = NULL);
   
   /**
    * Destructs this Operation.
    */
   virtual ~Operation();
   
   /**
    * Gets the Operation for the current thread, if there is one.
    * 
    * @return the Operation for the current thread which may be NULL.
    */
   static Operation currentOperation();
};

} // end namespace modest
} // end namespace db
#endif
