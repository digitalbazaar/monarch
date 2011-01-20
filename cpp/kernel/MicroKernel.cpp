/*
 * Copyright (c) 2009-2010 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS
#define __STDC_FORMAT_MACROS

#include "monarch/kernel/MicroKernel.h"

#include "monarch/data/json/JsonWriter.h"
#include "monarch/io/File.h"
#include "monarch/io/FileList.h"
#include "monarch/logging/Logging.h"
#include "monarch/rt/System.h"
#include "monarch/util/StringTokenizer.h"
#include "monarch/validation/Validation.h"

#include <algorithm>

using namespace std;
using namespace monarch::config;
using namespace monarch::data::json;
using namespace monarch::event;
using namespace monarch::io;
using namespace monarch::kernel;
using namespace monarch::fiber;
using namespace monarch::logging;
using namespace monarch::modest;
using namespace monarch::net;
using namespace monarch::rt;
namespace v = monarch::validation;

MicroKernel::MicroKernel() :
   mConfigManager(NULL),
   mCleanupConfigManager(false),
   mFiberScheduler(NULL),
   mCleanupFiberScheduler(false),
   mFiberMessageCenter(NULL),
   mCleanupFiberMessageCenter(false),
   mEventController(NULL),
   mCleanupEventController(false),
   mEventDaemon(NULL),
   mCleanupEventDaemon(false),
   mServer(NULL),
   mCleanupServer(false),
   mMaxConnections(100)
{
   // set default maximum auxiliary threads
   setMaxAuxiliaryThreads(100);
}

MicroKernel::~MicroKernel()
{
   MicroKernel::setConfigManager(NULL, false);
   MicroKernel::setFiberScheduler(NULL, false);
   MicroKernel::setFiberMessageCenter(NULL, false);
   MicroKernel::setEventController(NULL, false);
   MicroKernel::setEventDaemon(NULL, false);
   MicroKernel::setServer(NULL, false);
}

bool MicroKernel::start()
{
   bool rval = true;

   // start engine
   getEngine()->start();
   MO_CAT_INFO(MO_KERNEL_CAT,
      "Engine started using %" PRIu32
      " microkernel thread(s) and %" PRIu32 " auxiliary thread(s).",
      mMinRequiredThreads, mMaxAuxThreads);

   // start fiber scheduler if one exists
   if(mFiberScheduler != NULL)
   {
      mFiberScheduler->start(this, mCoresDetected);
      MO_CAT_INFO(MO_KERNEL_CAT,
         "FiberScheduler started using %" PRIu32 " cpu core(s).",
         mCoresDetected);
   }

   // start event controller if one exists
   if(mEventController != NULL)
   {
      mEventController->start(this);
      MO_CAT_INFO(MO_KERNEL_CAT, "EventController started.");
   }

   // start event daemon if one exists
   if(mEventDaemon != NULL)
   {
      mEventDaemon->start(this, mEventController);
      MO_CAT_INFO(MO_KERNEL_CAT, "EventDaemon started.");
   }

   // start server if one exists
   if(mServer != NULL)
   {
      if(mServer->start(this))
      {
         MO_CAT_INFO(MO_KERNEL_CAT, "Server started.");
      }
      else
      {
         // server failed to start, stop microkernel
         MO_CAT_ERROR(MO_KERNEL_CAT, "Server start failed.");
         stop();
         rval = false;
      }
   }

   if(rval && mEventController != NULL)
   {
      // schedule microkernel started event
      Event e;
      e["type"] = "monarch.kernel.MicroKernel.started";
      mEventController->schedule(e);
   }

   return rval;
}

void MicroKernel::stop()
{
   // stop server, if any
   if(mServer != NULL)
   {
      mServer->stop();
      MO_CAT_INFO(MO_KERNEL_CAT, "Server stopped.");
   }

   // stop event daemon, if any
   if(mEventDaemon != NULL)
   {
      mEventDaemon->stop();
      MO_CAT_INFO(MO_KERNEL_CAT, "EventDaemon stopped.");
   }

   // stop event controller, if any
   if(mEventController != NULL)
   {
      mEventController->stop();
      MO_CAT_INFO(MO_KERNEL_CAT, "EventController stopped.");
   }

   // unload all modules
   unloadAllModules();

   // stop fiber scheduler, if any
   if(mFiberScheduler != NULL)
   {
      mFiberScheduler->stop();
      MO_CAT_INFO(MO_KERNEL_CAT, "FiberScheduler stopped.");
   }

   // stop engine
   getEngine()->stop();
   MO_CAT_INFO(MO_KERNEL_CAT, "Engine stopped.");
}

/**
 * Gets a MicroKernelModule from a modest Module, if one exists.
 *
 * @param m the modest Module.
 * @param file the file the modest Module was loaded from, NULL if none.
 * @param mkm to be set to the MicroKernelModule if one was found, NULL if not.
 *
 * @return true if on success, false if an exception occurred.
 */
