/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_crypto_AsymmetricKeyFactory_H
#define monarch_crypto_AsymmetricKeyFactory_H

#include "monarch/crypto/BigInteger.h"
#include "monarch/crypto/PrivateKey.h"
#include "monarch/crypto/PublicKey.h"
#include "monarch/crypto/X509Certificate.h"
#include "monarch/util/Date.h"

namespace monarch
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
    * Generates an X.509 certificate that contains the given public key and
    * is signed by the given private key that belongs to the given issuer,
    * where the issuer is the same as the subject to create a self-signed
    * certificate.
    *
    * @param version the X.509 version to use (0x0 = 1, 0x1 = 2, 0x2 = 3).
    * @param privateKey the private key to sign with.
    * @param publicKey the public key for the certificate.
    * @param subject the subject information in a map:
    *    CN: Common Name (site's domain, i.e. localhost, myserver.com)
    *    OU: Organizational Unit
    *    O : Organization
    *    L : Locality (city, i.e. New York)
    *    ST: State (i.e., Virginia)
    *    C : Country (i.e., US)
    * @param issuer the issuer of the certificate (same fields as subject),
    *               can be the same object as the subject to create a
    *               self-signed certificate.
    * @param startDate the starting date at which the certificate is valid,
    *                  NULL for now.
    * @param endDate the ending date at which the certificate is valid,
    *                NULL for now.
    * @param serial the serial number to use.
    * @param extensions any extensions to include.
    * @param issuerCert the issuer certificate to use, NULL for none.
    *
    * @return the X.509 certificate, NULL if an exception occurred.
    */
   virtual X509CertificateRef createCertificate(
      uint32_t version,
      PrivateKeyRef& privateKey, PublicKeyRef& publicKey,
      monarch::rt::DynamicObject subject, monarch::rt::DynamicObject issuer,
      monarch::util::Date* startDate, monarch::util::Date* endDate,
      BigInteger& serial, monarch::rt::DynamicObject* extensions,
      X509CertificateRef* issuerCert);

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
} // end namespace monarch
#endif
