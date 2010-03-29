/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_CONSTANT_MACROS

#include <cstdlib>
#include <cstdio>

#include "monarch/app/App.h"
#include "monarch/app/AppPluginFactory.h"
#include "monarch/app/ConfigPlugin.h"
#include "monarch/app/LoggingPlugin.h"
#include "monarch/app/MonarchPlugin.h"
#include "monarch/app/KernelPlugin.h"
#include "monarch/data/json/JsonWriter.h"
#include "monarch/event/EventWaiter.h"
#include "monarch/logging/Logging.h"

#include "monarch/app/KernelPlugin.h"

using namespace std;
using namespace monarch::app;
using namespace monarch::config;
using namespace monarch::data::json;
using namespace monarch::event;
using namespace monarch::fiber;
using namespace monarch::io;
using namespace monarch::kernel;
using namespace monarch::logging;
using namespace monarch::modest;
using namespace monarch::net;
using namespace monarch::rt;

#define PLUGIN_NAME "monarch.app.Kernel"
#define PLUGIN_CL_CFG_ID PLUGIN_NAME ".commandLine"


KernelPlugin::KernelPlugin() :
   mState(Stopped),
   mKernel(NULL)
{
}

KernelPlugin::~KernelPlugin()
{
}

bool KernelPlugin::initMetaConfig(Config& meta)
{
   bool rval = monarch::app::AppPlugin::initMetaConfig(meta);

   // defaults
   if(rval)
   {
      Config& c =
         App::makeMetaConfig(meta, PLUGIN_NAME ".defaults", "defaults")
            [ConfigManager::MERGE][PLUGIN_NAME];
      // modulePath is a map of arrays
      // Keys are unique per module path source to allow for configured other
      // lists of paths. Values are arrays of files or dirs to load.
      c["modulePath"]->setType(Map);
      c["env"] = true;
      c["printModuleVersions"] = false;
      c["maxThreadCount"] = UINT32_C(100);
      c["maxConnectionCount"] = UINT32_C(100);
      // waitEvents is a map of arrays of event ids. The map keys should be
      // unique such as plugin ids. The kernel will wait for all these events
      // to occur before exiting. (Some special kernel events also can cause
      // a quicker exit.)
      c["waitEvents"]->setType(Map);
   }

   // command line options
   if(rval)
   {
      Config& c = App::makeMetaConfig(
         meta, PLUGIN_CL_CFG_ID, "command line", "options")
            [ConfigManager::MERGE][PLUGIN_NAME];
      c["modulePath"][PLUGIN_CL_CFG_ID]->setType(Array);
   }

   return rval;
}

DynamicObject KernelPlugin::getCommandLineSpecs()
{
   DynamicObject spec;
   spec["help"] =
"Module options:\n"
"  -m, --module-path PATH\n"
"                      A colon separated list of modules or directories where\n"
"                      modules are stored. May be specified multiple times.\n"
"                      Loaded after modules in MONARCH_MODULE_PATH.\n"
"      --no-module-path-env\n"
"                      Disable MONARCH_MODULE_PATH.\n"
"      --module-versions\n"
"                      Prints the module versions.\n"
"\n";

   DynamicObject opt;
   Config& options = getApp()->getMetaConfig()
      ["options"][PLUGIN_CL_CFG_ID][ConfigManager::MERGE];
   Config& configOptions = options[PLUGIN_NAME];

   opt = spec["options"]->append();
   opt["short"] = "-m";
   opt["long"] = "--module-path";
   opt["append"] = configOptions["modulePath"][PLUGIN_CL_CFG_ID];
   opt["argError"] = "No path specified.";

   opt = spec["options"]->append();
   opt["long"] = "--no-module-path-env";
   opt["setFalse"]["root"] = configOptions;
   opt["setFalse"]["path"] = "env";

   opt = spec["options"]->append();
   opt["long"] = "--module-versions";
   opt["setTrue"]["root"] = configOptions;
   opt["setTrue"]["path"] = "printModuleVersions";

   DynamicObject specs = AppPlugin::getCommandLineSpecs();
   specs->append(spec);
   return specs;
}

