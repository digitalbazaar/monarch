using System;
using System.Security;
using System.Security.Cryptography;

using DB.Common;
using DB.Common.Logging;

using Sms.Common;

/// <summary>
/// Unit Test for testing digital signing and digital signature verification.
/// </summary>
/// <author>Dave Longley</author>
/// <author>Manu Sporny</author>
class ITDigitalSignature
{
   /// <summary>
   /// Tests X509 public key exporting and importing.
   /// </summary>
   public static void TestX509PublicKey()
   {
      DSA dsa = new DSACryptoServiceProvider();
      DSAParameters dsap = dsa.ExportParameters(false);
      
      X509PublicKey publicKey = new X509PublicKey(dsap);
      byte[] encoded = publicKey.Bytes;
      
      DSAParameters dsap3 = publicKey.DSAParameters;
      
      Console.WriteLine("\nSTART DSAP CHECK\n");

      Console.WriteLine("P: {0}\n", Convert.ToBase64String(dsap.P));
      Console.WriteLine("Q: {0}\n", Convert.ToBase64String(dsap.Q));
      Console.WriteLine("G: {0}\n", Convert.ToBase64String(dsap.G));
      Console.WriteLine("Y: {0}\n", Convert.ToBase64String(dsap.Y));
      
      Console.WriteLine("AND NOW:\n");

      Console.WriteLine("P: {0}\n", Convert.ToBase64String(dsap3.P));
      Console.WriteLine("Q: {0}\n", Convert.ToBase64String(dsap3.Q));
      Console.WriteLine("G: {0}\n", Convert.ToBase64String(dsap3.G));
      Console.WriteLine("Y: {0}\n", Convert.ToBase64String(dsap3.Y));
      
      Console.WriteLine("\nEND DSAP CHECK\n");
      
      Console.WriteLine("key size=" + encoded.Length);
      
      int count = 0;
      foreach(byte b in encoded)
      {
         if(count > 20)
         {
            break;
         }
         
         count++;
         Console.WriteLine("byte=" + b);
      }
      
      string base64 = Convert.ToBase64String(encoded);
      Console.WriteLine("base64 length=" + base64.Length);
      Console.WriteLine("BASE64:\n" + base64);
      
      Console.WriteLine(
         "CONVERTING encoded bytes back to public key object...");
      
      // try loading the public key back in
      X509PublicKey publicKey2 =
         new X509PublicKey(Convert.FromBase64String(base64));
      
      Console.WriteLine("CONVERTED.");
      
      byte[] encoded2 = publicKey2.Bytes;
      
      Console.WriteLine("key size=" + encoded.Length);
      
      count = 0;
      foreach(byte b in encoded2)
      {
         if(count > 20)
         {
            break;
         }
         
         count++;
         Console.WriteLine("byte=" + b);
      }
      
      string base642 = Convert.ToBase64String(encoded2);
      Console.WriteLine("base64 length=" + base642.Length);
      Console.WriteLine("BASE64:\n" + base642);
      
      if(base642 == base64)
      {
         Console.WriteLine("PUBLIC KEYS MATCH!");
      }
      
      // try getting dsa parameters
      DSAParameters dsap2 = publicKey2.DSAParameters;
      DSACryptoServiceProvider dsa2 = new DSACryptoServiceProvider();
      
      Console.WriteLine("P: " + Convert.ToBase64String(dsap2.P));
      Console.WriteLine("Q: " + Convert.ToBase64String(dsap2.Q));
      Console.WriteLine("G: " + Convert.ToBase64String(dsap2.G));
      Console.WriteLine("Y: " + Convert.ToBase64String(dsap2.Y));

      dsa2.ImportParameters(dsap2);
      Console.WriteLine("PARAMETERS IMPORTED!");
   }
   
