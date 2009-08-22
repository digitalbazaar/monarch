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
   /// A class for storing a PKCS#8 encoded private key. The only algorithm
   /// currently supported is the Digital Signature Algorithm (DSA).
   /// </summary>
   /// 
   /// <remarks>
   /// A PKCS#8 encoded key is stored in ASN.1 format. The PrivateKeyInfo
   /// ASN.1 structure is composed of a version of type Integer, a 
   /// privateKeyAlgorithm of type AlgorithmIdentifier, a privateKey of
   /// type octet string, and attributes of type set.
   /// 
   /// The AlgorithmIdentifier contains an Object Identifier (OID) and 
   /// parameters for the algorithm, if any. DSA requires parameters P, Q,
   /// and G. These are used in the Digital Signature Algorithm to create
   /// and verify signatures. These parameters must be provided along with
   /// the private key in order to create a signature.
   /// 
   /// PrivateKeyInfo ::= SEQUENCE
   /// {
   ///    version INTEGER,
   ///    privateKeyAlgorithm AlgorithmIdentifier,
   ///    privateKey OCTET STRING
   ///    attributes [0] IMPLICIT SET OPTIONAL
   /// }
   /// 
   /// AlgorithmIdentifer ::= SEQUENCE
   /// {
   ///    algorithm OBJECT IDENTIFIER,
   ///    parameters ANY DEFINED BY algorithm OPTIONAL
   /// }
   /// 
   /// For a DSA private key --
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
   /// The privateKey octet string contains the DSA private key.
   /// 
   /// An OCTET STRING has the following structure:
   /// 
   /// OCTECT STRING
   /// {
   ///    number of octets
   ///    data
   /// }
   /// </remarks>
   /// 
   /// <author>Dave Longley</author>
   public class PKCS8PrivateKey
   {
      /// <summary>
      /// The ASN.1 structure for this PKCS#8 public key.
      /// </summary>
      protected ASN1 mASN1;
      
      /// <summary>
      /// The OID for a DSA signature key.
      /// </summary>
      public static readonly string DSA_KEY_OID = "1.2.840.10040.4.1";
      
      /// <summary>
      /// Creates a new PKCS8PrivateKey from a byte array.
      /// </summary>
      /// 
      /// <param name="bytes">the byte array containing the PKCS#8 encoded
      /// private key.</param>
      public PKCS8PrivateKey(byte[] bytes)
      {
         if(bytes != null && bytes.Length > 0 &&
            (bytes[0] & (byte)ASN1.ASN1Type.Sequence) ==
            (byte)ASN1.ASN1Type.Sequence)
         {
            mASN1 = new ASN1(bytes);
         }
      }
      
      /// <summary>
      /// Creates a new PKCS8PrivateKey based on the passed DSAParameters.
      /// </summary>
      /// 
      /// <param name="dsaParams">the DSA parameters to create the private
      /// key from.</param>
      public PKCS8PrivateKey(DSAParameters dsaParams)
      {
         ConvertFromDSAParameters(dsaParams);
      }
      
      /// <summary>
      /// Converts DSAParameters into a PKCS#8 encoded private key.
      /// </summary>
      /// 
      /// <param name="dsaParams">the DSA parameters to create the private
      /// key from.</param>
      protected virtual void ConvertFromDSAParameters(DSAParameters dsaParams)
      {
         // get the version
         ASN1 version = ASN1.UnsignedBigIntToASN1(new byte[1]);
         
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
         
         // get the DSA private key
         ASN1 privateKey =
            ASN1.UnsignedBigIntToASN1(ASN1.ASN1Type.OctetString, dsaParams.X);
         
         // get the attributes set
         ASN1 attributes = new ASN1(ASN1.ASN1Type.Set, new byte[0]);
         
         // get the private key info
         ASN1 privateKeyInfo = new ASN1(ASN1.ASN1Type.Sequence);
         privateKeyInfo.AddASN1(version);
         privateKeyInfo.AddASN1(algId);
         privateKeyInfo.AddASN1(privateKey);
         privateKeyInfo.AddASN1(attributes);
         
         // set the internal ASN.1 structure
         mASN1 = privateKeyInfo;  
      }
      
      /// <summary>
      /// Converts this PKCS#8 encoded private key into DSAParameters.
      /// </summary>
      /// 
      /// <returns>the generated DSAParameters for this private key.</returns>
      protected virtual DSAParameters ConvertToDSAParameters()
      {
         DSAParameters dsaParams = new DSAParameters();

         if(mASN1 != null)
         {
            // get the second ASN.1 (algorithm identifier)
            ASN1 algId = ASN1[1];
            
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

            // get the third ASN.1 (index 1 -- private key)
            ASN1 privateKey = ASN1[2];
            
            // get value
            byte[] val = privateKey.Value;
            if(val != null)
            {
               // X parameter is an OCTET STRING, get the value as a big int
               ASN1 octetString = new ASN1(val);
               byte[] bigIntValue = octetString.BigIntValue;
               dsaParams.X = new byte[bigIntValue.Length];
               Array.Copy(bigIntValue, 0, dsaParams.X, 0, dsaParams.X.Length);
            }
            else
            {
               dsaParams.X = new byte[0];
            }
                       
            // set public key to blank byte array
            dsaParams.Y = new byte[0];
         }
         
         return dsaParams;
      }
      
      /// <summary>
      /// Gets the DSAParameters for this PKCS#8 encoded private key.
      /// </summary>
      public virtual DSAParameters DSAParameters
      {
         get
         {
            return ConvertToDSAParameters();
         }
      }
      
      /// <summary>
      /// Gets this PKCS#8 encoded private key in binary form.
      /// </summary>
      /// 
      /// <returns>this PKCS#8 encoded private key in binary form.</returns>
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
      /// Gets this PKCS#8 encoded private key in ASN.1 format. Returns
      /// "PrivateKeyInfo".
      /// </summary>
      /// 
      /// <returns>this PKCS#8 encoded private key in ASN.1 format.</returns>
      public virtual ASN1 ASN1
      {
         get
         {
            return mASN1;
         }
      }
      
      /// <summary>
      /// Returns the algorithm for this private key (currently only DSA is
      /// supported).
      /// </summary>
      public virtual string Algorithm
      {
         get
         {
            return "DSA";
         }
      }
   }
}
