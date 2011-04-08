/*
 * Copyright (c) 2008-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/app/AppRunner.h"

#include "monarch/app/AppConfig.h"
#include "monarch/app/AppFactory.h"
#include "monarch/app/AppTools.h"
#include "monarch/app/CmdLineParser.h"
#include "monarch/data/json/JsonWriter.h"
#include "monarch/event/EventWaiter.h"
#include "monarch/logging/Logging.h"
#include "monarch/io/File.h"
#include "monarch/rt/Platform.h"
#include "monarch/rt/Thread.h"
#include "monarch/util/Random.h"
#include "monarch/validation/Validation.h"

#include <cstdio>

using namespace std;
using namespace monarch::app;
using namespace monarch::config;
using namespace monarch::data::json;
using namespace monarch::event;
using namespace monarch::fiber;
using namespace monarch::io;
using namespace monarch::kernel;
using namespace monarch::modest;
using namespace monarch::net;
using namespace monarch::rt;
using namespace monarch::util;
namespace v = monarch::validation;

#define MONARCH_CORE      "monarch.app.Core"
#define MONARCH_CONFIG    "monarch.app.Config"
#define MONARCH_KERNEL    "monarch.app.Kernel"
#define MONARCH_APP       "monarch.app.App"
#define MONARCH_APP_CL    "monarch.app.App.commandLine"

#define SHUTDOWN_EVENT_TYPE   "monarch.kernel.Kernel.shutdown"
#define RESTART_EVENT_TYPE    "monarch.kernel.Kernel.restart"

AppRunner::AppRunner() :
   mProgramName(NULL),
   mExitStatus(0),
   mKernel(NULL),
   mState(Stopped)
{
   setExitStatus(0);
   setProgramName("(unknown)");
}

AppRunner::~AppRunner()
{
   setProgramName(NULL);
}

void AppRunner::setProgramName(const char* name)
{
   free(mProgramName);
   mProgramName = (name != NULL) ? strdup(name) : NULL;
}

const char* AppRunner::getProgramName()
{
   return mProgramName;
}

void AppRunner::setExitStatus(int exitStatus)
{
   mExitStatus = exitStatus;
}

int AppRunner::getExitStatus()
{
   return mExitStatus;
}

MicroKernel* AppRunner::getKernel()
{
   return mKernel;
}

ConfigManager* AppRunner::getConfigManager()
{
   return mKernel->getConfigManager();
}

Config AppRunner::getConfig()
{
   // get config, but do not cache merged config to prevent config changes
   // from being tracked until absolutely necessary
   return getConfigManager()->getConfig("main", false, false);
}

Config AppRunner::makeConfig(
   ConfigManager::ConfigId id, ConfigManager::ConfigId groupId,
   ConfigManager::ConfigId parentId)
{
   Config rval;
   rval[ConfigManager::VERSION] = MO_DEFAULT_CONFIG_VERSION;
   if(groupId != NULL)
   {
      // set group ID
      rval[ConfigManager::GROUP] = groupId;

      // look up group parent
      if(getConfigManager()->hasConfig(groupId))
      {
         Config raw = getConfigManager()->getConfig(groupId, true);
         if(raw->hasMember(ConfigManager::PARENT))
         {
            rval[ConfigManager::PARENT] = raw[ConfigManager::PARENT];
         }
      }
   }
   if(parentId != NULL && !rval->hasMember(ConfigManager::PARENT))
   {
      rval[ConfigManager::PARENT] = parentId;
   }
   if(id != NULL)
   {
      rval[ConfigManager::ID] = id;
   }

   return rval;
}

Config AppRunner::getMetaConfig()
{
   return mMetaConfig;
}

Timer* AppRunner::getTimer()
{
   return &mTimer;
}

int AppRunner::start(int argc, const char* argv[])
{
   bool success = true;

   // set the default program name
   if(argc != 0)
   {
      setProgramName(argv[0]);
   }

   mState = Starting;
   while(mState == Starting || mState == Restarting)
   {
      // get start time
      mTimer.start();

      // clear meta config
      mMetaConfig->clear();
      mMetaConfig["specs"]->setType(Array);
      mMetaConfig["options"]->setType(Map);

      // parse the command line into options
      CmdLineParser cmdp;
      DynamicObject meta = getMetaConfig();
      DynamicObject& options = meta["commandLine"];
      success = cmdp.parse(argc, argv, options);

      // create the kernel
      mKernel = new MicroKernel();
      mKernel->setConfigManager(new ConfigManager(), true);

      // add supported config versions
      getConfigManager()->addVersion(DB_CONFIG_VERSION_3_0);
      getConfigManager()->addVersion(MO_CONFIG_VERSION_3_0);

      // initialize app configuration
      AppConfig ac;
      success = ac.initialize(this);

      // process each command line spec
      DynamicObjectIterator si = meta["specs"].getIterator();
      while(success && si->hasNext())
      {
         success = cmdp.processSpec(this, si->next(), options);
      }

      // load config files, setup logging
      success = success &&
         ac.loadCommandLineConfigs(this, false) &&
         ac.configureLogging(this);
      if(!success)
      {
         // failed to start
         mState = Stopped;
      }
      else
      {
         // [re]start the kernel
         MO_CAT_INFO(MO_APP_CAT, (mState == Restarting) ?
            "Restarting kernel..." : "Starting kernel...");

         // configure kernel
         Config cfg = getConfig();
         mKernel->setFiberScheduler(new FiberScheduler(), true);
         mKernel->setFiberMessageCenter(new FiberMessageCenter(), true);
         mKernel->setEventController(new EventController(), true);
         mKernel->setEventDaemon(new EventDaemon(), true);
         mKernel->setServer(new Server(), true);
         Config& c = cfg[MONARCH_KERNEL];
         mKernel->setMaxAuxiliaryThreads(c["maxThreadCount"]->getUInt32());
         mKernel->setMaxServerConnections(c["maxConnectionCount"]->getUInt32());

         // start kernel
         success = mKernel->start();
         if(!success)
         {
            MO_CAT_ERROR(MO_APP_CAT, "Kernel start failed: %s",
               JsonWriter::writeToString(
                  Exception::getAsDynamicObject()).c_str());
         }
         else
         {
            MO_CAT_INFO(MO_APP_CAT, "Kernel started.");

            // run app
            mState = Running;
            success = run();

            // stop kernel
            MO_CAT_INFO(MO_APP_CAT, !success ?
               "Stopping kernel due to exception." :
               ((mState == Restarting) ?
                  "Stopping kernel for restart..." :
                  "Stopping kernel..."));
            mKernel->stop();
            MO_CAT_INFO(MO_APP_CAT, "Kernel stopped.");

            // set to stopped unless restarting
            mState = (mState == Stopping) ? Stopped : mState;
         }
      }

      // clean up
      // FIXME: change after adding logger ref-counting
      ac.cleanupLogging(this);
      delete mKernel;
      mKernel = NULL;
   }

   // make sure exit status reflects if an error occurred
   if(!success)
   {
      if(getExitStatus() == EXIT_SUCCESS)
      {
         setExitStatus(EXIT_FAILURE);
      }
   }

   return getExitStatus();
}

int AppRunner::main(int argc, const char* argv[])
{
   int rval = EXIT_FAILURE;
   bool success;

   // seed random
   Random::seed();

   // enable & clear stats early
   DynamicObjectImpl::enableStats(true);
   DynamicObjectImpl::clearStats();

   // FIXME: make enable/disable network initialization configurable?

   // initialize
   success =
      AppTools::initializeNetworking() &&
      monarch::logging::Logging::initialize() &&
      monarch::rt::Platform::initialize();

   // start AppRunner
   if(success)
   {
      AppRunner runner;
      rval = runner.start(argc, argv);
      success = (rval == EXIT_SUCCESS);
   }

   // print exception if exit status reflects an error occurred
   if(!success && Exception::isSet())
   {
      AppTools::printException();
   }

   // clean up
   monarch::rt::Platform::cleanup();
   monarch::logging::Logging::cleanup();
   AppTools::cleanupNetworking();

   // exit main thread
   Thread::exit();

   return rval;
}

/**
 * Loads the App.
 *
 * @param ar the AppRunner to load the App for.
 * @param module to be set to the AppFactory module.
 *
 * @return the App or NULL on failure with exception set.
 */
