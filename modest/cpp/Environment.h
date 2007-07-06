/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef Environment_H
#define Environment_H

namespace db
{
namespace modest
{

/**
 * An Environment defines a set of state conditions. An Environment can inspect
 * an Engine's State to see whether or not that Engine is in it.
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
    * Returns true if the passed State is in the 
    * 
};

} // end namespace modest
} // end namespace db
#endif
