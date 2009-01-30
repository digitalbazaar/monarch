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
#include <ostream>
#include <sstream>
#include <vector>

#include "db/data/json/JsonReader.h"
#include "db/data/json/JsonWriter.h"
#include "db/logging/FileLogger.h"
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

App::App() :
   mProgramName(NULL),
   mName(NULL),
   mVersion(NULL),
   mExitStatus(0),
   mConfigManager(NULL),
   mCleanupConfigManager(false),
   mLogger(NULL),
   mDelegate(NULL),
   mOwner(NULL)
{
   setExitStatus(0);
   setProgramName("(unknown)");
   setName("(unknown)");
}

App::~App()
{
   mOwner = NULL;
   setProgramName(NULL);
   setName(NULL);
   setVersion(NULL);
   mLogger = NULL;
   cleanupConfigManager();
}

bool App::initialize()
{
   return
      ((mDelegate != NULL) ?
         mDelegate->initConfigManager() : initConfigManager()) &&
      willInitConfigGroups() &&
      ((mDelegate != NULL) ? mDelegate->willInitConfigGroups() : true) &&
      initConfigGroups() &&
      ((mDelegate != NULL) ? mDelegate->initConfigGroups() : true) &&
      didInitConfigGroups() &&
      ((mDelegate != NULL) ? mDelegate->didInitConfigGroups() : true);
   // FIXME: need helper func to support multi-level delegates
}

bool App::willInitConfigGroups()
{
   bool rval = true;
   
   // hard-coded application boot-up defaults
   if(rval && !getConfigManager()->hasConfig("app defaults"))
   {
      Config config;
      config[ConfigManager::ID] = "app defaults";
      config[ConfigManager::GROUP] = "boot";
      config[ConfigManager::VERSION] = DB_DEFAULT_CONFIG_VERSION;
      
      Config& cfg = config[ConfigManager::MERGE];
      cfg["app"]["debug"]["init"] = false;
      cfg["app"]["config"]["debug"] = false;
      cfg["app"]["config"]["dump"] = false;
      cfg["app"]["logging"]["enabled"] = true;
      cfg["app"]["logging"]["level"] = "warning";
      cfg["app"]["logging"]["log"] = "-";
      cfg["app"]["logging"]["append"] = true;
      cfg["app"]["logging"]["rotationFileSize"] = (uint64_t)(2000000ULL);
      cfg["app"]["logging"]["maxRotatedFiles"] = (uint32_t)10;
      cfg["app"]["logging"]["gzip"] = true;
      cfg["app"]["logging"]["location"] = false;
      cfg["app"]["logging"]["color"] = false;
      cfg["app"]["verbose"]["level"] = (uint64_t)0;
      rval = getConfigManager()->addConfig(config);
   }

   return rval;
}

bool App::initConfigGroups()
{
   return true;
}

bool App::didInitConfigGroups()
{
   bool rval = true;
   
   // application and command line configuration target
   if(rval && !getConfigManager()->hasConfig("command line"))
   {
      Config config;
      config[ConfigManager::ID] = "command line";
      config[ConfigManager::PARENT] = getParentOfMainConfigGroup();
      config[ConfigManager::GROUP] = getMainConfigGroup();
      config[ConfigManager::VERSION] = DB_DEFAULT_CONFIG_VERSION;
      rval = getConfigManager()->addConfig(config);
   }

   return rval;
}

