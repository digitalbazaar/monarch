/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/app/App.h"

// openssl includes
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/engine.h>

#include <iostream>
#include <sstream>
#include <vector>

#include "db/logging/Logging.h"
#include "db/rt/Exception.h"
#include "db/rt/Thread.h"
#include "db/data/json/JsonWriter.h"
#include "db/io/OStreamOutputStream.h"

using namespace std;
using namespace db::app;
using namespace db::config;
using namespace db::data::json;
using namespace db::logging;
using namespace db::io;
using namespace db::rt;

App::App()
{
   setExitStatus(0);
   mProgramName = NULL;
   setProgramName("(unknown)");
   mName = NULL;
   setName("(unknown)");
   mVersion = NULL;
   mDelegate = NULL;
   
   mConfig["app"]["logging"]["level"] = "warning";
   mConfig["app"]["verbose"]["level"] = (uint64_t)0;
}

App::~App()
{
   setProgramName(NULL);
   setName(NULL);
}

static void _printException(ExceptionRef& e, ostream& s, int level)
{
   ostringstream details;
   if(!e->getDetails().isNull() && e->getDetails()->length() > 0)
   {
      OStreamOutputStream os(&details);
      JsonWriter jw;
      jw.setCompact(false);
      jw.setIndentation(3 * level, 3);
      jw.write(e->getDetails(), &os);
   }
   else
   {
      details << '-';
   }

   ostringstream cause;
   if(!e->getCause().isNull())
   {
      cause << endl;
      _printException(e->getCause(), cause, level + 1);
   }
   else
   {
      cause << '-';
   }
   
   char indent[3 * level + 1];
   indent[3 * level] = '\0';
   memset(indent, ' ', 3 * level);

   if(level == 0)
   {
      s << "Exception:" << endl; 
   }
   s <<
      indent << "type:    " << e->getType() << endl <<
      indent << "code:    " << e->getCode() << endl <<
      indent << "message: " << e->getMessage() << endl <<
      indent << "details: " << details.str() << endl <<
      indent << "cause:   " << cause.str() << endl;
}

void App::printException(ExceptionRef& e)
{
   _printException(e, cerr, 0);
}

void App::printException()
{
   ExceptionRef e = Exception::getLast();
   printException(e);
}

void App::setDelegate(AppDelegate* delegate)
{
   if(mDelegate != NULL)
   {
      // FIXME: perhaps also have unregisteredForApp() call?
      // unregister previous delegate
      mDelegate->registeredForApp(NULL);
   }
   mDelegate = delegate;
   if(mDelegate != NULL)
   {
      mDelegate->registeredForApp(this);
   }
}

AppDelegate* App::getDelegate()
{
   return mDelegate;
}

void App::setProgramName(const char* name)
{
   if(mProgramName)
   {
      free(mProgramName);
   }
   mProgramName = name ? strdup(name) : NULL;
}

const char* App::getProgramName()
{
   return mProgramName;
}

void App::setName(const char* name)
{
   if(mName)
   {
      free(mName);
   }
   mName = name ? strdup(name) : NULL;
}

const char* App::getName()
{
   return mName;
}

void App::setVersion(const char* version)
{
   if(mVersion)
   {
      free(mVersion);
   }
   mVersion = version ? strdup(version) : NULL;
}

const char* App::getVersion()
{
   return mVersion;
}

void App::setExitStatus(int exitStatus)
{
   mExitStatus = exitStatus;
}

int App::getExitStatus()
{
   return mExitStatus;
}

void App::run()
{
   if(mDelegate != this)
   {
      mDelegate->run(this);
   }
}

