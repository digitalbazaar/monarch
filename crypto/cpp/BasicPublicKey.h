/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef BasicPublicKey_H
#define BasicPublicKey_H

#include "AsymmetricKey.h"
#include "PublicKey.h"

#include <string>

namespace db
{
namespace crypto
{

/**
 * A BasicPublicKey provides a basic implementation for public keys.
 * 
 * @author Dave Longley
 */
class BasicPublicKey : public AsymmetricKey, public PublicKey
{
protected:
   /**
    * The algorithm for this key.
    */
   std::string mAlgorithm;
   
public:
   /**
    * Creates a new BasicPublicKey.
    */
   BasicPublicKey();
   
   /**
    * Destructs this BasicPublicKey.
    */
   virtual ~BasicPublicKey();
   
   /**
    * Creates a DigitalSignature to verify data with.
    * 
    * @return the DigitalSignature to verify data with.
    */
   virtual DigitalSignature* createSignature();
   
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
