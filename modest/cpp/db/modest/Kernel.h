/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_modest_Kernel_H
#define db_modest_Kernel_H

#include "db/modest/Engine.h"
#include "db/modest/ModuleLibrary.h"
#include "db/modest/OperationRunner.h"

namespace db
{
namespace modest
{

/**
 * A Kernel maintains an Engine and a ModuleLibrary with Modules that extend
 * that Engine's functionality. It is the upper-most layer of the Modest
 * system and provides a public interface for applications (i.e. GUIs) to
 * make use of the Modest Engine and its Modules.
 * 
 * @author Dave Longley
 */
class Kernel : public OperationRunner
{
protected:
   /**
    * The Engine for this Kernel.
    */
   Engine* mEngine;
   
   /**
    * The ModuleLibrary for this Kernel.
    */
   ModuleLibrary* mModuleLibrary;
   
   /**
    * The version of this Kernel (major.minor).
    */
   const char* mVersion;
   
public:
   /**
    * Creates a new Kernel.
    */
   Kernel();
   
   /**
    * Destructs this Kernel.
    */
   virtual ~Kernel();
   
   /**
    * Queues the passed Operation with this Kernel's modest Engine.
    * 
    * @param op the Operation to queue with this Kernel's modest Engine.
    */
   virtual void runOperation(Operation& op);
   
   /**
    * Gets this Kernel's Engine.
    * 
    * @return this Kernel's Engine.
    */
   virtual Engine* getEngine();
   
   /**
    * Gets this Kernel's ModuleLibrary.
    * 
    * @return this Kernel's ModuleLibrary.
    */
   virtual ModuleLibrary* getModuleLibrary();
   
   /**
    * Gets this Kernel's version (major.minor).
    * 
    * @return this Kernel's version.
    */
   virtual const char* getVersion();
};

} // end namespace modest
} // end namespace db

// prevent C++ name mangling
#ifdef __cplusplus
extern "C" {
#endif

// define MODEST_API directive
#ifdef WIN32
   #ifdef MODEST_API_EXPORT
      #define MODEST_API __declspec(dllexport)
   #else
      #define MODEST_API __declspec(dllimport)
   #endif
#else
   #define MODEST_API
#endif

extern MODEST_API db::modest::Kernel* createModestKernel();
extern MODEST_API void freeModestKernel(db::modest::Kernel* k);

typedef db::modest::Kernel* (*CreateModestKernelFn)();
typedef void (*FreeModestKernelFn)(db::modest::Kernel*);

#ifdef __cplusplus
}
#endif

#endif
