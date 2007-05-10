/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef PrivateKey_H
#define PrivateKey_H

#include "DigitalEnvelope.h"
#include "DigitalSignature.h"

namespace db
{
namespace crypto
{

/**
 * A PrivateKey is a secret cryptographic asymmetric key.
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
 * This class uses OpenSSL's implementation of private keys.
 * 
 * @author Dave Longley
 */
class PrivateKey : public AsymmetricKey
{
public:
   /**
    * Creates a new PrivateKey from a PKEY structure.
    * 
    * @param pkey the PKEY structure with the data for the key.
    */
   PrivateKey(EVP_PKEY* pkey);
   
   /**
    * Destructs this PrivateKey.
    */
   virtual ~PrivateKey();
   
   /**
    * Creates a DigitalEnvelope to receive a confidential message with.
    * 
    * The encrypted symmetric key that was used to seal the envelope must be
    * passed to this method so that it can be unlocked with this PrivateKey.
    * Once it has been unlocked by this method, the DigitalEnvelope can be
    * used to open the message.
    * 
    * The caller of this method is responsible for freeing the generated
    * DigitalEnvelope.
    * 
    * @param key the encrypted SymmetricKey to open the envelope with.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   virtual DigitalEnvelope* createEnvelope(SymmetricKey* key)
   throw(db::io::IOException);
   
   /**
    * Creates a DigitalSignature to sign data with.
    * 
    * The caller of this method is responsible for freeing the generated
    * DigitalSignature.
    * 
    * @return the DigitalSignature to sign data with.
    */
   virtual DigitalSignature* createSignature();
};

} // end namespace crypto
} // end namespace db
#endif
