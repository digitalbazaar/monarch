/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/modest/ModuleLoader.h"
#include "db/modest/ModuleApi.h"
#include "db/rt/DynamicLibrary.h"

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
   void* handle = dlopen(filename, RTLD_NOW);
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
         char temp[100 + strlen(filename) + strlen(error)];
         sprintf(temp, "Could not load module '%s', error=%s", filename, error);
         ExceptionRef e = new Exception(temp, "db.modest.BadModule");
         Exception::setLast(e, false);
      }
   }
   else
   {
      // failed to open module
      char* error = dlerror();
      char temp[100 + strlen(filename) + strlen(error)];
      sprintf(temp,
         "Could not load module '%s', could not open module file, error=%s",
         filename, error);
      ExceptionRef e = new Exception(temp, "db.modest.BadModuleFile");
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
