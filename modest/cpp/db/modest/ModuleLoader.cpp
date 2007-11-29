/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/modest/ModuleLoader.h"
#include "db/modest/ModuleApi.h"
#include "db/rt/DynamicLibrary.h"

using namespace std;
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
         string msg = "Could not load module '";
         msg.append(filename);
         msg.append("', error=");
         msg.append(error);
         Exception::setLast(new Exception(msg.c_str()));
      }
   }
   else
   {
      // failed to open module
      char* error = dlerror();
      string msg = "Could not load module '";
      msg.append(filename);
      msg.append("', could not open module file, error=");
      msg.append(error);
      Exception::setLast(new Exception(msg.c_str()));
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
