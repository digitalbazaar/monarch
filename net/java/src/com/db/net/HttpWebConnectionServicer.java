/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.util.HashMap;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * An HttpWebConnectionServicer services an HTTP web connection. 
 * 
 * @author Dave Longley
 */
public class HttpWebConnectionServicer implements WebConnectionServicer
{
   /**
    * The default http version for this http web connection servicer.
    */
   public static final String HTTP_VERSION = "HTTP/1.1";
   
   /**
    * Http web request servicers. Maps from path to non-secure http
    * web request servicer.
    */
   protected HashMap mPathToNonSecureServicer;
   
   /**
    * Http web request servicer. Maps from path to secure http
    * web request servicer.
    */
   protected HashMap mPathToSecureServicer;
   
   /**
    * The default server name to use in http web responses.
    */
   protected String mDefaultServerName;
   
   /**
    * Creates a new HttpWebConnectionServicer.
    */
   public HttpWebConnectionServicer()
   {
      this("Digital Bazaar Http Server/1.0");
   }

   /**
    * Creates a new HttpWebConnectionServicer.
    * 
    * @param serverName the name of the server to use as a default for
    *                   http web responses. 
    */
   public HttpWebConnectionServicer(String serverName)
   {
      mDefaultServerName = serverName;
      
      // create servicer maps
      mPathToNonSecureServicer = new HashMap();
      mPathToSecureServicer = new HashMap();
   }
   
   /**
    * Appends and prepends slashes to a path.
    * 
    * @param path the path to ready.
    * 
    * @return the readied path.
    */
   protected String readyPath(String path)
   {
      // make sure to put slashes on beginning and end of path
      if(!path.startsWith("/"))
      {
         path = "/" + path;
      }
      
      if(!path.endsWith("/"))
      {
         path += "/";
      }
      
      return path;
   }
   
   /**
    * Adds an http web request servicer to the passed map.
    * 
    * @param map the map to add the servicer to.
    * @param hwrs the servicer to add.
    * @param path the path to the servicer.
    */
   protected void addHttpWebRequestServicer(
      HashMap map, HttpWebRequestServicer hwrs, String path)
   {
      path = readyPath(path);
      
      // set servicer path
      hwrs.setHttpWebRequestServicerPath(path);
      
      // add to the map
      map.put(path, hwrs);
   }
   
   /**
    * Removes an http web request servicer from the passed map.
    * 
    * @param map the map to remove the servicer from.
    * @param path the path to the servicer.
    */
   protected void removeHttpWebRequestServicer(HashMap map, String path)
   {
      map.remove(path);
   }
   
   /**
    * Gets an http web request servicer from the specified map, according to
    * its path.
    * 
    * @param map the map to get the servicer from.
    * @param path the path to the http web request servicer.
    * 
    * @return the http web request servicer.
    */
   protected HttpWebRequestServicer getHttpWebRequestServicer(
      HashMap map, String path)
   {
      path = readyPath(path);
      
      getLogger().debug("looking for http web request servicer for path: " +
                        path);
      
      HttpWebRequestServicer hwrs = (HttpWebRequestServicer)map.get(path);
      while(hwrs == null && !path.equals(""))
      {
         // get parent path
         int index = path.lastIndexOf('/');
         path = path.substring(0, index);
         if(index > 0)
         {
            index = path.lastIndexOf('/');
            path = path.substring(0, index + 1);
            
            getLogger().debug("looking for http web request servicer " +
                              "for path: " + path);
            
            // try to get servicer again
            hwrs = (HttpWebRequestServicer)map.get(path);
         }
      }
      
      if(hwrs != null)
      {
         getLogger().debug("http web request servicer found (" +
                           hwrs.getClass().getName() + ") for path: " + path);
      }
      else
      {
         getLogger().debug("no http web request servicer found!");
      }
      
      return hwrs;
   }
   
