/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_app_KernelPlugin_h
#define monarch_app_KernelPlugin_h

#include "monarch/app/AppPlugin.h"
#include "monarch/kernel/MicroKernel.h"
#include "monarch/modest/Module.h"

namespace monarch
{
namespace app
{

/**
 * AppPlugin that starts a kernel, loads modules, runs a sub-app with all the
 * modules loaded, waits for plugins to finish running, and either stops or
 * restarts. Options are provided for specifying module paths and kernel
 * parameters.
 *
 * @author David I. Lehn
 */
class KernelPlugin : public AppPlugin
{
protected:
   /**
    * The app state types.
    */
   enum State
   {
      // Node is stopped.
      Stopped,
      // In the process of starting the node.
      Starting,
      // Node has been started and is running.
      Running,
      // In the process of restarting the node.
      Restarting,
      // In the process of stopping the node.
      Stopping
   };

   /**
    * Current app state.
    */
   State mState;

   /**
    * Main application MicroKernel.
    */
   monarch::kernel::MicroKernel* mKernel;

public:
   /**
    * {@inheritDoc}
    */
   KernelPlugin();

   /**
    * {@inheritDoc}
    */
   virtual ~KernelPlugin();

   /**
    * {@inheritDoc}
    */
   virtual bool initMetaConfig(monarch::config::Config& meta);

   /**
    * {@inheritDoc}
    */
   virtual monarch::rt::DynamicObject getCommandLineSpecs();

   /**
    * {@inheritDoc}
    */
   virtual bool didParseCommandLine();

   /**
    * {@inheritDoc}
    */
   virtual bool run();

protected:
   /**
    * Start a kernel, load modules, and start a sub-application.
    *
    * @return true on succes, false and exception set on error
    */
   virtual bool runApp();
};

/**
 * Create a KernelPlugin factory.
 *
 * @return a KernelPlugin factory module.
 */
monarch::modest::Module* createKernelPluginFactory();

} // end namespace app
} // end namespace monarch

#endif
