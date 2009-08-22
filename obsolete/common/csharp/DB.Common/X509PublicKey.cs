/*
 * Copyright (c) 2005 Digital Bazaar, Inc.  All rights reserved.
 */
using System;
using System.Collections;
using System.Security.Cryptography;

using DB.Common.Logging;

namespace DB.Common
{
   /// <summary>
   /// A class for storing an X.509 encoded public key. The only algorithm
   /// currently supported is the Digital Signature Algorithm (DSA).
   /// </summary>
   /// 
   /// <remarks>
   /// An X.509 encoded key is stored in ASN.1 format. The SubjectPublicKeyInfo
   /// ASN.1 structure is composed of an algorithm of type AlgorithmIdentifier
   /// and a subjectPublicKey of type bit string.
   /// 
   /// The AlgorithmIdentifier contains an Object Identifier (OID) and 
   /// parameters for the algorithm, if any. DSA requires parameters P, Q,
   /// and G. These are used in the Digital Signature Algorithm to create
   /// and verify signatures. These parameters must be provided along with
   /// the public key in order to verify a signature.
   /// 
   /// SubjectPublicKeyInfo ::= SEQUENCE
   /// {
   ///    algorithm AlgorithmIdentifier,
   ///    subjectPublicKey BIT STRING
   /// }
   /// 
   /// AlgorithmIdentifer ::= SEQUENCE
   /// {
   ///    algorithm OBJECT IDENTIFIER,
   ///    parameters ANY DEFINED BY algorithm OPTIONAL
   /// }
   /// 
   /// For a DSA public key --
   /// 
   /// The parameters for the AlgorithmIdentifier are such:
   /// 
   /// Dss-Parms ::= SEQUENCE
   /// {
   ///    P INTEGER,
   ///    Q INTEGER,
   ///    G INTEGER
   /// }
   /// 
   /// The subjectPublicKey bit string contains the DSA public key as
   /// an integer:
   /// 
   /// DSAPublicKey ::= INTEGER
   /// 
   /// </remarks>
   /// 
   /// <author>Dave Longley</author>
   public class X509PublicKey
   {
      /// <summary>
      /// The ASN.1 structure for this X.509 public key.
      /// </summary>
      protected ASN1 mASN1;
      
      /// <summary>
      /// The OID for a DSA signature key.
      /// </summary>
      public static readonly string DSA_KEY_OID = "1.2.840.10040.4.1";
      
      /// <summary>
      /// Creates a new X509PublicKey from a byte array.
      /// </summary>
      /// 
      /// <param name="bytes">the byte array containing the X.509 encoded
      /// public key.</param>
      public X509PublicKey(byte[] bytes)
      {
         if(bytes != null && bytes.Length > 0 &&
            (bytes[0] & (byte)ASN1.ASN1Type.Sequence) ==
            (byte)ASN1.ASN1Type.Sequence)
         {
            mASN1 = new ASN1(bytes);
         }
      }
      
      /// <summary>
      /// Creates a new X509PublicKey based on the passed DSAParameters.
      /// </summary>
      /// 
      /// <param name="dsaParams">the DSA parameters to create the public
      /// key from.</param>
      public X509PublicKey(DSAParameters dsaParams)
      {
         ConvertFromDSAParameters(dsaParams);
      }
      
      /// <summary>
      /// Converts DSAParameters into an X.509 encoded public key.
      /// </summary>
      /// 
      /// <param name="dsaParams">the DSA parameters to create the public
      /// key from.</param>
      protected virtual void ConvertFromDSAParameters(DSAParameters dsaParams)
      {
         // get the DSA signature key OBJECT IDENTIFIER in ASN.1 format
         ASN1 oidASN1 = ASN1.OIDToASN1(DSA_KEY_OID);
         
         // get the DSS-Params
         ASN1 parameters = new ASN1(ASN1.ASN1Type.Sequence);
         parameters.AddASN1(ASN1.UnsignedBigIntToASN1(dsaParams.P));
         parameters.AddASN1(ASN1.UnsignedBigIntToASN1(dsaParams.Q));
         parameters.AddASN1(ASN1.UnsignedBigIntToASN1(dsaParams.G));
         
         // get the algorithm identifier
         ASN1 algId = new ASN1(ASN1.ASN1Type.Sequence);
         algId.AddASN1(oidASN1);
         algId.AddASN1(parameters);
         
         // get the DSA public key
         ASN1 dsaPublicKey = ASN1.UnsignedBigIntToASN1(dsaParams.Y);
         
         // create a BIT STRING
         // the first bit in the BIT STRING is the number of unused
         // bits in the first byte -- which is assumed to be zero because
         // the keys are stored as bytes
         byte[] val = dsaPublicKey.Bytes;
         byte[] bitString = new byte[val.Length + 1];
         bitString[0] = 0;
         Array.Copy(val, 0, bitString, 1, val.Length);
         
         // get the subject public key
         ASN1 subjectPublicKey = new ASN1(ASN1.ASN1Type.BitString, bitString);
         
         // get the subject public key info
         ASN1 subjectPublicKeyInfo = new ASN1(ASN1.ASN1Type.Sequence);
         subjectPublicKeyInfo.AddASN1(algId);
         subjectPublicKeyInfo.AddASN1(subjectPublicKey);
         
         // set the internal ASN.1 structure
         mASN1 = subjectPublicKeyInfo;         
      }
      
