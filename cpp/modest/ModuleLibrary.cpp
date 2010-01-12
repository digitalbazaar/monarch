/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/modest/ModuleLibrary.h"

using namespace std;
using namespace monarch::modest;
using namespace monarch::rt;

ModuleLibrary::ModuleLibrary(Kernel* k)
{
   mKernel = k;
}

ModuleLibrary::~ModuleLibrary()
{
   // unload all modules
   ModuleLibrary::unloadAllModules();
}

Module* ModuleLibrary::findModule(const ModuleId* id)
{
   Module* rval = NULL;

   // find module
   ModuleMap::iterator i = mModules.find(id);
   if(i != mModules.end())
   {
      rval = i->second->module;
   }

   return rval;
}

Module* ModuleLibrary::findModule(const char* name)
{
   // find module
   ModuleId id(name);
   return findModule(&id);
}

Module* ModuleLibrary::loadModule(const char* filename)
{
   Module* rval = NULL;

   mLoadLock.lock();
   {
      // try to load module
      ModuleInfo* mi = mLoader.loadModule(filename);
      if(mi != NULL)
      {
         rval = loadOnce(mi, filename);
      }
   }
   mLoadLock.unlock();

   return rval;
}

Module* ModuleLibrary::loadModule(
   CreateModestModuleFn cm, FreeModestModuleFn fm)
{
   Module* rval = NULL;

   mLoadLock.lock();
   {
      // try to load module
      ModuleInfo* mi = mLoader.loadModule(cm, fm);
      if(mi != NULL)
      {
         rval = loadOnce(mi, NULL);
      }
   }
   mLoadLock.unlock();

   return rval;
}

void ModuleLibrary::unloadModule(const ModuleId* id)
{
   mLoadLock.lock();
   {
      // find module
      ModuleMap::iterator i = mModules.find(id);
      if(i != mModules.end())
      {
         // get module
         ModuleInfo* mi = i->second;

         // erase module from map and list
         mModules.erase(i);
         for(ModuleList::iterator li = mLoadOrder.begin();
             li != mLoadOrder.end(); li++)
         {
            if(**li == *id)
            {
               mLoadOrder.erase(li);
               break;
            }
         }

         // clean up and unload module
         mi->module->cleanup(mKernel);
         mLoader.unloadModule(mi);
      }
   }
   mLoadLock.unlock();
}

void ModuleLibrary::unloadAllModules()
{
   mLoadLock.lock();
   {
      // clean up and free every module
      while(!mLoadOrder.empty())
      {
         // find ModuleInfo
         ModuleMap::iterator i = mModules.find(mLoadOrder.back());
         ModuleInfo* mi = i->second;

         // remove module from map and list
         mModules.erase(i);
         mLoadOrder.pop_back();

         // clean up and unload module
         mi->module->cleanup(mKernel);
         mLoader.unloadModule(mi);
      }
   }
   mLoadLock.unlock();
}

Module* ModuleLibrary::getModule(const ModuleId* id)
{
   Module* rval = NULL;

   mLoadLock.lock();
   {
      // find Module
      rval = findModule(id);
   }
   mLoadLock.unlock();

   return rval;
}

const ModuleId* ModuleLibrary::getModuleId(const char* name)
{
   const ModuleId* rval = NULL;

   mLoadLock.lock();
   {
      // find Module
      Module* m = findModule(name);
      if(m != NULL)
      {
         rval = &m->getId();
      }
   }
   mLoadLock.unlock();

   return rval;
}

ModuleInterface* ModuleLibrary::getModuleInterface(const ModuleId* id)
{
   ModuleInterface* rval = NULL;

   mLoadLock.lock();
   {
      // find Module
      Module* m = findModule(id);
      if(m != NULL)
      {
         rval = m->getInterface();
      }
   }
   mLoadLock.unlock();

   return rval;
}

Module* ModuleLibrary::loadOnce(ModuleInfo* mi, const char* filename)
{
   Module* rval = NULL;

   // ensure the module isn't already loaded
   if(findModule(&mi->module->getId()) == NULL)
   {
      // initialize the module
      if(mi->module->initialize(mKernel))
      {
         // add Module to the map and list
         mModules[&mi->module->getId()] = mi;
         mLoadOrder.push_back(&mi->module->getId());
         rval = mi->module;
      }
      else
      {
         // could not initialize module, so unload it
         ExceptionRef e = new Exception(
            "Could not initialize module.",
            "monarch.modest.ModuleInitializationError");
         if(filename != NULL)
         {
            e->getDetails()["filename"] = filename;
         }
         e->getDetails()["name"] = mi->module->getId().name;
         e->getDetails()["version"] = mi->module->getId().version;
         Exception::push(e);
         mLoader.unloadModule(mi);
      }
   }
   else
   {
      // module is already loaded, set exception and unload it
      ExceptionRef e = new Exception(
         "Could not load module, module already loaded.",
         "monarch.modest.DuplicateModule");
      if(filename != NULL)
      {
         e->getDetails()["filename"] = filename;
      }
      e->getDetails()["name"] = mi->module->getId().name;
      e->getDetails()["version"] = mi->module->getId().version;
      Exception::set(e);
      mLoader.unloadModule(mi);
   }

   return rval;
}
