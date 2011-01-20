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
   DynamicObject& options, InputStream* is, const char* srcName,
   const char* baseUri)
{
   bool rval;

   // options
   bool doFilter = options["filter"]->getBoolean();
   bool doNormalize = options["normalize"]->getBoolean();
   bool doHash = options["hash"]->getBoolean();
   bool doDump = options["dump"]->getBoolean();
   bool verbose = true;

   // read in rdfa
   RdfaReader reader;
   reader.setBaseUri(baseUri);
   DynamicObject dyno;
   rval = reader.start(dyno) && reader.read(is) && reader.finish();

   // pipe data as requested
   DynamicObject output = dyno;

   if(rval && doFilter)
   {
      DynamicObject& input = output;
      DynamicObject filtered;
      DynamicObject context(NULL);
      if(output->hasMember("#"))
      {
         // context from normalized data
         context = output["#"];
      }
      else
      {
         // empty context
         context = DynamicObject();
         context->setType(Map);
      }
      DynamicObject filter;
      filter["@"] = srcName;
      rval = JsonLd::filter(context, filter, input, filtered);
      if(rval)
      {
         output = filtered;
      }
   }

   if(rval && doNormalize)
   {
      DynamicObject& input = output;
      DynamicObject normalized;
      rval = JsonLd::normalize(input, normalized);
      if(rval)
      {
         output = normalized;
      }
   }

   string hash;
   if(rval && doHash)
   {
      // digest normalized data
      MessageDigest md;
      string json = JsonWriter::writeToString(output, true, false);
      rval = md.start("SHA1") && md.update(json.c_str(), json.length());
      if(rval)
      {
         hash = md.getDigest().c_str();
      }
   }

   if(rval)
   {
      if(verbose)
      {
         // print output
         printf("* RDFa to JSON-LD\n");
         printf("* <source>:\n%s\n",
            (srcName != NULL) ? srcName : "(unknown)");
      }
   }

   if(rval && doHash)
   {
      if(verbose)
      {
         // output info
         printf("* <source>|RDFa|JSON-LD|%s%s%s<stdout>:\n",
            doFilter ? "filter|" : "",
            doNormalize ? "normalize|" : "",
            doHash ? "SHA-1|" : "");
      }
      // output hash
      printf("%s\n", hash.c_str());
   }

   if(rval && doDump)
   {
      if(verbose)
      {
         // output info
         printf("* <source>|RDFa|JSON-LD|%s%s<stdout>:\n",
            doFilter ? "filter|" : "",
            doNormalize ? "normalize|" : "");
      }
      // output JSON-LD
      rval = JsonWriter::writeToStdOut(output, false, false);
   }

   return rval;
}

static bool _processFile(
   DynamicObject& options, const char* inFile, const char* baseUri)
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

   rval = _processStream(options, fis, _srcName.c_str(), _baseUri.c_str());

   // close input stream
   fis->close();
   delete fis;

   return rval;
}

static bool _processHttp(
   DynamicObject& options, Url& url, const char* baseUri)
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
            rval = _processStream(
               options, &is, _baseUri.c_str(), _baseUri.c_str());
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
      c["filter"] = false;
      c["normalize"] = false;
      c["hash"] = true;
      c["dump"] = true;
      c["verbose"] = true;

      DynamicObject spec;
      spec["help"] =
"Rdfa2JsonLd Options\n"
"      --base-uri      The base URI to use.\n"
"      --filter        Filter for source URI properties.\n"
"      --no-filter     Do not filter on source URI. (default)\n"
"      --normalize     Normalize JSON-LD.\n"
"      --no-normalize  Do not normalize JSON-LD. (default)\n"
"      --hash          Hash JSON-LD. (default).\n"
"      --no-hash       Do not hash JSON-LD.\n"
"      --dump          Dump JSON-LD. (default).\n"
"      --no-dump       Do not dump JSON-LD.\n"
"      --verbose       Verbose output. (default).\n"
"      --quiet         Quieter output.\n"
"\n";

      DynamicObject opt(NULL);

      // create option to set base URI
      opt = spec["options"]->append();
      opt["long"] = "--base-uri";
      opt["argError"] = "Base URI must be a string.";
      opt["arg"]["root"] = c;
      opt["arg"]["path"] = "baseUri";

      // simple boolean options
      DynamicObject bools;
      bools->append("filter");
      bools->append("normalize");
      bools->append("hash");
      bools->append("dump");

      DynamicObjectIterator i = bools.getIterator();
      while(i->hasNext())
      {
         DynamicObject& next = i->next();
         const char* name = next->getString();

         // do it
         opt = spec["options"]->append();
         opt["long"]->format("--%s", name);
         opt["setTrue"]["root"] = c;
         opt["setTrue"]["path"] = name;

         // do not do it
         opt = spec["options"]->append();
         opt["long"]->format("--no-%s", name);
         opt["setFalse"]["root"] = c;
         opt["setFalse"]["path"] = name;
      }

      // verbose output
      opt = spec["options"]->append();
      opt["long"] = "--verbose";
      opt["setTrue"]["root"] = c;
      opt["setTrue"]["path"] = "verbose";

      // quiet output
      opt = spec["options"]->append();
      opt["long"] = "--quiet";
      opt["setFalse"]["root"] = c;
      opt["setFalse"]["path"] = "verbose";

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
         _processFile(cfg, NULL, baseUri);
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
                  success = _processFile(cfg, next, baseUri);
               }
               else if(scheme == "file")
               {
                  success = _processFile(cfg, url.getPath().c_str(), baseUri);
               }
               else if(scheme == "http" || scheme == "https")
               {
                  success = _processHttp(cfg, url, baseUri);
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
               success = _processFile(cfg, next, baseUri);
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
