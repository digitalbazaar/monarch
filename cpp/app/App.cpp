/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/app/App.h"

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

#include "monarch/app/CommonAppPlugin.h"
#include "monarch/app/MultiAppPlugin.h"
#include "monarch/data/json/JsonReader.h"
#include "monarch/data/json/JsonWriter.h"
#include "monarch/logging/FileLogger.h"
#include "monarch/logging/Logging.h"
#include "monarch/io/ByteArrayInputStream.h"
#include "monarch/io/File.h"
#include "monarch/io/OStreamOutputStream.h"
#include "monarch/rt/Exception.h"
#include "monarch/rt/Platform.h"
#include "monarch/rt/Thread.h"
#include "monarch/util/Random.h"
#include "monarch/util/StringTokenizer.h"

using namespace std;
using namespace monarch::app;
using namespace monarch::config;
using namespace monarch::data::json;
using namespace monarch::io;
using namespace monarch::rt;
using namespace monarch::util;

// declare table of openSSL mutexes
pthread_mutex_t* App::sOpenSSLMutexes = NULL;

App::App() :
   mProgramName(NULL),
   mName(NULL),
   mVersion(NULL),
   mExitStatus(0),
   mConfigManager(NULL),
   mCleanupConfigManager(false),
   mPlugins(new MultiAppPlugin)
{
   setExitStatus(0);
   setProgramName("(unknown)");
   setName("(unknown)");

   // command line option specs
   mMetaConfig["specs"]->setType(Array);
   // command line option storage
   mMetaConfig["options"]->setType(Map);

   mMetaConfig["groups"]->setType(Map);
   mMetaConfig["parents"]->setType(Map);
   mMetaConfig["configs"]->setType(Map);
}

App::~App()
{
   delete mPlugins;
   setProgramName(NULL);
   setName(NULL);
   setVersion(NULL);
   cleanupConfigManager();
}

bool App::addPlugin(AppPluginRef plugin)
{
   bool rval;

   rval = plugin->willAddToApp(this);
   if(rval)
   {
      mPlugins->addPlugin(plugin);
      rval = plugin->didAddToApp(this);
   }
   return true;
}

