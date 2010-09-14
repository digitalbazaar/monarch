/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_v8_V8Controller_h
#define monarch_v8_V8Controller_h

#include "monarch/kernel/MicroKernel.h"
#include "monarch/v8/V8EngineApi.h"
#include "monarch/v8/V8ModuleApi.h"
#include <v8.h>

namespace monarch
{
namespace v8
{

/**
 * A V8Engine can run JavaScript using the V8 engine.
 * 
 * @author David I. Lehn
 */
class V8Controller : public V8ModuleApi
{
protected:
   /**
    * The related bitmunk node.
    */
   monarch::kernel::MicroKernel* mKernel;

   /**
    * Persistent globals.
    */
   ::v8::Persistent< ::v8::ObjectTemplate> mGlobals;

public:
   /**
    * Creates a new V8Controller.
    */
   V8Controller();
   
   /**
    * Destructs this V8Controller.
    */
   virtual ~V8Controller();

   /**
    * Initializes this implementation.
    *
    * @param node the bitmunk node to initialize with.
    *
    * @return true on success, false on failure with exception set.
    */
   virtual bool initialize(monarch::kernel::MicroKernel* kernel);

   /**
    * Cleans up this implementation.
    */
   virtual void cleanup();

   /**
    * {@inheritDoc}
    */
   virtual bool createEngine(V8EngineRef& v8Engine);

   /**
    * Get the globals object template
    *
    * @return the globals ObjectTemplate.
    */
   virtual ::v8::Persistent< ::v8::ObjectTemplate> getGlobals();

   static ::v8::Handle< ::v8::Object> wrapDynamicObject(
      monarch::rt::DynamicObject* obj);
   static monarch::rt::DynamicObject* unwrapDynamicObject(
      ::v8::Handle< ::v8::Object> obj);

   static ::v8::Handle< ::v8::Value> d2j(monarch::rt::DynamicObject& d);
   static monarch::rt::DynamicObject j2d(::v8::Handle< ::v8::Value> value);
};

} // end namespace v8
} // end namespace monarch
#endif