unsigned long App::openSSLSetId()
{
#ifdef WIN32
   // win32 uses ptw32_handle_t for pthread IDs which is
   // a struct with a pointer "p" and some number used for "etc"
   return (unsigned long)pthread_self().p;
#else
   return (unsigned long)pthread_self();
#endif
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
      jw.setIndentation(level, 3);
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

void App::printException(ExceptionRef& e, ostream& s)
{
   _printException(e, s, 0);
}

void App::printException()
{
   ExceptionRef e = Exception::getLast();
   printException(e);
}

void App::setDelegate(App* delegate)
{
   if(mDelegate != NULL)
   {
      // FIXME: perhaps also have unregisteredForApp() call?
      // unregister previous delegate
      mDelegate->setOwner(NULL);
   }
   mDelegate = delegate;
   mDelegate->setOwner(this);
}

App* App::getDelegate()
{
   return mDelegate;
}

void App::setOwner(App* owner)
{
   mOwner = owner;
}

App* App::getOwner()
{
   return mOwner;
}

void App::setProgramName(const char* name)
{
   if(mOwner == NULL)
   {
      if(mProgramName != NULL)
      {
         free(mProgramName);
      }
      mProgramName = (name != NULL) ? strdup(name) : NULL;
   }
   else
   {
      mOwner->setProgramName(name);
   }
}

const char* App::getProgramName()
{
   return (mOwner == NULL) ? mProgramName : mOwner->getProgramName();
}

void App::setName(const char* name)
{
   if(mName != NULL)
   {
      free(mName);
   }
   mName = (name != NULL) ? strdup(name) : NULL;
}

const char* App::getName()
{
   return mName;
}

void App::setVersion(const char* version)
{
   if(mOwner == NULL)
   {
      if(mVersion != NULL)
      {
         free(mVersion);
      }
      mVersion = version ? strdup(version) : NULL;
   }
   else
   {
      mOwner->setVersion(version);
   }
}

const char* App::getVersion()
{
   return (mOwner == NULL) ? mVersion : mOwner->getVersion();
}

void App::setExitStatus(int exitStatus)
{
   if(mOwner == NULL)
   {
      mExitStatus = exitStatus;
   }
   else
   {
      mOwner->setExitStatus(exitStatus);
   }
}

int App::getExitStatus()
{
   return (mOwner == NULL) ? mExitStatus : mOwner->getExitStatus();
}

bool App::initConfigManager()
{
   // default implementation
   setConfigManager(new ConfigManager);

   return true;
}

void App::cleanupConfigManager()
{
   if(mConfigManager != NULL && mCleanupConfigManager)
   {
      delete mConfigManager;
      mConfigManager = NULL;
   }
}

void App::setConfigManager(ConfigManager* configManager, bool cleanup)
{
   if(mOwner == NULL)
   {
      mConfigManager = configManager;
      mCleanupConfigManager = cleanup;
   }
   else
   {
      mOwner->setConfigManager(configManager, cleanup);
   }
}

ConfigManager* App::getConfigManager()
{
   return (mOwner == NULL) ? mConfigManager : mOwner->getConfigManager();
}

Config App::getConfig()
{
   return getConfigManager()->getConfig(getMainConfigGroup());
}

const char* App::getMainConfigGroup()
{
   return (mOwner == NULL) ? "main" : mOwner->getMainConfigGroup();
}

const char* App::getParentOfMainConfigGroup()
{
   return (mOwner == NULL) ? "boot" : mOwner->getParentOfMainConfigGroup();
}

bool App::startLogging()
{
   bool rval = true;
   
   // get logging config
   Config cfg = getConfig()["app"]["logging"];
   
   if(cfg["enabled"]->getBoolean())
   {
      // setup logging
      const char* logFile = cfg["log"]->getString();
      if(strcmp(logFile, "-") == 0)
      {
         OutputStream* logStream = new OStreamOutputStream(&cout);
         mLogger = new OutputStreamLogger(logStream, true);
      }
      else
      {
         bool append = cfg["append"]->getBoolean();
         File f(logFile);
         FileLogger* fileLogger = new FileLogger();
         fileLogger->setFile(f, append);
         if(cfg["gzip"]->getBoolean())
         {
            fileLogger->setFlags(FileLogger::GzipCompressRotatedLogs);
         }
         fileLogger->setRotationFileSize(
            cfg["rotationFileSize"]->getUInt64());
         fileLogger->setMaxRotatedFiles(
            cfg["maxRotatedFiles"]->getUInt32());
         mLogger = fileLogger;
      }
      // FIXME: add cfg option to pick categories to log
      //Logger::addLogger(&mLogger, BM_..._CAT);
      // FIXME: add cfg options for logging options
      //logger.setDateFormat("%H:%M:%S");
      //logger.setFlags(Logger::LogThread);
      Logger::Level logLevel;
      const char* levelStr = cfg["level"]->getString(); 
      bool found = Logger::stringToLevel(levelStr, logLevel);
      if(found)
      {
         mLogger->setLevel((Logger::Level)logLevel);
      }
      else
      {
         ExceptionRef e =
            new Exception(
               "Invalid app.logging.level", "bitmunk.app.ConfigError");
         e->getDetails()["level"] = (levelStr ? levelStr : "\"\"");
         Exception::setLast(e, false);
         rval = false;
      }
      if(cfg["color"]->getBoolean())
      {
         mLogger->setFlags(Logger::LogColor);
      }
      if(cfg["location"]->getBoolean())
      {
         mLogger->setFlags(Logger::LogLocation);
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
   
   success = initializeRun();
   if(success)
   {
      loggingStarted = startLogging();
   }
   if(success && loggingStarted)
   {
      success = runApp();
   }
   if(loggingStarted)
   {
      stopLogging();
   }
   cleanupRun();
   if(!success)
   {
      printException();
   }
}

bool App::initializeRun()
{
   return (mDelegate != NULL) ? mDelegate->initializeRun() : true;
}
   
bool App::runApp()
{
   return (mDelegate != NULL) ? mDelegate->runApp() : true;
}

void App::cleanupRun()
{
   if(mDelegate != NULL)
   {
      mDelegate->cleanupRun();
   }
}

static DynamicObject* findPath(
   DynamicObject& root, const char* path, bool createPaths = true)
{
   // start target at given root dyno
   DynamicObject* target = &root;
   
   if(path != NULL)
   {
      // find real target
      // split query up by dots with special segment end escaping:
      // "sss\.sss" == ["sss.sss"]
      // "sss\\.sss" == ["sss\"]["sss"]
      // "\\s.s" == ["\\s"]["s"]
      StringTokenizer st(path, '.');
      string segment;
      bool segmentdone = false;
      while(target != NULL && st.hasNextToken())
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
            ExceptionRef e = new Exception(
               "Internal DynamicObject path parse error.",
               "db.app.CommandLineError");
            e->getDetails()["path"] = path;
            Exception::setLast(e, false);
            target = NULL;
         }
         if(target != NULL && (segmentdone || !st.hasNextToken()))
         {
            if(!createPaths)
            {
               if(!(*target)->hasMember(segment.c_str()))
               {
                  ExceptionRef e = new Exception(
                     "DynamicObject path not found.",
                     "db.app.CommandLineError");
                  e->getDetails()["path"] = path;
                  Exception::setLast(e, false);
                  target = NULL;
               }
            }
            if(target != NULL)
            {
               target = &(*target)[segment.c_str()];
               segment.clear();
               segmentdone = false;
            }
         }
      }
   }
   
   return target;
}

static bool setTargetPath(
   DynamicObject& root, const char* path, DynamicObject& value)
{
   bool rval = true;
   
   // start target at given root dyno
   DynamicObject* target = findPath(root, path);
   rval = (target != NULL);
   
   if(rval)
   {
      // assign the source object
      **target = *value;
   }
   
   return rval;
}

// get read-only object
// use main config rather than optionally specified one
static bool getTarget(
   App* app, DynamicObject& spec, DynamicObject& out,
   bool setExceptions = false)
{
   bool rval = true;
   
   if(spec->hasMember("target"))
   {
      out = spec["target"];
   }
   else if(spec->hasMember("root") && spec->hasMember("path"))
   {
      const char* path = spec["path"]->getString();
      DynamicObject* obj = findPath(spec["root"], path);
      if(obj != NULL)
      {
         out = *obj;
      }
      else
      {
         if(setExceptions)
         {
            ExceptionRef e = new Exception("Object path not found.",
               "db.app.CommandLineError");
            e->getDetails()["path"] = path;
            Exception::setLast(e, false);
         }
         rval = false;
      }
   }
   else if(spec->hasMember("config") && spec->hasMember("path"))
   {
      const char* path = spec["path"]->getString();
      DynamicObject config = app->getConfig();
      DynamicObject* obj = findPath(config, path, false);
      if(obj != NULL)
      {
         out = *obj;
      }
      else
      {
         if(setExceptions)
         {
            ExceptionRef e = new Exception("Object path not found.",
               "db.app.CommandLineError");
            e->getDetails()["path"] = path;
            Exception::setLast(e, false);
         }
         rval = false;
      }
   }
   else
   {
      if(setExceptions)
      {
         ExceptionRef e = new Exception("Invalid option spec.",
            "db.app.CommandLineError");
         e->getDetails()["spec"] = spec;
         Exception::setLast(e, false);
      }
      rval = false;
   }
   
   return rval;
}

static bool setTarget(
   App* app, DynamicObject& spec, DynamicObject& value)
{
   bool rval;
   
   if(spec->hasMember("target"))
   {
      rval = setTargetPath(spec["target"], NULL, value);
   }
   else if(spec->hasMember("root") && spec->hasMember("path"))
   {
      rval = setTargetPath(spec["root"], spec["path"]->getString(), value);
   }
   else if(spec->hasMember("config") && spec->hasMember("path"))
   {
      const char* path = spec["path"]->getString();
      const char* configName = spec["config"]->getString();
      DynamicObject rawConfig = app->getConfigManager()->getConfig(
         configName, true);
      rval = setTargetPath(rawConfig[ConfigManager::MERGE], path, value);
      if(rval)
      {
         rval = app->getConfigManager()->setConfig(rawConfig);
      }
   }
   else
   {
      ExceptionRef e = new Exception("Invalid option spec.",
         "db.app.CommandLineError");
      e->getDetails()["spec"] = spec;
      Exception::setLast(e, false);
      rval = false;
   }
   
   return rval;
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
         
         // must have config for set
         if(optSpec["set"]->hasMember("config"))
         {
            // re-use "arg" processing to set value
            DynamicObject subSpec;
            // set config and path to use
            subSpec["arg"]["config"] = optSpec["set"]["config"];
            subSpec["arg"]["path"] = path;
            // FIXME: should copy all important fields
            if(optSpec->hasMember("isJsonValue"))
            {
               subSpec["isJsonValue"] = optSpec["isJsonValue"];
            }
            rval = processOption(app, args, argsi, opt, subSpec);
         }
         else
         {
            ExceptionRef e =
               new Exception("Invalid command line spec.",
                  "db.app.CommandLineError");
            e->getDetails()["option"] = opt;
            e->getDetails()["spec"] = optSpec;
            Exception::setLast(e, false);
            rval = false;
         }
      }
      else
      {
         rval = hadEnoughArgs = false;
      }
   }
   
   if(rval && optSpec->hasMember("setTrue"))
   {
      DynamicObject value;
      value = true;
      DynamicObject& spec = optSpec["setTrue"];
      if(spec->getType() == Array)
      {
         DynamicObjectIterator i = spec.getIterator();
         while(rval && i->hasNext())
         {
            DynamicObject& next = i->next();
            rval = setTarget(app, next, value);
         }
      }
      else
      {
         rval = setTarget(app, spec, value);
      }
   }
   
   if(rval && optSpec->hasMember("setFalse"))
   {
      DynamicObject value;
      value = false;
      DynamicObject& spec = optSpec["setFalse"];
      if(spec->getType() == Array)
      {
         DynamicObjectIterator i = spec.getIterator();
         while(rval && i->hasNext())
         {
            DynamicObject& next = i->next();
            rval = setTarget(app, next, value);
         }
      }
      else
      {
         rval = setTarget(app, spec, value);
      }
   }
   
   if(rval && optSpec->hasMember("inc"))
   {
      DynamicObject original;
      rval = getTarget(app, optSpec["inc"], original);
      if(rval)
      {
         DynamicObject value;
         switch(original->getType())
         {
            // TODO: deal with overflow?
            case Int32:
               value = original->getInt32() + 1;
               break;
            case UInt32:
               value = original->getUInt32() + 1;
               break;
            case Int64:
               value = original->getInt64() + 1;
               break;
            case UInt64:
               value = original->getUInt64() + 1;
               break;
            case Double:
               value = original->getDouble() + 1.0;
               break;
            default:
               ExceptionRef e =
                  new Exception("Invalid command line spec.",
                     "db.app.CommandLineError");
               e->getDetails()["option"] = opt;
               e->getDetails()["spec"] = optSpec;
               Exception::setLast(e, false);
               rval = false;
         }
         if(!value.isNull())
         {
            setTarget(app, optSpec["inc"], value);
         }
      }
   }
   
   if(rval && optSpec->hasMember("dec"))
   {
      DynamicObject original;
      rval = getTarget(app, optSpec["dec"], original);
      if(rval)
      {
         DynamicObject value;
         switch(original->getType())
         {
            // TODO: deal with underflow?
            case Int32:
               value = original->getInt32() - 1;
               break;
            case UInt32:
               value = original->getUInt32() - 1;
               break;
            case Int64:
               value = original->getInt64() - 1;
               break;
            case UInt64:
               value = original->getUInt64() - 1;
               break;
            case Double:
               value = original->getDouble() - 1.0;
               break;
            default:
               ExceptionRef e =
                  new Exception("Invalid command line spec.",
                     "db.app.CommandLineError");
               e->getDetails()["option"] = opt;
               e->getDetails()["spec"] = optSpec;
               Exception::setLast(e, false);
               rval = false;
         }
         if(!value.isNull())
         {
            setTarget(app, optSpec["dec"], value);
         }
      }
   }
   
   if(rval && optSpec->hasMember("arg"))
   {
      (*argsi)++;
      if((*argsi) != args->end())
      {
         // advance to argument
         const char* arg = **argsi;
         DynamicObject value;
         // do json conversion if requested
         if(optSpec->hasMember("isJsonValue") &&
            optSpec["isJsonValue"]->getBoolean())
         {
            // JSON value conversion
            // use non-strict reader
            JsonReader jr(false);
            ByteArrayInputStream is(arg, strlen(arg));
            jr.start(value);
            rval = rval && jr.read(&is) && jr.finish();
         }
         else
         {
            // regular type conversion
            // try to get old value type else use string
            DynamicObject valueType;
            bool found = getTarget(app, optSpec["arg"], valueType, false);
            if(!found)
            {
               valueType->setType(String);
            }
            // set target as string
            value = arg;
            // convert back to original type
            value->setType(valueType->getType());
         }
         if(rval && optSpec->hasMember("type"))
         {
            value->setType(optSpec["type"]->getType());
         }
         rval = setTarget(app, optSpec["arg"], value);
      }
      else
      {
         rval = hadEnoughArgs = false;
      }
   }
   
   if(rval && optSpec->hasMember("args"))
   {
      // FIXME implement
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
         e = new Exception(
            "Not enough arguments for option.",
            "db.app.CommandLineError");
         e->getDetails()["option"] = opt;
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
               ExceptionRef e =
                  new Exception("Unknown option.", "db.app.CommandLineError");
               e->getDetails()["option"] = opt;
               Exception::setLast(e, false);
               rval = false;
            }
         }
      }
   }

   return rval;
}

