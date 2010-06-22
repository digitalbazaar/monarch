/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_test_TestLoader_h
#define monarch_test_TestLoader_h

#include "monarch/app/App.h"
#include "monarch/test/TestRunner.h"

namespace monarch
{
namespace test
{

/**
 * A TestLoader is used to load and run test modules. It can update an App's
 * command line spec to add options for loading test modules or specifying
 * specific tests to run via the command line.
 *
 * An App that wishes to provide the ability to run tests loaded from modules
 * should create a TestLoader.
 *
 * @author Dave Longley
 * @author David I. Lehn <dlehn@digitalbazaar.com>
 */
class TestLoader
{
public:
   /**
    * Create a TestLoader.
    */
   TestLoader();

   /**
    * Deconstruct this TestLoader.
    */
   virtual ~TestLoader();

   /**
    * Initializes the configs for this TestLoader. Should be called by an App
    * when it is initializing its configs.
    *
    * @param defaults the App defaults.
    *
    * @return true on success, false on failure.
    */
   virtual bool initConfigs(monarch::config::Config& defaults);

   /**
    * Adds TestLoader command line options to the passed command line config.
    *
    * Should be called by an App when it is creating its command line spec if
    * it wants to add options to control test modules.
    *
    * @param cfg the App's command line config.
    */
   virtual monarch::rt::DynamicObject getCommandLineSpec(
      monarch::config::Config& cfg);

   /**
    * Runs all tests or those specified via command line options.
    *
    * @param app the App.
    *
    * @return true on success, false and exception set on failure.
    */
   virtual bool run(monarch::app::App* app);
};

} // end namespace test
} // end namespace monarch

#endif
