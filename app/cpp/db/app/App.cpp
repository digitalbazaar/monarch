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

#include "db/data/json/JsonReader.h"
#include "db/data/json/JsonWriter.h"
#include "db/logging/Logging.h"
#include "db/logging/OutputStreamLogger.h"
#include "db/io/ByteArrayInputStream.h"
#include "db/io/File.h"
#include "db/io/FileOutputStream.h"
#include "db/io/OStreamOutputStream.h"
#include "db/rt/Exception.h"
#include "db/rt/Thread.h"
#include "db/util/StringTokenizer.h"

using namespace std;
using namespace db::app;
using namespace db::config;
using namespace db::data::json;
using namespace db::logging;
using namespace db::io;
using namespace db::rt;
using namespace db::util;

// declare table of openSSL mutexes
pthread_mutex_t* App::sOpenSSLMutexes;

App::App()
{
   setExitStatus(0);
   mProgramName = NULL;
   setProgramName("(unknown)");
   mName = NULL;
   setName("(unknown)");
   mVersion = NULL;
   mDelegate = NULL;
   setDelegate(this);
   mConfigManager = new ConfigManager;
   mLogger = NULL;
   
   mAppConfig->setType(Map);
   mAppConfig["app"]["debug"]["init"] = false;
   mAppConfig["app"]["config"]["debug"] = false;
   mAppConfig["app"]["config"]["dump"] = false;
   mAppConfig["app"]["logging"]["enabled"] = true;
   mAppConfig["app"]["logging"]["level"] = "warning";
   mAppConfig["app"]["logging"]["log"] = "-";
   mAppConfig["app"]["logging"]["logAppend"] = true;
   mAppConfig["app"]["verbose"]["level"] = (uint64_t)0;
   getConfigManager()->addConfig(mAppConfig);
}

App::~App()
{
   setProgramName(NULL);
   setName(NULL);
   setVersion(NULL);
   mLogger = NULL;
}

unsigned long App::openSSLSetId()
{
   return (unsigned long)pthread_self();
}

