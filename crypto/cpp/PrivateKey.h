/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef PrivateKey_H
#define PrivateKey_H

#include "DigitalSignature.h"

namespace db
{
namespace crypto
{

/**
 * A PrivateKey provides an interface for private keys.
 * 
 * In asymmetric cryptography a pair of keys, one public and one private
 * are used. An entity's public key, as the name suggests, is public and does
 * not contain any confidential data. The entity's private key, however, is
 * confidential and must be kept secret.
 * 
 * There are two ways in which asymmetric cryptography can be used to aid
 * in secure communication.
 * 
 * 1. Public key encryption - a message can be encrypted using the message
 * recipient's public key so that only the entity may decrypt it using their
 * private key. This ensures that only the recipient can read the secret
 * message.
 * 
 * 2. Digital signatures - a message signed with the message sender's private
 * key can be verified by anyone who has the sender's public key. This ensures
 * that any verified message was sent from the sender and has not been altered.
 * 
 * @author Dave Longley
 */
class PrivateKey
{
public:
   /**
    * Creates a new PrivateKey.
    */
   PrivateKey() {};
   
   /**
    * Destructs this PrivateKey.
    */
   virtual ~PrivateKey() {};
   
   /**
    * Creates a DigitalSignature to sign data with.
    * 
    * @return the DigitalSignature to sign data with.
    */
   virtual DigitalSignature* createSignature() = 0;
   
   /**
    * Gets the algorithm for this key.
    * 
    * @return the algorithm for this key.
    */
   virtual const std::string& getAlgorithm() = 0;
};

} // end namespace crypto
} // end namespace db
#endif
