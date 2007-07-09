/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_modest_Module_H
#define db_modest_Module_H

#include "ModuleInterface.h"
#include "Exception.h"

namespace db
{
namespace modest
{

// forward declare Kernel
class Kernel;

typedef struct ModuleId
{
   /**
    * The unique name of this Module.
    */
   std::string name;
   
   /**
    * The version (major.minor) of this Module.
    */
   std::string version;
};

/**
 * A Module is any extension to Modest. It can be loaded into an instance of
 * Modest run its available Operations. It can also create and provide new
 * Operations for other Modules to run.
 * 
 * @author Dave Longley
 */
class Module
{
public:
   /**
    * Creates a new Module.
    */
   Module() {};
   
   /**
    * Destructs this Module.
    */
   virtual ~Module() {};
   
   /**
    * Gets the ID of this Module.
    * 
    * @return the ID of this Module.
    */
   virtual const ModuleId& getId() = 0;
   
   /**
    * Gets the version (major.minor) of Modest that this Module is compatible
    * with.
    * 
    * @return the version (major.minor) of Modest that this Module is
    *         compatible with.
    */
   virtual std::string getCompatibleModestVersion() = 0;
   
   /**
    * Initializes this Module with the modest Kernel once it has been
    * loaded.
    * 
    * @param k the the modest Kernel to initialize with.
    * 
    * @return an Exception if one occurred, NULL if not.
    */
   virtual db::rt::Exception* initialize(Kernel* k) = 0;
   
   /**
    * Cleans up this Module just prior to its unloading.
    * 
    * @param k the modest Kernel that is unloading this Module.
    */
   virtual void cleanup(Kernel* k) = 0;
   
   /**
    * Gets the interface for this Module. The returned object should be
    * cast to the appropriate extended ModuleInterface class for this Module.
    * 
    * @return the interface that provides access to this Module's functionality.
    */
   virtual ModuleInterface* getInterface() = 0;
};

} // end namespace modest
} // end namespace db

// prevent C++ name mangling
#ifdef __cplusplus
extern "C" {
#endif

// define MODULE_API directive
#ifdef WIN32
   #ifdef MODEST_MODULE_API_EXPORT
      #define MODEST_MODULE_API __declspec(dllexport);
   #else
      #define MODEST_MODULE_API __declspec(dllimport);
   #endif
#else
   #define MODEST_MODULE_API
#endif

extern MODEST_MODULE_API db::modest::Module* createModestModule();
extern MODEST_MODULE_API void freeModestModule(db::modest::Module* m);

typedef db::modest::Module* (*CreateModestModuleFn)();
typedef void (*FreeModestModuleFn)(db::modest::Module*);

#ifdef __cplusplus
}
#endif

#endif
