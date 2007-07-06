/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef Engine_H
#define Engine_H

#include "Object.h"

namespace db
{
namespace modest
{

/**
 * A Modest Engine (MODular Extensible State Engine) is a lightweight
 * processing engine that keeps track of state information and can be
 * extended by Modules that make use of existing functionality and
 * provide new functionality.
 * 
 * The Modest Engine executes Operations to change its current State.
 * Operations may be executed concurrently.
 * 
 * The Modest Engine is intended to be "modest" in its complexity and
 * code base, but powerful in its extensibility. The core engine provides
 * a cross-platform thread pool for executing Operations in an orderly
 * fashion. The design intends to allow developers to create Modules that
 * can concurrently run multiple Operations that must by synchronized with
 * one another in some fashion. 
 * 
 * @author Dave Longley
 */
class Engine : public virtual db::rt::Object
{
public:
   /**
    * Creates a new Engine.
    */
   Engine();
   
   /**
    * Destructs this Engine.
    */
   virtual ~Engine();
};

} // end namespace modest
} // end namespace db
#endif
