/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/util/App.h"

// openssl includes
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/engine.h>

#include <assert.h>
#include <iostream>
#include <sstream>
#include <vector>

#include "db/logging/Logging.h"
#include "db/rt/Exception.h"
#include "db/rt/Thread.h"
#include "db/data/json/JsonWriter.h"
#include "db/io/OStreamOutputStream.h"

using namespace std;
using namespace db::data::json;
using namespace db::logging;
using namespace db::io;
using namespace db::rt;
using namespace db::util;

App::App()
{
   setExitStatus(0);
   mProgramName = NULL;
   setProgramName("(unknown)");
   mName = NULL;
   setName("(unknown)");
   mDelegate = this;
}

App::~App()
{
   setProgramName(NULL);
   setName(NULL);
}

static void _printException(ExceptionRef& e, ostream& s, int level)
{
   ostringstream details;
   if(!e->getDetails().isNull() && e->getDetails()->length() > 0)
   {
      OStreamOutputStream os(&details);
      JsonWriter jw;
      jw.setCompact(false);
      jw.setIndentation(3 * level, 3);
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

void App::printException()
{
   ExceptionRef e = Exception::getLast();
   printException(e);
}

void App::setDelegate(AppDelegate* delegate)
{
   mDelegate = (delegate != NULL) ? delegate : this;
}

AppDelegate* App::getDelegate()
{
   return mDelegate;
}

void App::setProgramName(const char* name)
{
   if(mProgramName)
   {
      free(mProgramName);
   }
   mProgramName = name ? strdup(name) : NULL;
}

const char* App::getProgramName()
{
   return mProgramName;
}

void App::setName(const char* name)
{
   if(mName)
   {
      free(mName);
   }
   mName = name ? strdup(name) : NULL;
}

const char* App::getName()
{
   return mName;
}

void App::setExitStatus(int exitStatus)
{
   mExitStatus = exitStatus;
}

int App::getExitStatus()
{
   return mExitStatus;
}

void App::run()
{
   if(mDelegate != this)
   {
      mDelegate->run(this);
   }
}

bool App::parseCommandLine(vector<const char*>* args)
{
   return true;
}

void App::initializeOpenSSL()
{
   ERR_load_crypto_strings();
   SSL_library_init();
   SSL_load_error_strings();
   OpenSSL_add_all_algorithms();
}

void App::cleanupOpenSSL()
{
   ERR_remove_state(0);
   ENGINE_cleanup();
   ERR_free_strings();
   EVP_cleanup();
   CRYPTO_cleanup_all_ex_data();
}

void App::initializeLogging()
{
   db::logging::Logging::initialize();
}

void App::cleanupLogging()
{
   db::logging::Logging::cleanup();
}

int App::main(int argc, const char* argv[])
{
   // Make command line vector
   for(int i = 0; i < argc; i++)
   {
      mCommandLineArgs.push_back(argv[i]);
   }
   
   setProgramName(mCommandLineArgs[0]);
   if(!(mDelegate->willParseCommandLine(this, &mCommandLineArgs) &&
      parseCommandLine(&mCommandLineArgs) &&
      mDelegate->didParseCommandLine(this, &mCommandLineArgs)))
   {
      printException();
      exit(EXIT_FAILURE);
   }
   
   // initialize winsock
   #ifdef WIN32
      WSADATA wsaData;
      if(WSAStartup(MAKEWORD(2, 0), &wsaData) < 0)
      {
         cerr << "ERROR! Could not initialize winsock!" << endl;
      }
   #endif
   
   initializeOpenSSL();
   initializeLogging();
   
   Thread t(this);
   t.start();
   t.join();
   
   cleanupLogging();
   cleanupOpenSSL();
   
   // cleanup winsock
#ifdef WIN32
   WSACleanup();
#endif

#ifndef WIN32
   // FIXME: calling Thread::exit() on windows causes a busy loop of
   // some sort (perhaps a deadlock spin lock)
   Thread::exit();
#endif
   
   return mExitStatus;
}

AppDelegate::AppDelegate() {}
   
AppDelegate::~AppDelegate() {}
   
void AppDelegate::run(App* app) {}
   
bool AppDelegate::willParseCommandLine(App* app, vector<const char*>* args)
{
   return true;
}

bool AppDelegate::didParseCommandLine(App* app, vector<const char*>* args)
{
   return true;
}

void AppDelegate::didInitializeLogging(App* app) {}
   
void AppDelegate::willCleanupLogging(App* app) {}
