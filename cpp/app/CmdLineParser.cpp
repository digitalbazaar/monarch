/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/app/CmdLineParser.h"

#include "monarch/app/AppRunner.h"
#include "monarch/data/json/JsonReader.h"
#include "monarch/io/ByteArrayInputStream.h"
#include "monarch/util/StringTokenizer.h"

using namespace std;
using namespace monarch::app;
using namespace monarch::config;
using namespace monarch::data::json;
using namespace monarch::io;
using namespace monarch::rt;
using namespace monarch::util;

#define CMDLINE_ERROR   "monarch.app.CommandLineError"

CmdLineParser::CmdLineParser()
{
}

CmdLineParser::~CmdLineParser()
{
}

/**
 * Returns 1 if the given argument is a short option, 2 if it is a long option.
 * 3 if it is multiple short options, 0 if it is the end of options, -1 on not
 * an option.
 *
 * @param option to potential option.
 *
 * @return the type of option.
 */
static int _getOptionType(const char* option)
{
   int rval = -1;

   // end of options
   if(strcmp(option, "--") == 0)
   {
      rval = 0;
   }
   // long option
   else if(strncmp(option, "--", 2) == 0)
   {
      rval = 2;
   }
   // short option
   else if(strncmp(option, "-", 1) == 0)
   {
      option++;
      for(; *option != '\0'; option++)
      {
         if(isalpha(*option))
         {
            // 1 = single short, 3 = multiple short
            rval = (rval == -1) ? 1 : 3;
         }
         else
         {
            // invalid option
            rval = -1;
            break;
         }
      }
   }

   return rval;
}

bool CmdLineParser::parse(int argc, const char* argv[], DynamicObject& options)
{
   bool rval = true;

   DynamicObject& opts = options["options"];
   opts->setType(Array);
   DynamicObject& extra = options["extra"];
   extra->setType(Array);

   // iterate over all command line arguments, except the first (which is
   // the program name)
   bool end = false;
   int i = 1;
   for(; rval && !end && i < argc; i++)
   {
      const char* option = argv[i];

      // get the option type
      int type = _getOptionType(option);

      // check for the end of the options
      if(type == 0)
      {
         end = true;
      }
      // either no options or parse error
      else if(type == -1)
      {
         // no options, back up 1 argument to store it in "extra"
         if(i == 1)
         {
            end = true;
            i--;
         }
         // parse error
         else
         {
            ExceptionRef e = new Exception(
               "Command line parsing error. Options must be a hyphen followed "
               "by a single character options or be a double-hyphen followed "
               "by a keyword. One argument may follow an option.",
               CMDLINE_ERROR);
            e->getDetails()["option"] = option;
            Exception::set(e);
            rval = false;
         }
      }
      // long option
      else if(type == 2)
      {
         DynamicObject& opt = opts->append();
         opt["consumed"] = false;

         // option might be using equals
         // Note: No special escaping support for having an '=' in a value

         // parse out the keyword and value
         const char* eq = strchr(option, '=');
         if(eq == NULL)
         {
            // no equals
            opt["long"] = option;
         }
         else
         {
            // keyword before equals, value after
            opt["long"] = string(option, eq - option).c_str();
            opt["value"] = string(eq + 1).c_str();
         }
      }
      // single short option
      else if(type == 1)
      {
         DynamicObject& opt = opts->append();
         opt["consumed"] = false;
         opt["short"] = option;
      }
      // multiple short options
      else
      {
         size_t arglen = strlen(option);
         for(size_t shorti = 1; shorti < arglen; shorti++)
         {
            string tmp;
            tmp.push_back('-');
            tmp.push_back(option[shorti]);
            DynamicObject& opt = opts->append();
            opt["consumed"] = false;
            opt["short"] = tmp.c_str();
         }
      }

      if(type > 0)
      {
         DynamicObject opt = opts.last();
         if(!opt->hasMember("value"))
         {
            // store option argument value if applicable
            if((i + 1 < argc) && _getOptionType(argv[i + 1]) == -1)
            {
               DynamicObject opt = opts.last();
               opt["value"] = argv[++i];
            }
         }
      }
   }

   // add remaining options to options["extra"]
   if(rval && end)
   {
      for(; i < argc; i++)
      {
         extra->append() = argv[i];
      }
   }

   return rval;
}

/**
 * Finds the path to a dynamic object. Paths are formatted using dots to
 * delimit map keyst
 *
 * @param root the root object to look in.
 * @param path the path to the object.
 * @param createPaths true to create the paths if they don't exist.
 * @param setExceptions true to set exceptions if not found, false not to.
 *
 * @return a pointer to the found object.
 */
