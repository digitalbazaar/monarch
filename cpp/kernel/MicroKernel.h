/*
 * Copyright (c) 2009-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_kernel_MicroKernel_H
#define monarch_kernel_MicroKernel_H

#include "monarch/config/ConfigManager.h"
#include "monarch/event/EventController.h"
#include "monarch/event/EventDaemon.h"
#include "monarch/fiber/FiberMessageCenter.h"
#include "monarch/fiber/FiberScheduler.h"
#include "monarch/kernel/MicroKernelModule.h"
#include "monarch/modest/Kernel.h"
#include "monarch/net/Server.h"

#include <list>

namespace monarch
{
namespace kernel
{

/**
 * A Monarch MicroKernel is a microkernel in the sense that it allows for
 * arbitrary modular userspace services to extend its functionality. These
 * services are provided by what are called MicroKernelModules. They may
 * contain arbitrary code and can take advantage of the operation, fiber,
 * event scheduling, and server capabilities of the MicroKernel.
 *
 * A MicroKernel extends the basic Modest Kernel by providing the additional
 * capabilities listed above. Its MicroKernelModules also extend Modest
 * Modules by providing a built-in dependency resolution system.
 *
 * @author Dave Longley
 */
class MicroKernel : public monarch::modest::Kernel
{
protected:
   /**
    * ConfigManager for this MicroKernel.
    */
   monarch::config::ConfigManager* mConfigManager;

   /**
    * True to handle cleanup for the ConfigManager.
    */
   bool mCleanupConfigManager;

   /**
    * The FiberScheduler for this MicroKernel.
    */
   monarch::fiber::FiberScheduler* mFiberScheduler;

   /**
    * True to handle cleanup for the FiberScheduler.
    */
   bool mCleanupFiberScheduler;

   /**
    * The fiber message center used by this MicroKernel.
    */
   monarch::fiber::FiberMessageCenter* mFiberMessageCenter;

   /**
    * True to handle cleanup for the FiberMessageCenter.
    */
   bool mCleanupFiberMessageCenter;

   /**
    * EventController for this MicroKernel.
    */
   monarch::event::EventController* mEventController;

   /**
    * True to handle cleanup for the EventController.
    */
   bool mCleanupEventController;

   /**
    * The EventDaemon for this MicroKernel.
    */
   monarch::event::EventDaemon* mEventDaemon;

   /**
    * True to handle cleanup for the EventDaemon.
    */
   bool mCleanupEventDaemon;

   /**
    * The Server for this MicroKernel.
    */
   monarch::net::Server* mServer;

   /**
    * True to handle cleanup for the Server.
    */
   bool mCleanupServer;

   /**
    * A list of all the loaded MicroKernelModules.
    */
   typedef std::list<MicroKernelModule*> ModuleList;
   ModuleList mModuleList;

   /**
    * Stores the number of CPU cores detected.
    */
   uint32_t mCoresDetected;

   /**
    * Stores the minimum number of threads required to run the kernel.
    */
   uint32_t mMinRequiredThreads;

   /**
    * The maximum number of auxiliary threads to permit.
    */
   uint32_t mMaxAuxThreads;

   /**
    * The maximum number of server connections to permit.
    */
   uint32_t mMaxConnections;

public:
   /**
    * Creates a new MicroKernel with no specified ConfigManager, FiberScheduler,
    * FiberMessageCenter, EventController, EventDaemon, or Server. To assign
    * any of these objects to this MicroKernel, call their associated set()
    * method.
    *
    * For instance, to set up a MicroKernel with a ConfigManager and
    * EventController:
    *
    * MicroKernel k;
    * k.setConfigManager(new ConfigManager(), true);
    * k.setEventController(new EventController(), true);
    */
   MicroKernel();

   /**
    * Destructs this MicroKernel.
    */
   virtual ~MicroKernel();

   /**
    * Starts this MicroKernel. To load MicroKernelModules, call loadModules()
    * after a successful start.
    *
    * @return true on success, false on failure with exception set.
    */
   virtual bool start();

   /**
    * Stops this MicroKernel and unloads its modules.
    */
   virtual void stop();

   /**
    * Loads all MicroKernelModules in the current path.
    *
    * @param path the path to use to load the modules.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool loadModules(const char* path);

   /**
    * Loads a MicroKernelModule from a modest Module, checking its dependencies,
    * etc.
    *
    * @param cm the create module function.
    * @param fm the free module function.
    *
    * @return true if it was loaded, false if not (an Exception occurred).
    */
   virtual bool loadModule(CreateModestModuleFn cm, FreeModestModuleFn fm);

   /**
    * Gets the current thread's Operation. *DO NOT* call this if you
    * aren't sure the current thread is on an Operation, it may result
    * in memory corruption. It is safe to call this inside of a btp
    * service or an event handler.
    *
    * @return the current thread's Operation.
    */
   virtual monarch::modest::Operation currentOperation();

