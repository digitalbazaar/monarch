/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/app/AppFactory.h"
#include "monarch/crypto/MessageDigest.h"
#include "monarch/data/json/JsonWriter.h"
#include "monarch/data/json/JsonLd.h"
#include "monarch/data/rdfa/RdfaReader.h"
#include "monarch/http/HttpClient.h"
#include "monarch/io/ByteArrayInputStream.h"
#include "monarch/io/FileInputStream.h"
#include "monarch/logging/Logging.h"
#include "monarch/net/Url.h"
#include "monarch/util/StringTools.h"

#include <cstdio>

using namespace std;
using namespace monarch::app;
using namespace monarch::config;
using namespace monarch::crypto;
using namespace monarch::data::json;
using namespace monarch::data::rdfa;
using namespace monarch::http;
using namespace monarch::io;
using namespace monarch::logging;
using namespace monarch::modest;
using namespace monarch::net;
using namespace monarch::rt;
using namespace monarch::util;

#define APP_NAME "monarch.apps.rdfa2jsonld.Rdfa2JsonLd"

namespace monarch
{
namespace apps
{
namespace rdfa2jsonld
{

static bool _processStream(
   InputStream* is, const char* srcName, const char* baseUri)
{
   bool rval;

   // read in rdfa
   RdfaReader reader;
   reader.setBaseUri(baseUri);
   DynamicObject dyno;
   rval = reader.start(dyno) && reader.read(is) && reader.finish();

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
         if(srcName != NULL)
         {
            printf("RDFa to JSON-LD: '%s'\n", srcName);
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

static bool _processFile(const char* inFile, const char* baseUri)
{
   bool rval;

   string _baseUri = baseUri;
   if(_baseUri.length() == 0)
   {
      // set base URI based on file name
      string path;
      if(!File::getAbsolutePath(inFile, path))
      {
         MO_CAT_ERROR(MO_APP_CAT,
            "Error getting absolute path for '%s'.", inFile);
      }
      _baseUri = StringTools::format("file://%s", path.c_str());
   }
   string _srcName = ((inFile != NULL) ? inFile : "");
   if(_srcName.length() == 0)
   {
      _srcName = "stdin";
   }

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

   rval = _processStream(fis, _srcName.c_str(), _baseUri.c_str());

   // close input stream
   fis->close();
   delete fis;

   return rval;
}

static bool _processHttp(Url& url, const char* baseUri)
{
   bool rval;

   string _baseUri = baseUri;
   if(_baseUri.length() == 0)
   {
      // default to URL
      _baseUri = url.toString();
   }

   HttpClient client;

   rval = client.connect(&url);
   if(rval)
   {
      string content;
      HttpResponse* response = client.get(&url);
      if(response != NULL)
      {
         if(client.receiveContent(content))
         {
            ByteArrayInputStream is(content.c_str(), content.length());
            rval = _processStream(&is, _baseUri.c_str(), _baseUri.c_str());
         }
         else
         {
            MO_CAT_ERROR(MO_APP_CAT,
               "IO Exception for URL '%s'.", url.toString().c_str());
         }
      }
   }
   else
   {
      MO_CAT_ERROR(MO_APP_CAT,
         "Connection problem for URL '%s'.", url.toString().c_str());
   }

   client.disconnect();

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
      opt["extra"]["path"] = "urls";

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
      DynamicObject& urls = cfg["urls"];
      if(urls->length() == 0)
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
         printf("Reading RDFa from URLs or files...\n");

         DynamicObjectIterator i = urls.getIterator();
         bool success = true;
         while(success && i->hasNext())
         {
            const char* next = i->next()->getString();

            Url url;
            if(url.setUrl(next))
            {
               string scheme = url.getScheme();
               if(scheme == "")
               {
                  // assume a regular file
                  success = _processFile(next, baseUri);
               }
               else if(scheme == "file")
               {
                  success = _processFile(url.getPath().c_str(), baseUri);
               }
               else if(scheme == "http" || scheme == "https")
               {
                  success = _processHttp(url, baseUri);
               }
               else
               {
                  MO_CAT_ERROR(MO_APP_CAT,
                     "Unknown URL scheme for '%s'.", next);
                  success = false;
               }
            }
            else
            {
               // failed to set as URL, assume a simple file string
               Exception::clear();
               success = _processFile(next, baseUri);
            }
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