Config App::makeMetaConfig(Config& meta, const char* id, const char* groupId)
{
   Config rval;
   rval[ConfigManager::VERSION] = DB_DEFAULT_CONFIG_VERSION;
   if(groupId != NULL)
   {
      const char* g = meta["groups"][groupId]->getString();
      rval[ConfigManager::PARENT] = meta["parents"][g]->getString();
      rval[ConfigManager::GROUP] = g;
   }
   if(id != NULL)
   {
      rval[ConfigManager::ID] = id;
      meta["configs"][id] = rval;
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
   ExceptionRef e = Exception::get();
   printException(e);
}

void App::setProgramName(const char* name)
{
   if(mProgramName != NULL)
   {
      free(mProgramName);
   }
   mProgramName = (name != NULL) ? strdup(name) : NULL;
}

const char* App::getProgramName()
{
   return mProgramName;
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
   if(mVersion != NULL)
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
   cleanupConfigManager();
   mConfigManager = configManager;
   mCleanupConfigManager = cleanup;
}

ConfigManager* App::getConfigManager()
{
   return mConfigManager;
}

Config App::getConfig()
{
   // get config, but do not cache merged config to prevent config changes
   // from being tracked until absolutely necessary
   return getConfigManager()->getConfig(
      getMetaConfig()["groups"]["main"]->getString(), false, false);
}

Config App::getMetaConfig()
{
   return mMetaConfig;
}

/**
 * Recursively traverse up the hierarchy to order parent configs first.
 */
static bool _orderIds(
   Config& meta, DynamicObject& ids, DynamicObject& seen,
   DynamicObject& visited, const char* id)
{
   bool rval = true;

   visited[id] = true;
   Config& c = meta["configs"][id];
   const char* parent =
      c->hasMember(ConfigManager::PARENT) ?
      c[ConfigManager::PARENT]->getString() :
      NULL;
   if(parent != NULL && !seen->hasMember(parent))
   {
      // parent not yet seen
      // find and load a config with id or group id of parent
      ConfigIterator i = meta["configs"].getIterator();
      bool found = false;
      while(!found && i->hasNext())
      {
         Config& next = i->next();
         const char* nextId = i->getName();
         if(!visited->hasMember(nextId) &&
            strcmp(nextId, id) != 0 &&
            (strcmp(nextId, parent) == 0 ||
            (next->hasMember(ConfigManager::GROUP) &&
               strcmp(next[ConfigManager::GROUP]->getString(), parent) == 0)))
         {
            found = true;
            rval = _orderIds(meta, ids, seen, visited, nextId);
            break;
         }
      }
      if(!found)
      {
         ExceptionRef e = new Exception(
            "Could not find parent config.", "db.app.ConfigError");
         e->getDetails()["id"] = id;
         e->getDetails()["parent"] = parent;
         Exception::push(e);
         rval = false;
      }
   }
   if(rval)
   {
      seen[id] = true;
      ids->append() = id;
   }

   return rval;
}

/**
 * Sorts the configs in the meta config based on parent relationships and
 * loads them in the proper order.
 *
 * @return true on success, false on failure and exception set
 */
static bool _loadMetaConfigs(App* app)
{
   bool rval = true;

   Config meta = app->getMetaConfig();

   // ids to load in order
   DynamicObject ids;
   ids->setType(Array);
   DynamicObject seen;
   seen->setType(Map);

   // order all ids
   {
      ConfigIterator i = meta["configs"].getIterator();
      while(rval && i->hasNext())
      {
         i->next();
         const char* name = i->getName();
         if(!seen->hasMember(name))
         {
            // start with empty visited list
            DynamicObject visited;
            rval = _orderIds(meta, ids, seen, visited, name);
         }
      }
   }

   // load ids in order
   if(rval)
   {
      ConfigIterator i = ids.getIterator();
      while(rval && i->hasNext())
      {
         DynamicObject& configId = i->next();
         rval = app->getConfigManager()->addConfig(
            meta["configs"][configId->getString()]);
      }
   }

   return rval;
}

/**
 * Loads command line option configs from the meta config.
 *
 * @return true on success, false on failure and exception set
 */
static bool _loadOptionConfigs(App* app)
{
   bool rval = true;

   ConfigIterator i = app->getMetaConfig()["options"].getIterator();
   while(rval && i->hasNext())
   {
      rval = app->getConfigManager()->addConfig(i->next());
   }

   return rval;
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
            Exception::set(e);
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
                  Exception::set(e);
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
            Exception::set(e);
         }
         rval = false;
      }
   }
   else if(spec->hasMember("config") && spec->hasMember("path"))
   {
      const char* path = spec["path"]->getString();
      Config config = app->getConfig();
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
            Exception::set(e);
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
         Exception::set(e);
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
      Config rawConfig = app->getConfigManager()->getConfig(
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
      Exception::set(e);
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

         // set a path in a config or in a root object
         if(optSpec["set"]->hasMember("config") ||
            optSpec["set"]->hasMember("root"))
         {
            // re-use "arg" processing to set value
            DynamicObject subSpec;
            if(optSpec["set"]->hasMember("config"))
            {
               // set config to use
               subSpec["arg"]["config"] = optSpec["set"]["config"];
            }
            else
            {
               // set root object to use
               subSpec["arg"]["root"] = optSpec["set"]["root"];
            }
            // set path to use
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
            ExceptionRef e = new Exception(
               "Invalid command line spec. "
               "The option does not specify a configuration or root to set.",
               "db.app.CommandLineError");
            e->getDetails()["option"] = opt;
            e->getDetails()["spec"] = optSpec;
            Exception::set(e);
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
               ExceptionRef e = new Exception(
                  "Invalid command line spec. "
                  "The option cannot be increased because it is not a number.",
                  "db.app.CommandLineError");
               e->getDetails()["option"] = opt;
               e->getDetails()["spec"] = optSpec;
               Exception::set(e);
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
               ExceptionRef e = new Exception(
                  "Invalid command line spec. "
                  "The option cannot be decreased because it is not a number.",
                  "db.app.CommandLineError");
               e->getDetails()["option"] = opt;
               e->getDetails()["spec"] = optSpec;
               Exception::set(e);
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
      Exception::set(e);
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
      Exception::set(e);
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
            DynamicObjectIterator si = mMetaConfig["specs"].getIterator();
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
            DynamicObjectIterator si = mMetaConfig["specs"].getIterator();
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
               Exception::set(e);
               rval = false;
            }
         }
      }
   }

   return rval;
}

