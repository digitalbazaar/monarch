/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef BasicPrivateKey_H
#define BasicPrivateKey_H

#include "AsymmetricKey.h"
#include "PrivateKey.h"

#include <string>

namespace db
{
namespace crypto
{

/**
 * A BasicPrivateKey provides a basic implementation for private keys.
 * 
 * @author Dave Longley
 */
class BasicPrivateKey : public AsymmetricKey, public PrivateKey
{
protected:
   /**
    * The algorithm for this key.
    */
   std::string mAlgorithm;
   
public:
   /**
    * Creates a new BasicPrivateKey.
    */
   BasicPrivateKey();
   
   /**
    * Destructs this BasicPrivateKey.
    */
   virtual ~BasicPrivateKey();
   
   /**
    * Creates a DigitalSignature to sign data with.
    * 
    * @return the DigitalSignature to sign data with.
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
