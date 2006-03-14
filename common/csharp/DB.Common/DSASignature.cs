/*
 * Copyright (c) 2005 Digital Bazaar, Inc.  All rights reserved.
 */
using System;
using System.Security.Cryptography;
 
using ThirdParty.Mono.Math;

using DB.Common.Logging;

namespace DB.Common
{
	/// <summary>
	/// The DSASignature class is responsible for creating and validating
	/// digital signatures.
	/// </summary>
	/// 
	/// <author>Manu Sporny</author>
	public class DSASignature
	{
	   /// <summary>
	   /// Creates a valid DSA signature given a hash for a set of data as
	   /// well as the DSA parameters
	   /// </summary>
	   ///
	   /// <param name="hash">The calculated hash for the data being signed.
	   /// </param>
	   /// <param name="dsaParameters">The DSA parameters that are used to
	   /// calculate the signature (including the private key).</param>
	   public static byte[] CreateSignature(byte[] hash, 
	                                        DSAParameters dsaParameters)
	   {
         if(hash == null)
         {
            throw new ArgumentNullException("hash");
         }
         
         if(hash.Length != 20)
         {
            throw new CryptographicException("invalid hash length");
         }
         
         // Setup the standard variables
         BigInteger m = new BigInteger(hash);
         BigInteger q = new BigInteger(dsaParameters.Q);
         BigInteger g = new BigInteger(dsaParameters.G);
         BigInteger x = new BigInteger(dsaParameters.X);
         BigInteger p = new BigInteger(dsaParameters.P);

         // if required key must be generated before checking for X
         if(dsaParameters.X == null)
         {
            throw new CryptographicException
               ("no private key available for signature");
         }

         // (a) Select a random secret integer k; 0 < k < q.
         BigInteger k = BigInteger.GenerateRandom(160);
         while(k >= q)
         {
            k.Randomize();
         }
         
         // (b) Compute r = (g^k mod p) mod q
         BigInteger r = (g.ModPow(k, p)) % q;
         
         // (c) Compute k -1 mod q (e.g., using Algorithm 2.142).
         // (d) Compute s = k -1 fh(m) +arg mod q.
         BigInteger s = ((k.ModInverse(q) * (m + x * r)) % q);
         
         // (e) get bytes for r and s
         byte[] rBytes = r.GetBytes();
         byte[] sBytes = s.GetBytes();
         
         // put signature into ASN.1 format
         ASN1 asn1 = new ASN1(ASN1.ASN1Type.Sequence);

         // create ASN.1 INTEGERS for r and s
         asn1.AddASN1(ASN1.UnsignedBigIntToASN1(rBytes));
         asn1.AddASN1(ASN1.UnsignedBigIntToASN1(sBytes));
         
         // get ASN.1 encoded signature
         byte[] signature = asn1.Bytes;
         
         return signature;
	   }

      /// <summary>
      /// Verifies a signature given a calculated hash of the data and a 
      /// signature object.
      /// </summary>
      ///
      /// <param name="hash">The calculated hash for the data that was signed.
      /// </param>
      /// <param name="signature">The signature associated with the calculated
      /// hash.</param>
      /// <param name="dsaParameters">The DSA parameters that are used to 
      /// validate the signature (including the public key for the signer).
      /// </param>
	   public static bool VerifySignature(byte[] hash, byte[] signature, 
	                                      DSAParameters dsaParameters)
	   {
         if(hash == null)
         {
            throw new ArgumentNullException("hash");
         }
         
         if(signature == null)
         {
            throw new ArgumentNullException("signature");
         }

         if(hash.Length != 20)
         {
            throw new CryptographicException("invalid hash length");
         }
         
         // convert ASN.1 encoded signature data into ASN.1s
         ASN1 asn1 = new ASN1(signature);
         ASN1 rAsn1 = asn1[0];
         ASN1 sAsn1 = asn1[1];
         
         // store r and s
         byte[] rBytes = rAsn1.BigIntValue;
         byte[] sBytes = sAsn1.BigIntValue;
         
         // signature is always 38-40 bytes (no matter the size of the
         // public key). In fact it is 2 times the size of the private
         // key (which is 20 bytes for 512 to 1024 bits DSA keypairs)
         int sigLength = rBytes.Length + sBytes.Length;
         if(sigLength < 38 || sigLength > 40)
         {
            throw new CryptographicException("invalid signature length," +
                                             "length=" + sigLength);
         }

         try
         {
            BigInteger q = new BigInteger(dsaParameters.Q);
            BigInteger g = new BigInteger(dsaParameters.G);
            BigInteger y = new BigInteger(dsaParameters.Y);
            BigInteger p = new BigInteger(dsaParameters.P);
            BigInteger m = new BigInteger(hash);
            BigInteger r = new BigInteger(rBytes);
            BigInteger s = new BigInteger(sBytes);

            if((r < 0) || (q <= r))
            {
               return false;
            }

            if((s < 0) || (q <= s))
            {
               return false;
            }

            BigInteger w = s.ModInverse(q);
            BigInteger u1 = (m * w) % q;
            BigInteger u2 = (r * w) % q;

            u1 = g.ModPow(u1, p);
            u2 = y.ModPow(u2, p);

            BigInteger v = (((u1 * u2) % p) % q);
            
            return (v == r);
         }
         catch 
         {
            throw new CryptographicException
               ("could not compute signature");
         }
      }  
   }
}