static bool processOption(
   App* app,
   vector<const char*>* args,
   vector<const char*>::iterator* argsi,
   const char* opt,
   DynamicObject& optSpec)
{
   bool rval = true;
   
   if(optSpec->hasMember("setTrue"))
   {
      DynamicObject& target = optSpec["setTrue"];
      if(target->getType() == Array)
      {
         DynamicObjectIterator i = target.getIterator();
         while(i->hasNext())
         {
            DynamicObject& next = i->next();
            next = true;
         }
      }
      else
      {
         target = true;
      }
   }
   
   if(optSpec->hasMember("setFalse"))
   {
      DynamicObject& target = optSpec["setFalse"];
      if(target->getType() == Array)
      {
         DynamicObjectIterator i = target.getIterator();
         while(i->hasNext())
         {
            DynamicObject& next = i->next();
            next = false;
         }
      }
      else
      {
         target = false;
      }
   }
   
   if(optSpec->hasMember("inc"))
   {
      DynamicObject& target = optSpec["inc"];
      switch(target->getType())
      {
         // TODO: deal with overflow?
         case Int32:
            target = target->getInt32() + 1;
            break;
         case UInt32:
            target = target->getUInt32() + 1;
            break;
         case Int64:
            target = target->getInt64() + 1;
            break;
         case UInt64:
            target = target->getUInt64() + 1;
            break;
         case Double:
            target = target->getDouble() + 1.0;
            break;
         default:
            ostringstream oss;
            oss << "Invalid command line spec for option: " <<
               opt << ".";
            ExceptionRef e =
               new Exception(oss.str().c_str(),
                  "db.app.CommandLineError");
            Exception::setLast(e, false);
            rval = false;
      }
   }
   
   if(optSpec->hasMember("dec"))
   {
      DynamicObject& target = optSpec["inc"];
      switch(target->getType())
      {
         // TODO: deal with underflow?
         case Int32:
            target = target->getInt32() - 1;
            break;
         case UInt32:
            target = target->getUInt32() - 1;
            break;
         case Int64:
            target = target->getInt64() - 1;
            break;
         case UInt64:
            target = target->getUInt64() - 1;
            break;
         case Double:
            target = target->getDouble() - 1.0;
            break;
         default:
            ostringstream oss;
            oss << "Invalid command line spec for option: " <<
               opt << ".";
            ExceptionRef e =
               new Exception(oss.str().c_str(),
                  "db.app.CommandLineError");
            Exception::setLast(e, false);
            rval = false;
      }
   }
   
   if(optSpec->hasMember("arg"))
   {
      //mCLConfig["node"]["port"] = 
      //   (int)strtol(arg, (char **)NULL, 10);
      (*argsi)++;
      if((*argsi) != args->end())
      {
         // target and save type
         DynamicObject& target = optSpec["arg"];
         DynamicObjectType type = target->getType();
         // advance to argument, then set target as string
         target = **argsi;
         // convert back to original type
         target->setType(type);
         
         // add to append target if requested
         if(optSpec->hasMember("append"))
         {
            DynamicObject& appendTarget = optSpec["append"];
            appendTarget->append() = target.clone();
         }
      }
      else
      {
         ostringstream oss;
         oss << "Not enough arguments for option: " << opt << ".";
         ExceptionRef e =
            new Exception(oss.str().c_str(), "db.app.CommandLineError");
         Exception::setLast(e, false);
         rval = false;
      }
   }
   
   if(optSpec->hasMember("args"))
   {
      // FIXME implenent
      ExceptionRef e =
         new Exception("args target not implemented yet", "db.app.CommandLineError");
      Exception::setLast(e, false);
      rval = false;
   }
   
   if(optSpec->hasMember("append"))
   {
      (*argsi)++;
      if((*argsi) != args->end())
      {
         // target and save type
         DynamicObject& target = optSpec["arg"];
         DynamicObjectType type = target->getType();
         // advance to argument, then set target as string
         target = **argsi;
         // convert back to original type
         target->setType(type);
      }
      else
      {
         ostringstream oss;
         oss << "Not enough arguments for option: " << opt << ".";
         ExceptionRef e =
            new Exception(oss.str().c_str(), "db.app.CommandLineError");
         Exception::setLast(e, false);
         rval = false;
      }
   }
   
   return rval;
}

