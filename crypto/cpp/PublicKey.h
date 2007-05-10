/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef PublicKey_H
#define PublicKey_H

#include "DigitalEnvelope.h"
#include "DigitalSignature.h"

namespace db
{
namespace crypto
{

/**
 * A PublicKey is a non-secret cryptographic asymmetric key.
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
 * This class uses OpenSSL's implementation of public keys.
 * 
 * @author Dave Longley
 */
class PublicKey : public AsymmetricKey
{
public:
   /**
    * Creates a new PublicKey from a PKEY structure.
    * 
    * @param pkey the PKEY structure with the data for the key.
    */
   PublicKey(EVP_PKEY* pkey);
   
   /**
    * Destructs this PublicKey.
    */
   virtual ~PublicKey();
   
   /**
    * Creates a DigitalEnvelope to send a confidential message with.
    * 
    * A random symmetric key will be generated and used to seal the envelope.
    * It will be encrypted with this PublicKey so that it can only be unlocked
    * by the PrivateKey associated with this PublicKey.
    * 
    * The caller of this method is responsible for freeing the
    * generated SymmetricKey and DigitalEnvelope.
    * 
    * @param algorithm the algorithm to use for the encryption.
    * @param key to store the encrypted SymmetricKey used to seal the envelope.
    * 
    * @exception IOException thrown if an IO error occurs.
    * @exception UnsupportedAlgorithmException thrown if an unsupported key
    *            algorithm is used.
    */
   virtual DigitalEnvelope* createEnvelope(
      const std::string& algorithm, SymmetricKey** key)
   throw(db::io::IOException, UnsupportedAlgorithmException);
   
   /**
    * Creates a DigitalSignature to verify data with.
    * 
    * The caller of this method is responsible for freeing the generated
    * DigitalSignature.
    * 
    * @return the DigitalSignature to verify data with.
    */
   virtual DigitalSignature* createSignature();
};

} // end namespace crypto
} // end namespace db
#endif