   /// <summary>
   /// Test PKCS8 encoding and decoding. 
   /// </summary>
   public static void TestPKCS8PrivateKey()
   {
      DSA dsa = new DSACryptoServiceProvider();
      DSAParameters dsap = dsa.ExportParameters(true);
      
      PKCS8PrivateKey privateKey = new PKCS8PrivateKey(dsap);
      byte[] encoded = privateKey.Bytes;
      
      DSAParameters dsap3 = privateKey.DSAParameters;
      
      Console.WriteLine("\nSTART DSAP CHECK\n");

      Console.WriteLine("P: {0}\n", Convert.ToBase64String(dsap.P));
      Console.WriteLine("Q: {0}\n", Convert.ToBase64String(dsap.Q));
      Console.WriteLine("G: {0}\n", Convert.ToBase64String(dsap.G));
      Console.WriteLine("X: {0}\n", Convert.ToBase64String(dsap.X));
      
      Console.WriteLine("AND NOW:\n");

      Console.WriteLine("P: {0}\n", Convert.ToBase64String(dsap3.P));
      Console.WriteLine("Q: {0}\n", Convert.ToBase64String(dsap3.Q));
      Console.WriteLine("G: {0}\n", Convert.ToBase64String(dsap3.G));
      Console.WriteLine("X: {0}\n", Convert.ToBase64String(dsap3.X));
      
      Console.WriteLine("\nEND DSAP CHECK\n");
      
      Console.WriteLine("key size=" + encoded.Length);
      
      int count = 0;
      foreach(byte b in encoded)
      {
         if(count > 20)
         {
            break;
         }
         
         count++;
         Console.WriteLine("byte=" + b);
      }
      
      string base64 = Convert.ToBase64String(encoded);
      Console.WriteLine("base64 length=" + base64.Length);
      Console.WriteLine("BASE64:\n" + base64);
      
      Console.WriteLine(
         "CONVERTING encoded bytes back to private key object...");
      
      // try loading the private key back in
      PKCS8PrivateKey privateKey2 =
         new PKCS8PrivateKey(Convert.FromBase64String(base64));
      
      Console.WriteLine("CONVERTED.");
      
      byte[] encoded2 = privateKey2.Bytes;
      
      Console.WriteLine("key size=" + encoded.Length);
      
      count = 0;
      foreach(byte b in encoded2)
      {
         if(count > 20)
         {
            break;
         }
         
         count++;
         Console.WriteLine("byte=" + b);
      }
      
      string base642 = Convert.ToBase64String(encoded2);
      Console.WriteLine("base64 length=" + base642.Length);
      Console.WriteLine("BASE64:\n" + base642);
      
      if(base642 == base64)
      {
         Console.WriteLine("PRIVATE KEYS MATCH!");
      }
      
      // try getting dsa parameters
      DSAParameters dsap2 = privateKey2.DSAParameters;
      DSACryptoServiceProvider dsa2 = new DSACryptoServiceProvider();
      
      Console.WriteLine("P: " + Convert.ToBase64String(dsap2.P));
      Console.WriteLine("Q: " + Convert.ToBase64String(dsap2.Q));
      Console.WriteLine("G: " + Convert.ToBase64String(dsap2.G));
      Console.WriteLine("X: " + Convert.ToBase64String(dsap2.X));

      dsa2.ImportParameters(dsap2);
      Console.WriteLine("PARAMETERS IMPORTED!");
   }
   
