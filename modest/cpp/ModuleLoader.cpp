/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "ModuleLoader.h"
#include "DynamicLibrary.h"

using namespace std;
using namespace db::modest;
using namespace db::rt;

ModuleLoader::ModuleLoader()
{
}

ModuleLoader::~ModuleLoader()
{
}

ModuleInfo* ModuleLoader::loadModule(std::string const& filename)
{
   ModuleInfo* rval = NULL;
   
   // open library
   void* handle = dlopen(filename.c_str(), RTLD_NOW);
   if(handle != NULL)
   {
      // clear error
      char* error = dlerror();
      CreateModestModuleFn create;
      FreeModestModuleFn free;
      
      // try to get create module function
      create = (CreateModestModuleFn)dlsym(handle, "createModestModule");
      if((error = dlerror()) == NULL)
      {
         // clear error
         char* error = dlerror();
         
         // try to get free module function
         free = (FreeModestModuleFn)dlsym(handle, "freeModestModule");
         error = dlerror();
      }
      
      if(error != NULL)
      {
         // create ModuleInfo
         rval = new ModuleInfo();
         rval->module = create();
         rval->freeModule = free;
      }
      else
      {
         // could not load create or free functions
         string msg = "Could not load module '" + filename + "'";
         Exception::setLast(new Exception(msg.c_str()));
      }
   }
   else
   {
      // failed to open module
      string msg = "Could not load module '" + filename + "'";
      Exception::setLast(new Exception(msg.c_str()));
   }
   
   return rval;
}

void ModuleLoader::unloadModule(ModuleInfo* mi)
{
   // free module
   mi->freeModule(mi->module);
   
   // delete module info
   delete mi;
   mi = NULL;
}
