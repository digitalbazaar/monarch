/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.autoupdater;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.net.URL;
import java.net.URLConnection;
import java.nio.channels.FileChannel;
import java.util.HashMap;
import java.util.Iterator;
import java.util.StringTokenizer;
import java.util.Vector;

import com.db.common.Cryptor;
import com.db.common.ThreadedEventDelegate;
import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * A BasicUpdateScript is one particular implementation of UpdateScript for
 * an AutoUpdater.
 * 
 * This implementation provides basic command functionality for copying/moving
 * files for a software update, etc.
 * 
 * @author Dave Longley
 */
public class BasicUpdateScript implements UpdateScript
{
   /**
    * The script as a string.
    */
   protected String mScript;
   
   /**
    * Stores temporary file names mapped to their appropriate
    * destination files. 
    */
   protected HashMap mTempFiles;
   
   /**
    * The commands that should be executed to perform the update, they are
    * stored in order of execution.
    */
   protected Vector mCommands;
   
   /**
    * The total number of items to install.
    */
   protected int mInstallItemCount;
   
   /**
    * The total size of this update in bytes.
    */
   protected long mUpdateSize;
   
   /**
    * True if processing should be cancelled, false if not.
    */
   protected boolean mCancelProcessing;
   
   /**
    * A BasicUpdateScriptProcessEventDelegate for firing
    * BasicUpdateScriptProcessEvents.
    */
   protected ThreadedEventDelegate
      mBasicUpdateScriptProcessEventDelegate;
   
   /**
    * Creates a new blank BasicUpdateScript.
    */
   public BasicUpdateScript()
   {
      this("");
   }
   
   /**
    * Creates a new BasicUpdateScript from the passed script as a String.
    * 
    * @param script the String from of the script.
    */
   public BasicUpdateScript(String script)
   {
      // store script
      mScript = script;

      // create temp files map
      mTempFiles = new HashMap();
      
      // create vector for storing commands
      mCommands = new Vector();
      
      // default number of items to 0
      mInstallItemCount = 0;
      
      // default update size to 0
      mUpdateSize = 0;
      
      // do not cancel processing by default
      mCancelProcessing = false;
      
      // create delegate
      mBasicUpdateScriptProcessEventDelegate =
         new ThreadedEventDelegate();
   }
   
   /**
    * Fires a BasicUpdateScriptProcessEvent.
    * 
    * @param name the name of the event.
    * @param command the command currently being executed.
    * @param changedFile the changed file, if any.
    * @param fileAction the action performed on the file, if any.
    * @param downloaded the change in bytes downloaded, if any.
    * @param fileProgress the progress on the file (in bytes), if any.
    */
   protected void fireBasicUpdateScriptProcessEvent(
      String name, BasicUpdateScriptCommand command,
      File changedFile, String fileAction, long downloaded, long fileProgress)
   {
      // create event
      BasicUpdateScriptProcessEvent event =
         new BasicUpdateScriptProcessEvent(name, this, command);
      
      if(command != null)
      {
         event.setChangedFile(command.getRelativePath());
         event.setFileAction(fileAction);
         event.setFileBytesDownloaded(downloaded);
         event.setFileProgress(fileProgress);
      }
      
      // fire event
      getBasicUpdateScriptProcessEventDelegate().fireEvent(event);
   }
   
   /**
    * Copies one file over another.
    * 
    * @param src the source file.
    * @param dest the destination file.
    * @return true if successfully copied, false if not.
    */
   protected boolean copyFile(File src, File dest)
   {
      boolean rval = false;
      
      FileChannel in = null;
      FileChannel out = null;
      
      // copy the file in one go
      try
      {
         in = new FileInputStream(src).getChannel();
         out = new FileOutputStream(dest).getChannel();
         
         if(in.transferTo(0, in.size(), out) == in.size())
         {
            rval = true;
         }
      }
      catch(Throwable t)
      {
      }
      finally
      {
         try
         {
            if(in != null)
            {
               in.close();
            }
            
            if(out != null)
            {
               out.close();
            }
         }
         catch(Throwable t)
         {
         }
      }
      
      return rval;
   }
   
   /**
    * Backs up a given file to another file in the same directory.
    * 
    * @param targetFile the source file to backup.
    * @return true if the backup was successful, false otherwise.
    */
   protected boolean backupFile(File targetFile)
   {
      boolean rval = true;
      
      // only perform a backup if the targetFile exists, quietly fail if it
      // doesn't
      if(targetFile.exists() && targetFile.isFile())
      {
         File backupFile = new File(targetFile.getAbsolutePath() + ".backup");
         rval = copyFile(targetFile, backupFile);
      }
      
      return rval;
   }

