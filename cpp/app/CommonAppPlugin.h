/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_app_CommonAppPlugin_H
#define monarch_app_CommonAppPlugin_H

#include "monarch/app/AppPlugin.h"
#include "monarch/logging/Logging.h"

namespace monarch
{
namespace app
{

/**
 * AppPlugin providing common functionality such as basic app logging.
 *
 * @author David I. Lehn
 */
class CommonAppPlugin : public AppPlugin
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
   CommonAppPlugin();

   /**
    * {@inheritDoc}
    */
   virtual ~CommonAppPlugin();

   /**
    * {@inheritDoc}
    */
   virtual bool willInitMetaConfig(monarch::config::Config& meta);

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
   virtual bool willParseCommandLine(std::vector<const char*>* args);

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
    * {@inheritDoc}
    */
   virtual bool willRun();

   /**
    * {@inheritDoc}
    */
   virtual bool run();

   /**
    * {@inheritDoc}
    */
   virtual bool didRun();
};

} // end namespace app
} // end namespace monarch

#endif
