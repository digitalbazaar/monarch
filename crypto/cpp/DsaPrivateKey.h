/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef DsaPrivateKey_H
#define DsaPrivateKey_H

#include "AbstractPrivateKey.h"

namespace db
{
namespace crypto
{

/**
 * A DsaPrivateKey is a private key that uses the DSA algorithm.
 * 
 * @author Dave Longley
 */
class DsaPrivateKey : public AbstractPrivateKey
{
protected:
   /**
    * The algorithm for this key.
    */
   static const std::string KEY_ALGORITHM;
   
public:
   /**
    * Creates a new DsaPrivateKey.
    */
   DsaPrivateKey();
   
   /**
    * Destructs this DsaPrivateKey.
    */
   virtual ~DsaPrivateKey();
      
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
