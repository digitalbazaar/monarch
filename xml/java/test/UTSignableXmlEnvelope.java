/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
import com.db.crypto.*;
import com.db.xml.*;

/**
 * A unit test for SignableXmlEnvelope.
 * 
 * @author Dave Longley
 */
public class UTSignableXmlEnvelope
{
   /**
    * Does the envelope test.
    */
   public static void doEnvelopeTest()
   {
      System.out.println("Putting string \"chicken\" into envelope...");

      String xmlText = "";

      System.out.println("Generating keys for signing envelope...");

      KeyManager km = new KeyManager();
      boolean generated = km.generateKeyPair();

      System.out.println("Keys generated: " + generated);

      if(!generated)
      {
         System.exit(1);
      }
      
      StringXmlSerializer sxs = new StringXmlSerializer("chicken");
      SignableXmlEnvelope sxe = new SignableXmlEnvelope(sxs);

      if(sxe.sign(0, km.getPrivateKey()))
      {
         System.out.println("Successfully signed envelope");
      }
      else
      {
         System.err.println("Unable to sign envelope.");
         return;
      }
      
      // put that envelope in another envelope and sign it
      SignableXmlEnvelope sxe2 = new SignableXmlEnvelope(sxe);
      sxe2.sign(0, km.getPrivateKey());
      
      System.out.println("--- Converting envelope in envelope to XML ---");
      System.out.println(sxe2.convertToXml());
      System.out.println("--- Loading envelope in envelope from XML ---");
      String text = sxe2.convertToXml();
      SignableXmlEnvelope sxe3 =
         new SignableXmlEnvelope(new StringXmlSerializer());
      sxe2 = new SignableXmlEnvelope(sxe3, text);
      System.out.println(sxe2.convertToXml());
      System.out.println("--- Verify envelope in envelope test ---");
      System.out.println("Verified: " + sxe2.verify(km.getPublicKeyString()));
      System.out.println("Verified: " + sxe3.verify(km.getPublicKeyString()));

      System.out.println("--- Convert to XML test ---");

      xmlText = sxe.convertToXml();
      System.out.println(xmlText);
      
      System.out.println("--- Load XML test ---");

      // get a new envelope to test
      sxe = new SignableXmlEnvelope(new StringXmlSerializer());

      if(sxe.convertFromXml(xmlText))
      {
         System.out.println("Successfully loaded envelope from XML\n");
      }
      else
      {
         System.out.println("ERROR!! COULD NOT LOAD ENVELOPE");
      }

      try
      {
         System.out.println("--- Convert to XML after XML load test ---");
         System.out.println(sxe.convertToXml());
      }
      catch(NullPointerException e)
      {
         e.printStackTrace();
      }

      System.out.println("--- Verify Test ---");

      if(sxe.verify(km.getPublicKeyString()))
      {
         System.out.println("SUCCESS: envelope verified!");
      }
      else
      {
         System.out.println("FAILURE: envelope not verified!");
      }
      
      System.out.println("--- Envelope in envelope test ---");
      
      SignableXmlEnvelope outerEnvelope = new SignableXmlEnvelope(sxe);
      String outerXml = outerEnvelope.convertToXml();
      
      StringXmlSerializer innerSxs = new StringXmlSerializer();
      SignableXmlEnvelope innerEnvelope =
         new SignableXmlEnvelope(innerSxs);
      SignableXmlEnvelope outerEnvelope2 =
         new SignableXmlEnvelope(innerEnvelope);
      
      outerEnvelope2.convertFromXml(outerXml);
      
      System.out.println("ENVELOPE IN ENVELOPE=\n" +
         outerEnvelope2.convertToXml());
   }
   
   /**
    * Performs the unit test.
    * 
    * @param args the arguments.
    */
   public static void main(String[] args)
   {
      com.db.logging.LoggerManager.setConsoleVerbosity(
         "dbxml", com.db.logging.Logger.DEBUG_VERBOSITY);
      
      System.out.println("\nTesting SignableXMLEnvelope...");
      System.out.println("----------------------------\n");

      doEnvelopeTest();
   }
}
