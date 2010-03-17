/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/modest/ModuleLoader.h"

#include "monarch/rt/DynamicLibrary.h"

#include <cstring>

using namespace monarch::modest;
using namespace monarch::rt;

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
         rval = loadModule(create, free);
         if(rval != NULL)
         {
            rval->handle = handle;
         }
         else
         {
            // could not load module due to loadModule(c,f) exception
            ExceptionRef e = new Exception(
               "Could not load module.", "monarch.modest.BadModule");
            e->getDetails()["filename"] = filename;
            Exception::push(e);
         }
      }
      else
      {
         // could not load create or free functions
         ExceptionRef e = new Exception(
            "Could not load module.", "monarch.modest.BadModule");
         e->getDetails()["filename"] = filename;
         e->getDetails()["error"] = error;
         Exception::set(e);
      }
   }
   else
   {
      // failed to open module
      char* error = dlerror();
      ExceptionRef e = new Exception(
         "Could not open module file.", "monarch.modest.BadModuleFile");
      e->getDetails()["filename"] = filename;
      e->getDetails()["error"] = error;
      Exception::set(e);
   }

   return rval;
}

ModuleInfo* ModuleLoader::loadModule(
   CreateModestModuleFn cm, FreeModestModuleFn fm)
{
   ModuleInfo* rval = NULL;

   Module* module = cm();

   if(module != NULL)
   {
      // create ModuleInfo
      rval = new ModuleInfo();
      rval->handle = NULL;
      rval->module = module;
      rval->freeModule = fm;
   }
   else
   {
      // create returned NULL
      ExceptionRef e = new Exception(
         "Failed to create module.", "monarch.modest.ModuleCreationFailure");
      Exception::push(e);
   }

   return rval;
}

void ModuleLoader::unloadModule(ModuleInfo* mi)
{
   // free module
   mi->freeModule(mi->module);

   if(mi->handle != NULL)
   {
      // close handle
      dlclose(mi->handle);
   }

   // delete module info
   delete mi;
}