static DynamicObject* _findPath(
   DynamicObject& root, const char* path,
   bool createPaths = true, bool setExceptions = true)
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
               CMDLINE_ERROR);
            e->getDetails()["path"] = path;
            Exception::set(e);
            target = NULL;
         }
         if(target != NULL && (segmentdone || !st.hasNextToken()))
         {
            // if not creating paths and path segment doesn't exist, error
            if(!createPaths && !(*target)->hasMember(segment.c_str()))
            {
               if(setExceptions)
               {
                  ExceptionRef e = new Exception(
                     "DynamicObject path not found.",
                     CMDLINE_ERROR);
                  e->getDetails()["path"] = path;
                  Exception::set(e);
               }
               target = NULL;
            }
            else
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

/**
 * Sets a target at the given path to the given value.
 *
 * @param root the root object to apply the path to.
 * @param path the path into the root object to the target.
 * @param value the value to set the target to.
 *
 * @return true if successful, false if the target was not found.
 */
static bool _setTargetAtPath(
   DynamicObject& root, const char* path, DynamicObject& value)
{
   bool rval = true;

   // start target at given root dyno
   DynamicObject* target = _findPath(root, path);
   rval = (target != NULL);

   if(rval)
   {
      // assign the source object
      **target = *value;
   }

   return rval;
}

/**
 * Gets the target for the given spec.
 *
 * @param ar the AppRunner.
 * @param spec the option spec.
 * @param out to be set to the target.
 * @param setExceptions true to set exceptions if not found, false not to.
 *
 * @return true if found, false if not.
 */
static bool _getTarget(
   AppRunner* ar, DynamicObject& spec, DynamicObject& out,
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
      DynamicObject* obj = _findPath(spec["root"], path, false, setExceptions);
      if(obj != NULL)
      {
         out = *obj;
      }
      else
      {
         if(setExceptions)
         {
            ExceptionRef e = new Exception(
               "Object path not found.",
               CMDLINE_ERROR);
            e->getDetails()["path"] = path;
            Exception::set(e);
         }
         rval = false;
      }
   }
   else if(spec->hasMember("config") && spec->hasMember("path"))
   {
      const char* path = spec["path"]->getString();
      Config config = ar->getConfig();
      DynamicObject* obj = _findPath(config, path, false, setExceptions);
      if(obj != NULL)
      {
         out = *obj;
      }
      else
      {
         if(setExceptions)
         {
            ExceptionRef e = new Exception(
               "Object path not found.",
               CMDLINE_ERROR);
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
         ExceptionRef e = new Exception(
            "Invalid option spec.",
            CMDLINE_ERROR);
         e->getDetails()["spec"] = spec;
         Exception::set(e);
      }
      rval = false;
   }

   return rval;
}

/**
 * Sets a target object based on the given command line spec and the given
 * value.
 *
 * @param ar the AppRunner.
 * @param spec the command line spec option.
 * @param value the value to set the target to.
 *
 * @return true if the target was set, false if not.
 */
static bool _setTarget(AppRunner* ar, DynamicObject& spec, DynamicObject& value)
{
   bool rval;

   // target is specified directly, no path to follow
   if(spec->hasMember("target"))
   {
      rval = _setTargetAtPath(spec["target"], NULL, value);
   }
   // target is specified as a path into a root object
   else if(spec->hasMember("root") && spec->hasMember("path"))
   {
      rval = _setTargetAtPath(spec["root"], spec["path"]->getString(), value);
   }
   // target is specified as a path into a config
   else if(spec->hasMember("config") && spec->hasMember("path"))
   {
      const char* path = spec["path"]->getString();
      const char* configName = spec["config"]->getString();
      Config rawConfig = ar->getConfigManager()->getConfig(configName, true);
      rval = _setTargetAtPath(rawConfig[ConfigManager::MERGE], path, value);
      if(rval)
      {
         rval = ar->getConfigManager()->setConfig(rawConfig);
      }
   }
   else
   {
      ExceptionRef e = new Exception(
         "Invalid command line option spec.",
         CMDLINE_ERROR);
      e->getDetails()["spec"] = spec;
      Exception::set(e);
      rval = false;
   }

   return rval;
}

/**
 * Processes a command line option using the given option spec.
 *
 * @param ar the AppRunner.
 * @param optSpec the spec for the command line option.
 * @param opt the command line option to process.
 *
 * @return true if successful, false if an exception occurred.
 */
static bool _processOption(
   AppRunner* ar, DynamicObject& optSpec, DynamicObject& opt)
{
   bool rval = true;

   // make sure value's existence in option is correct based on the spec type
   if(!opt->hasMember("value") &&
      (optSpec->hasMember("set") ||
       optSpec->hasMember("arg") ||
       optSpec->hasMember("append") ||
       optSpec->hasMember("include") ||
       optSpec->hasMember("keyword")))
   {
      ExceptionRef e;
      if(optSpec->hasMember("argError"))
      {
         e = new Exception(
            optSpec["argError"]->getString(),
            CMDLINE_ERROR);
      }
      else
      {
         e = new Exception(
            "Not enough arguments for option.",
            CMDLINE_ERROR);
      }
      e->getDetails()["option"] = opt;
      e->getDetails()["spec"] = optSpec;
      Exception::set(e);
      rval = false;
   }
   else if(opt->hasMember("value") &&
      (optSpec->hasMember("inc") ||
       optSpec->hasMember("dec") ||
       optSpec->hasMember("setTrue") ||
       optSpec->hasMember("setFalse")))
   {
      ExceptionRef e = new Exception(
         "Invalid command line option. Too many arguments for option.",
         CMDLINE_ERROR);
      e->getDetails()["option"] = opt;
      e->getDetails()["spec"] = optSpec;
      Exception::set(e);
      rval = false;
   }
   else
   {
      // set target to true or false
      if(optSpec->hasMember("setTrue") || optSpec->hasMember("setFalse"))
      {
         DynamicObject value;
         value = optSpec->hasMember("setTrue");
         DynamicObject& spec = value->getBoolean() ?
            optSpec["setTrue"] : optSpec["setFalse"];
         if(spec->getType() == Array)
         {
            DynamicObjectIterator i = spec.getIterator();
            while(rval && i->hasNext())
            {
               DynamicObject& next = i->next();
               rval = _setTarget(ar, next, value);
            }
         }
         else
         {
            rval = _setTarget(ar, spec, value);
         }
      }
      // increase or decrease target
      if(optSpec->hasMember("inc") || optSpec->hasMember("dec"))
      {
         bool inc = optSpec->hasMember("inc");
         int diff = inc ? 1 : -1;
         DynamicObject original;
         rval = _getTarget(ar, inc ? optSpec["inc"] : optSpec["dec"], original);
         if(rval)
         {
            DynamicObject value(NULL);
            switch(original->getType())
            {
               // TODO: deal with overflow?
               case Int32:
                  value = original->getInt32() + diff;
                  break;
               case UInt32:
                  value = original->getUInt32() + diff;
                  break;
               case Int64:
                  value = original->getInt64() + diff;
                  break;
               case UInt64:
                  value = original->getUInt64() + diff;
                  break;
               case Double:
                  value = original->getDouble() + diff;
                  break;
               default:
                  ExceptionRef e = new Exception(
                     "Invalid command line spec. "
                     "The option cannot be changed because it is not a number.",
                     CMDLINE_ERROR);
                  e->getDetails()["option"] = opt;
                  e->getDetails()["spec"] = optSpec;
                  Exception::set(e);
                  rval = false;
            }
            if(rval)
            {
               rval = _setTarget(
                  ar, inc ? optSpec["inc"] : optSpec["dec"], value);
            }
         }
      }
      // append argument to an array
      if(optSpec->hasMember("append"))
      {
         // append string to "append" target
         optSpec["append"]->append() = opt["value"].clone();
      }
      // handle setting a keyword
      if(optSpec->hasMember("keyword"))
      {
         ar->getConfigManager()->setKeyword(
            optSpec["keyword"]->getString(), opt["value"]->getString());
      }
      // handle setting a value
      if(optSpec->hasMember("arg") || optSpec->hasMember("set"))
      {
         // a config or root object must be specified (although "set" could
         // also have "keyword" set to true)
         bool isArg = optSpec->hasMember("arg");
         const char* key = isArg ? "arg" : "set";
         if((!optSpec[key]->hasMember("config") &&
             !optSpec[key]->hasMember("root")) &&
             (!isArg && !optSpec["set"]->hasMember("keyword")))
         {
            ExceptionRef e = new Exception(
               "Invalid command line spec. The option does not specify a "
               "configuration, root object, or keyword to set.",
               CMDLINE_ERROR);
            e->getDetails()["option"] = opt;
            e->getDetails()["spec"] = optSpec;
            Exception::set(e);
            rval = false;
         }
         else
         {
            // to be set to the value from the command line
            DynamicObject value(NULL);

            // "set" takes a path and value from the command line
            if(optSpec->hasMember("set"))
            {
               // Note: No special escaping support for having an '=' in a value

               // parse out the path and value
               const char* arg = opt["value"]->getString();
               const char* eq = strchr(arg, '=');
               if(eq == NULL)
               {
                  ExceptionRef e = new Exception(
                     "Invalid 'set' command line option, no "
                     "<path>=<value> specified.",
                     CMDLINE_ERROR);
                  e->getDetails()["option"] = opt;
                  e->getDetails()["spec"] = optSpec;
                  Exception::set(e);
                  rval = false;
               }
               else
               {
                  // update path and set value
                  optSpec["set"]["path"] = string(arg, eq - arg).c_str();
                  value = DynamicObject();
                  value = string(eq + 1).c_str();
               }
            }
            // "arg" takes just the value from the command line
            else
            {
               value = opt["value"].clone();
            }

            if(rval)
            {
               // do json conversion if requested
               bool isJson =
                  optSpec->hasMember("isJsonValue") &&
                  optSpec["isJsonValue"]->getBoolean();
               if(isJson)
               {
                  // JSON value conversion, use non-strict reader
                  JsonReader jr(false);
                  string tmp = value->getString();
                  ByteArrayInputStream is(tmp.c_str(), tmp.length());
                  jr.start(value);
                  rval = jr.read(&is) && jr.finish();
               }

               // set keyword (interpret path as keyword)
               if(rval && !isArg && optSpec["set"]->hasMember("keyword"))
               {
                  ar->getConfigManager()->setKeyword(
                     optSpec["set"]["path"]->getString(), value->getString());
               }
               // do type conversion
               else if(rval)
               {
                  // default value type to whatever was parsed
                  DynamicObject vt;
                  vt->setType(value->getType());

                  // try to get type from spec
                  if(optSpec->hasMember("type"))
                  {
                     vt = optSpec[key];
                  }
                  // no type in spec so, if not json, preserve old type, if any
                  else if(!isJson)
                  {
                     _getTarget(ar, optSpec[key], vt, false);
                  }

                  // set type and target
                  value->setType(vt->getType());
                  rval = _setTarget(ar, optSpec[key], value);
               }
            }
         }
      }
      // handle setting include value
      if(optSpec->hasMember("include"))
      {
         if(!optSpec["include"]->hasMember("config"))
         {
            ExceptionRef e = new Exception(
               "Invalid command line spec. The option does not specify a "
               "command line config to append config includes to.",
               CMDLINE_ERROR);
            e->getDetails()["option"] = opt;
            e->getDetails()["spec"] = optSpec;
            Exception::set(e);
            rval = false;
         }
         else
         {
            // include config
            Config inc(NULL);
            if(optSpec["include"]->hasMember("params"))
            {
               // use include params from spec
               inc = optSpec["include"]["params"].clone();
            }
            else
            {
               // create default include params
               inc = Config();
               inc["load"] = true;
               inc["optional"] = false;
               inc["includeSubdirectories"] = true;
            }
            inc["path"] = opt["value"].clone();
            optSpec["include"]["config"][ConfigManager::INCLUDE]->append(inc);
         }
      }
   }

   if(rval)
   {
      // option consumed
      opt["consumed"] = true;
   }

   return rval;
}

bool CmdLineParser::processSpec(
   AppRunner* ar, DynamicObject& spec, DynamicObject& options)
{
   bool rval = true;

   // iterate over spec
   DynamicObjectIterator si = spec["options"].getIterator();
   while(rval && si->hasNext())
   {
      DynamicObject& optSpec = si->next();

      // iterate over parsed command line options
      DynamicObjectIterator oi = options.getIterator();
      while(rval && oi->hasNext())
      {
         DynamicObject& opt = oi->next();

         // don't bother with consumed options unless spec says so
         if(!opt["consumed"]->getBoolean() ||
            (optSpec->hasMember("ignoreConsumed") &&
             !optSpec["ignoreConsumed"]->getBoolean()))
         {
            if((opt->hasMember("short") && optSpec->hasMember("short") &&
               opt["short"] == optSpec["short"]) ||
               (opt->hasMember("long") && optSpec->hasMember("long") &&
               opt["long"] == optSpec["long"]))
            {
               rval = _processOption(ar, optSpec, opt);
            }
         }
      }
   }

   return rval;
}

bool CmdLineParser::checkUnknownOptions(DynamicObject& options)
{
   bool rval = true;

   // iterate over parsed command line options looking for unconsumed options
   DynamicObjectIterator oi = options.getIterator();
   while(rval && oi->hasNext())
   {
      DynamicObject& opt = oi->next();
      if(!opt["consumed"]->getBoolean())
      {
         // raise exception for unknown options
         ExceptionRef e = new Exception(
            "Unknown option.",
            CMDLINE_ERROR);
         e->getDetails()["option"] = opt;
         Exception::set(e);
         rval = false;
      }
   }

   return rval;
}
