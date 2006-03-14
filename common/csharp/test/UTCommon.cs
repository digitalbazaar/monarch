// created on 09/26/2005 at 15:39
using System;
using System.Security;
using System.Security.Cryptography;

using DB.Common;
using DB.Common.Logging;

using Sms.Common;

class SmsCommonUnitTests
{
   public static void TestLogger()
   {
      LoggerManager.SetFile("smscommon", "logtext.log", false);
      LoggerManager.SetVerbosity("smscommon",
         Logger.MAX_VERBOSITY, Logger.MAX_VERBOSITY);
      
      LoggerManager.SetFile("smscommon2", "logtext.log", true);
      LoggerManager.SetVerbosity("smscommon2",
         Logger.MAX_VERBOSITY, Logger.MAX_VERBOSITY);
            
      LoggerManager.Debug("smscommon", "testing log.");
      LoggerManager.Error("smscommon", "testing log.");
      
      LoggerManager.Debug("smscommon2", "testing log.");
      LoggerManager.Error("smscommon2", "testing log.");
   }
   
   public static void TestASN1()
   {
      byte[] bytes = new byte[256 * 256];
      bytes[0] = 1;
      bytes[1] = 2;
      bytes[2] = 3;
      bytes[3] = 4;
      bytes[4] = 5;
      
      ASN1 asn1 = new ASN1(ASN1.ASN1Type.Integer, bytes);
      
      ASN1 iAsn = new ASN1(ASN1.ASN1Type.Sequence);
      iAsn.AddASN1(asn1);
      
      int count = 0;
      //byte[] asn1Bytes = asn1.Bytes;
      byte[] asn1Bytes = iAsn.Bytes;
      foreach(byte b in asn1Bytes)
      {
         if(count > 10)
         {
            break;
         }
         
         count++;
         Console.WriteLine("byte: " + b);
      }
      
      // create a new ASN1 converted from bytes
      ASN1 newAsn1 = new ASN1(asn1Bytes);
      
      Console.WriteLine("THE NEW ASN:");
      
      count = 0;
      byte[] asn1Bytes2 = newAsn1.Bytes;
      foreach(byte b in asn1Bytes2)
      {
         if(count > 10)
         {
            break;
         }
         
         count++;
         Console.WriteLine("byte: " + b);
      }
   }
   
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
   
   public static void TestEncryption()
   {
      Console.WriteLine("ENCRYPTION:");
   
      byte[] data = {0x01,0x02,0x03,0x04};
      
      Cryptor c = new Cryptor();
      
      Console.WriteLine("DATA=" + Convert.ToBase64String(data));
      
      byte[] enc = c.Encrypt(data);
      
      Console.WriteLine("enc=" + Convert.ToBase64String(enc));
      
      byte[] dec = c.Decrypt(enc);
      
      Console.WriteLine("dec=" + Convert.ToBase64String(dec));
   
      Console.WriteLine("PASSWORD BASED ENCRYPTION:");
   
      string encrypted = Cryptor.Encrypt("chicken", "password");
      
      Console.WriteLine("encrypted data=" + encrypted);
      
      string decrypted = Cryptor.Decrypt(encrypted, "password");
      
      Console.WriteLine("decrypted data=" + decrypted);
   }
   
   public static void TestSXE()
   {
      Console.WriteLine("Testing Signable XML Envelope...");
      
      bool doc = false;
      
      // generate keys for signing/verifying
      /*KeyManager km = new KeyManager();
      km.GenerateKeyPair();
      
      PKCS8PrivateKey privateKey = km.PrivateKey;
      X509PublicKey publicKey = km.PublicKey;
      
      SignableXMLEnvelope sxe = new SignableXMLEnvelope();
      if(sxe.Sign(1, privateKey))
      {
         Console.WriteLine("envelope signed!");
      }
      
      string xml = sxe.ConvertToXML(doc);
      
      Console.WriteLine("XML:\n" + xml);*/
      
      string xml =
         "<envelope version=\"1.0\" signer=\"1\" status=\"valid\">" +
         "<signature algorithm=\"SHA1/DSA\">1234</signature>" +
         "</envelope>";
      
      Console.WriteLine("xml:\n" + xml);
      
      SignableXMLEnvelope sxe2 = new SignableXMLEnvelope();
      sxe2.ConvertFromXML(xml);
      
      /*if(sxe2.Verify(publicKey))
      {
         Console.WriteLine("envelope verified!");
      }*/
      
      string xml2 = sxe2.ConvertToXML(doc);
      Console.WriteLine("XML2:\n" + xml2);
   }
   
