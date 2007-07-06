/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef State_H
#define State_H

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
class State
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
