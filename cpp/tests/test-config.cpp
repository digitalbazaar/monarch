/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/config/ConfigManager.h"
#include "monarch/data/json/JsonWriter.h"
#include "monarch/io/File.h"
#include "monarch/io/FileOutputStream.h"
#include "monarch/rt/Exception.h"
#include "monarch/test/Test.h"
#include "monarch/test/Tester.h"
#include "monarch/test/TestRunner.h"

using namespace std;
using namespace monarch::config;
using namespace monarch::io;
using namespace monarch::rt;
using namespace monarch::test;

void runConfigManagerTest(TestRunner& tr)
{
   tr.group("ConfigManager");

   tr.test("init");
   {
      DynamicObject expect;
      expect->setType(Map);
      ConfigManager cm;
      Config cfg;
      cfg[ConfigManager::ID] = "config";
      cfg[ConfigManager::MERGE]->setType(Map);
      assert(cm.addConfig(cfg));
      assertDynoCmp(cm.getConfig("config", true), cfg);
      assertDynoCmp(cm.getConfig("config", false), expect);
      assertDynoCmp(cm.getConfig("config"), expect);
   }
   tr.passIfNoException();

   tr.test("init & clear");
   {
      DynamicObject expect;
      expect->setType(Map);
      ConfigManager cm;
      Config cfg;
      cfg[ConfigManager::ID] = "config";
      cfg[ConfigManager::MERGE]->setType(Map);
      assert(cm.addConfig(cfg));
      cm.clear();
      Config cfg2 = cm.getConfig("config");
      assert(cfg2.isNull());
   }
   tr.passIfException();

   tr.test("1 config");
   {
      DynamicObject expect;
      expect->setType(Map);
      expect["a"] = 0;
      ConfigManager cm;
      Config cfg;
      cfg[ConfigManager::ID] = "config";
      cfg[ConfigManager::MERGE]["a"] = 0;
      assert(cm.addConfig(cfg));
      assertNoException();
      assertDynoCmp(cm.getConfig("config"), expect);
   }
   tr.passIfNoException();

   tr.test("config change");
   {
      ConfigManager cm;
      Config cfg;
      cfg[ConfigManager::ID] = "config";
      cfg[ConfigManager::MERGE]["a"] = 0;
      assert(cm.addConfig(cfg));
      DynamicObject a;
      a["a"] = 0;
      assertDynoCmp(cm.getConfig("config"), a);
      Config change = cm.getConfig("config", true);
      change[ConfigManager::MERGE]["a"] = 1;
      assert(cm.setConfig(change));
      DynamicObject expect;
      expect["a"] = 1;
      assert(cm.getConfig("config") != a);
      assertDynoCmp(cm.getConfig("config"), expect);
   }
   tr.passIfNoException();

   tr.test("invalid set config");
   {
      ConfigManager cm;
      Config cfg;
      cfg[ConfigManager::ID] = "config";
      cfg[ConfigManager::MERGE]["a"] = 0;
      assert(!cm.setConfig(cfg));
   }
   tr.passIfException();

   tr.test("double add config");
   {
      ConfigManager cm;
      Config cfg;
      cfg[ConfigManager::ID] = "config";
      cfg[ConfigManager::MERGE]["a"] = 0;
      assert(cm.addConfig(cfg));
      cfg[ConfigManager::MERGE]["a"] = 1;
      assert(cm.addConfig(cfg));
      DynamicObject expect;
      expect["a"] = 1;
      assertDynoCmp(cm.getConfig("config"), expect);
   }
   tr.passIfNoException();

   tr.test("add");
   {
      DynamicObject expect;
      expect["a"] = 0;
      expect["b"] = 1;
      expect["c"] = 2;
      ConfigManager cm;
      Config a;
      a[ConfigManager::ID] = "config";
      a[ConfigManager::MERGE]["a"] = 0;
      Config b;
      b[ConfigManager::ID] = "config";
      b[ConfigManager::MERGE]["b"] = 1;
      Config c;
      c[ConfigManager::ID] = "config";
      c[ConfigManager::MERGE]["c"] = 2;
      assert(cm.addConfig(a));
      assertNoException();
      assert(cm.addConfig(b));
      assertNoException();
      assert(cm.addConfig(c));
      assertNoException();
      assertDynoCmp(cm.getConfig("config"), expect);
   }
   tr.passIfNoException();

   tr.test("bad remove");
   {
      ConfigManager cm;
      assert(!cm.removeConfig("error"));
      assertException();
      Exception::clear();
   }
   tr.passIfNoException();

   tr.test("remove");
   {
      DynamicObject expect;
      expect["a"] = 0;
      expect["b"] = 1;
      expect["c"] = 2;
      ConfigManager cm;
      Config a;
      a[ConfigManager::ID] = "config a";
      a[ConfigManager::GROUP] = "group";
      a[ConfigManager::MERGE]["a"] = 0;
      Config b;
      b[ConfigManager::ID] = "config b";
      b[ConfigManager::GROUP] = "group";
      b[ConfigManager::MERGE]["b"] = 1;
      Config c;
      c[ConfigManager::ID] = "config c";
      c[ConfigManager::GROUP] = "group";
      c[ConfigManager::MERGE]["c"] = 2;
      assert(cm.addConfig(a));
      assertNoException();
      assert(cm.addConfig(b));
      assertNoException();
      assert(cm.addConfig(c));
      assertNoException();
      assertDynoCmp(cm.getConfig("group"), expect);
      DynamicObject expect2;
      expect2["a"] = 0;
      expect2["c"] = 2;
      assert(cm.removeConfig("config b"));
      assertDynoCmp(cm.getConfig("group"), expect2);
   }
   tr.passIfNoException();

   tr.test("default value");
   {
      ConfigManager cm;
      Config a;
      a[ConfigManager::ID] = "config a";
      a[ConfigManager::MERGE] = 1;
      assert(cm.addConfig(a));
      assertNoException();
      Config b;
      b[ConfigManager::ID] = "config b";
      b[ConfigManager::PARENT] = "config a";
      b[ConfigManager::MERGE] = ConfigManager::DEFAULT_VALUE;
      assert(cm.addConfig(b));
      assertNoException();
      DynamicObject expect;
      expect = 1;
      assertDynoCmp(cm.getConfig("config b"), expect);
   }
   tr.passIfNoException();

   tr.test("default values");
   {
      ConfigManager cm;
      Config cfga;
      cfga[ConfigManager::ID] = "config a";
      Config& a = cfga[ConfigManager::MERGE];
      a[0] = 10;
      a[1] = 11;
      a[2]["0"] = 120;
      a[2]["1"] = 121;
      assert(cm.addConfig(cfga));
      assertNoException();
      Config cfgb;
      cfgb[ConfigManager::ID] = "config b";
      cfgb[ConfigManager::PARENT] = "config a";
      Config& b = cfgb[ConfigManager::MERGE];
      b[0] = ConfigManager::DEFAULT_VALUE;
      b[1] = 21;
      b[2]["0"] = ConfigManager::DEFAULT_VALUE;
      b[2]["1"] = 221;
      assert(cm.addConfig(cfgb));
      assertNoException();
      DynamicObject expect;
      expect[0] = 10;
      expect[1] = 21;
      expect[2]["0"] = 120;
      expect[2]["1"] = 221;
      assertDynoCmp(cm.getConfig("config b"), expect);
   }
   tr.passIfNoException();


   tr.test("keyword substitution {RESOURCE_DIR}");
   {
      DynamicObject expect;
      expect["dir"] = "/the/real/dir";
      expect["dir-plus"] = "/the/real/dir/plus/more";
      //expect["name"] = "Digital Bazaar, Inc.";
      ConfigManager cm;
      Config a;
      a[ConfigManager::ID] = "config";
      a[ConfigManager::MERGE]["dir"] = "{RESOURCE_DIR}";
      a[ConfigManager::MERGE]["dir-plus"] = "{RESOURCE_DIR}/plus/more";
      // FIXME: only supports "{RESOURCE_DIR}" now
      //a[ConfigManager::MERGE]["other"] = "{DB}";
      cm.setKeyword("RESOURCE_DIR", "/the/real/dir");
      //cm.setKeyword("DB", "Digital Bazaar, Inc.");
      assert(cm.addConfig(a));
      assertNoException();
      assertDynoCmp(cm.getConfig("config"), expect);
   }
   tr.passIfNoException();

   tr.test("keyword substitution {CURRENT_DIR}");
   {
      DynamicObject expect;
      string cwd;
      string cwdPlusMore;
      string absoluteDir;
      File configFile = File::createTempFile("test-config-file");
      FileOutputStream fos(configFile);

      // create and populate the config file
      string configFileText =
         "{\n"
         "\"_id_\": \"config\",\n"
         "\"_merge_\": {\n"
         "   \"dir\": \"{CURRENT_DIR}\",\n"
         "   \"dir-plus\": \"{CURRENT_DIR}/plus/more\" }\n"
         "}\n";
      fos.write(configFileText.c_str(), configFileText.length());
      fos.close();

      // modify the current working directory to the expected value
      absoluteDir = File::dirname(configFile->getAbsolutePath());
      cwd = absoluteDir.c_str();
      cwdPlusMore = cwd.c_str();
      cwdPlusMore.append("/plus/more");

      // set the expected values
      expect["dir"] = cwd.c_str();
      expect["dir-plus"] = cwdPlusMore.c_str();

      // create the configuration
      ConfigManager cm;
      assert(cm.addConfigFile(configFile->getAbsolutePath(),
         true, absoluteDir.c_str(), true, false));
      assertNoException();
      assertDynoCmp(cm.getConfig("config"), expect);
   }
   tr.passIfNoException();

#if 0
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
      assert(cm.addConfig(nodec));
      assertNoException();

      // user
      // loaded defaults
      DynamicObject userc;
      userc["node"]["port"] = 19100;
      userc["node"]["comment"] = "My precious...";
      assert(cm.addConfig(userc, ConfigManager::Custom));
      assertNoException();

      // user makes changes during runtime
      DynamicObject c = cm.getConfig();
      c["node"]["port"] = 19200;
      c["node"]["userModulePath"] = "~/.bitmunk/modules:~/.bitmunk/modules-dev";
      c["node"][ConfigManager::TMP]["not in changes"] = true;

      // get the changes from defaults to current config
      // serialize this to disk as needed
      DynamicObject changes;
      cm.getChanges(changes);

      // check it's correct
      DynamicObject expect;
      expect["node"]["port"] = 19200;
      expect["node"]["comment"] = "My precious...";
      expect["node"]["userModulePath"] = "~/.bitmunk/modules:~/.bitmunk/modules-dev";
      // NOTE: will not have TMP var
      assertDynoCmp(changes, expect);
   }
   tr.passIfNoException();
