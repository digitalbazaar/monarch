/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.autoupdater;

import java.io.File;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.StringTokenizer;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * A basic UpdateScriptCommand is an atomic BasicUpdateScript command.
 * 
 * This class can be used to parse and verify that a particular command
 * in a BasicUpdateScript is valid.
 * 
 * @author Manu Sporny
 * @author Dave Longley
 */
public class BasicUpdateScriptCommand
{
   /**
    * The name of the command.
    */
   protected String mName;
      
   /**
    * The optional argument if it exists.
    */
   protected String mOptionalArgument;
      
   /**
    * The URL associated with the command.
    */
   protected URL mUrl;
      
   /**
    * The MD5 sum associated with the command.
    */
   protected String mMd5Sum;
   
   /**
    * The size associated with the command.
    */
   protected int mSize;
   
   /**
    * The relative file path associated with the command.
    */
   protected File mRelativePath;
   
   /**
    * A floating point number denoting the version number of the
    * software.
    */
   protected double mVersion;

   /**
    * Creates a BasicUpdateScriptCommand.
    */
   public BasicUpdateScriptCommand()
   {
      mName = null;
      mOptionalArgument = null;
      mUrl = null;
      mMd5Sum = null;
      mSize = -1;
      mRelativePath = null;
      mVersion = -1.0;
   }
   
   /**
    * Parses a command and it's arguments. Returns true if the parse
    * was successful, false otherwise. The BNF for the possible commands
    * is as follows:
    * <pre>
    * FILE := version VARGS
    *         (install IARGS | delete DARGS | mkdir DARGS | rmdir DARGS)*
    *         on_success OARGS
    * 
    * LINE := COMMAND
    * 
    * COMMAND := (version VARGS | install IARGS | delete DARGS | 
    *             mkdir DARGS | rmdir DARGS | on_success OARGS)
    * 
    * N := DECIMAL_NUMBER
    * VERSION_NUMBER := (N.N|N.N.N)
    * VARGS := VERSION_NUMBER
    * 
    * URL := FILE_OR_HTTP_URL_STRING
    * SIZE := N
    * MD5SUM := STRING
    * RELATIVE_PATH := RELATIVE_FILE_PATH
    * IARGS := URL SIZE MD5SUM RELATIVE_PATH
    * 
    * DARGS := RELATIVE_PATH
    * 
    * OARGS := (restart|shutdown|manually_download)
    * </pre>
    * 
    * VARGS are version arguments.
    * DARGS are directory arguments.
    * OARGS are optional arguments.
    * 
    * @param command the name of the command.
    * @param arguments the argument list.
    * 
    * @return true if the parse was successful, false otherwise.
    */
   public boolean parseCommand(String command, String arguments)
   {
      boolean success = false;
      StringTokenizer st = new StringTokenizer(arguments, " ");
      
      // set the command name
      mName = command;
      
      if(command.equals("version"))
      {
         String versionNumber = st.nextToken();
         
         // try and parse the version number in the script file and convert
         // it into a floating point number that can be easily compared.
         try
         {
            String fpVersion = "";
            versionNumber = versionNumber.replace(".", "");
            fpVersion = versionNumber.substring(0, 1) + "." + 
                        versionNumber.substring(1);
            mVersion = Float.parseFloat(fpVersion);
            
            success = true;
         }
         catch(NumberFormatException nfe)
         {
            getLogger().error(
               "Version number in update script is invalid: " + versionNumber);
         }
      }
      else if(command.equals("mkdir") || command.equals("rmdir") ||
              command.equals("delete"))
      {
         String relativePath = st.nextToken();
         
         // check to see if the path that is passed in has a length
         if(relativePath.length() > 0)
         {
            mRelativePath = new File(relativePath);
            success = true;
         }
      }
      else if(command.equals("install"))
      {
         success = true;
         String url = st.nextToken();
         String size = st.nextToken();
         String md5sum = st.nextToken();
         String relativePath = st.nextToken();
         
         if(url.length() > 0)
         {
            // parse the source URL for the install command
            try
            {
               mUrl = new URL(url);
            }
            catch(MalformedURLException mue)
            {
               getLogger().error(
                  "Update script install URL is invalid: " + url);
               success &= false;
            }
         }
         
         if(size.length() > 0)
         {
            // get the size of the file associated with the install command
            try
            {
               mSize = Integer.parseInt(size);
            }
            catch(NumberFormatException nfe)
            {
               getLogger().error(
                  "Update script install URL is invalid: " + url);
               success &= false;
            }
         }
         
         if(md5sum.length() > 0)
         {
            // the MD5 sum string
            mMd5Sum = md5sum;
         }

         // check to see if the path that is passed in has a length
         if(relativePath.length() > 0)
         {
            mRelativePath = new File(relativePath);
         }
      }
      else if(command.equals("on_success"))
      {
         String argument = st.nextToken();
         
         if(argument.equals("restart") || argument.equals("exit") ||
            argument.equals("manually_download"))
         {
            mOptionalArgument = argument;
            success = true;
         }
         else
         {
            getLogger().error(
               "Update script on_success command is invalid: " + argument);
            success &= false;
         }
      }
      
      return success;
   }
   
   /**
    * Gets the name of the command.
    * 
    * @return the name of the command.
    */
   public String getName()
   {
      return mName;
   }
   
   /**
    * Gets the optional argument if it exists.
    * 
    * @return the optional argument if it exists.
    */
   public String getOptionalArgument()
   {
      return mOptionalArgument;
   }
   
   /**
    * Gets the URL associated with the command.
    * 
    * @return the URL associated with the command, null if the command does
    *         not exist.
    */
   public URL getUrl()
   {
      return mUrl;
   }
   
   /**
    * Gets the MD5 sum associated with the command.
    * 
    * @return the MD5 sum associated with the command if it exists, null is
    *         returned if no MD5 sum is associated with it.
    */
   public String getMd5Sum()
   {
      return mMd5Sum;
   }
   
   /**
    * Gets the size associated with the command.
    * 
    * @return the size of the file associated with the command, -1 if 
    *         there is no size associated with the command.
    */
   public int getSize()
   {
      return mSize;
   }
   
   /**
    * Gets the relative file path associated with the command.
    * 
    * @return the relative file path associated with the command, null if
    *         there is no file path associated with the command.
    */
   public File getRelativePath()
   {
      return mRelativePath;
   }
   
   /**
    * Gets the version number associated with the command.
    * 
    * @return a double precision number denoting the software version.
    */
   public double getVersion()
   {
      return mVersion;
   }
   
   /**
    * Gets the logger for this BasicUpdateScriptCommand.
    * 
    * @return the logger for this BasicUpdateScriptCommand.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbautoupdater");
   }
}
