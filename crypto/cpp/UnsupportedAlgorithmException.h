/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef UnsupportedAlgorithmException_H
#define UnsupportedAlgorithmException_H

#include "Exception.h"

namespace db
{
namespace crypto
{

/**
 * An UnsupportedAlgorithmException is thrown when a some kind of request for
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
      const std::string& message = "", const std::string& code = "");
   
   /**
    * Destructs this UnsupportedAlgorithmException.
    */
   virtual ~UnsupportedAlgorithmException();
};

} // end namespace crypto
} // end namespace db
#endif