static App* _loadApp(AppRunner* ar, MicroKernelModule** module)
{
   App* app = NULL;

   // get app path
   MicroKernel* k = ar->getKernel();
   Config cfg = ar->getConfig()[MONARCH_KERNEL];
   if(cfg["appPath"]->length() == 0)
   {
      MO_CAT_INFO(MO_APP_CAT, "No App specified. Loading empty App.");

      // create dummy app
      app = new App();
      *module = NULL;
   }
   else
   {
      MO_CAT_INFO(MO_APP_CAT, "Preparing to load App...");

      // load app
      *module = k->loadMicroKernelModule(cfg["appPath"]->getString());
      if(*module != NULL)
      {
         AppFactory* af = dynamic_cast<AppFactory*>((*module)->getApi(k));
         if(af == NULL)
         {
            // unload module, fail
            k->unloadModule(&(*module)->getId());
            ExceptionRef e = new Exception(
               "Could not load AppFactory.",
               "monarch.app.InvalidApp");
            Exception::set(e);
            *module = NULL;
         }
         else
         {
            // create app
            app = af->createApp();
            if(app == NULL)
            {
               // unload module, fail
               k->unloadModule(&(*module)->getId());
               *module = NULL;
            }
         }
      }
   }

   if(*module != NULL)
   {
      const ModuleId& id = (*module)->getId();
      MO_CAT_INFO(MO_APP_CAT,
         "Loaded AppFactory: \"%s\" version: \"%s\".", id.name, id.version);
      MO_CAT_INFO(MO_APP_CAT, "Loaded App.");
   }

   return app;
}

