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
#include "monarch/modest/OperationDispatcher.h"
#include "monarch/rt/System.h"
#include "monarch/validation/Validation.h"

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
      " microkernel threads and %" PRIu32 " auxiliary threads.",
      mMinRequiredThreads, mMaxAuxThreads);

   // start fiber scheduler if one exists
   if(mFiberScheduler != NULL)
   {
      mFiberScheduler->start(this, mCoresDetected);
      MO_CAT_INFO(MO_KERNEL_CAT,
         "FiberScheduler started using %" PRIu32 " cpu cores.", mCoresDetected);
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

   // unload modules
   unloadModules();

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
            "Loaded MicroKernel module: %s v%s%s",
            id.name, id.version, from.c_str());
      }
      else
      {
         MO_CAT_INFO(MO_KERNEL_CAT,
            "Loaded Modest module: %s v%s%s",
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

/**
 * Initializes a MicroKernelModule.
 *
 * @param k the MicroKernel.
 * @param m the MicroKernelModule to initialize.
 *
 * @return true if successful, false if not.
 */
static bool _initializeMicroKernelModule(MicroKernel* k, MicroKernelModule* m)
{
   bool rval = true;

   const ModuleId& id = m->getId();

   MO_CAT_INFO(MO_KERNEL_CAT,
      "Initializing MicroKernel module: %s v%s", id.name, id.version);

   if(m->initialize(k))
   {
      MO_CAT_INFO(MO_KERNEL_CAT,
         "Initialized MicroKernel module: %s v%s", id.name, id.version);
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

bool MicroKernel::loadModules(const char* path)
{
   bool rval = true;

   MO_CAT_INFO(MO_KERNEL_CAT, "Loading modules from %s", path);

   // create a list of pending MicroKernelModules
   ModuleList pending;

   // FIXME: split the modules path into individual directories
   // (add support for more than one module directory)
   File moduleDir(path);

   // get a list of all the files in the modules directory
   FileList files;
   moduleDir->listFiles(files);

   // load all modules
   ModuleLibrary* lib = getModuleLibrary();
   IteratorRef<File> i = files->getIterator();
   while(rval && i->hasNext())
   {
      File& file = i->next();
      if(file->isFile())
      {
         Module* module = lib->loadModule(file->getAbsolutePath());
         MicroKernelModule* m;
         rval = _getMicroKernelModule(module, &file, &m);
         if(rval && m != NULL)
         {
            pending.push_back(m);
         }
      }
   }

   if(rval)
   {
      // check dependencies for all pending modules
      rval = checkDependencies(pending);
   }

   // iterate over list and initialize successfully loaded modules
   for(ModuleList::iterator i = mModuleList.begin();
       rval && i != mModuleList.end(); i++)
   {
      rval = _initializeMicroKernelModule(this, *i);
   }

   if(rval)
   {
      MO_CAT_INFO(MO_KERNEL_CAT, "Modules loaded.");
   }
   else
   {
      // unload modules, all modules *must* load and initialize properly
      // to continue and this is not the case here
      unloadModules();
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
      rval =
         checkDependencies(pending) &&
         _initializeMicroKernelModule(this, m);
      if(!rval)
      {
         // dependency check or initialize failed, so unload module
         lib->unloadModule(&m->getId());
      }
   }

   return rval;
}

Operation MicroKernel::currentOperation()
{
   return getEngine()->getOperationDispatcher()->getCurrentOperation();
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
       i != mModuleList.end(); i++)
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
       i != mModuleList.end(); i++)
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

bool MicroKernel::checkDependencies(ModuleList& pending)
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
   for(ModuleList::iterator i = pending.begin(); i != pending.end(); i++)
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
      /* There are two lists we work with to determine module load order:
       1. mModuleList
       2. pending

       When we begin, mModuleList is empty. It will be populated with modules
       that have had their dependencies met. This is done by removing a module
       from the pending list and adding it to mModuleList. A module can only be
       removed from that list once all of its dependencies have been found in
       mModuleList. The pending list will be repeatedly iterated until it is
       empty (having moved all modules into mModuleList) or until no modules
       could be moved out of its list and into mModuleList in a single pass.

       In the former case, we are successful and all modules have had their
       dependencies met and they are sorted in an order they can be
       successfully initialized in. In the latter case, at least one module
       could not have its dependencies met and we have error'ed out.
       */
      bool moved;
      do
      {
         moved = false;
         for(ModuleList::iterator i = pending.begin(); i != pending.end();)
         {
            // get dependency info for the current pending module
            DynamicObject depInfo = (*i)->getDependencyInfo();

            // default to all dependencies met... change if one is found that
            // has not been met
            bool met = true;

            // iterate over dependencies until one is not met or until they've
            // all been successfully checked
            DynamicObjectIterator depi = depInfo["dependencies"].getIterator();
            while(met && depi->hasNext())
            {
               DynamicObject& dep = depi->next();

               // dependency not yet met
               met = false;

               // depends on a module with a specific name
               if(dep->hasMember("name"))
               {
                  // check for a name that matches in mModulesList
                  for(ModuleList::iterator mi = mModuleList.begin();
                      !met && mi != mModuleList.end(); mi++)
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
                           met = true;
                        }
                     }
                  }
               }
               // depends on any module of a certain type (typically
               // there is a common interface per type)
               else
               {
                  // check for a type that matches in the mModulesControl list
                  for(ModuleList::iterator mi = mModuleList.begin();
                      !met && mi != mModuleList.end(); mi++)
                  {
                     DynamicObject di = (*mi)->getDependencyInfo();
                     if(di["type"] == dep["type"])
                     {
                        // type matches, so this dependency has been met
                        met = true;
                     }
                  }
               }
            }

            if(met)
            {
               // all dependencies for the current pending module have been
               // met, so clear it from the pending list and add it to
               // mModuleList so it can be initialized later
               mModuleList.push_back(*i);
               i = pending.erase(i);

               // at least one module has been moved out of pending
               moved = true;
            }
            else
            {
               // dependencies not yet met, so try next pending module
               i++;
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
         for(ModuleList::iterator i = pending.begin(); i != pending.end(); i++)
         {
            DynamicObject depInfo = (*i)->getDependencyInfo();
            e->getDetails()["failures"]->append(depInfo);
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

void MicroKernel::unloadModules()
{
   // iterate over list and clean up modules -- doing so in reverse order
   for(ModuleList::reverse_iterator ri = mModuleList.rbegin();
       ri != mModuleList.rend(); ri++)
   {
      MicroKernelModule* m = *ri;
      MO_CAT_INFO(MO_KERNEL_CAT,
         "Cleaning up MicroKernel module: %s v%s",
         m->getId().name, m->getId().version);
      m->cleanup(this);
   }

   // clear list
   mModuleList.clear();

   // unload all modules
   ModuleLibrary* lib = getModuleLibrary();
   lib->unloadAllModules();
}
