/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef Operation_H
#define Operation_H

namespace db
{
namespace modest
{

/**
 * An Operation is some set of instructions that can be executed by an
 * Engine provided that it's current State is compatible with this Operation's
 * execution Environment. An Operation may change the current State of the
 * Engine that executes it.
 * 
 * Operations running on the same Engine share its State information and can
 * therefore specify the conditions underwhich they can execute. This provides
 * developers with an easy yet powerful way to protect resources and restrict
 * the flow of their code such that it only executes in the specific manner
 * desired.
 * 
 * @author Dave Longley
 */
class Operation
{
public:
   /**
    * Creates a new Operation.
    */
   Operation();
   
   /**
    * Destructs this Operation.
    */
   virtual ~Operation();
};

} // end namespace modest
} // end namespace db
#endif