static bool _getMicroKernelModule(
   Module* m, File* file, MicroKernelModule** mkm)
{
   bool rval = true;

   if(m != NULL)
   {
      const ModuleId& id = m->getId();

      string from;
      if(file != NULL)
      {
         from = " (";
         from.append((*file)->getAbsolutePath());
         from.push_back(')');
      }

      // update modules list if module is MicroKernelModule
      *mkm = dynamic_cast<MicroKernelModule*>(m);
      if(*mkm != NULL)
      {
         MO_CAT_INFO(MO_KERNEL_CAT,
            "Loaded MicroKernel module: \"%s\" version: \"%s\"%s",
            id.name, id.version, from.c_str());
      }
      else
      {
         MO_CAT_INFO(MO_KERNEL_CAT,
            "Loaded Modest module: \"%s\" version: \"%s\"%s",
            id.name, id.version, from.c_str());
      }
   }
   else
   {
      MO_CAT_ERROR(MO_KERNEL_CAT,
         "Exception while loading module: %s.",
         JsonWriter::writeToString(Exception::getAsDynamicObject()).c_str());
      rval = false;
   }

   return rval;
}

Module* MicroKernel::loadModule(const char* filename)
{
   Module* rval = NULL;

   MO_CAT_INFO(MO_KERNEL_CAT, "Loading module from \"%s\"", filename);

   File file(filename);
   if(file->isDirectory())
   {
      ExceptionRef e = new Exception(
         "Could not load module. File is a directory.",
         "monarch.kernel.InvalidModuleFile");
      e->getDetails()["filename"] = filename;
      Exception::set(e);
   }
   else
   {
      ModuleLibrary* lib = getModuleLibrary();
      rval = lib->loadModule(filename);
      MicroKernelModule* m;
      if(_getMicroKernelModule(rval, NULL, &m) && m != NULL)
      {
         ModuleList pending;
         pending.push_back(m);

         // check dependencies and initialize
         ModuleList uninitialized;
         bool pass =
            checkDependencies(pending, uninitialized) &&
            initializeMicroKernelModule(m);
         if(!pass)
         {
            // dependency check or initialize failed, unload module
            unloadModules(uninitialized);
            rval = NULL;
         }
      }
   }

   return rval;
}

MicroKernelModule* MicroKernel::loadMicroKernelModule(const char* filename)
{
   MicroKernelModule* rval = NULL;

   Module* m = loadModule(filename);
   if(m != NULL)
   {
      rval = dynamic_cast<MicroKernelModule*>(m);
      if(rval == NULL)
      {
         // unload non-MicroKernelModule
         getModuleLibrary()->unloadModule(&m->getId());
         ExceptionRef e = new Exception(
            "Module is not a MicroKernelModule.",
            "monarch.kernel.InvalidMicroKernelModuleFile");
         e->getDetails()["filename"] = filename;
         Exception::set(e);
      }
   }

   return rval;
}

bool MicroKernel::loadModules(const char* path)
{
   MO_CAT_INFO(MO_KERNEL_CAT, "Loading modules from \"%s\"", path);

   // split the modules path into individual paths of files/directories
   FileList paths = File::parsePath(path);
   return loadModules(paths);
}

/**
 * Loads a module from a file, putting it into one of two lists: if the module
 * is a micro-kernel module it goes into one list, if not, it goes into
 * another.
 *
 * @param file the file to load the module from.
 * @param lib the module library to load with.
 * @param mkms the list to put MicroKernelModules into.
 * @param nonMkms the list to put non-MicroKernelModules into.
 *
 * @return the Module that was loaded, NULL if an exception occurred.
 */