DynamicObject App::getCommandLineSpecs()
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
"      --log-rotation-size SIZE\n"
"                      Log size that triggers rotation in bytes. 0 to disable.\n"
"                      (default: 2000000)\n"
"      --log-max-rotated MAX\n"
"                      Maximum number of rotated log files. 0 for no limit.\n"
"                      (default: 10)\n"
"      --log-no-gzip   Do not gzip rotated logs. (default: gzip logs)\n"
"      --log-color     Log with any available ANSI color codes. (default: false)\n"
"      --log-location  Log source code locations.\n"
"                      (compile time option, default: false)\n"
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
   opt["setTrue"]["target"] = mCLConfig["options"]["printHelp"];
   
   opt = spec["options"]->append();
   opt["short"] = "-V";
   opt["long"] = "--version";
   opt["setTrue"]["target"] = mCLConfig["options"]["printVersion"];
   
   opt = spec["options"]->append();
   opt["short"] = "-v";
   opt["long"] = "--verbose";
   opt["inc"]["config"] = "command line";
   opt["inc"]["path"] = "app.verbose.level";
   
   opt = spec["options"]->append();
   opt["long"] = "--no-log";
   opt["setFalse"]["config"] = "command line";
   opt["setFalse"]["path"] = "app.logging.enabled";
   
   opt = spec["options"]->append();
   opt["long"] = "--log-level";
   opt["arg"]["config"] = "command line";
   opt["arg"]["path"] = "app.logging.level";
   opt["argError"] = "No log level specified.";
   
   opt = spec["options"]->append();
   opt["long"] = "--log";
   opt["arg"]["config"] = "command line";
   opt["arg"]["path"] = "app.logging.log";
   opt["argError"] = "No log file specified.";
   
   opt = spec["options"]->append();
   opt["long"] = "--log-overwrite";
   opt["setFalse"]["config"] = "command line";
   opt["setFalse"]["path"] = "app.logging.append";
   
   opt = spec["options"]->append();
   opt["long"] = "--log-rotation-size";
   opt["arg"]["config"] = "command line";
   opt["arg"]["path"] = "app.logging.rotationFileSize";
   opt["argError"] = "No rotation size specified.";
   
   opt = spec["options"]->append();
   opt["long"] = "--log-max-rotated";
   opt["arg"]["config"] = "command line";
   opt["arg"]["path"] = "app.logging.maxRotatedFiles";
   opt["argError"] = "Max rotated files not specified.";
   
   opt = spec["options"]->append();
   opt["long"] = "--log-no-gzip";
   opt["setFalse"]["config"] = "command line";
   opt["setFalse"]["path"] = "app.logging.gzip";
   
   opt = spec["options"]->append();
   opt["long"] = "--log-location";
   opt["setTrue"]["config"] = "command line";
   opt["setTrue"]["path"] = "app.logging.location";
   
   opt = spec["options"]->append();
   opt["long"] = "--log-color";
   opt["setTrue"]["config"] = "command line";
   opt["setTrue"]["path"] = "app.logging.color";
   
   opt = spec["options"]->append();
   opt["long"] = "--option";
   opt["set"]["config"] = "command line";
   
   opt = spec["options"]->append();
   opt["long"] = "--json-option";
   opt["set"]["config"] = "command line";
   opt["isJsonValue"] = true;
   
   opt = spec["options"]->append();
   opt["long"] = "--config-debug";
   opt["setTrue"]["config"] = "command line";
   opt["setTrue"]["path"] = "app.config.debug";
   
   opt = spec["options"]->append();
   opt["long"] = "--config-dump";
   opt["setTrue"]["config"] = "command line";
   opt["setTrue"]["path"] = "app.config.dump";
   
   DynamicObject specs;
   specs->setType(Array);
   specs->append(spec);
   return specs;
}