/**
 * Cleans up an App.
 *
 * @param ar the AppRunner.
 * @param module the AppFactory module.
 * @param app the App to cleanup.
 */
static void _unloadApp(AppRunner* ar, MicroKernelModule* module, App* app)
{
   if(module != NULL)
   {
      MicroKernel* k = ar->getKernel();
      AppFactory* af = dynamic_cast<AppFactory*>(module->getApi(k));
      af->destroyApp(app);
      const ModuleId& id = module->getId();
      MO_CAT_INFO(MO_APP_CAT,
         "Unloading AppFactory: \"%s\" version: \"%s\".",
         id.name, id.version);
      k->unloadModule(&id);
   }
   else
   {
      // delete dummy app
      delete app;
   }
}

/**
 * Validates app wait events.
 *
 * @param waitEvents app wait events.
 *
 * @return true if successful, false if an exception occurred.
 */
static bool _validateWaitEvents(DynamicObject& waitEvents)
{
   bool rval = false;

   // create validator for app wait events
   v::ValidatorRef v = new v::All(
      new v::Type(Array),
      new v::Each(
         new v::Map(
            "id", new v::Type(String),
            "type", new v::Type(String),
            NULL)),
      NULL);
   rval = v->isValid(waitEvents);
   if(!rval)
   {
      ExceptionRef e = new Exception(
         "Invalid App wait event configuration.",
         "monarch.app.InvalidWaitEvents");
      e->getDetails()["waitEvents"] = waitEvents;
      Exception::push(e);
   }

   return rval;
}

/**
 * Prints help and version if specified in the given config.
 *
 * @param ar the AppRunner.
 * @param app the App.
 * @param cfg the main app config.
 *
 * @return true if the help was printed and the app should quit, false if not.
 */
static bool _printHelp(AppRunner* ar, App* app, Config& cfg)
{
   bool quit = false;

   // print help if requested
   if(cfg[MONARCH_CORE]["printHelp"]->getBoolean())
   {
      printf("Usage: %s [options]\n", ar->getProgramName());
      DynamicObjectIterator si = ar->getMetaConfig()["specs"].getIterator();
      while(si->hasNext())
      {
         DynamicObject& spec = si->next();
         if(spec->hasMember("help"))
         {
            printf("%s", spec["help"]->getString());
         }
      }
      quit = true;
   }
   // print version if requested
   if(cfg[MONARCH_CORE]["printVersion"]->getBoolean())
   {
      const char* version = app->getVersion();
      printf("%s%s%s\n",
         app->getName(),
         (version != NULL) ? " v" : "",
         (version != NULL) ? version : "");
      quit = true;
   }

   return quit;
}

bool AppRunner::run()
{
   bool rval = true;
   bool quit = false;

   // for loading main config
   Config cfg(NULL);
   DynamicObject waitEvents(NULL);

   // load app
   MicroKernelModule* module;
   App* app = _loadApp(this, &module);
   rval = (app != NULL);
   if(rval)
   {
      // configure app
      rval = configureApp(app);
      if(rval)
      {
         // main config
         cfg = getConfig();

         // validate wait events
         waitEvents = app->getWaitEvents();
         rval = _validateWaitEvents(waitEvents);

         // print help if requested
         quit = rval && _printHelp(this, app, cfg);
      }
   }

   if(rval && !quit)
   {
      // handle config dump
      if(cfg[MONARCH_CONFIG]["dump"]->getBoolean())
      {
         JsonWriter::writeToStdOut(getConfig());
      }
      if(cfg[MONARCH_CONFIG]["dumpAll"]->getBoolean())
      {
         JsonWriter::writeToStdOut(getConfigManager()->getDebugInfo());
      }
      if(cfg[MONARCH_CONFIG]["dumpMeta"]->getBoolean())
      {
         JsonWriter::writeToStdOut(getMetaConfig());
      }

      /* Collect all module paths so they can be loaded in bulk. This helps
         to avoid issues with needing to specify load order explicitly. */
      FileList modulePaths;
      ConfigIterator mpi = cfg[MONARCH_KERNEL]["modulePath"].getIterator();
      while(rval && mpi->hasNext())
      {
         const char* path = mpi->next()->getString();
         FileList pathList = File::parsePath(path);
         modulePaths->concat(*pathList);
      }
      // load all module paths at once
      rval = mKernel->loadModules(modulePaths);
      if(rval && cfg[MONARCH_KERNEL]["printModuleVersions"]->getBoolean())
      {
         // FIXME: print out module info
         ExceptionRef e = new Exception(
            "Module version printing not implemented.",
            "monarch.app.NotImplemented");
         Exception::set(e);
         rval = false;
      }

      // run app
      rval = rval && runApp(app, waitEvents);
   }

   // clean up app
   if(app != NULL)
   {
      app->cleanup();
      _unloadApp(this, module, app);
   }

   return rval;
}