static Module* _loadModuleFromFile(
   File& file, ModuleLibrary* lib,
   std::list<MicroKernelModule*>& mkms,
   std::list<Module*>& nonMkms)
{
   Module* rval = lib->loadModule(file->getAbsolutePath());
   MicroKernelModule* mkm;
   if(_getMicroKernelModule(rval, &file, &mkm))
   {
      if(mkm != NULL)
      {
         mkms.push_back(mkm);
      }
      else
      {
         nonMkms.push_back(rval);
      }
   }

   return rval;
}

bool MicroKernel::loadModules(FileList& paths)
{
   bool rval = true;

   // create a list of pending MicroKernelModules and a list for
   // non-MicroKernelModules
   ModuleList pending;
   std::list<Module*> nonMkms;

   // iterate over paths (might be files or directories) adding pending modules
   ModuleLibrary* lib = getModuleLibrary();
   IteratorRef<File> pi = paths->getIterator();
   while(rval && pi->hasNext())
   {
      File& file = pi->next();
      if(file->exists())
      {
         if(!file->isDirectory())
         {
            // load from the file or symbolic link
            rval = (_loadModuleFromFile(file, lib, pending, nonMkms) != NULL);
         }
         else
         {
            // list all files in the directory
            // Note: this code intentionally does not recurse directories
            FileList files;
            file->listFiles(files);
            IteratorRef<File> fi = files->getIterator();
            while(rval && fi->hasNext())
            {
               File& f = fi->next();
               if(!f->isDirectory())
               {
                  rval = (_loadModuleFromFile(f, lib, pending, nonMkms) != NULL);
               }
            }
         }
      }
      else
      {
         // log a warning and continue
         MO_CAT_WARNING(MO_KERNEL_CAT,
            "Module path not found: \"%s\"", file->getPath());
      }
   }

   // check dependencies for all pending modules
   ModuleList uninitialized;
   if(rval)
   {
      rval = checkDependencies(pending, uninitialized);
   }

   // iterate over list and initialize successfully loaded modules
   for(ModuleList::iterator i = uninitialized.begin();
       rval && i != uninitialized.end(); ++i)
   {
      rval = initializeMicroKernelModule(*i);
   }

   if(rval)
   {
      MO_CAT_INFO(MO_KERNEL_CAT, "Modules loaded.");
   }
   else
   {
      // all uninitialized modules *must* load and initialize and at least
      // one failed here, so unload any pending or uninitialized modules
      unloadModules(pending);
      unloadModules(uninitialized);

      // unload non-MicroKernelModules
      for(std::list<Module*>::iterator i = nonMkms.begin();
          i != nonMkms.end(); ++i)
      {
         Module* m = *i;
         MO_CAT_INFO(MO_KERNEL_CAT,
            "Cleaning up Modest module: \"%s\" version: \"%s\"",
            m->getId().name, m->getId().version);
         lib->unloadModule(&m->getId());
      }
   }

   return rval;
}

bool MicroKernel::loadModule(CreateModestModuleFn cm, FreeModestModuleFn fm)
{
   bool rval = true;

   ModuleLibrary* lib = getModuleLibrary();
   Module* module = lib->loadModule(cm, fm);
   MicroKernelModule* m;
   rval = _getMicroKernelModule(module, NULL, &m);
   if(rval && m != NULL)
   {
      ModuleList pending;
      pending.push_back(m);

      // check dependencies and initialize
      ModuleList uninitialized;
      rval =
         checkDependencies(pending, uninitialized) &&
         initializeMicroKernelModule(m);
      if(!rval)
      {
         // dependency check or initialize failed, unload module
         unloadModules(uninitialized);
      }
   }

   return rval;
}

bool MicroKernel::unloadModule(const char* name)
{
   ModuleId id(name, NULL);
   return unloadModule(&id);
}

bool MicroKernel::unloadModule(const ModuleId* id)
{
   bool rval = true;

   ModuleLibrary* lib = getModuleLibrary();
   Module* m = lib->getModule(id);
   if(m == NULL)
   {
      // module not found
      rval = false;
   }
   else
   {
      // see if the module is a MicroKernelModule
      MicroKernelModule* mkm = dynamic_cast<MicroKernelModule*>(m);
      if(mkm == NULL)
      {
         // just a vanilla modest module, unload it
         lib->unloadModule(id);
      }
      else
      {
         // find the iterator for the module to remove (assume it exists
         // otherwise there is some bug in the code)
         ModuleList::iterator start = find(
            mModuleList.begin(), mModuleList.end(), mkm);

         // start a list of modules that will not be unloaded (this includes
         // all modules before the target module and possibly some after)
         ModuleList keep;
         for(ModuleList::iterator i = mModuleList.begin(); i != start; ++i)
         {
            keep.push_back(*i);
         }

         // start collecting modules to unload and others to keep
         ModuleList unload;
         for(ModuleList::iterator i = start; i != mModuleList.end(); ++i)
         {
            MicroKernelModule* curr = *i;
            if(curr == mkm)
            {
               // always unload the target module
               unload.push_back(curr);
            }
            else
            {
               // see if the current module will still have its dependencies
               // met by the modules that aren't being removed
               DynamicObject depInfo = curr->getDependencyInfo();
               if(checkDependencyInfo(keep, depInfo))
               {
                  keep.push_back(curr);
               }
               else
               {
                  unload.push_back(curr);
               }
            }
         }

         // unload modules
         unloadModules(unload);
      }
   }

   return rval;
}