   /**
    * Delegates the passed http web request to the appropriate servicer.
    * 
    * @param request the http web request to handle.
    * @param response the http web response.
    * 
    * @return true if the request was delegated to a servicer, false if it
    *         was not.
    */
   public boolean delegateHttpWebRequest(
      HttpWebRequest request, HttpWebResponse response)
   {
      boolean rval = false;
      
      // get the appropriate servicer
      HttpWebRequestServicer hwrs = null;

      // get the relative path without any variables
      String path = request.getHeader().getRelativePathWithoutVariables();

      // get servicer based on security
      if(request.getWebConnection().isSecure())
      {
         // get the servicer for the given path
         hwrs = getSecureHttpWebRequestServicer(path);
      }
      else
      {
         // get the servicer for the given path
         hwrs = getNonSecureHttpWebRequestServicer(path);
      }

      // service request
      if(hwrs != null)
      {
         hwrs.serviceHttpWebRequest(request, response);
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Services a web connection that is using the HTTP protocol.
    * 
    * The web connection should be serviced and disconnected when the
    * servicing is completed.
    *  
    * @param webConnection the web connection to be serviced.
    */
   public void serviceWebConnection(WebConnection webConnection)
   {
      // service http web connection
      HttpWebConnection hwc = new HttpWebConnection(webConnection);
      
      // create an http web request from the web connection
      HttpWebRequest request = new HttpWebRequest(hwc);
      
      // create an http web response from the web connection
      HttpWebResponse response = request.createHttpWebResponse();
      
      // set default server header in response
      response.getHeader().addHeader("Server", mDefaultServerName);
      
      // default web connection to 30 second timeout
      hwc.setReadTimeout(30000);
      
      // keep receiving requests
      // handle keep-alive
      boolean keepAlive = true;
      while(!Thread.currentThread().isInterrupted() &&
            keepAlive && request.receiveHeader())
      {
         try
         {
            // try to set the response version based on the request version
            String requestVersion = request.getHeader().getVersion(); 
            if(requestVersion.startsWith("HTTP"))
            {
               response.getHeader().setVersion(requestVersion);
            }
            else
            {
               // request version invalid, so use default version
               response.getHeader().setVersion(HTTP_VERSION);
            }

            // set response connection header
            if(request.getHeader().getConnection() != null)
            {
               response.getHeader().setConnection(
                     request.getHeader().getConnection());
            }
            else
            {
               response.getHeader().setConnection("close");
            }
            
            // make sure request is valid
            if(request.isValid())
            {
               // get request keep alive value
               keepAlive = request.getHeader().isKeepAlive();
               
               // set web connection read timeout based on keep alive value
               if(keepAlive)
               {
                  // no timeout
                  hwc.setReadTimeout(0);
               }
               else
               {
                  // 30 second timeout
                  hwc.setReadTimeout(30000);
               }
               
               // try to delegate request to an http web request servicer
               if(!delegateHttpWebRequest(request, response))
               {
                  // send forbidden response
                  response.sendForbiddenResponse();
               }
            }
            else
            {
               // send bad request response
               response.sendBadRequestResponse();
            }
            
            // get response keep-alive value
            keepAlive = response.getHeader().isKeepAlive();
            
            // if keep-alive, create next request and response
            if(keepAlive)
            {
               // no timeout
               hwc.setReadTimeout(0);

               // clear response headers
               response.getHeader().clearHeaders();
               
               // set default server header in response
               response.getHeader().addHeader("Server", mDefaultServerName);
            }
            else
            {
               // 30 second timeout
               hwc.setReadTimeout(30000);
            }
         }
         catch(Throwable t)
         {
            getLogger().error("Exception thrown while servicing " +
                              "http web request!");
            getLogger().debug(Logger.getStackTrace(t));
            
            // invalid request, do not keep connection alive
            keepAlive = false;
            
            // send server error response
            response.sendServerErrorResponse();
         }
      }
   }
   
   /**
    * Adds a non-secure http web request servicer to this servicer. If the path
    * specified does not begin and end with slashes ("/"), they will be
    * appended.
    * 
    * The path of the servicer will be set.
    * 
    * @param hwrs the http web request servicer to add.
    * @param path the path to the servicer.
    */
   public void addNonSecureHttpWebRequestServicer(
      HttpWebRequestServicer hwrs, String path)
   {
      getLogger().debug("adding non-secure http web request servicer: " +
                        path + "->" + hwrs.getClass().getName());

      addHttpWebRequestServicer(mPathToNonSecureServicer, hwrs, path);
   }
   
   /**
    * Adds a secure http web request servicer to this servicer. If the path
    * specified does not begin and end with slashes ("/"), they will be
    * appended.
    * 
    * The path of the servicer will be set.
    * 
    * @param hwrs the http request servicer to add.
    * @param path the path to the servicer.
    */
   public void addSecureHttpWebRequestServicer(
      HttpWebRequestServicer hwrs, String path)
   {
      getLogger().debug("adding secure http web request servicer: " +
                        path + "->" + hwrs.getClass().getName());

      addHttpWebRequestServicer(mPathToSecureServicer, hwrs, path);
   }
   
   /**
    * Removes a non-secure http web request servicer from this servicer.
    * 
    * @param path the path to the servicer.
    */
   public void removeNonSecureHttpWebRequestServicer(String path)
   {
      getLogger().debug("removing non-secure http web request servicer: " +
                        path);
      removeHttpWebRequestServicer(mPathToNonSecureServicer, path);
   }
   
   /**
    * Removes a secure http web request servicer from this servicer.
    * 
    * @param path the path to the servicer.
    */
   public void removeSecureHttpWebRequestServicer(String path)
   {
      getLogger().debug("removing secure http web request servicer: " + path);
      removeHttpWebRequestServicer(mPathToSecureServicer, path);
   }
   
   /**
    * Gets a non-secure http web request servicer according to its path.
    * 
    * @param path the path to the http web request servicer.
    * @return the http web request servicer.
    */
   public HttpWebRequestServicer getNonSecureHttpWebRequestServicer(String path)
   {
      return getHttpWebRequestServicer(mPathToNonSecureServicer, path);
   }
   
   /**
    * Gets a secure http web request servicer according to its path.
    * 
    * @param path the path to the http request servicer.
    * @return the http web request servicer.
    */
   public HttpWebRequestServicer getSecureHttpWebRequestServicer(String path)
   {
      return getHttpWebRequestServicer(mPathToSecureServicer, path);
   }
   
   /**
    * Sets the default server name to use in http web responses.
    * 
    * @param serverName the default server name to use in http web responses.
    */
   public void setDefaultServerName(String serverName)
   {
      mDefaultServerName = serverName;
   }
   
   /**
    * Gets the logger.
    * 
    * @return the logger.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbnet");
   }
}
