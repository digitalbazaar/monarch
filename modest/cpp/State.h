/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef State_H
#define State_H

#include "ImmutableState.h"

namespace db
{
namespace modest
{

/**
 * A State maintains the current information about a Modest Engine. It can
 * be modified by an Operation.
 * 
 * @author Dave Longley
 */
class State : public ImmutableState
{
public:
   /**
    * Creates a new State.
    */
   State();
   
   /**
    * Destructs this State.
    */
   virtual ~State();
};

} // end namespace modest
} // end namespace db
#endif
