/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef AsymmetricKeyFactory_H
#define AsymmetricKeyFactory_H

#include "PrivateKey.h"
#include "PublicKey.h"

namespace db
{
namespace crypto
{

/**
 * An AsymmetricKeyFactory is used to create and load asymmetric
 * cryptographic keys.
 * 
 * @author Dave Longley
 */
class AsymmetricKeyFactory : public virtual db::rt::Object
{
protected:
   /**
    * Creates a new DSA key pair.
    * 
    * @param privateKey a pointer to point at the new PrivateKey.
    * @param publicKey a pointer to point at the new PublicKey.
    */
   void createDsaKeyPair(PrivateKey** privateKey, PublicKey** publicKey);
   
   /**
    * Creates a new RSA key pair.
    * 
    * @param privateKey a pointer to point at the new PrivateKey.
    * @param publicKey a pointer to point at the new PublicKey.
    */
   void createRsaKeyPair(PrivateKey** privateKey, PublicKey** publicKey);
   
   /**
    * A callback function that is called to obtain a password to unlock
    * an encrypted key.
    * 
    * @param b the buffer to populate with a password.
    * @param length the length of the buffer to populate.
    * @param flag a flag that is reserved for future use.
    * @param userData a pointer to some user data.
    * 
    * @return the length of the password.
    */
   static int passwordCallback(char* b, int length, int flag, void* userData);
   
public:
   /**
    * Creates a new AsymmetricKeyFactory.
    */
   AsymmetricKeyFactory();
   
   /**
    * Destructs this AsymmetricKeyFactory.
    */
   virtual ~AsymmetricKeyFactory();
   
   /**
    * Creates a new key pair using the given algorithm.
    * 
    * The caller of this method is responsible for freeing the generated
    * PrivateKey and PublicKey.
    * 
    * @param algorithm the algorithm to use.
    * @param privateKey a pointer to point at the new PrivateKey.
    * @param publicKey a pointer to point at the new PublicKey.
    * 
    * @return true if no exception occurred, false if not.
    */
   bool createKeyPair(
      std::string const& algorithm,
      PrivateKey** privateKey, PublicKey** publicKey);
   
   /**
    * Loads a private key from a PEM formatted string. A PEM formatted
    * string is just the base64-encoded version of an ASN.1 DER-encoded key
    * structure that has a header and footer.
    * 
    * @param pem the PEM string to load the key from.
    * @param password the password to use to load the key.
    * 
    * @return the loaded PrivateKey or NULL if an exception occurred.
    */
   virtual PrivateKey* loadPrivateKeyFromPem(
      const std::string& pem, const std::string& password);
   
   /**
    * Writes a private key to a PEM formatted string. A PEM formatted
    * string is just the base64-encoded version of an ASN.1 DER-encoded key
    * structure that has a header and footer.
    * 
    * @param key the PrivateKey to write to a PEM string.
    * @param password the password to use to encrypt the key.
    * 
    * @return the PEM string or a blank string of an exception occurred.
    */
   std::string writePrivateKeyToPem(
      PrivateKey* key, const std::string& password);
   
   /**
    * Loads a public key from a PEM formatted string. A PEM formatted
    * string is just the base64-encoded version of an ASN.1 DER-encoded key
    * structure that has a header and footer.
    * 
    * @param pem the PEM string to load the key from.
    * @param password the password to use to load the key.
    * 
    * @return the loaded PublicKey or NULL if an exception occurred.
    */
   virtual PublicKey* loadPublicKeyFromPem(const std::string& pem);
   
   /**
    * Writes a public key to a PEM formatted string. A PEM formatted
    * string is just the base64-encoded version of an ASN.1 DER-encoded key
    * structure that has a header and footer.
    * 
    * @param key the PublicKey to write to a PEM string.
    * @param password the password to use to encrypt the key.
    * 
    * @return the PEM string or a blank string of an exception occurred.
    */
   std::string writePublicKeyToPem(PublicKey* key);
};

} // end namespace crypto
} // end namespace db
#endif