bool App::willParseCommandLine(std::vector<const char*>* args)
{
   bool rval = true;
   
   // ensure temporary command line config holder is empty
   mCLConfig->setType(Map);
   mCLConfig->clear();
   
   // temporary flags for command line processing
   mCLConfig["options"]["printHelp"] = false;
   mCLConfig["options"]["printVersion"] = false;
   
   // temp storage for command line specs
   mCLConfig["specs"] = getCommandLineSpecs();
   if(mCLConfig["specs"]->getType() != Array)
   {
      ExceptionRef e = new Exception(
         "Command line specs are not an array.",
         "db.app.CommandLineError");
      Exception::setLast(e, false);
      rval = false;
   }
   if(rval && mDelegate != NULL)
   {
      DynamicObject delegateSpecs;
      delegateSpecs = mDelegate->getCommandLineSpecs();
      if(delegateSpecs->getType() != Array)
      {
         ExceptionRef e = new Exception(
            "Delegate command line specs are not an array.",
            "db.app.CommandLineError");
         Exception::setLast(e, false);
         rval = false;
      }
      if(rval)
      {
         DynamicObjectIterator i = delegateSpecs.getIterator();
         while(i->hasNext())
         {
            mCLConfig["specs"]->append(i->next());
         }
      }
   }
   
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
         getConfig()["app"]["logging"]["level"]->getString();
      Logger::Level level;
      bool found = Logger::stringToLevel(cfgLogLevel, level);
      if(!found)
      {
         ExceptionRef e =
            new Exception("Invalid log level.", "db.app.CommandLineError");
         e->getDetails()["level"] = cfgLogLevel;
         Exception::setLast(e, false);
         rval = false;
      }
   }
    
   // done with temporary command line config
   mCLConfig.setNull();
   
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
   if(mDelegate != NULL)
   {
      mDelegate->initializeLogging();
   }
}