bool App::parseCommandLine(vector<const char*>* args)
{
   bool rval = true;
   
   for(vector<const char*>::iterator i = ++(args->begin());
      rval && i < args->end();
      i++)
   {
      const char* opt = (*i);
      
      // check if we are and end of options
      if(strcmp(opt, "--") == 0 || strlen(opt) == 0 || opt[0] != '-')
      {
         // add rest of args to all specs if "args" key present
         while(i < args->end())
         {
            i++;
            const char* arg = *i;
            DynamicObjectIterator si =
               mConfig["app"]["cl"]["specs"].getIterator();
            while(rval && si->hasNext())
            {
               DynamicObject& spec = si->next();
               if(spec->hasMember("args"))
               {
                  DynamicObject& nextArg = spec["args"]->append();
                  nextArg = arg;
               }
            }
         }
      }
      else
      {
         // process arg for each spec
         bool found = false;
         DynamicObjectIterator si =
            mConfig["app"]["cl"]["specs"].getIterator();
         while(rval && si->hasNext())
         {
            DynamicObject& spec = si->next();
            if(spec->hasMember("options"))
            {
               // loop over all options in each spec
               DynamicObjectIterator oi = spec["options"].getIterator();
               while(rval && oi->hasNext())
               {
                  // check if this opt is found
                  DynamicObject& o = oi->next();
                  if((o->hasMember("short") &&
                        strcmp(o["short"]->getString(), opt) == 0) ||
                     (o->hasMember("long") &&
                        strcmp(o["long"]->getString(), opt) == 0))
                  {
                     found = true;
                     rval = processOption(this, args, &i, opt, o);
                  }
               }
            }
         }
         if(rval && !found)
         {
            ostringstream oss;
            oss << "Unknown option: " << opt << ".";
            ExceptionRef e =
               new Exception(oss.str().c_str(), "db.app.CommandLineError");
            Exception::setLast(e, false);
            rval = false;
         }
      }
   }
   
   return rval;
}

DynamicObject App::getCommandLineSpec(App* app)
{
   DynamicObject spec;
   spec["help"] =
"Help options:\n"
"  -h, --help          Prints information on how to use the application.\n"
"\n"
"General options:\n"
"  -V, --version       Prints the software version.\n"
"  -v, --verbose       Increase verbosity level by 1 (default: 0).\n"
"      --log-level LEVEL\n"
"                      Set log level to one of the following (listed in\n"
"                      increasing level of detail): n[one], e[rror], w[arning],\n"
"                      i[nfo], d[ebug], debug-data, debug-detail, m[ax].\n"
"                      (default: \"warning\")\n"
"      --              Treat all remaining options as application arguments.\n"
"\n";
   
   DynamicObject opt;
   
   opt = spec["options"]->append();
   opt["short"] = "-h";
   opt["long"] = "--help";
   opt["setTrue"] = mConfig["app"]["cl"]["printHelp"];
  
   opt = spec["options"]->append();
   opt["short"] = "-V";
   opt["long"] = "--version";
   opt["setTrue"] = mConfig["app"]["cl"]["printVersion"];
  
   opt = spec["options"]->append();
   opt["short"] = "-v";
   opt["long"] = "--verbose";
   opt["inc"] = mConfig["app"]["verbose"]["level"];
  
   opt = spec["options"]->append();
   opt["long"] = "--log-level";
   opt["arg"] = mConfig["app"]["logging"]["level"];
  
   return spec;
}

bool App::willParseCommandLine(App* app, std::vector<const char*>* args)
{
   bool rval = true;
   
   // temporary flags for command line processing
   mConfig["app"]["cl"]["printHelp"] = false;
   mConfig["app"]["cl"]["printVersion"] = false;
   
   // temp storage for command line specs
   DynamicObject& specs = mConfig["app"]["cl"]["specs"];
   specs[0] = app->getCommandLineSpec(app);
   AppDelegate* delegate = app->getDelegate();
   if(delegate != NULL)
   {
      specs[1] = delegate->getCommandLineSpec(app);
   }
   
   return rval;
}