   /// <summary>
   /// Tests the digital signature algorithm.
   /// </summary>
   public static void TestDSA()
   {
      // generate keys
      KeyManager km = new KeyManager();
      if(km.GenerateKeyPair())
      {
         Console.WriteLine("key pair generated!");
      }
      
      string text = "the text";
      
      byte[] sig = Cryptor.Sign(text, km.PrivateKey);
      
      Console.WriteLine("sig: " + Convert.ToBase64String(sig));
      
      Console.WriteLine("verified: " + Cryptor.Verify(sig, text, km.PublicKey));
      
      // store private and public keys
      km.StorePrivateKey("private.key", "password");
      km.StorePublicKey("public.key");
      
      // load private and public keys
      KeyManager km2 = new KeyManager();
      km2.LoadPrivateKey("private.key", "password");
      km2.LoadPublicKey("public.key");
      
      string text2 = "the text";
      byte[] sig2 = Cryptor.Sign(text2, km2.PrivateKey);
      
      Console.WriteLine("sig2: " + Convert.ToBase64String(sig2));
      
      Console.WriteLine("verified2: " +
         Cryptor.Verify(sig2, text2, km2.PublicKey));
      Console.WriteLine("verified3: " +
         Cryptor.Verify(sig, text, km2.PublicKey));
   }
   
   /// <summary>
   /// Tests the signable XML envelope class.
   /// </summary>
   public static void TestSXE()
   {
      Console.WriteLine("Testing Signable XML Envelope...");
      
      bool doc = false;
      
      // generate keys for signing/verifying
      KeyManager km = new KeyManager();
      km.GenerateKeyPair();
      
      PKCS8PrivateKey privateKey = km.PrivateKey;
      X509PublicKey publicKey = km.PublicKey;
      
      // create a patient and form to be placed inside the envelope
      Patient patient = new Patient();
      Form form = new Form();
      form.Id = 1;
      form.ShortName = "DEMS-1";
      form.LongName = "Demographics";
      form.SetField("first_name", "John");
      form.SetField("last_name", "Doe");
      patient.AddForm(form);
      
      SignableXMLEnvelope sxe = new SignableXMLEnvelope(patient);
      if(sxe.Sign(1, privateKey))
      {
         Console.WriteLine("TEST: Successfully signed envelope.");
      }
      else
      {
         Console.WriteLine("FAILURE: Failed to sign envelope!");
      }
      
      string xml = sxe.ConvertToXML(doc);
      
      Console.WriteLine("XML:\n" + xml);
      
      Patient patient2 = new Patient();
      SignableXMLEnvelope sxe2 = new SignableXMLEnvelope(patient2);
      sxe2.ConvertFromXML(xml);
      
      // we should be able to verify the signature
      if(sxe2.Verify(publicKey))
      {
         Console.WriteLine("TEST: Successfully verified envelope.");
      }
      else
      {
         Console.WriteLine("FAILURE: Envelope signature is invalid!");
      }
      
      string xml2 = sxe2.ConvertToXML(doc);
      Console.WriteLine("XML2:\n" + xml2);
      
      // This should cause the test to fail due to modifying the contents
      // of the envelope
      sxe2.ConvertFromXML(xml.Insert(xml.IndexOf("Doe") + 3, "rety"));
      
      // This test should fail because the last name of the patient 
      // has been changed
      if(!sxe2.Verify(publicKey))
      {
         Console.WriteLine("TEST: Successfully detected forged signature.");
      }
      else
      {
         Console.WriteLine("FAILURE: Verified FORGED signature!");
      }
      string xml3 = sxe2.ConvertToXML(doc);
      Console.WriteLine("XML3:\n" + xml3);
   }
   
   /// <summary>
   /// Tests X509 Public key storage and retrieval, X509 public key stuff,
   /// digital signature algorithm execution and signable XML envelope. 
   /// </summary>
   public static void Main(string[] args)
   {
      Console.WriteLine("Running Tests...");
      
      LoggerManager.SetFile("smscommon", "it-digitalsignature.log", false);
      LoggerManager.SetVerbosity("smscommon",
         Logger.MAX_VERBOSITY, Logger.MAX_VERBOSITY);
      try
      {
         //TestX509PublicKey();
         //TestPKCS8PrivateKey();
         //TestDSA();
         TestSXE();
      }
      catch(Exception e)
      {
         Console.WriteLine("Exception thrown.");
         Console.WriteLine(e.StackTrace);
         Console.WriteLine(e.ToString());
      }
   }
}
