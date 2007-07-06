/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef ImmutableState_H
#define ImmutableState_H

namespace db
{
namespace modest
{

/**
 * An ImmutableState is a wrapper for a State that protects it from mutation.
 * 
 * @author Dave Longley
 */
class ImmutableState
{
public:
   /**
    * Creates a new ImmutableState.
    */
   ImmutableState() {};
   
   /**
    * Destructs this ImmutableState.
    */
   virtual ~ImmutableState() {};
};

} // end namespace modest
} // end namespace db
#endif