bool App::didParseCommandLine(App* app)
{
   bool rval = true;
   
   // process help and version flags first
   if(mConfig["app"]["cl"]["printHelp"]->getBoolean())
   {
      cout << "Usage: " << getProgramName() << " [options]" << endl;
      DynamicObjectIterator si = mConfig["app"]["cl"]["specs"].getIterator();
      while(si->hasNext())
      {
         DynamicObject& spec = si->next();
         if(spec->hasMember("help"))
         {
            cout << spec["help"]->getString();
         }
      }
      exit(EXIT_SUCCESS);
   }
   else if(mConfig["app"]["cl"]["printVersion"]->getBoolean())
   {
      // TODO: allow other version info (modules, etc) via delegate?
      cout << getName();
      const char* version = getVersion();
      if(version != NULL)
      {
         cout << " v" << version;
      }
      cout << endl;
      exit(EXIT_SUCCESS);
   }

   // check logging level
   {
      const char* cfgLogLevel = mConfig["app"]["logging"]["level"]->getString();
      Logger::Level level;
      bool found = Logger::stringToLevel(cfgLogLevel, level);
      if(!found)
      {
         ostringstream oss;
         oss << "Invalid log level: \"" << cfgLogLevel << "\".";
         ExceptionRef e =
            new Exception(oss.str().c_str(), "db.app.CommandLineError");
         Exception::setLast(e, false);
         rval = false;
      }
   }
    
   // remove temporaries
   mConfig["app"]["cl"]->removeMember("printHelp");
   mConfig["app"]["cl"]->removeMember("printVersion");
   mConfig["app"]["cl"]->removeMember("specs");
   mConfig["app"]->removeMember("cl");

   return rval;
}

Config& App::getConfig()
{
   return mConfig;
}

void App::initializeOpenSSL()
{
   ERR_load_crypto_strings();
   SSL_library_init();
   SSL_load_error_strings();
   OpenSSL_add_all_algorithms();
}

void App::cleanupOpenSSL()
{
   ERR_remove_state(0);
   ENGINE_cleanup();
   ERR_free_strings();
   EVP_cleanup();
   CRYPTO_cleanup_all_ex_data();
}

void App::initializeLogging()
{
   db::logging::Logging::initialize();
}

void App::cleanupLogging()
{
   db::logging::Logging::cleanup();
}

int App::main(int argc, const char* argv[])
{
   // Make command line vector
   for(int i = 0; i < argc; i++)
   {
      mCommandLineArgs.push_back(argv[i]);
   }
   
   setProgramName(mCommandLineArgs[0]);
   if(!(willParseCommandLine(this, &mCommandLineArgs) &&
      mDelegate->willParseCommandLine(this, &mCommandLineArgs) &&
      parseCommandLine(&mCommandLineArgs) &&
      didParseCommandLine(this) &&
      mDelegate->didParseCommandLine(this)))
   {
      printException();
      exit(EXIT_FAILURE);
   }
   
   // initialize winsock
   #ifdef WIN32
      WSADATA wsaData;
      if(WSAStartup(MAKEWORD(2, 0), &wsaData) < 0)
      {
         cerr << "ERROR! Could not initialize winsock!" << endl;
      }
   #endif
   
   initializeOpenSSL();
   initializeLogging();
   
   Thread t(this);
   t.start();
   t.join();
   
   cleanupLogging();
   cleanupOpenSSL();
   
   // cleanup winsock
#ifdef WIN32
   WSACleanup();
#endif

#ifndef WIN32
   // FIXME: calling Thread::exit() on windows causes a busy loop of
   // some sort (perhaps a deadlock spin lock)
   Thread::exit();
#endif
   
   return mExitStatus;
}

AppDelegate::AppDelegate() {}
   
AppDelegate::~AppDelegate() {}

void AppDelegate::registeredForApp(App* app) {}
   
void AppDelegate::run(App* app) {}

DynamicObject AppDelegate::getCommandLineSpec(App* app)
{
   DynamicObject nullSpec;
   
   return nullSpec;
}

bool AppDelegate::willParseCommandLine(App* app, vector<const char*>* args)
{
   return true;
}

bool AppDelegate::didParseCommandLine(App* app)
{
   return true;
}

void AppDelegate::didInitializeLogging(App* app) {}
   
void AppDelegate::willCleanupLogging(App* app) {}
