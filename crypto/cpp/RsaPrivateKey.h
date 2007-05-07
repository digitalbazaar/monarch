/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef RsaPrivateKey_H
#define RsaPrivateKey_H

#include "AbstractPrivateKey.h"

namespace db
{
namespace crypto
{

/**
 * An RsaPrivateKey is a private key that uses the RSA algorithm.
 * 
 * @author Dave Longley
 */
class RsaPrivateKey : public AbstractPrivateKey
{
protected:
   /**
    * The algorithm for this key.
    */
   static const std::string KEY_ALGORITHM;
   
public:
   /**
    * Creates a new RsaPrivateKey.
    */
   RsaPrivateKey();
   
   /**
    * Destructs this RsaPrivateKey.
    */
   virtual ~RsaPrivateKey();
      
   /**
    * Gets the algorithm for this key.
    * 
    * @return the algorithm for this key.
    */
   virtual const std::string& getAlgorithm();   
};

} // end namespace crypto
} // end namespace db
#endif
