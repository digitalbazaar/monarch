/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_util_App_H
#define db_util_App_H

#include "db/rt/Runnable.h"
#include "db/rt/Exception.h"

namespace db
{
namespace util
{

/**
 * Top-level class to make running applications easier.
 * 
 * To use: create a App subclass, override the appropriate methods, and use a
 * main() function which works similar to the following or use the
 * DB_APP_MAIN(AppClassName) macro.
 * 
 * int main(int argc, const char* argv[])
 * {
 *    int rval;
 *    AppClassName app();
 *    return app.main(argc, argv);
 * }
 * 
 * The common order that overridden App methods are called is:
 * - parseCommandLine(argc, argv)
 * - initializeLogging()
 * - initialize()
 * - run()
 * - cleanup()
 * - cleanupLogging()
 * 
 * Author: David I. Lehn
 */
class App : public virtual db::rt::Object, public db::rt::Runnable
{
protected:
   /**
    * Program name for this App.  Taken from the command line args.
    */
   char* mProgramName;
   
   /**
    * Name for this App
    */
   char* mName;
   
   /**
    * Exit status to use for all tests.
    */  
   int mExitStatus;
   
   /**
    * Pretty print an exception.
    */
   void printException(db::rt::ExceptionRef& e);

   /**
    * Pretty print last exception.
    */
   void printException();
   
public:
   /**
    * Create an App instance.
    */
   App();
   
   /**
    * Deconstruct this App instance.
    */
   virtual ~App();
   
   /**
    * Set the program name.
    * 
    * @param name the program name.
    */
   virtual void setProgramName(const char* name);

   /**
    * Get the program name.
    * 
    * @return the program name.
    */
   virtual const char* getProgramName();

   /**
    * Set the name.
    * 
    * @param name the name.
    */
   virtual void setName(const char* name);

   /**
    * Get the name.
    * 
    * @return the name.
    */
   virtual const char* getName();

   /**
    * Set the application exit status.
    * 
    * @param status the application exit status.
    */
   virtual void setExitStatus(int exitStatus);

   /**
    * Get the application exit status.
    * 
    * @return the application exit status.
    */
   virtual int getExitStatus();
   
   /**
    * Run all tests and set mExitStatus.
    */
   virtual void run();
   
   /**
    * Parses the command line options that were passed to the application.
    * Implementations may call exit() depending on the arguements.  For normal
    * errors it is preferable to return false and set an exception. 
    * 
    * @param argc the number of arguments passed to the application.
    * @param argv a list of arguments passed to the application.
    * 
    * @return true on success, false on failure and exception set
    */
   virtual bool parseCommandLine(int argc, const char* argv[]);

   /**
    * Initialize OpenSSL.
    */
   virtual void initializeOpenSSL();
    
   /**
    * Cleanup OpenSSL.
    */
   virtual void cleanupOpenSSL();
   
   /**
    * Initialize logging.
    */
   virtual void initializeLogging();
   
   /**
    * Cleanup logging.
    */
   virtual void cleanupLogging();
   
   /**
    * Start an app andPerform the main Run all tests.
    * 
    * @param argc number of command line arguments.
    * @param argv command line arguments.
    * 
    * @return exit status. 0 for success.
    */
   virtual int main(int argc, const char* argv[]);
};

/**
 * Macro to ease defining and calling App::main().
 */
#define DB_APP_MAIN(appClassName)      \
int main(int argc, const char* argv[]) \
{                                      \
   int rval;                           \
   appClassName app;                   \
   rval = app.main(argc, argv);        \
   return rval;                        \
}

} // end namespace util
} // end namespace db

#endif
