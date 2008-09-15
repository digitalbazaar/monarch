/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/modest/ModuleLoader.h"

#include "db/modest/ModuleApi.h"
#include "db/rt/DynamicLibrary.h"
#include "db/rt/DynamicObject.h"

#include <cstring>

using namespace db::modest;
using namespace db::rt;

ModuleLoader::ModuleLoader()
{
}

ModuleLoader::~ModuleLoader()
{
}

ModuleInfo* ModuleLoader::loadModule(const char* filename)
{
   ModuleInfo* rval = NULL;
   
   // open library
   void* handle = dlopen(filename, RTLD_NOW | RTLD_GLOBAL);
   if(handle != NULL)
   {
      CreateModestModuleFn create;
      FreeModestModuleFn free;
      
      // clear error
      char* error = dlerror();
      
      // try to get create module function
      create = (CreateModestModuleFn)dlsym(handle, "createModestModule");
      if((error = dlerror()) == NULL)
      {
         // clear error
         error = dlerror();
         
         // try to get free module function
         free = (FreeModestModuleFn)dlsym(handle, "freeModestModule");
         error = dlerror();
      }
      
      if(error == NULL)
      {
         // create ModuleInfo
         rval = new ModuleInfo();
         rval->handle = handle;
         rval->module = create();
         rval->freeModule = free;
      }
      else
      {
         // could not load create or free functions
         ExceptionRef e = new Exception(
            "Could not load module.", "db.modest.BadModule");
         e->getDetails()["filename"] = filename;
         e->getDetails()["error"] = error;
         Exception::setLast(e, false);
      }
   }
   else
   {
      // failed to open module
      char* error = dlerror();
      ExceptionRef e = new Exception(
         "Could not open module file.", "db.modest.BadModuleFile");
      e->getDetails()["filename"] = filename;
      e->getDetails()["error"] = error;
      Exception::setLast(e, false);
   }
   
   return rval;
}

void ModuleLoader::unloadModule(ModuleInfo* mi)
{
   // free module
   mi->freeModule(mi->module);
   
   // close handle
   dlclose(mi->handle);
   
   // delete module info
   delete mi;
}
