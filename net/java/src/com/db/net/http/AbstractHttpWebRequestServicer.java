/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.http;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.util.HashMap;

/**
 * An abstract http web request servicer. Contains convenience methods for
 * setting relative path permissions, sending and receiving files, and
 * storing the servicer's base path. 
 * 
 * @author Dave Longley
 */
public abstract class AbstractHttpWebRequestServicer
implements HttpWebRequestServicer
{
   /**
    * The servicer's path. This is the path http clients must hit to
    * get a response from this servicer.
    */
   protected String mServicerPath;
   
   /**
    * A map of relative paths to their permissions.
    */
   protected HashMap<String, String> mPathToPermissions;
   
   /**
    * Creates an abstract http servicer.
    */
   public AbstractHttpWebRequestServicer()
   {
      setHttpWebRequestServicerPath("/");
      mPathToPermissions = new HashMap<String, String>();
   }
   
   /**
    * Reads a file from an http web request and saves it to disk.
    * 
    * @param request the http web request to read the file from.
    * @param filename the name for the file.
    * 
    * @return true if successfully read, false if not.
    */
   protected boolean readFile(HttpWebRequest request, String filename)   
   {
      return readFile(request, new File(filename));
   }
   
   /**
    * Reads a file from an http web request and saves it to disk.
    * 
    * @param request the http web request to read the file from.
    * @param file the file to write to.
    * 
    * @return true if successfully read, false if not.
    */
   protected boolean readFile(HttpWebRequest request, File file)
   {
      boolean rval = false;
      
      try
      {
         FileOutputStream fos = new FileOutputStream(file);
         
         // read the body into a file
         request.receiveBody(fos);
         
         // close the file
         fos.close();
         
         rval = true;
      }
      catch(Throwable t)
      {
         getLogger().error(getClass(),
            "could not read file from http web request!");
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Reads a file from an http web request body part body and saves it to disk.
    * 
    * @param request the http web request to read the file from.
    * @param header the http body part header.
    * @param filename the name for the file.
    * 
    * @return true if successfully read, false if not.
    */
   protected boolean readFileFromBodyPartBody(
      HttpWebRequest request, HttpBodyPartHeader header, String filename)   
   {
      return readFileFromBodyPartBody(request, header, new File(filename));
   }
   
   /**
    * Reads a file from an http web request body part body and saves it to disk.
    * 
    * @param request the http web request to read the file from.
    * @param header the http body part header.
    * @param file the file to write to.
    * 
    * @return true if successfully read, false if not.
    */
   protected boolean readFileFromBodyPartBody(
      HttpWebRequest request, HttpBodyPartHeader header, File file)
   {
      boolean rval = false;
      
      try
      {
         FileOutputStream fos = new FileOutputStream(file);
         
         // read the body into a file
         request.receiveBodyPartBody(fos, null);
         
         // close the file
         fos.close();
         
         rval = true;
      }
      catch(Throwable t)
      {
         getLogger().error(getClass(),
            "could not read file from http web request body part body!");
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Reads from a file and writes it with the passed http web response.
    * 
    * @param response the http web response to write to.
    * @param filename the name of the file to read from.
    * 
    * @return true if successfully sent, false if not.
    */
   protected boolean sendFile(HttpWebResponse response, String filename)   
   {
      return sendFile(response, new File(filename));
   }
   
   /**
    * Reads from a file and writes it with the passed http web response.
    * 
    * @param response the http web response to write to.
    * @param file the file to read from.
    * 
    * @return true if successfully sent, false if not.
    */
   protected boolean sendFile(HttpWebResponse response, File file)
   {
      boolean rval = false;
      
      try
      {
         FileInputStream fis = new FileInputStream(file);

         // send the file data via the response
         response.sendBody(fis);
         
         // close the file
         fis.close();
         
         rval = true;
      }
      catch(Throwable t)
      {
         getLogger().error(getClass(),
            "could not send file in http web response!");
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }      
      
      return rval;
   }
   
   /**
    * Reads from a file and writes it with the passed http web response in
    * a body part body.
    * 
    * @param response the http web response to write to.
    * @param bodyPartHeader the http body part header to use.
    * @param filename the name of the file to read from.
    * @param lastBodyPart true if the file is the last body part, false if not.
    * 
    * @return true if successfully sent, false if not.
    */
   protected boolean sendFileInBodyPartBody(
      HttpWebResponse response, HttpBodyPartHeader bodyPartHeader,
      String filename, boolean lastBodyPart)   
   {
      return sendFileInBodyPartBody(response, bodyPartHeader,
         new File(filename), lastBodyPart);
   }
   
   /**
    * Reads from a file and writes it with the passed http web response.
    * 
    * @param response the http web response to write to.
    * @param bodyPartHeader the http body part header to use.
    * @param file the file to read from.
    * @param lastBodyPart true if the file is the last body part, false if not.
    * 
    * @return true if successfully sent, false if not.
    */
   protected boolean sendFileInBodyPartBody(
      HttpWebResponse response, HttpBodyPartHeader bodyPartHeader,
      File file, boolean lastBodyPart)
   {
      boolean rval = false;
      
      try
      {
         // send body part header
         if(response.sendBodyPartHeader(bodyPartHeader))
         {
            FileInputStream fis = new FileInputStream(file);

            // send the file data in a body part body
            response.sendBodyPartBody(fis, bodyPartHeader, lastBodyPart);
         
            // close the file
            fis.close();
         
            rval = true;
         }
      }
      catch(Throwable t)
      {
         getLogger().error(getClass(),
            "could not send file in http web response body part body!");
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }      
      
      return rval;
   }
   
   /**
    * Gets a path that's relative to this servicer's path from
    * a request path.
    * 
    * @param path the request path.
    * 
    * @return the path relative to this servicer. 
    */
   protected String getServicerRelativePath(String path)
   {
      // convert all "\\" to "/"
      path = path.replaceAll("\\\\", "/");
      
      getLogger().debug(getClass(),
         "http web request servicer path is:\n'" +
         getHttpWebRequestServicerPath() + "'");
      
      // strip off servicer path
      int index = path.indexOf(getHttpWebRequestServicerPath());
      if(index == 0)
      {
         int length = index + getHttpWebRequestServicerPath().length();
         if(path.length() > length)
         {
            path = path.substring(length);
         }
         else
         {
            path = "";
         }
      }
      
      // get path up to last slash
      index = path.lastIndexOf("/");
      if(index != -1)
      {
         path = path.substring(0, index + 1);
      }
      else
      {
         path = "";
      }
      
      return path;
   }
   
   /**
    * Determines acceptable http versions for an http web request.
    *
    * @param request the http web request to check.
    * 
    * @return true if the passed request has an acceptable version
    *         false if not.
    */
   protected boolean versionSupported(HttpWebRequest request)
   {
      boolean rval = false;
      
      // supports only http/1.0, http/1.1 by default
      if(request.getHeader().getVersion().equals("HTTP/1.0") ||
         request.getHeader().getVersion().equals("HTTP/1.1"))
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Services an http web request (via whatever means is appropriate) and
    * responds using the passed HttpWebResponse object.
    * 
    * @param request the http web request.
    * @param response the http web response.
    */
   public abstract void serviceHttpWebRequest(
      HttpWebRequest request, HttpWebResponse response);
   
   /**
    * Allows an http web request path to be set for this servicer. This is the
    * base path http clients must request for this servicer to be given the
    * http request to service. A servicer may need to know this information.
    * 
    * @param path the servicer's path.
    */
   public void setHttpWebRequestServicerPath(String path)
   {
      mServicerPath = path;
   }

   /**
    * Gets the http web request servicer's path. This is the path http clients
    * must hit to get a response from this servicer.
    * 
    * @return the servicer's path.
    */
   public String getHttpWebRequestServicerPath()
   {
      return mServicerPath;
   }
   
   /**
    * Sets the path permissions for a path.
    * 
    * @param path the path, relative to the servicer path, to set the
    *             permissions for.
    * @param permissions the permissions to set (i.e.: "rw" for read-write).
    * @param recursive true if subpaths should receive the same permissions,
    *                  false if not.
    */
   public void setPathPermissions(
      String path, String permissions, boolean recursive)
   {
      // add a "c" for recursive, if appropriate
      if(recursive)
      {
         permissions += "c";
      }
      
      // convert all "\\" to "/"
      path = path.replaceAll("\\\\", "/");
      
      // make sure path doesn't begin with a slash
      if(path.startsWith("/"))
      {
         if(path.length() > 1)
         {
            path = path.substring(1);
         }
         else
         {
            path = "";
         }
      }
      
      // make sure the path ends with a slash if not blank
      if(!path.equals("") && !path.endsWith("/"))
      {
         path = path + "/";
      }
      
      mPathToPermissions.put(path, permissions);
   }
   
   /**
    * Gets the permissions for the passed path.
    * 
    * @param path the path to get permissions for.
    * 
    * @return the permissions for the passed path.
    */
   public String getPathPermissions(String path)
   {
      String permissions = "";
      
      // makes sure the path is relative to the servicer path
      path = getServicerRelativePath(path);
      
      getLogger().debug(getClass(), 
         "get permissions for servicer relative path:\n'" + path + "'");
      
      // see if the exact path is in the map
      String value = mPathToPermissions.get(path);
      if(value != null)
      {
         // found permissions
         permissions = value;
         getLogger().debug(getClass(),
            "permissions found:\n'" + permissions + "'");
      }
      else
      {
         getLogger().debug(getClass(),
            "checking for recursive path permissions...");
         
         // get the path components of the path
         String[] pathComps = path.split("/");
         
         for(String nextPath: mPathToPermissions.keySet())
         {
            boolean recursive = hasRecursivePermissions(nextPath);
            
            String gluedComps = "";
            for(int n = 0; n < pathComps.length &&
                gluedComps.length() < nextPath.length(); n++)
            {
               // if the glued components equal nextPath and the path
               // is recursive, then get permissions
               if(gluedComps.equals(nextPath) && recursive)
               {
                  permissions = mPathToPermissions.get(nextPath);
                  getLogger().debug(getClass(),
                     "permissions found:\n'" + permissions + "'");
                  break;
               }
               
               // glue another component on
               gluedComps += pathComps[n] + "/";
            }
         }
      }
      
      return permissions;
   }
   
   /**
    * Returns true if the passed path is readable, false if not.
    *  
    * @param path the path to inspect.
    * 
    * @return true if the path is readable, false if not.
    */
   public boolean isReadable(String path)
   {
      boolean rval = false;
      
      String permissions = getPathPermissions(path);
      if(permissions != null)
      {
         rval = (permissions.indexOf("r") != -1);
      }
      
      return rval;
   }
   
   /**
    * Returns true if the passed path is writable, false if not.
    *  
    * @param path the path to inspect.
    * 
    * @return true if the path is writable, false if not.
    */
   public boolean isWritable(String path)
   {
      boolean rval = false;
      
      String permissions = getPathPermissions(path);
      if(permissions != null)
      {
         rval = (permissions.indexOf("r") != -1);
      }
      
      return rval;
   }
   
   /**
    * Returns true if the passed path has recursive permissions, false if
    * not.
    *
    * @param path the path to inspect.
    * 
    * @return true if path permissions are recursive, false if not.
    */
   public boolean hasRecursivePermissions(String path)
   {
      boolean rval = false;
      
      // convert all "\\" to "/"
      path = path.replaceAll("\\\\", "/");
      
      String permissions = mPathToPermissions.get(path);
      rval = (permissions.indexOf("c") != -1);
      
      return rval;
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
