/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include <iostream>
#include <sstream>

#include "db/rt/Exception.h"
#include "db/test/Test.h"
#include "db/test/Tester.h"
#include "db/test/TestRunner.h"
#include "db/config/ConfigManager.h"

using namespace std;
using namespace db::rt;
using namespace db::test;
using namespace db::config;

void runConfigManagerTest(TestRunner& tr)
{
   tr.group("ConfigManager");
   
   tr.test("init");
   {
      DynamicObject expect;
      expect->setType(Map);
      ConfigManager cm;
      assert(cm.getConfig() == expect);
   }
   tr.passIfNoException();
   
   tr.test("init & clear");
   {
      DynamicObject expect;
      expect->setType(Map);
      ConfigManager cm;
      cm.clear();
      assert(cm.getConfig() == expect);
   }
   tr.passIfNoException();
   
   tr.test("1 config");
   {
      DynamicObject expect;
      expect->setType(Map);
      expect["a"] = 0;
      ConfigManager cm;
      DynamicObject a;
      a["a"] = 0;
      cm.addConfig(a);
      assert(cm.getConfig() == expect);
   }
   tr.passIfNoException();
   
   tr.test("clear & 1 config");
   {
      DynamicObject expect;
      expect->setType(Map);
      expect["a"] = 0;
      ConfigManager cm;
      cm.clear();
      DynamicObject a;
      a["a"] = 0;
      cm.addConfig(a);
      assert(cm.getConfig() == expect);
   }
   tr.passIfNoException();
   
   tr.test("config change");
   {
      ConfigManager cm;
      DynamicObject a;
      a["a"] = 0;
      cm.addConfig(a);
      assert(cm.getConfig() == a);
      cm.getConfig()["a"] = 1;
      DynamicObject expect;
      expect["a"] = 1;
      assert(cm.getConfig() != a);
      assert(cm.getConfig() == expect);
   }
   tr.passIfNoException();

   tr.test("add");
   {
      DynamicObject expect;
      expect["a"] = 0;
      expect["b"] = 1;
      expect["c"] = 2;
      ConfigManager cm;
      DynamicObject a;
      a["a"] = 0;
      DynamicObject b;
      b["b"] = 1;
      DynamicObject c;
      c["c"] = 2;
      cm.addConfig(a);
      cm.addConfig(b);
      cm.addConfig(c);
      assert(cm.getConfig() == expect);
   }
   tr.passIfNoException();

   tr.test("bad remove");
   {
      ConfigManager cm;
      assert(!cm.removeConfig(0));
      assertException();
      Exception::clearLast();
   }
   tr.passIfNoException();

   tr.test("remove");
   {
      DynamicObject expect;
      expect["a"] = 0;
      expect["b"] = 1;
      expect["c"] = 2;
      ConfigManager cm;
      DynamicObject a;
      a["a"] = 0;
      DynamicObject b;
      b["b"] = 1;
      DynamicObject c;
      c["c"] = 2;
      ConfigManager::ConfigId id;
      cm.addConfig(a);
      cm.addConfig(b, ConfigManager::Default, &id);
      cm.addConfig(c);
      assert(cm.getConfig() == expect);
      DynamicObject expect2;
      expect2["a"] = 0;
      expect2["c"] = 2;
      assert(cm.removeConfig(id));
      assert(cm.getConfig() == expect2);
   }
   tr.passIfNoException();

   tr.test("update");
   {
      ConfigManager cm;
      DynamicObject expect;
      expect["a"] = 0;
      DynamicObject a;
      a["a"] = 0;
      cm.addConfig(a);
      assert(cm.getConfig() == expect);
      DynamicObject expect2;
      expect2["a"] = 1;
      a["a"] = 1;
      assert(cm.getConfig() != expect2);
      cm.update();
      assert(cm.getConfig() == expect2);
   }
   tr.passIfNoException();

   tr.test("set");
   {
      ConfigManager cm;
      DynamicObject expect;
      expect["a"] = 0;
      DynamicObject a;
      a["a"] = 0;
      ConfigManager::ConfigId id;
      cm.addConfig(a, ConfigManager::Default, &id);
      assert(cm.getConfig() == expect);
      DynamicObject expect2;
      expect2["b"] = 0;
      DynamicObject b;
      b["b"] = 0;
      cm.setConfig(id, b);
      assert(cm.getConfig() == expect2);
   }
   tr.passIfNoException();

   tr.test("get");
   {
      ConfigManager cm;
      DynamicObject expect;
      expect["a"] = 0;
      DynamicObject a;
      a["a"] = 0;
      ConfigManager::ConfigId id;
      cm.addConfig(a, ConfigManager::Default, &id);
      assert(cm.getConfig() == expect);
      DynamicObject b;
      assert(cm.getConfig(id, b));
      assert(b == expect);
   }
   tr.passIfNoException();

   tr.test("map changes");
   {
      ConfigManager cm;
      DynamicObject a;
      a["a"] = 0;
      a["b"] = 0;
      cm.addConfig(a);
      cm.getConfig()["a"] = 1;
      DynamicObject expect;
      expect["a"] = 1;
      DynamicObject changes;
      cm.getChanges(changes);
      assert(changes == expect);
   }
   tr.passIfNoException();

   tr.test("deep map changes");
   {
      ConfigManager cm;
      DynamicObject a;
      a["a"]["b"] = 0;
      a["a"]["c"] = 0;
      cm.addConfig(a);
      cm.getConfig()["a"]["c"] = 1;
      cm.getConfig()["d"] = 0;
      DynamicObject expect;
      expect["a"]["c"] = 1;
      expect["d"] = 0;
      DynamicObject changes;
      cm.getChanges(changes);
      assert(changes == expect);
   }
   tr.passIfNoException();

   tr.test("array changes");
   {
      ConfigManager cm;
      DynamicObject a;
      a[0] = 10;
      a[1] = 11;
      a[2] = 12;
      cm.addConfig(a);
      cm.getConfig()[1] = 21;
      DynamicObject expect;
      expect[0] = "__default__";
      expect[1] = 21;
      expect[2] = "__default__";
      DynamicObject changes;
      cm.getChanges(changes);
      assert(changes == expect);
   }
   tr.passIfNoException();

   tr.test("bigger array changes");
   {
      ConfigManager cm;
      DynamicObject a;
      a[0] = 10;
      a[1] = 11;
      cm.addConfig(a);
      cm.getConfig()[2] = 22;
      DynamicObject expect;
      expect[0] = "__default__";
      expect[1] = "__default__";
      expect[2] = 22;
      DynamicObject changes;
      cm.getChanges(changes);
      assert(changes == expect);
   }
   tr.passIfNoException();

   tr.test("system vs user changes");
   {
      ConfigManager cm;

      // system
      DynamicObject a;
      a[0] = 10;
      a[1] = 11;
      cm.addConfig(a, ConfigManager::Default);

      // user
      DynamicObject b;
      b[0] = 20;
      b[1] = 21;
      cm.addConfig(b, ConfigManager::User);
      
      // custom
      cm.getConfig()[1] = 31;

      {
         // Changes from system configs
         DynamicObject expect;
         expect[0] = 20;
         expect[1] = 31;
         DynamicObject changes;
         cm.getChanges(changes);
         assert(changes == expect);
      }
      
      {
         // Changes from system+user configs
         DynamicObject expect;
         expect[0] = "__default__";
         expect[1] = 31;
         DynamicObject changes;
         cm.getChanges(changes, ConfigManager::All);
         assert(changes == expect);
      }
   }
   tr.passIfNoException();

   tr.test("default value");
   {
      ConfigManager cm;
      DynamicObject a;
      a = 1;
      cm.addConfig(a);
      DynamicObject b;
      b = "__default__";
      cm.addConfig(b);
      DynamicObject expect;
      expect = 1;
      assert(cm.getConfig() == expect);
   }
   tr.passIfNoException();

   tr.test("default values");
   {
      ConfigManager cm;
      DynamicObject a;
      a[0] = 10;
      a[1] = 11;
      a[2]["0"] = 120;
      a[2]["1"] = 121;
      cm.addConfig(a);
      DynamicObject b;
      b[0] = "__default__";
      b[1] = 21;
      b[2]["0"] = "__default__";
      b[2]["1"] = 221;
      cm.addConfig(b);
      DynamicObject expect;
      expect[0] = 10;
      expect[1] = 21;
      expect[2]["0"] = 120;
      expect[2]["1"] = 221;
      assert(cm.getConfig() == expect);
   }
   tr.passIfNoException();

   tr.test("schema check");
   {
      DynamicObject schema;
      DynamicObject config;
      assert(ConfigManager::isValidConfig(config, schema));
      schema->setType(Map);
      config->setType(Map);
      assert(ConfigManager::isValidConfig(config, schema));
      schema["s"] = "";
      schema["i"] = 0;
      config["s"] = "string";
      config["i"] = 1;
      assert(ConfigManager::isValidConfig(config, schema));
      schema["m"]["s"] = "";
      schema["m"]["s2"] = "";
      schema["a"][0] = 0;
      schema["a"][1] = 1;
      config["m"]["s"] = "s";
      config["m"]["s2"] = "s2";
      config["a"][0] = 0;
      config["a"][1] = 1;
   }
   tr.passIfNoException();

   tr.test("schema check bad");
   {
      DynamicObject schema;
      DynamicObject config;
      assert(ConfigManager::isValidConfig(config, schema));
      schema->setType(Map);
      config->setType(Array);
      assert(!ConfigManager::isValidConfig(config, schema));
      config->setType(Map);
      schema["s"] = "";
      schema["i"] = 0;
      config["s"] = 1;
      config["i"] = "string";
      assert(!ConfigManager::isValidConfig(config, schema));
   }
   tr.passIfNoException();

   tr.test("user preferences");
   {
      ConfigManager cm;

      // node
      // built in or loaded defaults
      DynamicObject nodec;
      nodec["node"]["host"] = "localhost";
      nodec["node"]["port"] = 19100;
      nodec["node"]["modulePath"] = "/usr/lib/bitmunk/modules";
      nodec["node"]["userModulePath"] = "~/.bitmunk/modules";
      cm.addConfig(nodec);

      // user
      // loaded defaults
      DynamicObject userc;
      userc["node"]["port"] = 19100;
      userc["node"]["comment"] = "My precious...";
      cm.addConfig(userc, ConfigManager::User);
      
      // user makes changes during runtime
      DynamicObject c = cm.getConfig();
      c["node"]["port"] = 19200;
      c["node"]["userModulePath"] = "~/.bitmunk/modules:~/.bitmunk/modules-dev";

      // get the changes from defaults to current config
      // serialize this to disk as needed
      DynamicObject changes;
      cm.getChanges(changes);

      // check it's correct
      DynamicObject expect;
      expect["node"]["port"] = 19200;
      expect["node"]["comment"] = "My precious...";
      expect["node"]["userModulePath"] = "~/.bitmunk/modules:~/.bitmunk/modules-dev";
      assert(changes == expect);
   }
   tr.passIfNoException();
   
   tr.ungroup();
}

class DbConfigTester : public db::test::Tester
{
public:
   DbConfigTester()
   {
      setName("config");
   }

   /**
    * Run automatic unit tests.
    */
   virtual int runAutomaticTests(TestRunner& tr)
   {
      runConfigManagerTest(tr);
      return 0;
   }

   /**
    * Runs interactive unit tests.
    */
   virtual int runInteractiveTests(TestRunner& tr)
   {
      return 0;
   }
};

#ifndef DB_TEST_NO_MAIN
DB_TEST_MAIN(DbConfigTester)
#endif
