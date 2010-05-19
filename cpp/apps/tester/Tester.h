/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_apps_tester_Tester_h
#define monarch_apps_tester_Tester_h

#include "monarch/app/AppPlugin.h"
#include "monarch/test/TestRunner.h"

namespace monarch
{
namespace apps
{
namespace tester
{

/**
 * AppPlugin which runs tests in modules of type "monarch.test.TestModule".
 * Test modules MUST implement the monarch::test::Testable interface.
 *
 * To use this class to perform testing, implement and load a module that
 * implements the Testable interface. The monarch::test::TestModule header
 * provides a simple set of macros (MO_TEST_MODULE_*) that can be used to
 * simplify the process. They will setup the proper module functions and allow
 * modules to just implement a single static testing function that takes a
 * TestRunner.
 *
 * Tester command line options can be used to control what the TestRunner
 * tests. By default the "default" test is enabled. However, you can
 * specify the particular modules and named tests to run. Modules should use
 * the TestRunner API to check if they should run certain tests.
 *
 * Author: David I. Lehn <dlehn@digitalbazaar.com>
 */
class Tester : public monarch::app::AppPlugin
{
public:
   /**
    * Create a Tester.
    */
   Tester();

   /**
    * Deconstruct this Tester.
    */
   virtual ~Tester();

   /**
    * {@inheritDoc}
    */
   virtual bool initConfigs(monarch::config::Config& defaults);

   /**
    * {@inheritDoc}
    */
   virtual monarch::rt::DynamicObject getCommandLineSpec(
      monarch::config::Config& cfg);

   /**
    * Runs all tests and sets exit status.
    *
    * @return true on success, false and exception set on failure.
    */
   virtual bool run();
};

} // end namespace tester
} // end namespace apps
} // end namespace monarch

#endif