   /**
    * Gets the API for the MicroKernelModule with the given name.
    *
    * @param name the name of the MicroKernelModule to get the API for.
    *
    * @return the module's API, or NULL if the module does not exist or it
    *         has no API.
    */
   virtual MicroKernelModuleApi* getModuleApi(const char* name);

   /**
    * Gets the API for the first MicroKernelModule with the given type.
    *
    * @param type the type of MicroKernelModule to get the API for.
    *
    * @return the module's API, or NULL if the module does not exist or it
    *         has no API.
    */
   virtual MicroKernelModuleApi* getModuleApiByType(const char* type);

   /**
    * Gets the APIs for all the MicroKernelModules with the given type.
    *
    * @param type the type of MicroKernelModule to get the APIs for.
    * @param apiList the list to put the API into.
    */
   virtual void getModuleApisByType(
      const char* type, std::list<MicroKernelModuleApi*>& apiList);

   /**
    * Sets the maximum number of auxiliary threads to permit.
    *
    * @param count the number of auxiliary threads to permit.
    */
   virtual void setMaxAuxiliaryThreads(uint32_t count);

   /**
    * Sets the maximum number of server connections to permit.
    *
    * @param count the number of server connections to permit.
    */
   virtual void setMaxServerConnections(uint32_t count);

   /**
    * Sets this MicroKernel's ConfigManager.
    *
    * @param the ConfigManager for this MicroKernel.
    * @param cleanup true to handle clean up.
    */
   virtual void setConfigManager(
      monarch::config::ConfigManager* cm, bool cleanup);

   /**
    * Gets this MicroKernel's ConfigManager.
    *
    * @return the ConfigManager for this MicroKernel.
    */
   virtual monarch::config::ConfigManager* getConfigManager();

   /**
    * Sets this MicroKernel's FiberScheduler.
    *
    * @param fs the FiberScheduler for this MicroKernel.
    * @param cleanup true to handle clean up.
    */
   virtual void setFiberScheduler(
      monarch::fiber::FiberScheduler* fs, bool cleanup);

   /**
    * Gets this MicroKernel's FiberScheduler.
    *
    * @return the FiberScheduler for this MicroKernel.
    */
   virtual monarch::fiber::FiberScheduler* getFiberScheduler();

   /**
    * Sets this MicroKernel's FiberMessageCenter.
    *
    * @param fmc the FiberMessageCenter for this MicroKernel.
    * @param cleanup true to handle clean up.
    */
   virtual void setFiberMessageCenter(
      monarch::fiber::FiberMessageCenter* fmc, bool cleanup);

   /**
    * Gets this MicroKernel's FiberMessageCenter.
    *
    * @return the FiberMessageCenter for this MicroKernel.
    */
   virtual monarch::fiber::FiberMessageCenter* getFiberMessageCenter();

   /**
    * Sets this MicroKernel's EventController.
    *
    * @param ec the EventController for this MicroKernel.
    * @param cleanup true to handle clean up.
    */
   virtual void setEventController(
      monarch::event::EventController* ec, bool cleanup);

   /**
    * Gets this MicroKernel's EventController.
    *
    * @return the EventController for this MicroKernel.
    */
   virtual monarch::event::EventController* getEventController();

   /**
    * Sets this MicroKernel's EventDaemon.
    *
    * @param ed the EventDaemon for this MicroKernel.
    * @param cleanup true to handle clean up.
    */
   virtual void setEventDaemon(
      monarch::event::EventDaemon* ed, bool cleanup);

   /**
    * Gets this MicroKernel's EventDaemon.
    *
    * @return the EventDaemon for this MicroKernel.
    */
   virtual monarch::event::EventDaemon* getEventDaemon();

   /**
    * Sets this MicroKernel's Server.
    *
    * @param s the Server for this MicroKernel.
    * @param cleanup true to handle clean up.
    */
   virtual void setServer(monarch::net::Server* s, bool cleanup);

   /**
    * Gets this MicroKernel's Server.
    *
    * @return the Server for this MicroKernel.
    */
   virtual monarch::net::Server* getServer();

protected:
   /**
    * Checks the dependencies for MicroKernelModules.
    *
    * @param pending the pending ModuleList.
    * @param uninitialized the list to move dependency-met modules into.
    *
    * @return true if all dependencies have been met and the modules are
    *         ready to be initialized, false if not.
    */
   virtual bool checkDependencies(
      ModuleList& pending, ModuleList& uninitialized);

   /**
    * Initializes a MicroKernelModule.
    *
    * @param m the MicroKernelModule to initialize.
    *
    * @return true if successful, false if not.
    */
   virtual bool initializeMicroKernelModule(MicroKernelModule* m);

   /**
    * Unloads all MicroKernelModules in the given list.
    *
    * @param modules the list of modules to unload.
    */
   virtual void unloadModules(ModuleList& modules);

   /**
    * Unloads all modules that have been loaded, including
    * non-MicroKernelModules.
    */
   virtual void unloadAllModules();
};

} // end namespace kernel
} // end namespace monarch
#endif