#endif

   tr.test("versioning");
   {
      ConfigManager cm;

      cm.getVersions()->clear();
      Config c;
      c[ConfigManager::ID] = "config";
      assert(cm.addConfig(c));
      assertNoException();

      cm.addVersion("1");
      assert(!cm.addConfig(c));
      assertException();
      Exception::clear();

      c[ConfigManager::VERSION] = "2";
      cm.removeConfig("config");
      assert(!cm.addConfig(c));
      assertException();
      Exception::clear();

      c[ConfigManager::VERSION] = "1";
      assert(cm.addConfig(c));
      assertNoException();

      c[ConfigManager::VERSION] = "2";
      cm.removeConfig("config");
      cm.addVersion("2");
      assert(cm.addConfig(c));
      assertNoException();
   }
   tr.passIfNoException();

   tr.test("empty array & map");
   {
      ConfigManager cm;
      DynamicObject a;
      a[ConfigManager::ID] = "config";
      a[ConfigManager::MERGE][0]->setType(Array);
      a[ConfigManager::MERGE][1]->setType(Map);
      assert(cm.addConfig(a));
      assertNoException();
      DynamicObject expect;
      expect[0]->setType(Array);
      expect[1]->setType(Map);
      assertDynoCmp(cm.getConfig("config"), expect);
   }
   tr.passIfNoException();

   tr.test("empty group ids");
   {
      ConfigManager cm;
      DynamicObject expect;
      expect->setType(Array);
      assertDynoCmp(cm.getIdsInGroup("Not-A-Group"), expect);
   }
   tr.passIfNoException();

   tr.test("group ids");
   {
      ConfigManager cm;
      DynamicObject c;

      c[ConfigManager::ID] = "c0";
      c[ConfigManager::GROUP] = "c";
      assert(cm.addConfig(c));
      assertNoException();

      c[ConfigManager::ID] = "c1";
      c[ConfigManager::GROUP] = "c";
      assert(cm.addConfig(c));
      assertNoException();

      DynamicObject expect;
      expect->setType(Array);
      expect[0] = "c0";
      expect[1] = "c1";
      assertDynoCmp(cm.getIdsInGroup("c"), expect);
   }
   tr.passIfNoException();

   tr.test("replace keywords");
   {
      ConfigManager cm;
      DynamicObject c;
      bool success;

      c[ConfigManager::ID] = "c";
      c[ConfigManager::MERGE]["test"] = "{A}";
      DynamicObject vars;
      vars["A"] = "a";
      success = ConfigManager::replaceKeywords(c, vars);
      assertNoException();
      assert(success);

      DynamicObject expect;
      expect[ConfigManager::ID] = "c";
      expect[ConfigManager::MERGE]["test"] = "a";
      assertDynoCmp(c, expect);
   }
   tr.passIfNoException();

   tr.test("replace keywords (invalid keyword)");
   {
      ConfigManager cm;
      DynamicObject c;
      bool success;

      c[ConfigManager::ID] = "c";
      c[ConfigManager::MERGE]["test"] = "{UNKNOWN}";
      DynamicObject vars;
      vars["A"] = "a";
      success = ConfigManager::replaceKeywords(c, vars);
      assertException();
      assert(!success);
   }
   tr.passIfException();

   tr.ungroup();
}

class DbConfigTester : public monarch::test::Tester
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

monarch::test::Tester* getDbConfigTester() { return new DbConfigTester(); }


DB_TEST_MAIN(DbConfigTester)
