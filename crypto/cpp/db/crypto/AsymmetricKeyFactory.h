/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_crypto_AsymmetricKeyFactory_H
#define db_crypto_AsymmetricKeyFactory_H

#include "db/crypto/PrivateKey.h"
#include "db/crypto/PublicKey.h"
#include "db/crypto/X509Certificate.h"

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
class AsymmetricKeyFactory
{
protected:
   /**
    * Creates a new DSA key pair.
    * 
    * @param privateKey a pointer to point at the new PrivateKey.
    * @param publicKey a pointer to point at the new PublicKey.
    */
   void createDsaKeyPair(PrivateKeyRef& privateKey, PublicKeyRef& publicKey);
   
   /**
    * Creates a new RSA key pair.
    * 
    * @param privateKey a pointer to point at the new PrivateKey.
    * @param publicKey a pointer to point at the new PublicKey.
    */
   void createRsaKeyPair(PrivateKeyRef& privateKey, PublicKeyRef& publicKey);
   
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
   virtual bool createKeyPair(
      const char* algorithm,
      PrivateKeyRef& privateKey, PublicKeyRef& publicKey);
   
   /**
    * Loads a private key from a PEM formatted string. A PEM formatted
    * string is just the base64-encoded version of an ASN.1 DER-encoded key
    * structure that has a header and footer.
    * 
    * @param pem the PEM string to load the key from.
    * @param length the length of the PEM string.
    * @param password the password to use to load the key, NULL to use none.
    * 
    * @return the loaded PrivateKey or NULL if an exception occurred.
    */
   virtual PrivateKeyRef loadPrivateKeyFromPem(
      const char* pem, int length, const char* password);
   
   /**
    * Writes a private key to a PEM formatted string. A PEM formatted
    * string is just the base64-encoded version of an ASN.1 DER-encoded key
    * structure that has a header and footer.
    * 
    * @param key the PrivateKey to write to a PEM string.
    * @param password the password to use to encrypt the key, NULL to use none.
    * 
    * @return the PEM string or a blank string of an exception occurred.
    */
   virtual std::string writePrivateKeyToPem(
      PrivateKeyRef& key, const char* password);
   
   /**
    * Loads a public key from a PEM formatted string. A PEM formatted
    * string is just the base64-encoded version of an ASN.1 DER-encoded key
    * structure that has a header and footer.
    * 
    * @param pem the PEM string to load the key from.
    * @param length the length of the PEM string.
    * 
    * @return the loaded PublicKey or NULL if an exception occurred.
    */
   virtual PublicKeyRef loadPublicKeyFromPem(const char* pem, int length);
   
   /**
    * Writes a public key to a PEM formatted string. A PEM formatted
    * string is just the base64-encoded version of an ASN.1 DER-encoded key
    * structure that has a header and footer.
    * 
    * @param key the PublicKey to write to a PEM string.
    * 
    * @return the PEM string or a blank string if an exception occurred.
    */
   virtual std::string writePublicKeyToPem(PublicKeyRef& key);
   
   /**
    * Generates a self-signed X.509 certificate that contains the given
    * public key.
    * 
    * @param privateKey the private key to sign with. 
    * @param publicKey the public key for the certificate.
    * @param subject the subject information in a map:
    *    CN: Common Name (site's domain, i.e. localhost, myserver.com)
    *    OU: Organizational Unit
    *    O : Organization
    *    L : Locality (city, i.e. New York)
    *    ST: State (i.e., Virginia)
    *    C : Country (i.e., US)
    * @param days the number of days the certificate should be valid.
    * 
    * @return the self-signed X.509 certificate, NULL if an exception occurred.
    */
   virtual X509CertificateRef createSelfSignedCertificate(
      PrivateKeyRef& privateKey, PublicKeyRef& publicKey,
      db::rt::DynamicObject& subject, time_t days);
   
   /**
    * Loads an X.509 certificate from a PEM formatted string. A PEM formatted
    * string is just the base64-encoded version of an ASN.1 DER-encoded cert
    * structure that has a header and footer.
    * 
    * @param pem the PEM string to load the certificate from.
    * @param length the length of the PEM string.
    * 
    * @return the loaded X.509 certificate or NULL if an exception occurred.
    */
   virtual X509CertificateRef loadCertificateFromPem(
      const char* pem, int length);
   
   /**
    * Writes an X.509 certificate to a PEM formatted string. A PEM formatted
    * string is just the base64-encoded version of an ASN.1 DER-encoded cert
    * structure that has a header and footer.
    * 
    * @param cert the X509Certificate to write to a PEM string.
    * 
    * @return the PEM string or a blank string if an exception occurred.
    */
   virtual std::string writeCertificateToPem(X509CertificateRef& cert);
};

} // end namespace crypto
} // end namespace db
#endif
