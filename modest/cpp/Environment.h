/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef Environment_H
#define Environment_H

#include "ImmutableState.h"

namespace db
{
namespace modest
{

/**
 * An Environment defines a set of state conditions. An Environment can inspect
 * an Engine's State to see whether or not that State is compatible with it.
 * 
 * @author Dave Longley
 */
class Environment
{
public:
   /**
    * Creates a new Environment.
    */
   Environment();
   
   /**
    * Destructs this Environment.
    */
   virtual ~Environment();
   
   /**
    * Returns true if the passed State meets the conditions of this Environment.
    * 
    * @param state the ImmutableState to inspect for compatibility.
    * 
    * @return true if the ImmutableState meets the conditions of this
    *         Environment, false if not.
    */
   virtual bool isCompatible(ImmutableState* state);
};

} // end namespace modest
} // end namespace db
#endif
