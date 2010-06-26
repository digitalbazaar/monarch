/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_app_AppTools_H
#define monarch_app_AppTools_H

#include "monarch/rt/Exception.h"
#include "monarch/io/OutputStream.h"

#include <pthread.h>
#include <string>

namespace monarch
{
namespace app
{

/**
 * The AppTools class provides static helper methods for an AppRunner. For
 * example, methods to initialize OS-specific network functionality, print
 * exceptions, etc.
 *
 * @author Dave Longley
 */
class AppTools
{
protected:
   /**
    * A table of pthread mutexes for openSSL.
    */
   static pthread_mutex_t* sOpenSSLMutexes;

   /**
    * Creates an AppTools instance.
    */
   AppTools();

public:
   /**
    * Deconstructs this AppTools instance.
    */
   virtual ~AppTools();

   /**
    * Pretty prints an exception.
    *
    * @param e the exception to print.
    */
   static void printException(monarch::rt::ExceptionRef& e);

   /**
    * Pretty prints an exception to a given string.
    *
    * @param e the exception to print.
    * @param str the string.
    */
   static void printException(
      monarch::rt::ExceptionRef& e, std::string& str);

   /**
    * Pretty prints an exception to a given output stream.
    *
    * @param e the exception to print.
    * @param os the output stream.
    */
   static void printException(
      monarch::rt::ExceptionRef& e, monarch::io::OutputStream* os);

   /**
    * Pretty prints the last exception that occurred.
    */
   static void printException();

   /**
    * Initializes network support.
    *
    * @return true if successful, false on failure with exception set.
    */
   static bool initializeNetworking();

   /**
    * Cleans up network support.
    */
   static void cleanupNetworking();

   /**
    * Initializes OpenSSL.
    *
    * @return true on success, false on failure with exception set.
    */
   static bool initializeOpenSSL();

   /**
    * Cleans up OpenSSL.
    */
   static void cleanupOpenSSL();

   /**
    * A callback function required by OpenSSL for multi-threaded applications.
    *
    * This method will return the current thread's ID.
    *
    * @return the current thread's ID.
    */
   static unsigned long openSSLSetId();

   /**
    * A callback function required by OpenSSL for multi-threaded applications.
    *
    * It sets the n-th lock if mode & CRYPTO_LOCK, and releases it otherwise.
    *
    * @param mode the current mode.
    * @param n the lock number to alter.
    * @param file the current source file (unused).
    * @param line the line in the file (unused).
    */
   static void openSSLHandleLock(int mode, int n, const char* file, int line);
};

} // end namespace app
} // end namespace monarch

#endif