Operation MicroKernel::currentOperation()
{
   return getEngine()->getCurrentOperation();
}

MicroKernelModuleApi* MicroKernel::getModuleApi(const char* name)
{
   MicroKernelModuleApi* rval = NULL;

   ModuleId id(name, NULL);
   MicroKernelModule* module = dynamic_cast<MicroKernelModule*>(
      getModuleLibrary()->getModule(&id));
   if(module != NULL)
   {
      rval = module->getApi(this);
   }

   return rval;
}

MicroKernelModuleApi* MicroKernel::getModuleApiByType(const char* type)
{
   MicroKernelModuleApi* rval = NULL;

   // look through dependency info for the type
   for(ModuleList::iterator i = mModuleList.begin();
       i != mModuleList.end(); ++i)
   {
      DynamicObject di = (*i)->getDependencyInfo();
      if(strcmp(di["type"]->getString(), type) == 0)
      {
         rval = (*i)->getApi(this);
      }
   }

   return rval;
}

void MicroKernel::getModuleApisByType(
   const char* type, list<MicroKernelModuleApi*>& apiList)
{
   // look through dependency info for the type
   for(ModuleList::iterator i = mModuleList.begin();
       i != mModuleList.end(); ++i)
   {
      DynamicObject di = (*i)->getDependencyInfo();
      if(strcmp(di["type"]->getString(), type) == 0)
      {
         apiList.push_back((*i)->getApi(this));
      }
   }
}

void MicroKernel::setMaxAuxiliaryThreads(uint32_t count)
{
   mMaxAuxThreads = count;

   // get the number of cpu cores (for optimal fiber scheduling)
   mCoresDetected = System::getCpuCoreCount();

   // set min thread count:
   // 1. fiber scheduler (# cores)
   // 2. event controller (2, one for dispatching, one for handling)
   // 3. event daemon (1)
   mMinRequiredThreads =
      (mFiberScheduler != NULL ? mCoresDetected : 0) +
      (mEventController != NULL ? 2 : 0) +
      (mEventDaemon != NULL ? 1 : 0);
   mEngine->getThreadPool()->setPoolSize(mMinRequiredThreads + count);
}

void MicroKernel::setMaxServerConnections(uint32_t count)
{
   mMaxConnections = count;
   if(mServer != NULL)
   {
      mServer->setMaxConnectionCount(count);
   }
}

void MicroKernel::setConfigManager(ConfigManager* cm, bool cleanup)
{
   if(mCleanupConfigManager && mConfigManager != NULL)
   {
      delete mConfigManager;
   }
   mConfigManager = cm;
   mCleanupConfigManager = cleanup;
}

ConfigManager* MicroKernel::getConfigManager()
{
   return mConfigManager;
}

void MicroKernel::setFiberScheduler(FiberScheduler* fs, bool cleanup)
{
   if(mCleanupFiberScheduler && mFiberScheduler != NULL)
   {
      delete mFiberScheduler;
   }
   mFiberScheduler = fs;
   mCleanupFiberScheduler = cleanup;
}

FiberScheduler* MicroKernel::getFiberScheduler()
{
   return mFiberScheduler;
}

void MicroKernel::setFiberMessageCenter(FiberMessageCenter* fmc, bool cleanup)
{
   if(mCleanupFiberMessageCenter && mFiberMessageCenter != NULL)
   {
      delete mFiberMessageCenter;
   }
   mFiberMessageCenter = fmc;
   mCleanupFiberMessageCenter = cleanup;
}

FiberMessageCenter* MicroKernel::getFiberMessageCenter()
{
   return mFiberMessageCenter;
}