   /**
    * Performs a install command given the update script command object.
    * 
    * @param command the update script command object.
    * 
    * @return true if the install was successful, false otherwise.
    */
   protected boolean performInstallCommand(BasicUpdateScriptCommand command)
   {
      boolean rval = true;

      File sourceFile = (File)mTempFiles.get(command.getRelativePath());
      File targetFile =
         new File(getWorkingPath() + command.getRelativePath().getPath());

      // fire event
      fireBasicUpdateScriptProcessEvent(
         "fileChanged", command, command.getRelativePath(), "backup", 0, 0);
      
      // backup file
      rval &= backupFile(targetFile);
      
      // fire event
      fireBasicUpdateScriptProcessEvent(
         "fileChanged", command, command.getRelativePath(), "backup", 0, 100);

      // fire event
      fireBasicUpdateScriptProcessEvent(
         "fileChanged", command, command.getRelativePath(), "install", 0, 0);
      
      // copy file
      rval &= copyFile(sourceFile, targetFile);
      
      // fire event
      fireBasicUpdateScriptProcessEvent(
         "fileChanged", command, command.getRelativePath(), "install", 0, 100);
      
      return rval;
   }
   
   /**
    * Performs the delete command given the update script command object.
    * 
    * @param command the update script command object.
    * 
    * @return true if the delete was successful, false otherwise.
    */
   protected boolean performDeleteCommand(BasicUpdateScriptCommand command)
   {
      boolean rval = false;
      
      File deleteFile =
         new File(getWorkingPath() + command.getRelativePath().getPath());
      
      // attempt to delete the file
      if(deleteFile.exists())
      {
         if(deleteFile.canWrite())
         {
            try
            {
               rval = deleteFile.delete();
            }
            catch(SecurityException se)
            {
               getLogger().error(
                  "Permission denied while attempting to delete file: "+
                  deleteFile.getAbsolutePath());
            }
         }
      }
      else
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Performs a directory creation command given the update script command
    * object.
    * 
    * @param command the update script command object.
    * 
    * @return true if the delete was successful, false otherwise.
    */
   protected boolean performMkdirCommand(BasicUpdateScriptCommand command)
   {
      boolean rval = false;
      
      File createDir =
         new File(getWorkingPath() + command.getRelativePath().getPath());
      
      // attempt to create the directory
      if(!createDir.exists())
      {
         try
         {
            rval = createDir.mkdirs();
         }
         catch(SecurityException se)
         {
            getLogger().error(
               "Update script failed to create directory: "+
               createDir.getAbsolutePath());
         }
      }
      else
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Performs a directory deletion command given the update script command
    * object.
    * 
    * @param command the update script command object.
    * 
    * @return true if the delete was successful, false otherwise.
    */
   protected boolean performRmdirCommand(BasicUpdateScriptCommand command)
   {
      boolean rval = false;
      
      File deleteDir =
         new File(getWorkingPath() + command.getRelativePath().getPath());
      
      // attempt to delete the directory
      if(deleteDir.exists())
      {
         if(deleteDir.isDirectory())
         {
            if(deleteDir.canWrite())
            {
               try
               {
                  rval = deleteDir.delete();
               }
               catch(SecurityException se)
               {
                  getLogger().error(
                     "Permission denied while attempting to delete directory: "+
                     deleteDir.getAbsolutePath());
               }
            }
         }
      }
      else
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Downloads a file to a temporary file name. The filename will be
    * stored in the temp map in case it is valid and is to be used. If
    * the temp file is invalid, it will be deleted.
    * 
    * @param command the current command.
    * @return true if the downloaded file is valid, false if not.
    */
   protected boolean downloadFile(BasicUpdateScriptCommand command)
   {
      boolean rval = false;
      File temp = null;
      
      try
      {
         URL url = command.getUrl();
         File destination = command.getRelativePath();
         String md5 = command.getMd5Sum();
         
         // see if the destination can be written to
         if(!destination.exists() || destination.canWrite() &&
            !mCancelProcessing)
         {
            // get a temp file for saving to
            temp = File.createTempFile("update", ".tmp");
            temp.deleteOnExit();

            // get a connection for downloading
            URLConnection c = url.openConnection();
         
            FileOutputStream fos = null;
            BufferedInputStream bis = null;
         
            try
            {
               getLogger().debug("Downloading file=" + command.getUrl());
               
               // create file output stream for writing to temp file
               fos = new FileOutputStream(temp);

               // get input stream for reading from connection
               bis = new BufferedInputStream(c.getInputStream());
               
               // read until the stream is empty
               byte[] buffer = new byte[65536];
               int numBytes = -1;
               int totalBytes = 0;
               while((numBytes = bis.read(buffer)) != -1 && !mCancelProcessing)
               {
                  fos.write(buffer, 0, numBytes);
                  
                  totalBytes += numBytes;

                  // fire event
                  fireBasicUpdateScriptProcessEvent(
                     "fileChanged", command, command.getRelativePath(),
                     "download", numBytes, totalBytes);
               }
               
               // close streams
               bis.close();
               fos.close();
               
               // check the MD5 sum of the file to see if it matches
               String tempMD5 = Cryptor.getMD5ChecksumString(temp);
               if(tempMD5.equals(md5))
               {
                  // file MD5 matches, if file is valid add to map
                  mTempFiles.put(destination, temp);
                  
                  rval = true;
               }
               else
               {
                  // fire event
                  fireBasicUpdateScriptProcessEvent(
                     "fileChanged", command, command.getRelativePath(),
                     "failed", 0, 100);
               }
            }
            catch(Throwable t)
            {
               t.printStackTrace();
               
               // fire event
               fireBasicUpdateScriptProcessEvent(
                  "fileChanged", command, command.getRelativePath(),
                  "failed", 0, 100);
            }
            
            if(bis != null)
            {
               bis.close();
            }

            if(fos != null)
            {
               fos.close();
            }
         }
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
      
      // remove any invalid temp file created
      if(!rval && temp != null)
      {
         temp.delete();
      }
      
      return rval;
   }   
   
   /**
    * Gets the working path, ending with the file separator appropriate for
    * the current system.
    * 
    * @return the working path, ending with the file separator appropriate for
    *         the current system.
    */
   protected String getWorkingPath()
   {
      File userDir = new File(System.getProperty("user.dir"));
      String workingPath = userDir.getAbsolutePath();
      if(!workingPath.endsWith(File.separator))
      {
         workingPath += File.separator;
      }
      
      return workingPath;
   }
   
   /**
    * Gets this script as a string.
    * 
    * @return this script as a string.
    */
   public String toString()
   {
      return mScript;
   }
   
   /**
    * Validates this script.
    * 
    * @return true if the script is valid, false if not.
    */
   public boolean validate()
   {
      boolean rval = true;
      
      // default number of items to 0
      mInstallItemCount = 0;
      
      // default update size to 0
      mUpdateSize = 0;      
      
      // parse the update script
      String script = toString();
      if(script.length() > 0)
      {
         String commands[] = script.split("\n");
         
         // parse each command in the file
         for(int i = 0; i < commands.length; i++)
         {            
            if(commands[i].length() > 4)
            {
               // parse the update script command
               BasicUpdateScriptCommand usc = new BasicUpdateScriptCommand();
               StringTokenizer st = new StringTokenizer(commands[i], " ");
               String command = st.nextToken();
               String arguments = st.nextToken("\n");
          
               // if the command parse is successful, store the command,
               // do not store it otherwise.
               if(usc.parseCommand(command, arguments))
               {
                  mCommands.add(usc);
                  
                  if(usc.getSize() != -1)
                  {
                     // add size to total size
                     mUpdateSize += usc.getSize();
                  }
                  
                  if(usc.getName().equals("install"))
                  {
                     // increment install item count
                     mInstallItemCount++;
                  }
               }
               else
               {
                  rval = false;
               }
            }
         }
      }
      else
      {
         rval = false;
      }
      
      return rval;
   }
   
   /**
    * Processes this update script. Any call to this method should cause
    * cancelled() to return false until cancel() is called.
    * 
    * @return true if the script was processed, false if it was cancelled or
    *         encountered an error.
    */
   public boolean process()
   {
      boolean rval = false;

      // clear temporary files
      mTempFiles.clear();
      
      Iterator i = mCommands.iterator();
         
      // Perform the download for every file that is needed for installation
      boolean noError = true;
      while(i.hasNext() && noError && !mCancelProcessing)
      {
         BasicUpdateScriptCommand command = (BasicUpdateScriptCommand)i.next();
         if(command.getName().equals("install"))
         {
            noError &= downloadFile(command);
         }
      }
      
      // execute each command of the script in order
      i = mCommands.iterator();
      int commandNumber = 0;
      while(i.hasNext() && noError && !mCancelProcessing)
      {
         BasicUpdateScriptCommand command = (BasicUpdateScriptCommand)i.next();
         
         if(command.getName().equals("install"))
         {
            noError &= performInstallCommand(command);
         }
         else if(command.getName().equals("delete"))
         {
            // fire event
            fireBasicUpdateScriptProcessEvent(
               "fileChanged", command,
               command.getRelativePath(),"delete", 0, 0);

            // perform delete
            noError &= performDeleteCommand(command);
            
            // fire event
            fireBasicUpdateScriptProcessEvent(
               "fileChanged", command,
               command.getRelativePath(), "delete", 0, 100);
         }
         else if(command.getName().equals("mkdir"))
         {
            // fire event
            fireBasicUpdateScriptProcessEvent(
               "directoryChanged", command,
               command.getRelativePath(), "create", 0, 0);

            // perform make directory
            noError &= performMkdirCommand(command);
            
            // fire event
            fireBasicUpdateScriptProcessEvent(
               "directoryChanged", command,
               command.getRelativePath(), "create", 0, 100);
         }
         else if(command.getName().equals("rmdir"))
         {
            // fire event
            fireBasicUpdateScriptProcessEvent(
               "directoryChanged", command,
               command.getRelativePath(), "delete", 0, 0);

            // perform remove directory
            noError &= performRmdirCommand(command);
            
            // fire event
            fireBasicUpdateScriptProcessEvent(
               "directoryChanged", command,
               command.getRelativePath(), "delete", 0, 100);
         }

         // fire event
         fireBasicUpdateScriptProcessEvent(
            "commandProcessed", command, null, null, 0, 0);

         // increment command number
         commandNumber++;
      }
      
      if(noError && !mCancelProcessing)
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Cancels processing this update script. Any call to this method should
    * cause cancelled() to return true.
    */
   public void cancel()
   {
      mCancelProcessing = true;
   }
   
   /**
    * Returns true if this script was cancelled, false if it was not. This
    * method should return false unless process() has been called followed
    * by a call to cancel(). Any subsequent call to process() should cause
    * this method to return true until cancel() is called.
    * 
    * 
    * @return true if this script has been cancelled since the last call
    *         to process().
    */
   public boolean cancelled()
   {
      return mCancelProcessing;
   }
   
   /**
    * Reverts changes made by this script, if possible.
    * 
    * @return true if the revert was successful, false if not and the
    *         installation is now in an indetermine state.
    */
   public boolean revert()
   {
      boolean rval = false;
      
      // move the old files back, installation failed
      boolean isValid = true;
      Iterator i = mTempFiles.values().iterator();
      while(i.hasNext())
      {
         File dest = (File)i.next();
         
         String oldPath = dest.getPath();
         if(oldPath.endsWith(".old"))
         {
            oldPath = oldPath.substring(0, oldPath.length() - 4);
            isValid &= dest.renameTo(new File(oldPath));
         }
      }
      
      return (rval == isValid);
   }
   
   /**
    * Cleans up an installation after this script has been processed.
    * 
    * @return true if cleaned up, false if not.
    */
   public boolean cleanup()
   {
      boolean rval = false;
      
      // delete all the old files
      boolean isValid = true;
      Iterator i = mTempFiles.values().iterator();
      while(i.hasNext())
      {
         File dest = (File)i.next();
         rval &= dest.delete();
      }

      mTempFiles.clear();
      
      return (rval == isValid);
   }
   
   /**
    * Returns true if the AutoUpdater that processed this script requires
    * a reload, false if not.
    * 
    * @return true if the AutoUpdater that processed this script requires
    *         a reload, false if not.
    */
   public boolean autoUpdaterRequiresReload()
   {
      boolean rval = false;
      
      // reload auto-updater if a shutdown is required
      if(getExitCommand() != null && getExitCommand().equals("shutdown"))
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Gets the command to execute when the script exits.
    * 
    * @return the command to execute when the script exits.
    */
   public String getExitCommand()
   {
      String rval = null;
      
      if(mCommands.size() > 0)
      {
         BasicUpdateScriptCommand lastCommand = 
            (BasicUpdateScriptCommand)mCommands.get(mCommands.size() - 1);
      
         rval = lastCommand.getOptionalArgument();
      }
      
      return rval;
   }
   
   /**
    * Gets the update size (in bytes).
    * 
    * @return the update size (in bytes).
    */
   public long getUpdateSize()
   {
      return mUpdateSize;
   }
   
   /**
    * Gets the number of install items.
    * 
    * @return the number of install items.
    */
   public int getInstallItemCount()
   {
      return mInstallItemCount;
   }
   
   /**
    * Gets the BasicUpdateScriptProcessDelegate.
    * 
    * @return the BasicUpdateScriptProcessDelegate.
    */
   public ThreadedEventDelegate getBasicUpdateScriptProcessEventDelegate()
   {
      return mBasicUpdateScriptProcessEventDelegate;
   }
   
   /**
    * Gets the logger for this BasicUpdateScript.
    * 
    * @return the logger for this BasicUpdateScript.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbautoupdater");
   }
}