      /// <summary>
      /// Converts this X.509 encoded public key into DSAParameters.
      /// </summary>
      /// 
      /// <returns>the generated DSAParameters for this public key.</returns>
      protected virtual DSAParameters ConvertToDSAParameters()
      {
         DSAParameters dsaParams = new DSAParameters();
         
         if(mASN1 != null)
         {
            // get the first ASN.1 (algorithm identifier)
            ASN1 algId = ASN1[0];
            
            // get the parameters, the second ASN.1 (index 1) in the
            // algorithm identifer -- the first is the OID
            ASN1 parameters = algId[1];
            
            // P, Q, G (indices 0, 1, 2 in the parameters ASN.1)
            ASN1 P = parameters[0];
            ASN1 Q = parameters[1];
            ASN1 G = parameters[2];
            
            // copy as big int
            dsaParams.P = new byte[P.BigIntValue.Length];
            Array.Copy(P.BigIntValue, 0, dsaParams.P, 0, dsaParams.P.Length);
            
            dsaParams.Q = new byte[Q.BigIntValue.Length];
            Array.Copy(Q.BigIntValue, 0, dsaParams.Q, 0, dsaParams.Q.Length);

            dsaParams.G = new byte[G.BigIntValue.Length];
            Array.Copy(G.BigIntValue, 0, dsaParams.G, 0, dsaParams.G.Length);
            
            // get the second ASN.1 (index 1 -- subject public key BIT STRING)
            ASN1 subjectPublicKey = ASN1[1];
            
            // the first bit in the BIT STRING is the number of unused
            // bits in the first byte -- which is assumed to be zero because
            // the keys are stored as bytes
            byte[] val = subjectPublicKey.Value;
            byte[] bitString = new byte[val.Length - 1];
            Array.Copy(val, 1, bitString, 0, bitString.Length);
            
            // get the dsa public key, an ASN.1 INTEGER in the BIT STRING
            ASN1 dsaPublicKey = new ASN1(bitString);
            
            // copy as big int
            byte[] bigIntValue = dsaPublicKey.BigIntValue;
            if(bigIntValue != null)
            {
               dsaParams.Y = new byte[bigIntValue.Length];
               Array.Copy(bigIntValue, 0, dsaParams.Y, 0, dsaParams.Y.Length);
            }
            else
            {
               dsaParams.Y = new byte[0];
            }
         }
         
         return dsaParams;
      }
      
      /// <summary>
      /// Gets the DSAParameters for this X.509 encoded public key.
      /// </summary>
      public virtual DSAParameters DSAParameters
      {
         get
         {
            return ConvertToDSAParameters();
         }
      }
      
      /// <summary>
      /// Gets this X.509 encoded public key in binary form.
      /// </summary>
      /// 
      /// <returns>this X.509 encoded public key in binary form.</returns>
      public virtual byte[] Bytes
      {
         get
         {
            if(ASN1 != null)
            {
               return ASN1.Bytes;
            }
            else
            {
               return null;
            }
         }
      }
      
      /// <summary>
      /// Gets this X.509 encoded public key in ASN.1 format. Returns
      /// "SubjectPublicKeyInfo".
      /// </summary>
      /// 
      /// <returns>this X.509 encoded public key in ASN.1 format.</returns>
      public virtual ASN1 ASN1
      {
         get
         {
            return mASN1;
         }
      }
   }
}