bool App::initializeOpenSSL()
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

   return true;
}

void App::cleanupOpenSSL()
{
   ERR_remove_state(0);
   ENGINE_cleanup();
   ERR_free_strings();
   EVP_cleanup();
   CRYPTO_cleanup_all_ex_data();

   // destroy mutexes
   if(sOpenSSLMutexes != NULL)
   {
      int numLocks = CRYPTO_num_locks();
      for(int i = 0; i < numLocks; i++)
      {
         // initialize mutex
         pthread_mutex_destroy(&sOpenSSLMutexes[i]);
      }

      // free mutexes
      free(sOpenSSLMutexes);
   }
}

int App::main(
   int argc, const char* argv[], vector<AppPluginRef>* plugins, bool standard)
{
   bool success = true;

   // seed random
   Random::seed();
   // enable stats early
   DynamicObjectImpl::enableStats(true);

   // Make command line vector
   for(int i = 0; i < argc; i++)
   {
      mCommandLineArgs.push_back(argv[i]);
   }

   // setup program name from command line
   setProgramName(mCommandLineArgs[0]);

   if(standard)
   {
      AppPlugin* common = new CommonAppPlugin;
      success = addPlugin(common);
   }
   for(vector<AppPluginRef>::iterator i = plugins->begin();
      success && i != plugins->end();
      i++)
   {
      success = addPlugin(*i);
   }

   Config meta = getMetaConfig();
   success = success &&
      initConfigManager() &&
      mPlugins->initConfigManager() &&
      mPlugins->willInitMetaConfig(meta) &&
      mPlugins->initMetaConfig(meta) &&
      mPlugins->didInitMetaConfig(meta) &&
      _loadMetaConfigs(this);

   // add plugin specs to command line config
   if(success)
   {
      DynamicObject pluginSpecs = mPlugins->getCommandLineSpecs();
      if(pluginSpecs->getType() == Array)
      {
         DynamicObjectIterator i = pluginSpecs.getIterator();
         while(i->hasNext())
         {
            mMetaConfig["specs"]->append(i->next());
         }
      }
      else
      {
         ExceptionRef e = new Exception(
            "Plugin command line specs are not an array.",
            "db.app.CommandLineError");
         Exception::set(e);
         success = false;
      }
   }

   success = success &&
      mPlugins->willParseCommandLine(&mCommandLineArgs) &&
      parseCommandLine(&mCommandLineArgs) &&
      mPlugins->didParseCommandLine() &&
      mPlugins->willLoadConfigs() &&
      _loadOptionConfigs(this) &&
      mPlugins->didLoadConfigs();

#ifdef WIN32
   if(success)
   {
      // initialize winsock
      WSADATA wsaData;
      if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
      {
         ExceptionRef e = new Exception(
            "Could not initialize winsock.",
            "db.app.WinSockError");
         Exception::set(e);
         success = false;
      }
      else if(LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
      {
         ExceptionRef e = new Exception(
            "Incompatible version of winsock.",
            "db.app.WinSockError");
         char tmp[10];
         snprintf(tmp, 10, "%d.%d",
            LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion));
         e->getDetails()["version"] = tmp;
         e->getDetails()["requiredVersion"] = "2.2";
         Exception::set(e);
         success = false;
      }
   }
#endif

   success = success &&
      initializeOpenSSL() &&
      monarch::logging::Logging::initialize() &&
      monarch::rt::Platform::initialize() &&
      mPlugins->initializeLogging() &&
      mPlugins->willRun() &&
      mPlugins->run() &&
      mPlugins->didRun();

   if(!success)
   {
      printException();
   }

   mPlugins->cleanupLogging();
   monarch::rt::Platform::cleanup();
   monarch::logging::Logging::cleanup();
   cleanupOpenSSL();

   // cleanup winsock
#ifdef WIN32
   // FIXME: WSACleanup() crashes when called on windows (after a successful
   // call to WSAStartup() ... so it has been disabled
   //WSACleanup();
#endif

   // if had an error and exit status not already set to failure then set it
   if(!success && mExitStatus != EXIT_SUCCESS)
   {
      mExitStatus = EXIT_FAILURE;
   }

   Thread::exit();

   return mExitStatus;
}
