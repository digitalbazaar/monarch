/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 * 
 * This test file is used to test the dbcore ConfigManager without the need
 * for it to use the dbcore App Tester framework -- which itself depends on
 * the ConfigManager. Only simple tests are run, namely tests to ensure that
 * the simplest config files can be loaded and that the simplest configuration
 * values can be combined and merged together, including groups.
 */
#include <iostream>

#include "db/config/ConfigManager.h"
#include "db/data/json/JsonWriter.h"
#include "db/io/FileOutputStream.h"
#include "db/test/Test.h"

using namespace std;
using namespace db::config;
using namespace db::data::json;
using namespace db::io;
using namespace db::rt;
using namespace db::test;

void testConfigFiles()
{
   ConfigManager cm;
   
   // FIXME: do stuff with config files
}

void testConfigs()
{
   ConfigManager cm;
   
   // build system config
   Config system;
   {
      // set properties
      system[ConfigManager::ID] = "system";
      
      // set merge info
      Config& merge = system[ConfigManager::MERGE];
      merge["path"]->setType(Array);
      merge["cowSays"] = "moo";
      merge["dogSays"] = "bowwow";
      merge["fruits"]["apple"] = "red";
      merge["vegetables"]["carrot"] = "orange";
      merge["vegetables"]["pepper"]->append() = "green";
   }
   
   // build engine config
   Config engine;
   {
      // set properties
      engine[ConfigManager::ID] = "engine";
      engine[ConfigManager::PARENT] = "system";
      engine[ConfigManager::GROUP] = "app";
      
      // set merge info
      Config& merge = engine[ConfigManager::MERGE];
      merge["dogSays"] = "woof";
      merge["fruits"]["banana"] = "yellow";
      merge["vegetables"]["pepper"]->append() = "red";
      
      // set append info
      Config& append = engine[ConfigManager::APPEND];
      append["path"]->append() = "/usr/bin";
   }
   
   // build ui config
   Config ui;
   {
      // set properties
      ui[ConfigManager::ID] = "ui";
      ui[ConfigManager::PARENT] = "system";
      ui[ConfigManager::GROUP] = "app";
      
      // set merge info
      Config& merge = ui[ConfigManager::MERGE];
      merge["fruits"]["pear"] = "green";
      merge["vegetables"]["pepper"]->append() = "red";
      
      // set append info
      Config& append = ui[ConfigManager::APPEND];
      append["path"]->append() = "/tmp/ui-tool";
   }
   
   // build user1 config
   Config user1;
   {
   }
   
   // build user2 config
   Config user2;
   {
   }
   
   // build child2 config
   // *Note: child2 is a child user that is based off of user2.
   Config child2;
   {
   }
   
   // add system config
   {
      printf("Testing adding system config...\n");
      
      cm.addConfig(system);
      
      assertNoException();
      printf("PASS.\n");
   }
   
   // add engine config
   {
      printf("Testing adding engine config...\n");
      
      cm.addConfig(engine);
      
      assertNoException();
      printf("PASS.\n");
   }
   
   // add ui config
   {
      printf("Testing adding ui config...\n");
      
      cm.addConfig(ui);
      
      assertNoException();
      printf("PASS.\n");
   }
#if 0
   // add user1 config
   {
      printf("Testing adding user1 config...\n");
      
      cm.addConfig(user1);
      
      assertNoException();
      printf("PASS.\n");
   }
   
   // add user2 config
   {
      printf("Testing adding user2 config...\n");
      
      cm.addConfig(user2);
      
      assertNoException();
      printf("PASS.\n");
   }
   
   // add child2 config
   {
      printf("Testing adding child2 config...\n");
      
      cm.addConfig(child2);
      
      assertNoException();
      printf("PASS.\n");
   }
#endif
   
   // FIXME: might have to recreate configs to compare them properly
   
   // test system raw config
   {
      printf("Testing system raw config...\n");
      
      Config raw(NULL);
      cm.getConfig("system", raw, true);
      assertNoException();
      assertDynoCmp(raw, system);
      
      printf("PASS.\n");
   }
   
   // test system merged config
   {
      printf("Testing system merged config...\n");
      
      // FIXME:
      
      assertNoException();
      printf("PASS.\n");
   }
   
   // test engine raw config
   {
      printf("Testing engine raw config...\n");
      
      Config raw(NULL);
      cm.getConfig("engine", raw, true);
      assertNoException();
      assertDynoCmp(raw, engine);
      
      assertNoException();
      printf("PASS.\n");
   }
   
   // test engine merged config
   {
      printf("Testing engine merged config...\n");
      
      // FIXME:
      
      assertNoException();
      printf("PASS.\n");
   }
   
   // test ui raw config
   {
      printf("Testing ui raw config...\n");
      
      Config raw(NULL);
      cm.getConfig("ui", raw, true);
      assertNoException();
      assertDynoCmp(raw, ui);
      
      assertNoException();
      printf("PASS.\n");
   }
   
   // test ui merged config
   {
      printf("Testing ui merged config...\n");
      
      // FIXME:
      
      assertNoException();
      printf("PASS.\n");
   }
   
   // test app group raw config
   {
      printf("Testing app group raw config...\n");
      
      // FIXME:
      
      assertNoException();
      printf("PASS.\n");
   }
   
   // test app group merged config
   {
      printf("Testing app group merged config...\n");
      
      // FIXME:
      
      assertNoException();
      printf("PASS.\n");
   }
   
   // test user1 raw config
   {
      printf("Testing user1 raw config...\n");
      
      // FIXME:
      
      assertNoException();
      printf("PASS.\n");
   }
   
   // test user1 merged config
   {
      printf("Testing user1 merged config...\n");
      
      // FIXME:
      
      assertNoException();
      printf("PASS.\n");
   }
   
   // test user2 raw config
   {
      printf("Testing user2 raw config...\n");
      
      // FIXME:
      
      assertNoException();
      printf("PASS.\n");
   }
   
   // test user2 merged config
   {
      printf("Testing user2 merged config...\n");
      
      // FIXME:
      
      assertNoException();
      printf("PASS.\n");
   }
   
   // test child2 raw config
   {
      printf("Testing child2 raw config...\n");
      
      // FIXME:
      
      assertNoException();
      printf("PASS.\n");
   }
   
   // test child2 merged config
   {
      printf("Testing child2 merged config...\n");
      
      // FIXME:
      
      assertNoException();
      printf("PASS.\n");
   }
}

int main()
{
   printf("Testing ConfigManager...\n\n");
   
   testConfigs();
   testConfigFiles();
   
   printf("\nALL TESTS PASS.\n");
   
   return 0;
}
