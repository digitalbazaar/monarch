/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_v8_V8EngineApi_h
#define monarch_v8_V8EngineApi_h

#include "monarch/kernel/MicroKernelModuleApi.h"
#include "monarch/rt/Collectable.h"

#include <string>

namespace monarch
{
namespace v8
{

/**
 * A V8EngineApi provides the application programming interface for a
 * V8Engine that can run JavaScript code.
 *
 * @author David I. Lehn
 */
class V8EngineApi : public monarch::kernel::MicroKernelModuleApi
{
public:
   /**
    * Creates a new V8EngineApi.
    */
   V8EngineApi() {};

   /**
    * Destructs this V8EngineApi.
    */
   virtual ~V8EngineApi() {};

   /**
    * Run JavaScript.
    *
    * @param js the JavaScript code.
    * @param result the result string to set.
    *
    * @return true on success, false with exception set on failure.
    */
   virtual bool runScript(const char* js, std::string& result) = 0;
};

// type definition for a reference counted V8Engine
typedef monarch::rt::Collectable<V8EngineApi> V8EngineRef;

} // end namespace v8
} // end namespace monarch
#endif
