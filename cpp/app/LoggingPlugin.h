/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_app_LoggingPlugin_h
#define monarch_app_LoggingPlugin_h

#include "monarch/app/AppPlugin.h"
#include "monarch/logging/Logging.h"
#include "monarch/logging/FileLogger.h"
#include "monarch/modest/Module.h"

namespace monarch
{
namespace app
{

/**
 * AppPlugin providing logging functionality.
 *
 * @author David I. Lehn
 */
class LoggingPlugin : public AppPlugin
{
protected:
   /**
    * The default logger.
    */
   monarch::logging::Logger* mLogger;

public:
   /**
    * {@inheritDoc}
    */
   LoggingPlugin();

   /**
    * {@inheritDoc}
    */
   virtual ~LoggingPlugin();

   /**
    * {@inheritDoc}
    */
   virtual bool initialize();

   /**
    * {@inheritDoc}
    */
   virtual void cleanup();

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
   virtual bool initializeLogging();

   /**
    * {@inheritDoc}
    */
   virtual bool cleanupLogging();

   /**
    * Get the current FileLogger or NULL if the current logger is not a
    * FileLogger or is not yet set.
    *
    * @return the FileLogger or NULL
    */
   virtual monarch::logging::FileLogger* getFileLogger();
};

/**
 * Create a LoggingPlugin factory.
 *
 * @return a LoggingPlugin factory module.
 */
monarch::modest::Module* createLoggingPluginFactory();

} // end namespace app
} // end namespace monarch

#endif
