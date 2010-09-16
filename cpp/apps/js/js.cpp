/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/app/AppFactory.h"
#include "monarch/data/json/JsonWriter.h"
#include "monarch/v8/V8ModuleApi.h"

#include <cstdio>

using namespace std;
using namespace monarch::app;
using namespace monarch::config;
using namespace monarch::data::json;
using namespace monarch::io;
using namespace monarch::modest;
using namespace monarch::net;
using namespace monarch::rt;
using namespace monarch::v8;

#define APP_NAME "monarch.apps.js.JavaScriptRunner"

namespace monarch
{
namespace apps
{
namespace js
{

class JsApp : public App
{
protected:
   /**
    * Options from the command line.
    */
   DynamicObject mOptions;

public:
   JsApp()
   {
      // set defaults
      mOptions["commands"]->setType(Array);
      mOptions["files"]->setType(Array);
   };

   virtual ~JsApp() {};

   virtual DynamicObject getCommandLineSpec(Config& cfg)
   {
      DynamicObject spec;
      spec["help"] =
"JS Options\n"
"  -s, --js FILE       Add JavaScript file to run.\n"
"  -c, --command CMD   Add JavaScript string to run.\n"
"\n";

      DynamicObject opt(NULL);

      // create option to add command
      opt = spec["options"]->append();
      opt["short"] = "-c";
      opt["long"] = "--command";
      opt["append"] = mOptions["commands"];
      opt["argError"] = "No command specified.";

      // create option to add js file
      opt = spec["options"]->append();
      opt["short"] = "-s";
      opt["long"] = "--js";
      opt["append"] = mOptions["files"];
      opt["argError"] = "No file specified.";

      return spec;
   };

   /**
    * Runs the app.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool run()
   {
      bool rval = true;
      V8EngineRef v8;

      V8ModuleApi* v8mod = dynamic_cast<V8ModuleApi*>(
         getKernel()->getModuleApi("monarch.v8.V8"));
      rval = (v8mod != NULL);

      if(rval)
      {
         // create an engine
         rval = v8mod->createEngine(v8);
      }

      if(rval)
      {
         // run files
         DynamicObjectIterator i = mOptions["files"].getIterator();
         while(rval && i->hasNext())
         {
            const char* filename = i->next()->getString();
            ByteBuffer script;

            // read whole file
            File f(filename);
            rval = f->exists();
            if(rval)
            {
               script.resize(f->getLength() + 1);
               rval =
                  f.readBytes(&script) &&
                  // ensure null terminated
                  (script.putByte('\0', 1, true) ==  1);
               if(!rval)
               {
                  ExceptionRef e = new Exception(
                     "Error reading scritp.",
                     APP_NAME ".ScriptReadError");
                  e->getDetails()["filename"] = filename;
                  Exception::push(e);
               }
            }
            else
            {
               ExceptionRef e = new Exception(
                  "File not found.",
                  APP_NAME ".FileNotFound");
               e->getDetails()["filename"] = filename;
               Exception::set(e);
            }

            if(rval)
            {
               string result;
               rval = v8->runScript(script.data(), result);
               if(rval)
               {
                  printf("%s", result.c_str());
               }
            }
         }
      }

      if(rval)
      {
         // run commands
         DynamicObjectIterator i = mOptions["commands"].getIterator();
         while(rval && i->hasNext())
         {
            const char* cmd = i->next()->getString();
            string result;
            rval = v8->runScript(cmd, result);
            if(rval)
            {
               printf("%s", result.c_str());
            }
         }
      }

      return rval;
   };
};

class JsAppFactory : public AppFactory
{
public:
   JsAppFactory() : AppFactory(APP_NAME, "1.0")
   {
   }

   virtual ~JsAppFactory() {}

   virtual App* createApp()
   {
      return new JsApp();
   }
};

} // end namespace js
} // end namespace apps
} // end namespace monarch

Module* createModestModule()
{
   return new monarch::apps::js::JsAppFactory();
}

void freeModestModule(Module* m)
{
   delete m;
}
