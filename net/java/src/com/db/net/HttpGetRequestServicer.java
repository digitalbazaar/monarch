/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.io.File;

/**
 * An HttpGetRequestServicer services http web requests that use the
 * http GET method. A requested file will only be returned if its
 * path has been marked as read-permitted by this servicer.
 * 
 * @author Dave Longley
 */
public class HttpGetRequestServicer
extends AbstractHttpWebRequestServicer
{
   /**
    * The base path, on the local file system, for all files this file
    * request servicer is allowed to send. 
    */
   protected String mBaseLocalPath;

   /**
    * Creates a new HttpGetRequestServicer.
    * 
    * @param localPath the base path, on the local file system, for all
    *                  files this servicer could be permitted to send.
    */
   public HttpGetRequestServicer(String localPath)
   {
      setBaseLocalPath(localPath);
   }
   
   /**
    * Translates the request path into a local path.
    * 
    * @param requestPath the request path.
    * @return the local path.
    */
   protected String requestPathToLocalPath(String requestPath)
   {
      String localPath = requestPath;
      
      // see if servicer path is the same as the base local path
      if(!getHttpWebRequestServicerPath().equals(getBaseLocalPath()))
      {
         // replace the servicer path with the base local path
         String servicerPath = getHttpWebRequestServicerPath();
         int length = servicerPath.length();
         int index = localPath.indexOf(servicerPath);
         if(index != -1 && localPath.length() > (index + length))
         {
            localPath = getBaseLocalPath() +
                        localPath.substring(index + length);
         }
         else
         {
            localPath = getBaseLocalPath();
         }
      }
      
      File file = new File(localPath);
      getLogger().debug(getClass(),
         "file request local path: " + file.getAbsolutePath());
      
      return localPath;
   }

   /**
    * Determines content type based on file extension.
    * 
    * @param ext the file extension.
    * @return the suggested content type.
    */
   public static String determineContentType(String ext)
   {
      String contentType = "unknown/unknown";
      
      if(ext.equals("html") || ext.equals("htm"))
      {
         contentType = "text/html";
      }
      else if(ext.equals("txt"))
      {
         contentType = "text/plain";
      }
      else if(ext.equals("xml"))
      {
         contentType = "text/xml";
      }
      else if(ext.equals("mp3"))
      {
         contentType = "audio/mp3";
      }
      else if(ext.equals("avi"))
      {
         contentType = "video/avi";
      }
      else
      {
         contentType = "unknown/" + ext;
      }
      
      return contentType;
   }
   
   /**
    * Services an http web request with a GET method and responds using
    * the passed HttpWebResponse object.
    * 
    * This servicer responds to a GET request with a file if it is
    * found. If not found, it responds witha a 404 Not Found response. If
    * the request is not GET, or the path is not readable, it responds
    * with a 403 Forbidden response.
    * 
    * @param request the http web request.
    * @param response the http web response.
    */
   public void serviceHttpWebRequest(
      HttpWebRequest request, HttpWebResponse response)   
   {
      // for the file to be send
      File file = null;
      
      // must be a GET request and the path must be readable
      if(request.getHeader().getMethod().equals("GET"))
      {
         // get the request path
         String requestPath =
            request.getHeader().getRelativePathWithoutVariables();

         if(isReadable(requestPath))
         {
            // get the file
            String localPath = requestPathToLocalPath(requestPath);
            file = new File(localPath);
            
            if(file.exists() && file.isFile())
            {
               // file exists, request OK
               response.getHeader().useOKStatusCode();
               response.getHeader().setConnection("close");
               
               // get the file extension
               String ext = "";
               
               int index = localPath.lastIndexOf(".");
               if(index != -1 && localPath.length() > (index + 1))
               {
                  ext = localPath.substring(index + 1);
               }
               
               // set the content type
               String contentType = determineContentType(ext);
               response.getHeader().setContentType(contentType);
               
               // set the content length
               response.getHeader().setContentLength(file.length());
               
               // include content disposition if not html
               if(!contentType.equals("text/html"))
               {
                  // set content disposition
                  response.getHeader().setContentDisposition(
                        "attachment; filename=" + file.getName());
               }
               
               // send response
               if(response.sendHeader())
               {
                  sendFile(response, file);
               }
            }
            else
            {
               // file not found
               getLogger().debug(getClass(),
                  "file not found, filename=" + file.getAbsolutePath());
               file = null;
               
               // send not found response
               response.sendNotFoundResponse();
            }
         }
         else
         {
            // file is read-protected, send forbidden response
            response.sendForbiddenResponse();
         }
      }
      else
      {
         // not a GET method, send a method not allowed response
         response.sendMethodNotAllowedResponse();
      }
   }
   
   /**
    * Sets the base path, on the local file system, for all files this
    * servicer is allowed to send.
    * 
    * @param path the base path.
    */
   public void setBaseLocalPath(String path)
   {
      // convert all "\\" to "/"
      path = path.replaceAll("\\\\", "/");
      
      // make sure base path ends with a slash, if it is not blank
      if(!path.equals("") && !path.endsWith("/"))
      {
         path = path + "/";
      }
      
      mBaseLocalPath = path;
      
      File file = new File(mBaseLocalPath);
      
      getLogger().debug(getClass(),
         "base local path=" + file.getAbsolutePath());
   }
   
   /**
    * Gets the base path, on the local file system, for all files this
    * servicer is allowed to send.
    * 
    * @return the base path.
    */
   public String getBaseLocalPath()
   {
      return mBaseLocalPath;
   }
}
