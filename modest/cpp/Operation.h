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
 * ExecutionEnvironment. An Operation may change the current State of the
 * Engine that executes it.
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