bool AppRunner::configureApp(App* app)
{
   MO_CAT_INFO(MO_APP_CAT, "Configuring App...");

   // create defaults config for app
   Config defaults = makeConfig(MONARCH_APP ".defaults", "defaults");

   // 1. Initialize app.
   // 2. Initialize configs.
   // 3. Process command line options.
   // 4. Load external config files.
   app->setAppRunner(this);
   bool rval =
      app->initialize() &&
      app->initConfigs(defaults);
   if(rval)
   {
      // create command line config for the app
      Config cfg = makeConfig(MONARCH_APP_CL, "command line");
      DynamicObject meta = getMetaConfig();
      meta["appOptions"] = cfg;

      // get app command line spec
      AppConfig ac;
      CmdLineParser cmdp;
      DynamicObject options = meta["commandLine"];
      DynamicObject spec = app->getCommandLineSpec(cfg);
      getMetaConfig()["specs"]->append(spec);

      // process spec, load configs
      rval =
         cmdp.processSpec(this, spec, options) &&
         cmdp.checkUnknownOptions(options["options"]) &&
         app->willLoadConfigs() &&
         ac.loadCommandLineConfigs(this, true) &&
         app->didLoadConfigs();
   }

   if(rval)
   {
      MO_CAT_INFO(MO_APP_CAT, "App configured.");
   }

   return rval;
}

bool AppRunner::runApp(App* app, DynamicObject& waitEvents)
{
   bool rval = true;

   // send kernel ready event
   {
      Event e;
      e["type"] = "monarch.kernel.Kernel.ready";
      mKernel->getEventController()->schedule(e);
   }

   // create waiter for wait events
   // used to wait for app to complete or for kernel control events
   EventWaiter waiter(mKernel->getEventController());

   // wait for generic kernel events
   MO_CAT_INFO(MO_APP_CAT,
      "EventWaiter: kernel waiting on \"%s\"", SHUTDOWN_EVENT_TYPE);
   waiter.start(SHUTDOWN_EVENT_TYPE);
   MO_CAT_INFO(MO_APP_CAT,
      "EventWaiter: kernel waiting on \"%s\"", RESTART_EVENT_TYPE);
   waiter.start(RESTART_EVENT_TYPE);

   // make a map of event types to waiting ids
   DynamicObject eventMap;
   eventMap->setType(Map);
   {
      // array of events and counts
      DynamicObjectIterator i = waitEvents.getIterator();
      while(i->hasNext())
      {
         DynamicObject next = i->next();
         const char* id = next["id"]->getString();
         const char* type = next["type"]->getString();
         if(!eventMap->hasMember(type))
         {
            DynamicObject newInfo;
            newInfo["ids"]->setType(Array);
            eventMap[type] = newInfo;
         }
         DynamicObject newId;
         newId = id;
         eventMap[type]["ids"]->append(newId);

         // start waiting for event
         MO_CAT_INFO(MO_APP_CAT,
            "EventWaiter: \"%s\" waiting on \"%s\"", id, type);
         waiter.start(type);
      }
   }

   // run app
   rval = rval && app->run();

   // wait for events if app started successfully
   if(rval)
   {
      while(mState == Running && eventMap->length() != 0)
      {
         waiter.waitForEvent();

         // check event type
         Event e = waiter.popEvent();
         const char* type = e["type"]->getString();
         MO_CAT_INFO(MO_APP_CAT, "EventWaiter got event: %s", type);
         if(strcmp(SHUTDOWN_EVENT_TYPE, type) == 0)
         {
            // app stop event
            mState = Stopping;
         }
         else if(strcmp(RESTART_EVENT_TYPE, type) == 0)
         {
            // app restart event
            mState = Restarting;
         }
         else if(eventMap->hasMember(type))
         {
            // event occurred, remove from wait list
            eventMap->removeMember(type);
         }
      }
      if(mState == Running)
      {
         mState = Stopping;
      }
   }

   return rval;
}
