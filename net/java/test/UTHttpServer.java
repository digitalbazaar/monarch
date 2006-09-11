/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
import com.db.util.MethodInvoker;
import com.db.logging.*;
import com.db.net.http.HttpGetRequestServicer;
import com.db.net.http.HttpProxyPortWebServer;
import com.db.net.http.HttpWebClient;
import com.db.net.http.HttpWebConnection;
import com.db.net.http.HttpWebRequest;
import com.db.net.http.HttpWebRequestServicer;
import com.db.net.http.HttpWebResponse;

/**
 * Tests an HttpServer.
 * 
 * @author Dave Longley
 */
public class UTHttpServer
{
   /**
    * The main method.
    * 
    * @param args the args.
    */
   public static void main(String[] args)
   {
      try
      {
         System.out.println("\nTesting HttpServer...");
         System.out.println("------------------\n");
         
         // set up logger
         LoggerManager.setFile("dbcommon", "ut-httpserver.log", false);
         LoggerManager.setFile("dbutil", "ut-httpserver.log");
         LoggerManager.setFileVerbosity("dbutil", Logger.DETAIL_VERBOSITY);
         LoggerManager.setFile("dbcrypto", "ut-httpserver.log");
         LoggerManager.setFile("dbxml", "ut-httpserver.log");
         LoggerManager.setFile("dbstream", "ut-httpserver.log");
         LoggerManager.setFile("dbnet", "ut-httpserver.log");
         //LoggerManager.setFileVerbosity("dbcommon", Logger.ERROR_VERBOSITY);
         
         int port = 9999;
         
         // create the http web server (proxy port version)
         HttpProxyPortWebServer server = new HttpProxyPortWebServer();
         
         // create http web request servicers
         HttpGetRequestServicer getRequestServicer =
            new HttpGetRequestServicer("test/data");
         getRequestServicer.setPathPermissions("/", "r", false);
         server.addNonSecureHttpWebRequestServicer(
               getRequestServicer, "/data/");
         
         // create http web request servicer
         HttpWebRequestServicer hwrs = new HttpWebRequestServicer()
         {
            public void serviceHttpWebRequest(
               HttpWebRequest request, HttpWebResponse response)
            {
               // if the request accepts gzip, set the content-encoding to gzip
               if(request.getHeader().getAcceptEncoding() != null &&
                  request.getHeader().getAcceptEncoding().contains("gzip"))
               {
                  response.getHeader().setContentEncoding("gzip");
                  response.getHeader().setContentLength(-1);
               }
               
               if(request.getHeader().getMethod().equals("POST"))
               {
                  String body = request.receiveBodyString();
                  System.out.println("RECEIVED POST BODY:\n" + body);
                  
                  response.sendMethodNotAllowedResponse();
               }
               else
               {
                  response.sendMethodNotAllowedResponse();
               }
            }
            
            public void setHttpWebRequestServicerPath(String path)
            {
               // do nothing
            }
         };
         
         // add http web request servicer
         server.addSecureHttpWebRequestServicer(hwrs, "/");
         
         // start the server
         server.setMaximumNonSecureConnections(0);
         server.setMaximumSecureConnections(0);
         server.start(port, port + 1, port + 2);
         
         System.out.println("Server started, running on port " + port);

         // send requests
         sendRequests(port, 30);
         
         // while running, sleep
         while(true)
         {
            try
            {
               Thread.sleep(10);
            }
            catch(Throwable t)
            {
               // do nothing
            }
         }
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
   
   /**
    * Sends a single request to the server.
    * 
    * @param port the port to connect to.
    */
   public static void sendRequest(int port)
   {
      System.out.println("Sending request on port " + port);
      
      // connect and send http request
      try
      {
         // get a web connection
         HttpWebClient client = new HttpWebClient();
         HttpWebConnection hwc = (HttpWebConnection)client.connect(
            "https://localhost:" + port);
         
         String body = "test body";
         
         // create a request
         HttpWebRequest request = new HttpWebRequest(hwc);
         request.getHeader().usePostMethod();
         request.getHeader().setHost("localhost");
         request.getHeader().setContentLength(body.length());
         request.getHeader().setConnection("close");
         //request.getHeader().setAccept("gzip");
         
         /*request.getHeader().useGetMethod();
         request.getHeader().setPath("/stuff");
         request.getHeader().setHost("localhost");
         request.getHeader().setContentLength(0);
         request.getHeader().setConnection("close");
         request.getHeader().setAccept("gzip");*/
         
         // send request header and body
         request.sendHeader();
         request.sendBody(body);
         
         System.out.println("POST BODY SENT:\n" + body);
         
         // create response
         HttpWebResponse response = request.createHttpWebResponse();
         
         // receive response
         response.receiveHeader();
         System.out.println("\nresponse header=\n" +
                            response.getHeader().toString());
         System.out.println("response body=\n" + response.receiveBodyString());
         
         // disconnect
         hwc.disconnect();
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
   
   /**
    * Sends multiple requests to the server on separate threads.
    * 
    * @param port the port to connect to.
    * @param count the number of requests.
    */
   public static void sendRequests(int port, int count)
   {
      for(int i = 0; i < count; i++)
      {
         //sendRequest(port);
         Object[] params = new Object[]{new Integer(port)};
         MethodInvoker mi =
            new MethodInvoker(UTHttpServer.class, "sendRequest", params);
         mi.backgroundExecute();
      }
   }
}