void MicroKernel::setEventController(EventController* ec, bool cleanup)
{
   if(mCleanupEventController && mEventController != NULL)
   {
      delete mEventController;
   }
   mEventController = ec;
   mCleanupEventController = cleanup;
}

EventController* MicroKernel::getEventController()
{
   return mEventController;
}

void MicroKernel::setEventDaemon(EventDaemon* ed, bool cleanup)
{
   if(mCleanupEventDaemon && mEventDaemon != NULL)
   {
      delete mEventDaemon;
   }
   mEventDaemon = ed;
   mCleanupEventDaemon = cleanup;
}

EventDaemon* MicroKernel::getEventDaemon()
{
   return mEventDaemon;
}

void MicroKernel::setServer(Server* s, bool cleanup)
{
   if(mCleanupServer && mServer != NULL)
   {
      delete mServer;
   }
   mServer = s;
   mCleanupServer = cleanup;
}

Server* MicroKernel::getServer()
{
   return mServer;
}

bool MicroKernel::checkDependencyInfo(
   ModuleList& dependencies, DynamicObject& di, DynamicObject* unmet)
{
   bool rval = true;

   // iterate over dependencies until one is not met or until they've
   // all been successfully checked
   DynamicObjectIterator depi = di["dependencies"].getIterator();
   while(rval && depi->hasNext())
   {
      // see if dependency is met
      DynamicObject& dep = depi->next();

      // assume dependency not met yet
      rval = false;

      // depends on a module with a specific name
      if(dep->hasMember("name"))
      {
         // check for a name that matches in dependencies
         for(ModuleList::iterator mi = dependencies.begin();
             !rval && mi != dependencies.end(); ++mi)
         {
            DynamicObject di = (*mi)->getDependencyInfo();
            if(di["name"] == dep["name"])
            {
               // name matches, check version if necessary
               if(!dep->hasMember("version") ||
                  di["version"] == dep["version"])
               {
                  // name and version (if needed) are a match,
                  // so this dependency has been met
                  rval = true;
               }
            }
         }
      }
      // depends on any module of a certain type (typically
      // there is a common interface per type)
      else
      {
         // check for a type that matches in the dependencies list
         for(ModuleList::iterator mi = dependencies.begin();
             !rval && mi != dependencies.end(); ++mi)
         {
            DynamicObject di = (*mi)->getDependencyInfo();
            if(di["type"] == dep["type"])
            {
               // type matches, so this dependency has been met
               rval = true;
            }
         }
      }

      if(unmet != NULL && !rval)
      {
         (*unmet)->append(dep);
      }
   }

   return rval;
}

