/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.net.http.HttpWebConnectionServer;
import com.db.net.http.HttpWebRequest;
import com.db.net.http.HttpWebRequestServicer;
import com.db.net.http.HttpWebResponse;
import com.db.net.soap.*;

/**
 * This class is used to test SOAP functionality.
 * 
 * @author Dave Longley
 */
public class UTSoap
{
   /**
    * Runs the unit test.
    *
    * @param args the arguments.
    */
   public static void main(String[] args)
   {
      try
      {
         // set up loggers
         LoggerManager.setFile(
            "dbnet", "ut-soap.log", false);
         LoggerManager.setConsoleVerbosity("dbnet", Logger.MAX_VERBOSITY);//Logger.DEBUG_VERBOSITY);
         
         System.out.println("Starting SOAP functionality test...\n");
         
         int port = 9999;
         
         // create http server
         final HttpWebConnectionServer server = new HttpWebConnectionServer();
         
         // create test soap service
         TestSoapService service = new TestSoapService(port);
         
         // create soap http web request servicer
         SoapHttpWebRequestServicer servicer =
            new SoapHttpWebRequestServicer(service);
         
         // add http request servicer to the http server
         server.addNonSecureHttpWebRequestServicer(servicer, "/soap/");
         
         // add an http web request servicer for shutting down the server
         HttpWebRequestServicer shutdownServicer =
            new HttpWebRequestServicer()
         {
            public void serviceHttpWebRequest(
                  HttpWebRequest request, HttpWebResponse response)
            {
               System.out.println("Shutting down http server...");
               response.getHeader().setConnection("close");
               
               // stop server
               server.stop();
               
               System.out.println("Http server shut down.");
            }
            
            public void setHttpWebRequestServicerPath(String path)
            {
               // do nothing
            }            
         };
         
         // add the shutdown servicer
         server.addNonSecureHttpWebRequestServicer(
            shutdownServicer, "/shutdown/");
         
         // add an http web request servicer for sending a soap request
         HttpWebRequestServicer sendRequestServicer =
            new HttpWebRequestServicer()
         {
            public void serviceHttpWebRequest(
                  HttpWebRequest request, HttpWebResponse response)
            {
               System.out.println("Sending soap request...");
               response.getHeader().setConnection("close");
               
               SoapHttpClient client = new SoapHttpClient(
                  "/soap/?WSDL", "ITestSoapService");
               
               try
               {
                  client.setUrl("http://localhost:9999/soap/");
                  //Object result = client.callSoapMethod("soapTest", null);
                  Object result = client.callSoapMethod(
                     "soapTest2", new Object[]{"rhino"});
                  
                  System.out.println("SOAP RESULT=" + String.valueOf(result));
               }
               catch(Throwable t)
               {
                  t.printStackTrace();
               }
               
               System.out.println("Soap request sent.");
            }
            
            public void setHttpWebRequestServicerPath(String path)
            {
               // do nothing
            }            
         };
         
         // add the send request servicer
         server.addNonSecureHttpWebRequestServicer(
            sendRequestServicer, "/sendrequest/");         
         
         // start server
         server.startNonSecure(port);
         
         /*
         RpcSoapEnvelope envelope = new RpcSoapEnvelope();
         SoapOperation operation = new SoapOperation("soapDoIt", "urn:Service");
         envelope.setSoapOperation(operation);
         
         String xml = envelope.convertToXml(true, 0, 1);
         
         System.out.println("envelope xml=\n" + xml);
         
         envelope = new RpcSoapEnvelope();
         envelope.convertFromXml(xml);
         
         String xml2 = envelope.convertToXml(true, 0, 1);
         System.out.println("\nenvelope xml2=\n" + xml2);
         
         // get the operation
         System.out.println("has op=" + envelope.containsSoapOperation());
         
         SoapOperation operation2 = envelope.getSoapOperation();
         System.out.println("op=" + operation2.convertToXml(false, 0, 1));*/
         
         /*
         SoapEnvelope envelope = new SoapEnvelope();
         SoapOperation operation = new SoapOperation("soapDoit", "urn:Service");
         SoapOperationParameter parameter =
            new SoapOperationParameter("String_1", "mystring", null);//"urn:whatever");
         operation.addParameter(parameter);
         envelope.addBodyContentSerializer(operation);
         
         String xml = envelope.convertToXml(true, 0, 1);
         
         System.out.println("envelope xml=\n" + xml);
         
         SoapEnvelope envelope2 = new SoapEnvelope();
         envelope2.convertFromXml(xml);
         
         //String xml2 = envelope2.convertToXml(true, 0, 1);
         
         SoapOperation operation2 = new SoapOperation();
         operation2.convertFromXmlElement(
            (XmlElement)envelope2.getBodyContents().get(0));
         //envelope2.addBodyContentSerializer(operation2);
         SoapEnvelope envelope3 = new SoapEnvelope();
         envelope3.addBodyContentSerializer(operation2);
         String xml2 = envelope3.convertToXml(true, 0, 1);
         
         System.out.println("\nenvelope xml2=\n" + xml2);*/
         
         //System.out.println("\nSOAP functionality test complete.");
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
   
   /**
    * A test soap service.
    * 
    * @author Dave Longley
    */
   public static class TestSoapService
   extends AbstractSoapWebService
   implements ITestSoapService
   {
      /**
       * Creates a new test soap service.
       * 
       * @param port the port for this soap service.
       */
      public TestSoapService(int port)
      {
         // setup soap service information
         super("TestSoapService", "urn:TestSoapService",
            ITestSoapService.class);
         
         // proxy port will forward to SSL port
         String hostname = "localhost";
         setURI("http://" + hostname + ":" + port + "/soap/");
         setWsdlPath("/soap/?WSDL");
      }
      
      /**
       * Gets the string "test successful".
       * 
       * @return the string "test successful".
       */
      public String soapTest()      
      {
         return "test successful";
      }
      
      /**
       * Returns the passed string + the string " returned".
       * 
       * @param test the test string.
       * 
       * @return the passed string + the string " returned".
       */
      public String soapTest2(String test)      
      {
         return test + " returned";
      }
   }
   
   /**
    * The soap interface for the TestSoapWebService.
    * 
    * @author Dave Longley
    */
   public static interface ITestSoapService
   {
      /**
       * Gets the string "test successful".
       * 
       * @return the string "test successful".
       */
      public String soapTest();
      
      /**
       * Returns the passed string + the string " returned".
       * 
       * @param test the test string.
       * 
       * @return the passed string + the string " returned".
       */
      public String soapTest2(String test);      
   }
}