   public static void TestCryptoMath()
   {
      /*Console.WriteLine("\nAdding:");
      
      //byte[] x = new byte[]{0x20,0x30,0x40};
      //byte[] y = new byte[]{0xFE,0x1C,0xFF};
      //byte[] x = new byte[]{0x01};
      //byte[] y = new byte[]{0xFF};
      byte[] x = new byte[]{0xFF};
      byte[] y = new byte[]{0xFE};
      
      byte[] sum = CryptoMath.Add(x, y);
      foreach(byte b in sum)
      {
         Console.WriteLine("byte=" + b);
      }
      
      Console.WriteLine("\nSubtracting:");
      byte[] diff = CryptoMath.Subtract(x, y);
      foreach(byte b in diff)
      {
         Console.WriteLine("byte=" + b);
      }*/
      
      Console.WriteLine("\nMultiplying:");
      byte[] m = new byte[]{0x02,0x02,0xFF};
      byte[] n = new byte[]{0xFF,0xFF};
      byte[] product = CryptoMath.Multiply(m, n);
      foreach(byte b in product)
      {
         Console.WriteLine("byte=" + b);
      }
   }
   
   public static void TestPatient()
   {
      Patient patient = new Patient();
      patient.Id = 1;
      
      Form form = new Form();
      form.Id = 2;
      form.SetField("basic_field", "test data");
      form.SetField("field_with_value_and_data", "test data", 
                    "MIIawme235hj43oi243509gj4398bj349gbj4u5i2cja923lhj98f2");
      form.SetField("field_with_data", "", "MII58gfj376dhg2875jg74hf73hjd83j");
      
      patient.AddForm(form);
      
      string xml = patient.ConvertToXML(false);
      
      Console.WriteLine("patient xml:\n" + xml);
      
      Patient patient2 = new Patient();
      patient2.ConvertFromXML(xml);
      
      string xml2 = patient2.ConvertToXML(false);
      
      Console.WriteLine("patient xml2:\n" + xml2);
   }
   
   public static void TestPatient2()
   {
      string testXml = "<patient id=\"34\">";
      testXml += "<form id=\"1\" short_name=\"\" long_name=\"\" active=\"true\">";
      testXml += "<government_id>448242384</government_id>";
      testXml += "<middle_name>Goulash</middle_name>";
      testXml += "<last_name>Yarkos</last_name>";
      testXml += "<date_of_birth>2005-10-04</date_of_birth>";
      testXml += "<first_name>Roman</first_name>";
      testXml += "</form>";
      testXml += "<form id=\"2\" short_name=\"\" long_name=\"\" active=\"true\">";
      testXml += "<government_id>123456789</government_id>";
      testXml += "<middle_name>David</middle_name>";
      testXml += "<last_name>Smith</last_name>";
      testXml += "<date_of_birth>2005-10-04</date_of_birth>";
      testXml += "<first_name>Gavin</first_name>";
      testXml += "</form>";
      testXml += "</patient>";
      
      Console.WriteLine("\n\nTestXML:\n[" + testXml + "]\n\n");
      Patient p = new Patient();
      p.ConvertFromXML(testXml);
      
      Console.WriteLine(p);
      
      Console.WriteLine(p.ConvertToXML(true));
   }
   
   public static void TestPKeyCryptor()
   {
      PKeyCryptor pkc = new PKeyCryptor();
      pkc.GenerateKeys("chicken");
      pkc.StorePrivateKey("testme.key");
      Console.WriteLine("key=" + pkc.PrivateKeyString);
      
      PKeyCryptor pkc2 = new PKeyCryptor("testme.key", "chicken");
      Console.WriteLine("key=" + pkc2.PrivateKeyString);
   }
   
   public static void TestProfileCryptor()
   {
      ProfileCryptor pc = new ProfileCryptor();
      pc.GenerateKeys("chicken");
      pc.StoreProfile("testme.profile", 12);
      Console.WriteLine("\n");
      Console.WriteLine("ID=" + pc.UserId);
      Console.WriteLine("key=" + pc.PrivateKeyString);
      
      ProfileCryptor pc2 = new ProfileCryptor();
      pc2.ProfileFilename = "testme.profile";
      pc2.Verify("chicken");
      Console.WriteLine("\n");
      Console.WriteLine("ID=" + pc2.UserId);
      Console.WriteLine("KEY=" + pc2.PrivateKeyString);
   }
   
