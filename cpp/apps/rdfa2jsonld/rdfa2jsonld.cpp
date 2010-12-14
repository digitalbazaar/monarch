/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/app/AppFactory.h"
#include "monarch/crypto/MessageDigest.h"
#include "monarch/data/json/JsonWriter.h"
#include "monarch/data/json/JsonLd.h"
#include "monarch/data/rdfa/RdfaReader.h"
#include "monarch/io/FileInputStream.h"
#include "monarch/util/StringTools.h"

#include <cstdio>

using namespace std;
using namespace monarch::app;
using namespace monarch::config;
using namespace monarch::crypto;
using namespace monarch::data::json;
using namespace monarch::data::rdfa;
using namespace monarch::io;
using namespace monarch::modest;
using namespace monarch::rt;
using namespace monarch::util;

#define APP_NAME "monarch.apps.rdfa2jsonld.Rdfa2JsonLd"

namespace monarch
{
namespace apps
{
namespace rdfa2jsonld
{

static bool _processFile(const char* inFile, const char* baseUri)
{
   bool rval;

   // prepare input stream
   File file((FileImpl*)NULL);
   FileInputStream* fis;
   if(inFile == NULL)
   {
      fis = new FileInputStream(FileInputStream::StdIn);
   }
   else
   {
      file = inFile;
      fis = new FileInputStream(file);
   }

   // read in rdfa
   RdfaReader reader;
   reader.setBaseUri(baseUri);
   DynamicObject dyno;
   rval = reader.start(dyno) && reader.read(fis) && reader.finish();

   // close input stream
   fis->close();
   delete fis;

   if(rval)
   {
      // normalize and hash output
      DynamicObject normalized;
      JsonLd::normalize(dyno, normalized);
      MessageDigest md;
      string json = JsonWriter::writeToString(dyno, true, false);
      rval = md.start("SHA1") && md.update(json.c_str(), json.length());
      if(rval)
      {
         // print output
         if(inFile != NULL)
         {
            printf("RDFa to JSON-LD: '%s'\n", file->getAbsolutePath());
         }
         else
         {
            printf("RDFa to JSON-LD:\n");
         }
         printf("Normalized SHA-1 hash: %s\n", md.getDigest().c_str());
         JsonWriter::writeToStdOut(dyno, false, false);
      }
   }

   return rval;
}

class Rdfa2JsonLdApp : public App
{
public:
   Rdfa2JsonLdApp() {};
   virtual ~Rdfa2JsonLdApp() {};
   virtual DynamicObject getCommandLineSpec(Config& cfg)
   {
      // initialize config
      Config& c = cfg[ConfigManager::MERGE][APP_NAME];
      c["baseUri"] = "";

      DynamicObject spec;
      spec["help"] =
"Rdfa2JsonLd Options\n"
"      --base-uri      The base URI to use.\n"
"\n";

      DynamicObject opt(NULL);

      // create option to set base URI
      opt = spec["options"]->append();
      opt["long"] = "--base-uri";
      opt["argError"] = "Base URI must be a string.";
      opt["arg"]["root"] = c;
      opt["arg"]["path"] = "baseUri";

      // use extra options as files to process
      opt = spec["options"]->append();
      opt["extra"]["root"] = c;
      opt["extra"]["path"] = "files";

      return spec;
   };

   /**
    * Runs the app.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool run()
   {
      Config cfg = getConfig()[APP_NAME];
      const char* baseUri = cfg["baseUri"];

      // process files
      DynamicObject& files = cfg["files"];
      if(files->length() == 0)
      {
         printf("Reading RDFa from standard input...\n");

         if(strlen(baseUri) == 0)
         {
            // set a fake stdin base URI
            baseUri = "stdin";
         }

         // create file from std input
         _processFile(NULL, baseUri);
      }
      else
      {
         printf("Reading RDFa from files...\n");

         DynamicObjectIterator i = files.getIterator();
         bool success = true;
         while(success && i->hasNext())
         {
            const char* next = i->next()->getString();

            string _baseUri = baseUri;
            if(_baseUri.length() == 0)
            {
               // set base URI based on file name
               string np;
               File::normalizePath(next, np);
               _baseUri = StringTools::format("file://%s", np.c_str());
            }

            success = _processFile(next, _baseUri.c_str());
         }
      }

      return !Exception::isSet();
   };
};

class Rdfa2JsonLdAppFactory : public AppFactory
{
public:
   Rdfa2JsonLdAppFactory() : AppFactory(APP_NAME, "1.0") {}
   virtual ~Rdfa2JsonLdAppFactory() {}
   virtual App* createApp()
   {
      return new Rdfa2JsonLdApp();
   }
};

} // end namespace rdfa2jsonld
} // end namespace apps
} // end namespace monarch

Module* createModestModule()
{
   return new monarch::apps::rdfa2jsonld::Rdfa2JsonLdAppFactory();
}

void freeModestModule(Module* m)
{
   delete m;
}
