/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_crypto_UnsupportedAlgorithmException_H
#define db_crypto_UnsupportedAlgorithmException_H

#include "Exception.h"

namespace db
{
namespace crypto
{

/**
 * An UnsupportedAlgorithmException is raised when a some kind of request for
 * an algorithm that is not recognized or implemented occurs.
 *
 * @author Dave Longley
 */
class UnsupportedAlgorithmException : public db::rt::Exception
{
public:
   /**
    * Creates a new UnsupportedAlgorithmException.
    *
    * A message and code may be optionally specified.
    *
    * @param message the message for this Exception.
    * @param code the code for this Exception.
    */
   UnsupportedAlgorithmException(
      const char* message = NULL, const char* code = NULL);
   
   /**
    * Destructs this UnsupportedAlgorithmException.
    */
   virtual ~UnsupportedAlgorithmException();
};

} // end namespace crypto
} // end namespace db
#endif
