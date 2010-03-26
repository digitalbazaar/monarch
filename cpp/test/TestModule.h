/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_test_TestModule_h
#define monarch_test_TestModule_h

#include "monarch/kernel/MicroKernelModule.h"
#include "monarch/modest/Module.h"
#include "monarch/test/Testable.h"
#include "monarch/test/TestRunner.h"

namespace monarch
{
namespace test
{

/**
 * A basic module that can be used to setup testing. This module can either be
 * subclassed and testing done in run() or can be used as a proxy to call a
 * simple testing function.
 *
 * @author David I. Lehn <dlehn@digitalbazaar.com>
 */
class TestModule :
   public monarch::kernel::MicroKernelModule,
   public monarch::kernel::MicroKernelModuleApi,
   public Testable
{
public:
   /**
    * Simple function that can be used similar to the Testable interface.
    */
   typedef bool (*RunTestRunnerFn)(monarch::test::TestRunner&);

protected:
   /**
    * Info for this plugin.
    */
   monarch::rt::DynamicObject mInfo;

   /**
    * Function to run
    */
   RunTestRunnerFn mRun;

public:
   /**
    * Create an TestModule instance.
    *
    * @param name the name for this MicroKernelModule.
    * @param version the version for this MicroKernelModule (major.minor).
    */
   TestModule(const char* name, const char* version);

   /**
    * Deconstruct this TestModule instance.
    */
   virtual ~TestModule();

   /**
    * Add a dependency.
    *
    * @param name the module name of the dependency.
    * @param version the module version of the dependency.
    */
   virtual void addDependency(const char* name, const char* version);

   /**
    * Gets dependency information.
    *
    * @return the dependency information.
    */
   virtual monarch::rt::DynamicObject getDependencyInfo();

   /**
    * Initializes this Module with the passed MicroKernel.
    *
    * @param k the MicroKernel.
    *
    * @return true if initialized, false if an Exception occurred.
    */
   virtual bool initialize(monarch::kernel::MicroKernel* k);

   /**
    * Cleans up this Module just prior to its unloading.
    *
    * @param k the MicroKernel.
    */
   virtual void cleanup(monarch::kernel::MicroKernel* k);

   /**
    * Gets the API for this MicroKernelModule.
    *
    * @param k the MicroKernel that loaded this module.
    *
    * @return the API for this MicroKernelModule.
    */
   virtual monarch::kernel::MicroKernelModuleApi* getApi(
      monarch::kernel::MicroKernel* k);

   /**
    * Creates a reference counted AppPlugin object.
    *
    * @return the created AppPlugin object or NULL if an exception occurred.
    */
   virtual void setTestFunction(RunTestRunnerFn fn);

   /**
    * Run tests with the given TestRunner. This method may be overloaded in
    * subclasses otherwise the default behavior is to call the test function if
    * one has been set with setTestFunction().
    *
    * @param tr a TestRunner to use for testing.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool run(monarch::test::TestRunner& tr);
};

#define MO_TEST_MODULE_CLASS_FN(clazz, name, version, fn)        \
monarch::modest::Module* createModestModule()                    \
{                                                                \
   monarch::test::TestModule* module = new clazz(name, version); \
   module->setTestFunction(fn);                                  \
   return module;                                                \
}                                                                \
                                                                 \
void freeModestModule(monarch::modest::Module* m)                \
{                                                                \
   delete m;                                                     \
}

#define MO_TEST_MODULE_CLASS(clazz, name, version) \
   MO_TEST_MODULE_CLASS_FN(clazz, name, version, NULL)

#define MO_TEST_MODULE_FN(name, version, fn) \
   MO_TEST_MODULE_CLASS_FN(monarch::test::TestModule, name, version, fn)


} // end namespace test
} // end namespace monarch

#endif