void App::didInitializeLogging()
{
   if(mDelegate != NULL)
   {
      mDelegate->didInitializeLogging();
   }
}

void App::willCleanupLogging()
{
   if(mDelegate != NULL)
   {
      mDelegate->willCleanupLogging();
   }
}

void App::cleanupLogging()
{
   if(mDelegate != NULL)
   {
      mDelegate->cleanupLogging();
   }
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
      ((mDelegate != NULL) ?
         mDelegate->willParseCommandLine(&mCommandLineArgs) :
         true) &&
      parseCommandLine(&mCommandLineArgs) &&
      didParseCommandLine() &&
      ((mDelegate != NULL) ?
         mDelegate->didParseCommandLine() :
         true)))
   {
      printException();
      exit(EXIT_FAILURE);
   }
   
#ifdef WIN32
// initialize winsock
   WSADATA wsaData;
   if(WSAStartup(MAKEWORD(2, 0), &wsaData) < 0)
   {
      cerr << "ERROR! Could not initialize winsock!" << endl;
   }
#endif
   
   // seed random
   srand(time(NULL));
   
   initializeOpenSSL();
   db::logging::Logging::initialize();
   initializeLogging();
   didInitializeLogging();
   
   Thread t(this);
   t.start();
   t.join();
   
   willCleanupLogging();
   db::logging::Logging::cleanup();
   cleanupLogging();
   cleanupOpenSSL();
   
   // cleanup winsock
#ifdef WIN32
   WSACleanup();
#endif
   
   Thread::exit();
   return mExitStatus;
}