bool MicroKernel::checkDependencies(
   ModuleList& pending, ModuleList& uninitialized)
{
   bool rval = true;

   MO_CAT_INFO(MO_KERNEL_CAT, "Checking module dependencies...");

   // FIXME: need better validators than just checking for strings
   // ie check major.minor, string length, etc.
   // FIXME: in the future we might include fields for "signer" and
   // "signature" to better protect against loading rogue modules
   v::ValidatorRef v =
      new v::Map(
         "name", new v::Type(String),
         "version", new v::Type(String),
         "type", new v::Type(String),
         "dependencies", new v::All(
            new v::Type(Array),
            new v::Each(new v::Any(
               new v::Map(
                  "name", new v::Type(String),
                  "version", new v::Optional(new v::Type(String)),
                  NULL),
               new v::Map(
                  "type", new v::Type(String),
                  NULL),
               NULL)),
            NULL),
         NULL);
   for(ModuleList::iterator i = pending.begin(); i != pending.end(); ++i)
   {
      DynamicObject depInfo = (*i)->getDependencyInfo();
      if(!v->isValid(depInfo))
      {
         ExceptionRef e = new Exception(
            "Invalid MicroKernelModule dependency information.",
            "monarch.kernel.InvalidDependencyInfo");
         Exception::push(e);
         rval = false;
      }
   }

   if(rval)
   {
      /* There are three lists we work with to determine module load order:
       1. dependencies
       2. uninitialized
       3. pending

       When we begin, the dependencies and uninitialized lists are empty. The
       dependencies list will first be populated with any already-initialized
       modules from mModuleList. Both dependencies and uninitalized will be
       populated with modules that have had their dependencies met but that
       haven't been initialized yet. This is done by removing a module from
       pending and adding it to uninitialized and to dependencies. A module can
       only be removed from pending once all of its dependencies have been
       found in the dependencies list. The pending list will be repeatedly
       iterated until it is empty (having moved all modules into the
       uninitialized list) or until no modules could be moved out of its list
       and into the unintialized list in a single pass.

       In the former case, we are successful and all modules have had their
       dependencies met and they are sorted in an order they can be
       successfully initialized in. In the latter case, at least one module
       could not have its dependencies met and we have error'ed out. */

      // construct the dependencies list from already-initialized modules
      ModuleList dependencies = mModuleList;
      bool moved;
      do
      {
         moved = false;
         for(ModuleList::iterator i = pending.begin(); i != pending.end();)
         {
            // check dependency info for the current pending module
            DynamicObject depInfo = (*i)->getDependencyInfo();
            if(checkDependencyInfo(dependencies, depInfo))
            {
               // all dependencies for the current pending module have been
               // met, so clear it from the pending list and add it to both
               // the dependencies list and the uninitialized list so it can
               // be initialized later
               dependencies.push_back(*i);
               uninitialized.push_back(*i);
               i = pending.erase(i);

               // at least one module has been moved out of pending
               moved = true;
            }
            else
            {
               // dependencies not yet met, so try next pending module
               ++i;
            }
         }
      }
      while(moved);

      // ensure there are no modules with unmet dependencies still pending
      if(!pending.empty())
      {
         ExceptionRef e = new Exception(
            "Could not load module(s). Missing dependencies.",
            "monarch.kernel.MissingModuleDependencies");

         // list all dependency info for pending modules
         for(ModuleList::iterator i = pending.begin(); i != pending.end(); ++i)
         {
            DynamicObject depInfo = (*i)->getDependencyInfo().clone();
            DynamicObject& unmet = depInfo["unmet"];
            unmet->setType(Array);
            checkDependencyInfo(dependencies, depInfo, &unmet);
         }
         Exception::set(e);
         rval = false;
      }
   }

   if(rval)
   {
      MO_CAT_INFO(MO_KERNEL_CAT, "Module dependencies met.");
   }

   return rval;
}

bool MicroKernel::initializeMicroKernelModule(MicroKernelModule* m)
{
   bool rval = true;

   const ModuleId& id = m->getId();

   MO_CAT_INFO(MO_KERNEL_CAT,
      "Initializing MicroKernel module: \"%s\" version: \"%s\"",
      id.name, id.version);

   // add module to list of modules that have attempted initialization
   mModuleList.push_back(m);

   if(m->initialize(this))
   {
      MO_CAT_INFO(MO_KERNEL_CAT,
         "Initialized MicroKernel module: \"%s\" version: \"%s\"",
         id.name, id.version);
   }
   else
   {
      ExceptionRef e = new Exception(
         "Failed to initialize module.",
         "monarch.kernel.ModuleInitializationFailure");
      e->getDetails()["module"] = m->getDependencyInfo();
      Exception::push(e);

      // log exception details
      MO_CAT_ERROR(MO_KERNEL_CAT,
         "Exception while initializing MicroKernel module: %s.",
         JsonWriter::writeToString(
            Exception::getAsDynamicObject()).c_str());
      rval = false;
   }

   return rval;
}

void MicroKernel::unloadModules(ModuleList& modules)
{
   // unload modules in reverse order from the list
   ModuleLibrary* lib = getModuleLibrary();
   while(!modules.empty())
   {
      MicroKernelModule* m = modules.back();

      // if module is in mModuleList then it has been initialized with this
      // MicroKernel and needs corresponding clean up, otherwise it only needs
      // to be unloaded
      ModuleList::iterator i = find(mModuleList.begin(), mModuleList.end(), m);
      if(i != mModuleList.end())
      {
         MO_CAT_INFO(MO_KERNEL_CAT,
            "Cleaning up MicroKernel module: \"%s\" version: \"%s\"",
            m->getId().name, m->getId().version);
         m->cleanup(this);
         mModuleList.erase(i);
      }

      // unload the module
      lib->unloadModule(&m->getId());

      // if this list isn't mModuleList, then remove the unloaded module
      if(&modules != &mModuleList)
      {
         modules.pop_back();
      }
   }
}

void MicroKernel::unloadAllModules()
{
   unloadModules(mModuleList);

   // unload all non-MicroKernelModules
   ModuleLibrary* lib = getModuleLibrary();
   lib->unloadAllModules();
}