   public static void TestJavaPKCS8PrivateKey()
   {
      string base64 =
         "MIIBSwIBADCCASwGByqGSM44BAEwggEfAoGBAP1/U4EddRIpUt9KnC7s5Of2EbdSPO9EAMMeP4C2USZpRV1AIlH7WT2NWPq/xfW6MPbLm1Vs14E7gB00b/JmYLdrmVClpJ+f6AR7ECLCT7up1/63xhv4O1fnxqimFQ8E+4P208UewwI1VBNaFpEy9nXzrith1yrv8iIDGZ3RSAHHAhUAl2BQjxUjC8yykrmCouuEC/BYHPUCgYEA9+GghdabPd7LvKtcNrhXuXmUr7v6OuqC+VdMCz0HgmdRWVeOutRZT+ZxBxCBgLRJFnEj6EwoFhO3zwkyjMim4TwWeotUfI0o4KOuHiuzpnWRbqN/C/ohNWLx+2J6ASQ7zKTxvqhRkImog9/hWuWfBpKLZl6Ae1UlZAFMO/7PSSoEFgIUXd36HEVpwJ/3x6rpoTZWb7zSYRk=";
         //"MIIBSwIBADCCASwGByqGSM44BAEwggEfAoGBAP1/U4EddRIpUt9KnC7s5Of2EbdSPO9EAMMeP4C2USZpRV1AIlH7WT2NWPq/xfW6MPbLm1Vs14E7gB00b/JmYLdrmVClpJ+f6AR7ECLCT7up1/63xhv4O1fnxqimFQ8E+4P208UewwI1VBNaFpEy9nXzrith1yrv8iIDGZ3RSAHHAhUAl2BQjxUjC8yykrmCouuEC/BYHPUCgYEA9+GghdabPd7LvKtcNrhXuXmUr7v6OuqC+VdMCz0HgmdRWVeOutRZT+ZxBxCBgLRJFnEj6EwoFhO3zwkyjMim4TwWeotUfI0o4KOuHiuzpnWRbqN/C/ohNWLx+2J6ASQ7zKTxvqhRkImog9/hWuWfBpKLZl6Ae1UlZAFMO/7PSSoEFgIUMVaXw4ASgLWDE5rbO82l52fzdYo=";
      
      Console.WriteLine("base64 length=" + base64.Length);
      Console.WriteLine("BASE64:\n" + base64);
      
      // try loading the private key
      PKCS8PrivateKey privateKey  =
         new PKCS8PrivateKey(Convert.FromBase64String(base64));
      
      Console.WriteLine("CONVERTED.");
      
      byte[] encoded = privateKey.Bytes;
      
      Console.WriteLine("key size=" + encoded.Length);
      
      string base642 = Convert.ToBase64String(encoded);
      Console.WriteLine("base64 length=" + base642.Length);
      Console.WriteLine("BASE64:\n" + base642);
      
      if(base642 == base64)
      {
         Console.WriteLine("PRIVATE KEYS MATCH!");
      }
      
      // try getting dsa parameters
      DSAParameters dsap = privateKey.DSAParameters;
      
      Console.WriteLine("\nP: " + Convert.ToBase64String(dsap.P));
      Console.WriteLine("\nQ: " + Convert.ToBase64String(dsap.Q));
      Console.WriteLine("\nG: " + Convert.ToBase64String(dsap.G));
      Console.WriteLine("\nX: " + Convert.ToBase64String(dsap.X));

      // try importing parameters
      DSACryptoServiceProvider dsa = new DSACryptoServiceProvider();
      dsa.ImportParameters(dsap);
      Console.WriteLine("PARAMETERS IMPORTED!");
   }

   public static void TestJavaX509PublicKey()
   {
      string base64 =
         "MIIBuDCCASwGByqGSM44BAEwggEfAoGBAP1/U4EddRIpUt9KnC7s5Of2EbdSPO9EAMMeP4C2USZpRV1AIlH7WT2NWPq/xfW6MPbLm1Vs14E7gB00b/JmYLdrmVClpJ+f6AR7ECLCT7up1/63xhv4O1fnxqimFQ8E+4P208UewwI1VBNaFpEy9nXzrith1yrv8iIDGZ3RSAHHAhUAl2BQjxUjC8yykrmCouuEC/BYHPUCgYEA9+GghdabPd7LvKtcNrhXuXmUr7v6OuqC+VdMCz0HgmdRWVeOutRZT+ZxBxCBgLRJFnEj6EwoFhO3zwkyjMim4TwWeotUfI0o4KOuHiuzpnWRbqN/C/ohNWLx+2J6ASQ7zKTxvqhRkImog9/hWuWfBpKLZl6Ae1UlZAFMO/7PSSoDgYUAAoGBAO/542mdAOAP+X+ShonbS/0/O+hYR8MuupGlpN394W6G9AcFXNyGCfvgEkGSqpL6OvvDrodU9bpLA9+u8kJm5kxnE27gB3nuo+s26xCNC0QKgYt+RR8hvA3f53wK2y8worRZmqcpsiyl3QQBN3QL3t3FTeffTc+pjWju9fkotYkx";
         
      Console.WriteLine("base64 length=" + base64.Length);
      Console.WriteLine("BASE64:\n" + base64);
      
      // try loading the public key
      X509PublicKey publicKey =
         new X509PublicKey(Convert.FromBase64String(base64));
      
      Console.WriteLine("CONVERTED.");
      
      byte[] encoded = publicKey.Bytes;
      
      Console.WriteLine("key size=" + encoded.Length);
      
      string base642 = Convert.ToBase64String(encoded);
      Console.WriteLine("base64 length=" + base642.Length);
      Console.WriteLine("BASE64:\n" + base642);
      
      if(base642 == base64)
      {
         Console.WriteLine("PUBLIC KEYS MATCH!");
      }
      
      // try getting dsa parameters
      DSAParameters dsap = publicKey.DSAParameters;
      
      Console.WriteLine("P: " + Convert.ToBase64String(dsap.P));
      Console.WriteLine("Q: " + Convert.ToBase64String(dsap.Q));
      Console.WriteLine("G: " + Convert.ToBase64String(dsap.G));
      Console.WriteLine("Y: " + Convert.ToBase64String(dsap.Y));

      // try importing parameters
      DSACryptoServiceProvider dsa = new DSACryptoServiceProvider();
      dsa.ImportParameters(dsap);
      Console.WriteLine("PARAMETERS IMPORTED!");
   }
   