bool KernelPlugin::didParseCommandLine()
{
   bool rval = AppPlugin::didParseCommandLine();

   // process flags
   // only done after bootstrap mode so that all modules info is available
   if(rval && getApp()->getMode() != App::BOOTSTRAP)
   {
      Config& cfg = getApp()->getMetaConfig()
         ["options"][PLUGIN_CL_CFG_ID][ConfigManager::MERGE][PLUGIN_NAME];

      if(cfg["printModuleVersions"]->getBoolean())
      {
         // FIXME: print out module info
         /*
         printf("%s v%s\n", ...);
         // Raise known exit exception.
         ExceptionRef e = new Exception(
            "Module versions printed.",
            "monarch.app.Exit", EXIT_SUCCESS);
         Exception::set(e);
         rval = false;
         */
         ExceptionRef e = new Exception(
            "Not implemented.",
            "monarch.app.NotImplemented", EXIT_FAILURE);
         Exception::set(e);
         rval = false;
      }
   }

   return rval;
}

bool KernelPlugin::runApp()
{
   bool rval = true;

   mState = Starting;
   while(mState == Starting || mState == Restarting)
   {
      // [re]start the node
      MO_CAT_INFO(MO_KERNEL_CAT,
         (mState == Restarting) ?
            "Restarting kernel..." : "Starting kernel...");

      // get kernel config
      Config cfg = getApp()->getConfig()[PLUGIN_NAME];
      App* app = new App;

      // create and start kernel
      mKernel = new MicroKernel();
      mKernel->setConfigManager(app->getConfigManager(), false);
      mKernel->setFiberScheduler(new FiberScheduler(), true);
      mKernel->setFiberMessageCenter(new FiberMessageCenter(), true);
      mKernel->setEventController(new EventController(), true);
      mKernel->setEventDaemon(new EventDaemon(), true);
      mKernel->setServer(new Server(), true);

      // set thread and connection limits
      mKernel->setMaxAuxiliaryThreads(cfg["maxThreadCount"]->getUInt32());
      mKernel->setMaxServerConnections(cfg["maxConnectionCount"]->getUInt32());

      rval = mKernel->start();
      if(rval)
      {
         rval =
            mKernel->loadModule(
               createMonarchPluginFactory, freeAppPluginFactory) &&
            mKernel->loadModule(
               createConfigPluginFactory, freeAppPluginFactory) &&
            mKernel->loadModule(
               createLoggingPluginFactory, freeAppPluginFactory) &&
            mKernel->loadModule(
               createKernelPluginFactory, freeAppPluginFactory);

         // FIXME: in did load configs should add env paths to config

         // Collect all module paths so they can be loaded in bulk.
         // This helps to avoid issues with needing to specify load order
         // explicitly.
         FileList modulePaths;
         ConfigIterator mpMods = cfg["modulePath"].getIterator();
         while(rval && mpMods->hasNext())
         {
            ConfigIterator mpPaths = mpMods->next().getIterator();
            while(rval && mpPaths->hasNext())
            {
               const char* path = mpPaths->next()->getString();
               FileList pathList = File::parsePath(path);
               modulePaths->concat(*pathList);
            }
         }
         // load all module paths at once
         rval = mKernel->loadModules(modulePaths);

         if(!rval)
         {
            MO_CAT_INFO(MO_KERNEL_CAT, "Stopping kernel due to exception.");
            mKernel->stop();
         }
      }
      mState = Running;

      if(rval)
      {
         MO_CAT_INFO(MO_KERNEL_CAT, "Kernel started.");

         // send ready event
         {
            Event e;
            e["type"] = "monarch.kernel.Kernel.ready";
            mKernel->getEventController()->schedule(e);
         }

         if(rval)
         {
            {
               // create AppPlugins from all loaded AppPluginFactories
               MicroKernel::ModuleApiList factories;
               mKernel->getModuleApisByType(
                  "monarch.app.AppPluginFactory", factories);
               for(MicroKernel::ModuleApiList::iterator i = factories.begin();
                  rval && i != factories.end(); i++)
               {
                  ModuleId id = dynamic_cast<Module*>(*i)->getId();
                  AppPluginFactory* f = dynamic_cast<AppPluginFactory*>(*i);
                  AppPluginRef p = f->createAppPlugin();
                  MO_CAT_OBJECT_INFO(
                     MO_KERNEL_CAT, app,
                     "Adding AppPlugin to App: %s v%s.", id.name, id.version);
                  rval = app->addPlugin(p);
               }
            }

            // waiter for kernel and plugin events
            // used to wait for plugins to complete or for kernel control events
            EventWaiter waiter(mKernel->getEventController());
            // wait for generic kernel events
            waiter.start("monarch.kernel.Kernel.shutdown");
            waiter.start("monarch.kernel.Kernel.restart");

            // collect event counts and start waiting for them
            // make a map of event ids to a counter
            DynamicObject waitEvents;
            waitEvents->setType(Map);
            // get the event config
            {
               // map of arrays of event ids
               Config cfgWaitEvents =
                  getApp()->getConfig()[PLUGIN_NAME]["waitEvents"];
               DynamicObjectIterator i = cfgWaitEvents.getIterator();
               while(i->hasNext())
               {
                  DynamicObjectIterator ii = i->next().getIterator();
                  while(ii->hasNext())
                  {
                     const char* type = i->next()->getString();
                     uint32_t count = 1;
                     if(waitEvents->hasMember(type))
                     {
                        count += waitEvents[type]->getUInt32();
                     }
                     waitEvents[type] = count;
                     // first time seeing event, start waiting for it
                     if(count == 1)
                     {
                        waiter.start(type);
                     }
                  }
               }
            }

            int status;
            if(rval)
            {
               // run sub app
               status = app->start(getApp()->getCommandLine());
               rval = (status == EXIT_SUCCESS);
            }

            // wait for events if app started successfully
            // checking for exception in case of success with an exit exception
            if(rval && !Exception::isSet())
            {
               while(mState == Running && waitEvents->length() != 0)
               {
                  waiter.waitForEvent();
                  Event e = waiter.popEvent();
                  const char* type = e["type"]->getString();
                  MO_CAT_INFO(MO_KERNEL_CAT,
                     "EventWaiter got event: %s", type);
                  if(strcmp("monarch.kernel.Kernel.shutdown", type) == 0)
                  {
                     mState = Stopping;
                  }
                  else if(strcmp("monarch.kernel.Kernel.restart", type) == 0)
                  {
                     mState = Restarting;
                  }
                  else
                  {
                     if(waitEvents->hasMember(type))
                     {
                        uint32_t count = waitEvents[type]->getUInt32();
                        count--;
                        if(count == 0)
                        {
                           waitEvents->removeMember(type);
                        }
                        else
                        {
                           waitEvents[type] = count;
                        }
                     }
                  }
               }
               mState = Stopping;
            }

            if(!rval)
            {
               getApp()->setExitStatus(app->getExitStatus());
            }
         }

         delete app;
         app = NULL;

         // FIXME: actually stopping microkernel, not just node
         // stop node
         MO_CAT_INFO(MO_KERNEL_CAT,
            (mState == Restarting) ?
               "Stopping kernel for restart..." :
               "Stopping kernel...");
         mKernel->stop();
         MO_CAT_INFO(MO_KERNEL_CAT, "Kernel stopped.");
         // set to stopped unless restarting
         mState = (mState == Stopping) ? Stopped : mState;
      }
      else
      {
         MO_CAT_ERROR(MO_KERNEL_CAT, "Kernel start failed: %s",
            JsonWriter::writeToString(Exception::getAsDynamicObject()).c_str());
      }

      if(app != NULL)
      {
         delete app;
         app = NULL;
      }

      // clean up kernel
      delete mKernel;
      mKernel = NULL;
   }

   return rval;
}

bool KernelPlugin::run()
{
   bool rval = AppPlugin::run();
   if(rval && getApp()->getMode() == App::BOOTSTRAP)
   {
      rval = runApp();
   }
   return rval;
}

class KernelPluginFactory :
   public AppPluginFactory
{
public:
   KernelPluginFactory() :
      AppPluginFactory(PLUGIN_NAME, "1.0")
   {
      addDependency("monarch.app.Config", "1.0");
      addDependency("monarch.app.Logging", "1.0");
   }

   virtual ~KernelPluginFactory() {}

   virtual AppPluginRef createAppPlugin()
   {
      return new KernelPlugin();
   }
};

Module* monarch::app::createKernelPluginFactory()
{
   return new KernelPluginFactory();
}
