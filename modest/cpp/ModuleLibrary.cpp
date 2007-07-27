/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "ModuleLibrary.h"
#include "Thread.h"

using namespace std;
using namespace db::modest;
using namespace db::rt;

ModuleLibrary::ModuleLibrary(Kernel* k)
{
   mKernel = k;
}

ModuleLibrary::~ModuleLibrary()
{
   // clean up and free every module
   for(map<string, ModuleInfo*>::iterator i = mModules.begin();
       i != mModules.end(); i++)
   {
      ModuleInfo* mi = i->second;
      mi->module->cleanup(mKernel);
      mLoader.unloadModule(mi);
   }
}

Module* ModuleLibrary::findModule(const string& name)
{
   Module* rval = NULL;
   
   // find module
   map<string, ModuleInfo*>::iterator i = mModules.find(name);
   if(i != mModules.end())
   {
      rval = i->second->module;
   }
   
   return rval;
}

bool ModuleLibrary::loadModule(const string& filename)
{
   bool rval = false;
   
   lock();
   {
      // try to load module
      ModuleInfo* mi = mLoader.loadModule(filename);
      if(mi != NULL)
      {
         // ensure the module isn't already loaded
         if(findModule(mi->module->getId().name) == NULL)
         {
            // add Module to the map
            mModules[mi->module->getId().name] = mi;
            rval = true;
         }
         else
         {
            // module is already loaded, set exception and unload it
            string msg =
               "Could not load module '" + filename +
               "', module named '" + mi->module->getId().name +
               "' with version '" + mi->module->getId().version +
               "' already loaded.";
            Thread::setException(new Exception(msg.c_str()));
            mLoader.unloadModule(mi);
         }
      }
   }
   unlock();
   
   return rval;
}

void ModuleLibrary::unloadModule(const string& name)
{
   lock();
   {
      // find module
      map<string, ModuleInfo*>::iterator i = mModules.find(name);
      if(i != mModules.end())
      {
         // clean up and unload module
         ModuleInfo* mi = i->second;
         mi->module->cleanup(mKernel);
         mLoader.unloadModule(mi);
         
         // erase module from map
         mModules.erase(i);
      }
   }
   unlock();
}

const ModuleId* ModuleLibrary::getModuleId(const std::string& name)
{
   const ModuleId* rval = NULL;
   
   lock();
   {
      // find Module
      Module* m = findModule(name);
      if(m != NULL)
      {
         rval = &m->getId();
      }
   }
   unlock();
   
   return rval;
}

ModuleInterface* ModuleLibrary::getModuleInterface(const string& name)
{
   ModuleInterface* rval = NULL;
   
   lock();
   {
      // find Module
      Module* m = findModule(name);
      if(m != NULL)
      {
         rval = m->getInterface();
      }
   }
   unlock();
   
   return rval;
}