void App::openSSLHandleLock(int mode, int n, const char* file, int line)
{
   if(mode & CRYPTO_LOCK)
   {
      pthread_mutex_lock(&sOpenSSLMutexes[n]);
   }
   else
   {
      pthread_mutex_unlock(&sOpenSSLMutexes[n]);
   }
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
   mDelegate = (delegate != NULL) ? delegate : this;
   mDelegate->registeredForApp(this);
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

Config& App::getConfig()
{
   return mAppConfig;
}

ConfigManagerRef& App::getConfigManager()
{
   return mConfigManager;
}

bool App::startLogging()
{
   bool rval = true;
   
   // get logging config
   Config& cfg = getConfigManager()->getConfig()["app"]["logging"];
   
   if(cfg["enabled"]->getBoolean())
   {
      // setup logging
      OutputStream* logStream;
      const char* logFile = cfg["log"]->getString();
      if(strcmp(logFile, "-") == 0)
      {
         logStream = new OStreamOutputStream(&cout);
      }
      else
      {
         bool append = cfg["logAppend"]->getBoolean();
         File f(logFile);
         logStream = new FileOutputStream(f, append);
      }
      mLogger = new OutputStreamLogger(logStream, true);
      // FIXME: add cfg option to pick categories to log
      //Logger::addLogger(&mLogger, BM_..._CAT);
      // FIXME: add cfg options for logging options
      //logger.setDateFormat("%H:%M:%S");
      //logger.setFlags(logger.getFlags() | Logger::LogThread);
      Logger::Level logLevel;
      const char* levelStr = cfg["level"]->getString(); 
      bool found = Logger::stringToLevel(levelStr, logLevel);
      if(found)
      {
         mLogger->setLevel((Logger::Level)logLevel);
      }
      else
      {
         ostringstream oss;
         oss << "Invalid app.logging.level: " <<
            (levelStr ? levelStr : "\"\"") << ".";
         ExceptionRef e =
            new Exception(oss.str().c_str(), "bitmunk.app.ConfigError");
         Exception::setLast(e, false);
         rval = false;
      }
      Logger::addLogger(mLogger);

      // NOTE: logging is now initialized.  use logging system after this point
   }
   
   return rval;
}

bool App::stopLogging()
{
   bool rval = true;
   
   if(mLogger != NULL)
   {
      Logger::removeLogger(mLogger);
      delete mLogger;
      mLogger = NULL;
   }
   
   return rval;
}

void App::run()
{
   bool success;
   bool loggingStarted;
   
   success = mDelegate->initializeRun();
   if(success)
   {
      loggingStarted = startLogging();
   }
   if(success && loggingStarted)
   {
      success = mDelegate->runApp();
   }
   if(loggingStarted)
   {
      stopLogging();
   }
   mDelegate->cleanupRun();
   if(!success)
   {
      printException();
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
   // flag used to set common exception
   bool hadEnoughArgs = true;
   
   if(rval && optSpec->hasMember("set"))
   {
      (*argsi)++;
      if((*argsi) != args->end())
      {
         // path is first argument
         const char* path = **argsi;
         
         // start target at given root dyno
         DynamicObject* target = &optSpec["set"];
         
         // find real target
         // split query up by dots with special segment end escaping:
         // "sss\.sss" == ["sss.sss"]
         // "sss\\.sss" == ["sss\"]["sss"]
         // "\\s.s" == ["\\s"]["s"]
         StringTokenizer st(path, '.');
         string segment;
         bool segmentdone = false;
         while(rval && st.hasNextToken())
         {
            const char* tok = st.nextToken();
            size_t toklen = strlen(tok);
            if(toklen == 0 || (toklen >= 1 && tok[toklen - 1] != '\\'))
            {
               // add basic segment and process
               segment.append(tok);
               segmentdone = true;
            }
            else if((toklen == 1 && tok[toklen - 1] == '\\') ||
               (toklen >= 2 && tok[toklen - 1] == '\\' &&
                  tok[toklen - 2] != '\\'))
            {
               // dot escape, use next segment
               segment.append(tok, toklen - 1);
               segment.push_back('.');
            }
            else if(toklen >= 2 &&
               tok[toklen] == '\\' && tok[toklen - 1] == '\\')
            {
               // \ escape, add all but last char and process
               segment.append(tok, toklen - 1);
               segmentdone = true;
            }
            else
            {
               ostringstream oss;
               oss << "Internal option parse error for path: " << path << ".";
               ExceptionRef e = new Exception(oss.str().c_str(),
                  "db.app.CommandLineError");
               Exception::setLast(e, false);
               rval = false;
            }
            if(rval && (segmentdone || !st.hasNextToken()))
            {
               target = &(*target)[segment.c_str()];
               segment.clear();
               segmentdone = false;
            }
         }
         
         if(rval)
         {
            // re-use "arg" processing to get value
            DynamicObject subSpec;
            // storage for value
            subSpec["arg"]->setType(String);
            // FIXME: should copy all importand fields
            if(optSpec->hasMember("isJsonValue"))
            {
               subSpec["isJsonValue"] = optSpec["isJsonValue"];
            }
            rval = processOption(app, args, argsi, opt, subSpec);
            
            if(rval)
            {
               *target = subSpec["arg"];
            }
         }
      }
      else
      {
         rval = hadEnoughArgs = false;
      }
   }
   
   if(rval && optSpec->hasMember("setTrue"))
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
   
   if(rval && optSpec->hasMember("setFalse"))
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
   
   if(rval && optSpec->hasMember("inc"))
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
   
   if(rval && optSpec->hasMember("dec"))
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
   
   if(rval && optSpec->hasMember("arg"))
   {
      (*argsi)++;
      if((*argsi) != args->end())
      {
         // advance to argument
         const char* arg = **argsi;
         DynamicObject& target = optSpec["arg"];
         if(optSpec->hasMember("isJsonValue") &&
            optSpec["isJsonValue"]->getBoolean())
         {
            // JSON value conversion
            // use non-strict reader
            JsonReader jr(false);
            ByteArrayInputStream is(arg, strlen(arg));
            jr.start(target);
            rval = rval && jr.read(&is) && jr.finish();
         }
         else
         {
            // regular type conversion
            // save target type
            DynamicObjectType type = target->getType();
            // set target as string
            target = arg;
            // convert back to original type
            target->setType(type);
         }
      }
      else
      {
         rval = hadEnoughArgs = false;
      }
   }
   
   if(rval && optSpec->hasMember("args"))
   {
      // FIXME implenent
      ExceptionRef e =
         new Exception("args target not implemented yet",
            "db.app.CommandLineError");
      Exception::setLast(e, false);
      rval = false;
   }
   
   if(rval && optSpec->hasMember("append"))
   {
      (*argsi)++;
      if((*argsi) != args->end())
      {
         // append string to "append" target
         DynamicObject& target = optSpec["append"];
         target->append() = **argsi;
      }
      else
      {
         rval = hadEnoughArgs = false;
      }
   }
   
   if(!rval && !hadEnoughArgs)
   {
      ExceptionRef e;
      if(optSpec->hasMember("argError"))
      {
         e = new Exception(
            optSpec["argError"]->getString(),
            "db.app.CommandLineError");
      }
      else
      {
         ostringstream oss;
         oss << "Not enough arguments for option: " << opt << ".";
         e = new Exception(oss.str().c_str(), "db.app.CommandLineError");
      }
      Exception::setLast(e, false);
   }
   
   return rval;
}

bool App::parseCommandLine(vector<const char*>* args)
{
   bool rval = true;
   
   // process all command line args
   for(vector<const char*>::iterator i = ++(args->begin());
      rval && i < args->end();
      i++)
   {
      const char* arg = *i;
      
      // check if we are at end of options
      if(strcmp(arg, "--") == 0 || strlen(arg) == 0 || arg[0] != '-')
      {
         // add rest of args to all specs where "args" key is present
         do
         {
            const char* arg = *i;
            DynamicObjectIterator si = mCLConfig["specs"].getIterator();
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
         while(++i < args->end());
      }
      else
      {
         // Create options to process.
         vector<string> opts;
         // either --... or -... at this point
         if(strncmp(arg, "--", 2) == 0)
         {
            // Long options (--opt) process as-is.
            opts.push_back(arg);
         }
         else
         {
            // Short options are split up (-abc => -a -b -c)
            // FIXME: handle "-" with no option case, ignored now
            size_t arglen = strlen(arg);
            for(size_t shorti = 1; shorti < arglen; shorti++)
            {
               string opt;
               opt.push_back('-');
               opt.push_back(arg[shorti]);
               opts.push_back(opt);
            }
         }
         
         for(vector<string>::iterator opti = opts.begin();
            opti != opts.end();
            opti++)
         {
            const char* opt = (*opti).c_str();
            
            // process arg for each spec
            bool found = false;
            DynamicObjectIterator si = mCLConfig["specs"].getIterator();
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
   }
   
   return rval;
}

DynamicObject App::getCommandLineSpec()
{
   DynamicObject spec;
   spec["help"] =
"Help options:\n"
"  -h, --help          Prints information on how to use the application.\n"
"\n"
"General options:\n"
"  -V, --version       Prints the software version.\n"
"  -v, --verbose       Increase verbosity level by 1. (default: 0)\n"
"      --no-log        Disable default logging. (default: enabled)\n"
"      --log-level LEVEL\n"
"                      Set log level to one of the following (listed in\n"
"                      increasing level of detail): n[one], e[rror], w[arning],\n"
"                      i[nfo], d[ebug], debug-data, debug-detail, m[ax].\n"
"                      (default: \"warning\")\n"
"      --log LOG       Set log file.  Use \"-\" for stdout. (default: \"-\")\n"
"      --log-overwrite Overwrite log file instead of appending. (default: false)\n"
"      --              Treat all remaining options as application arguments.\n"
"\n"
"Config options:\n"
"      --config-debug  Debug the configuration loading process to stdout.\n"
"      --config-dump   Load and dump all configuration data to stdout.\n"
"      --option NAME VALUE\n"
"                      Set dotted config path NAME to the string VALUE.\n"
"      --json-option NAME JSONVALUE\n"
"                      Set dotted config path NAME to the decoded JSONVALUE.\n"
"\n";
   
   DynamicObject opt;
   
   opt = spec["options"]->append();
   opt["short"] = "-h";
   opt["long"] = "--help";
   opt["setTrue"] = mCLConfig["options"]["printHelp"];
   
   opt = spec["options"]->append();
   opt["short"] = "-V";
   opt["long"] = "--version";
   opt["setTrue"] = mCLConfig["options"]["printVersion"];
   
   opt = spec["options"]->append();
   opt["short"] = "-v";
   opt["long"] = "--verbose";
   opt["inc"] = mAppConfig["app"]["verbose"]["level"];
   
   opt = spec["options"]->append();
   opt["long"] = "--no-log";
   opt["setFalse"] = mAppConfig["app"]["logging"]["enabled"];
   
   opt = spec["options"]->append();
   opt["long"] = "--log-level";
   opt["arg"] = mAppConfig["app"]["logging"]["level"];
   opt["argError"] = "No log level specified.";
   
   opt = spec["options"]->append();
   opt["long"] = "--log";
   opt["arg"] = mAppConfig["app"]["logging"]["log"];
   opt["argError"] = "No log file specified.";
   
   opt = spec["options"]->append();
   opt["long"] = "--log-overwrite";
   opt["setFalse"] = mAppConfig["app"]["logging"]["logAppend"];
   
   opt = spec["options"]->append();
   opt["long"] = "--option";
   opt["set"] = mAppConfig;
   
   opt = spec["options"]->append();
   opt["long"] = "--json-option";
   opt["set"] = mAppConfig;
   opt["isJsonValue"] = true;
   
   opt = spec["options"]->append();
   opt["long"] = "--config-debug";
   opt["setTrue"] = mAppConfig["app"]["config"]["debug"];
   
   opt = spec["options"]->append();
   opt["long"] = "--config-dump";
   opt["setTrue"] = mAppConfig["app"]["config"]["dump"];
   
   return spec;
}

bool App::willParseCommandLine(std::vector<const char*>* args)
{
   bool rval = true;
   
   // ensure config is clear
   mCLConfig->setType(Map);
   mCLConfig->clear();
   
   // temporary flags for command line processing
   mCLConfig["options"]["printHelp"] = false;
   mCLConfig["options"]["printVersion"] = false;
   
   // temp storage for command line specs
   DynamicObject& specs = mCLConfig["specs"];
   specs->setType(Array);
   specs[0] = this->getCommandLineSpec();
   AppDelegate* delegate = this->getDelegate();
   if(delegate != NULL)
   {
      specs[1] = delegate->getCommandLineSpec();
   }
   
   getConfigManager()->update();
   
   return rval;
}

bool App::didParseCommandLine()
{
   bool rval = true;
   
   // process help and version flags first
   if(mCLConfig["options"]["printHelp"]->getBoolean())
   {
      cout << "Usage: " << getProgramName() << " [options]" << endl;
      DynamicObjectIterator si = mCLConfig["specs"].getIterator();
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
   else if(mCLConfig["options"]["printVersion"]->getBoolean())
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
      const char* cfgLogLevel =
         mAppConfig["app"]["logging"]["level"]->getString();
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
    
   // done with temporary command line config
   mCLConfig.setNull();
   
   // update merged config
   getConfigManager()->update();

   return rval;
}

void App::initializeOpenSSL()
{
   ERR_load_crypto_strings();
   SSL_library_init();
   SSL_load_error_strings();
   OpenSSL_add_all_algorithms();
   
   // create mutex attributes, use fastest type of mutex
   pthread_mutexattr_t mutexAttr;
   pthread_mutexattr_init(&mutexAttr);
   pthread_mutexattr_settype(&mutexAttr, PTHREAD_MUTEX_NORMAL);
   
   // create CRYPTO_num_locks() mutexes
   int numLocks = CRYPTO_num_locks();
   sOpenSSLMutexes = (pthread_mutex_t*)calloc(
      numLocks, sizeof(pthread_mutex_t));
   
   // initialize mutexes
   for(int i = 0; i < numLocks; i++)
   {
      // initialize mutex
      pthread_mutex_init(&sOpenSSLMutexes[i], &mutexAttr);
   }
   
   // destroy mutex attributes
   pthread_mutexattr_destroy(&mutexAttr);
   
   // set openSSL multi-threaded callbacks
   CRYPTO_set_id_callback(&App::openSSLSetId);
   CRYPTO_set_locking_callback(&App::openSSLHandleLock);
}

void App::cleanupOpenSSL()
{
   ERR_remove_state(0);
   ENGINE_cleanup();
   ERR_free_strings();
   EVP_cleanup();
   CRYPTO_cleanup_all_ex_data();
   
   // destroy mutexes
   int numLocks = CRYPTO_num_locks();
   for(int i = 0; i < numLocks; i++)
   {
      // initialize mutex
      pthread_mutex_destroy(&sOpenSSLMutexes[i]);
   }
   
   // free mutexes
   free(sOpenSSLMutexes);
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
   if(!(willParseCommandLine(&mCommandLineArgs) &&
      mDelegate->willParseCommandLine(&mCommandLineArgs) &&
      parseCommandLine(&mCommandLineArgs) &&
      didParseCommandLine() &&
      mDelegate->didParseCommandLine()))
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
   
   // seed random
   srand(time(NULL));
   
   initializeOpenSSL();
   initializeLogging();
   mDelegate->didInitializeLogging();
   
   Thread t(this);
   t.start();
   t.join();
   
   mDelegate->willCleanupLogging();
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
   
bool AppDelegate::initializeRun()
{
   return true;
}
   
bool AppDelegate::runApp()
{
   return true;
}

void AppDelegate::cleanupRun() {}

DynamicObject AppDelegate::getCommandLineSpec()
{
   DynamicObject nullSpec;
   
   return nullSpec;
}

bool AppDelegate::willParseCommandLine(vector<const char*>* args)
{
   return true;
}

bool AppDelegate::didParseCommandLine()
{
   return true;
}

void AppDelegate::didInitializeLogging() {}
   
void AppDelegate::willCleanupLogging() {}
