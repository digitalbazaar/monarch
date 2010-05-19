/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/app/AppTools.h"

// openssl includes
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/engine.h>

#include "monarch/io/ByteArrayOutputStream.h"
#include "monarch/io/FileOutputStream.h"
#include "monarch/util/StringTools.h"

using namespace monarch::app;
using namespace monarch::io;
using namespace monarch::rt;
using namespace monarch::util;

// declare table of openSSL mutexes
pthread_mutex_t* AppTools::sOpenSSLMutexes = NULL;

AppTools::AppTools()
{
}

AppTools::~AppTools()
{
}

static void _printException(ExceptionRef& e, OutputStream* os, int level)
{
   ByteBuffer details(512);
   if(!e->getDetails().isNull() && e->getDetails()->length() > 0)
   {
      ByteArrayOutputStream baos(&details, true);
      JsonWriter jw;
      jw.setCompact(false);
      jw.setIndentation(level, 3);
      jw.write(e->getDetails(), &baos);
   }
   else
   {
      details.putByte('-', 1, true);
   }
   details.putByte('\0', 1, true);

   ByteBuffer cause(512);
   if(!e->getCause().isNull())
   {
      cause.putByte('\n', 1, true);
      ByteArrayOutputStream baos(&cause, true);
      _printException(e->getCause(), &baos, level + 1);
   }
   else
   {
      cause.putByte('-', 1, true);
   }
   cause.putByte('\0', 1, true);

   char indent[3 * level + 1];
   indent[3 * level] = '\0';
   memset(indent, ' ', 3 * level);

   string str;
   if(level == 0)
   {
      str = "Exception:\n";
   }
   else
   {
      str = StringTools::format(
         "%stype:    %s\n"
         "%scode:    %d\n"
         "%smessage: %s\n"
         "%sdetails: %s\n"
         "%scause:   %s\n",
         e->getType(),
         e->getCode(),
         e->getMessage(),
         details.bytes(),
         cause.bytes());
   }
   os->write(str.c_str(), str.length());
}

void AppTools::printException(ExceptionRef& e)
{
   FileOutputStream fos(FileOutputStream::StdErr);
   _printException(e, &fos, 0);
   fos.close();
}

void AppTools::printException(ExceptionRef& e, OutputStream* os)
{
   _printException(e, os, 0);
}

void AppTools::printException()
{
   ExceptionRef e = Exception::get();
   printException(e);
}

#ifdef WIN32
static bool _initializeWinSock()
{
   bool rval = true;

   WSADATA wsaData;
   if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
   {
      ExceptionRef e = new Exception(
         "Could not initialize winsock.",
         "monarch.app.WinSockError");
      Exception::set(e);
      rval = false;
   }
   else if(LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
   {
      ExceptionRef e = new Exception(
         "Incompatible version of winsock.",
         "monarch.app.WinSockError");
      e->getDetails()["version"]->format(
         "%d.%d", LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion));
      e->getDetails()["requiredVersion"] = "2.2";
      Exception::set(e);
      rval = false;
   }

   return rval;
}

static void _cleanupWinSock()
{
   // Note: WSACleanup() crashes when called on windows (after a successful
   // call to WSAStartup() because openSSL automatically calls it ... since
   // it can't handle being called twice it has been disabled here
   //WSACleanup();
}
#endif

static bool _initNetworking()
{
   bool rval = true;
#ifdef WIN32
   rval = _initializeWinSock();
#endif
   return rval && initializeOpenSSL();
}

static bool _cleanupNetworking()
{
   cleanupOpenSSL();
#ifdef WIN32
   _cleanupWinSock();
#endif
}

bool AppTools::initializeOpenSSL()
{
   ERR_load_crypto_strings();
   SSL_library_init();
   SSL_load_error_strings();
   OpenSSL_add_all_algorithms();

   // create mutex attributes, use fastest type of mutex
   pthread_mutexattr_t mutexAttr;
   pthread_mutexattr_init(&mutexAttr);
   pthread_mutexattr_settype(&mutexAttr, PTHREAD_MUTEX_NORMAL);

   // create CRYPTO_num_locks() mutexes
   int numLocks = CRYPTO_num_locks();
   sOpenSSLMutexes = (pthread_mutex_t*)calloc(
      numLocks, sizeof(pthread_mutex_t));

   // initialize mutexes
   for(int i = 0; i < numLocks; i++)
   {
      // initialize mutex
      pthread_mutex_init(&sOpenSSLMutexes[i], &mutexAttr);
   }

   // destroy mutex attributes
   pthread_mutexattr_destroy(&mutexAttr);

   // set openSSL multi-threaded callbacks
   CRYPTO_set_id_callback(&App::openSSLSetId);
   CRYPTO_set_locking_callback(&App::openSSLHandleLock);

   return true;
}

void AppTools::cleanupOpenSSL()
{
   ERR_remove_state(0);
   ENGINE_cleanup();
   ERR_free_strings();
   EVP_cleanup();
   CRYPTO_cleanup_all_ex_data();

   // destroy mutexes
   if(sOpenSSLMutexes != NULL)
   {
      int numLocks = CRYPTO_num_locks();
      for(int i = 0; i < numLocks; i++)
      {
         // initialize mutex
         pthread_mutex_destroy(&sOpenSSLMutexes[i]);
      }

      // free mutexes
      free(sOpenSSLMutexes);
   }
}

unsigned long AppTools::openSSLSetId()
{
#ifdef WIN32
   // win32 uses ptw32_handle_t for pthread IDs which is
   // a struct with a pointer "p" and some number used for "etc"
   return (unsigned long)pthread_self().p;
#else
   return (unsigned long)pthread_self();
#endif
}

void AppTools::openSSLHandleLock(int mode, int n, const char* file, int line)
{
   if(mode & CRYPTO_LOCK)
   {
      pthread_mutex_lock(&sOpenSSLMutexes[n]);
   }
   else
   {
      pthread_mutex_unlock(&sOpenSSLMutexes[n]);
   }
}
