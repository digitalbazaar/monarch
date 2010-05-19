/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/app/CmdLineParser.h"

#include "monarch/app/App.h"

using namespace std;
using namespace monarch::app;
using namespace monarch::config;
using namespace monarch::data::json;
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
               "by a single character start or be a double-hyphen followed "
               "by a keyword.",
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
         opt["long"] = option;

         // store option argument value if applicable
         if((i + 1 < argc) && _getOptionType(argv[i + 1]) == -1)
         {
            opt["value"] = argv[++i];
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
            tmp.push_back(arg[shorti]);
            DynamicObject& opt = opts->append();
            opt["consumed"] = false;
            opt["short"] = tmp.c_str();
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
 *
 * @return a pointer to the found object.
 */
static DynamicObject* _findPath(
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
               CMDLINE_ERROR);
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
                     CMDLINE_ERROR);
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

// get read-only object
// use main config rather than optionally specified one
static bool _getTarget(
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
      DynamicObject* obj = _findPath(spec["root"], path);
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
      Config config = app->getConfig();
      DynamicObject* obj = _findPath(config, path, false);
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
 * @param app the App.
 * @param spec the command line spec option.
 * @param value the value to set the target to.
 *
 * @return true if the target was set, false if not.
 */
static bool _setTarget(App* app, DynamicObject& spec, DynamicObject& value)
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
      Config rawConfig = app->getConfigManager()->getConfig(configName, true);
      rval = _setTargetAtPath(rawConfig[ConfigManager::MERGE], path, value);
      if(rval)
      {
         rval = app->getConfigManager()->setConfig(rawConfig);
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
 * @param app the App.
 * @param optSpec the spec for the command line option.
 * @param opt the command line option to process.
 *
 * @return true if successful, false if an exception occurred.
 */
static bool _processOption(
   App* app, DynamicObject& optSpec, DynamicObject& opt)
{
   bool rval = true;

   // make sure value's existence in option is correct based on the spec type
   if(!opt->hasMember("value") &&
      (optSpec->hasMember("set") ||
       optSpec->hasMember("arg") ||
       optSpec->hasMember("append") ||
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
   // set target to true or false
   else if(optSpec->hasMember("setTrue") || optSpec->hasMember("setFalse"))
   {
      DynamicObject value;
      value = (optSpec->hasMember("setTrue"));
      DynamicObject& spec = value->getBoolean() ?
         optSpec["setTrue"] : optSpec["setFalse"];
      if(spec->getType() == Array)
      {
         DynamicObjectIterator i = spec.getIterator();
         while(rval && i->hasNext())
         {
            DynamicObject& next = i->next();
            rval = _setTarget(app, next, value);
         }
      }
      else
      {
         rval = _setTarget(app, spec, value);
      }
   }
   // increase or decrease target
   else if(optSpec->hasMember("inc") || optSpec->hasMember("dec"))
   {
      bool inc = optSpec->hasMember("inc");
      int diff = inc ? 1 : -1;
      DynamicObject original;
      rval = _getTarget(app, inc ? optSpec["inc"] : optSpec["dec"], original);
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
               app, inc ? optSpec["inc"] : optSpec["dec"], value);
         }
      }
   }
   // append argument to an array
   else if(optSpec->hasMember("append"))
   {
      // append string to "append" target
      optSpec["append"]->append() = opt["value"].clone();
   }
   // handle setting a keyword
   else if(optSpec->hasMember("keyword"))
   {
      app->getConfigManager()->setKeyword(
         optSpec["keyword"]->getString(), opt["value"]->getString());
   }
   // handle setting include value
   else if(optSpec->hasMember("include"))
   {
      // FIXME: implement me (check for "config"), use value as path,
      // see AppConfig for other things to add
   }
   // handle setting a value
   else if(optSpec->hasMember("arg") || optSpec->hasMember("set"))
   {
      // a config or root object must be specified
      const char* key = optSpec->hasMember("arg") ? "arg" : "set";
      if(!optSpec[key]->hasMember("config") && !optSpec[key]->hasMember("root"))
      {
         ExceptionRef e = new Exception(
            "Invalid command line spec. The option does not specify a "
            "configuration or root object to set.",
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
            const char* eq = strstr(arg, '=');
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
               opt["set"]["path"] = string(arg, eq - arg).c_str();
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
            if(optSpec->hasMember("isJsonValue") &&
               optSpec["isJsonValue"]->getBoolean())
            {
               // JSON value conversion, use non-strict reader
               JsonReader jr(false);
               string tmp = value->getString();
               ByteArrayInputStream is(tmp, tmp.length());
               jr.start(value);
               rval = jr.read(&is) && jr.finish();
            }

            // do type conversion
            if(rval)
            {
               // default value type to string
               DynamicObject vt;
               vt->setType(String);

               // try to get get type from spec
               if(optSpec->hasMember("type"))
               {
                  vt = optSpec[key];
               }
               // no type in spec so preserve old type
               else
               {
                  _getTarget(app, optSpec[key], valueType, false);
               }

               // set type and target
               value->setType(vt->getType());
               rval = _setTarget(app, optSpec[key], value);
            }
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
   App* app, DynamicObject& spec, DynamicObject& options)
{
   bool rval = true;

   // iterate over spec
   DynamicObjectIterator si = spec.getIterator();
   while(rval && si->hasNext())
   {
      DynamicObject& optSpec = si->next();

      // iterate over parsed command line options
      DynamicObjectIterator oi = options.getIterator();
      while(rval && oi->hasNext())
      {
         DynamicObject& opt = oi->next();
         if((opt->hasMember("short") && optSpec->hasMember("short") &&
            opt["short"] == optSpec["short"]) ||
            (opt->hasMember("long") && optSpec->hasMember("long") &&
            opt["long"] == optSpec["long"]))
         {
            rval = _processOption(app, optSpec, opt);
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