   public static void TestGroup()
   {
        Group group = new Group();
        group.Id = 12;
        group.Name = "doctors";
       
        for(int i=0, shift=1; i<3; i++, shift+=10)
        {   
           for(int j=0; j<10; j++)
           {
              group.AddFormPermission(j+shift, i+1);
           }
        }
        Console.WriteLine(group);
      
      string xml = group.ConvertToXML(false);
      
      Console.WriteLine("group xml:\n" + xml);
      
      Group group2 = new Group();
      group2.ConvertFromXML(xml);
      
      string xml2 = group2.ConvertToXML(false);
      
      Console.WriteLine("group xml2:\n" + xml2);
   }
   
   public static void TestGroup2()
   {
   string testXml = "<group id=\"34\">";
      testXml += "<group_name>doctors</group_name>";
      testXml += "<acl_read_forms>1,2,3,4,5,6,7,8,9,10</acl_read_forms>";
      testXml += "<acl_create_forms>11,12,13,14,15,16,17,18,19,20";
      testXml += "</acl_create_forms>";
      testXml += "<acl_modify_forms>21,22,23,24,25,26,27,28,29,30";
      testXml += "</acl_modify_forms>";
      testXml += "</group>";
      
      Console.WriteLine("\n\nTestXML:\n[" + testXml + "]\n\n");
      Group g = new Group();
      g.ConvertFromXML(testXml);
      
      Console.WriteLine(g);
      
      Console.WriteLine(g.ConvertToXML(true));   
   }
   
   public static void TestOrganization()
   {
      Organization org = new Organization();
      org.Id = 12;
      org.Name = "Digital Bazaar";
      org.Location = "Bburg, VA";
      org.Description = "contractors for SMS";
      org.External = true;
             
      Console.WriteLine(org);
      
      string xml = org.ConvertToXML(false);
      
      Console.WriteLine("org xml:\n" + xml);
      
      Organization org2 = new Organization();
      org2.ConvertFromXML(xml);
      
      string xml2 = org2.ConvertToXML(false);
      
      Console.WriteLine("org xml2:\n" + xml2);
   }
   
   public static void TestOrganization2()
   {
      string testXml = "<organization id=\"34\">";
      testXml += "<name>Digital Bazaar</name>";
      testXml += "<location>Bburg, VA</location>";
      testXml += "<description>contractors for SMS";
      testXml += "</description>";
      testXml += "<external>true";
      testXml += "</external>";
      testXml += "</organization>";
      
      Console.WriteLine("\n\nTestXML:\n[" + testXml + "]\n\n");
      Organization o = new Organization();
      o.ConvertFromXML(testXml);
      
      Console.WriteLine(o);
      
      Console.WriteLine(o.ConvertToXML(true));   
   }
   
   public static void Main(string[] args)
   {
      Console.WriteLine("Running Tests...");
      
      try
      {
         TestLogger();
         //TestBase64();
         //TestASN1();
         //TestX509PublicKey();
         //TestPKCS8PrivateKey();
         //TestDSA();
         //TestEncryption();
         //TestSXE();
         //TestCryptoMath();
         TestPatient();
         TestPatient2();
         //TestGroup();
         //TestGroup2();
         //TestOrganization();
         //TestOrganization2();
         //TestPKeyCryptor();
         //TestProfileCryptor();
         //TestJavaX509PublicKey();
         //TestJavaPKCS8PrivateKey();
      }
      catch(Exception e)
      {
         Console.WriteLine("Exception thrown.");
         Console.WriteLine(e.StackTrace);
         Console.WriteLine(e.ToString());
      }
   }
}
