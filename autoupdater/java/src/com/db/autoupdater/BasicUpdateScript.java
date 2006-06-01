/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.autoupdater;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.nio.channels.FileChannel;
import java.util.HashMap;
import java.util.Iterator;
import java.util.StringTokenizer;
import java.util.Vector;

import com.db.crypto.Cryptor;
import com.db.event.EventDelegate;
import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.util.MethodInvoker;

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
    * The total number of items to download.
    */
   protected int mDownloadItemCount;
   
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
   protected EventDelegate mBasicUpdateScriptProcessEventDelegate;
   
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
      mDownloadItemCount = 0;
      
      // default update size to 0
      mUpdateSize = 0;
      
      // do not cancel processing by default
      mCancelProcessing = false;
      
      // create delegate
      mBasicUpdateScriptProcessEventDelegate = new EventDelegate();
   }
   
   /**
    * Fires a BasicUpdateScriptProcessEvent.
    * 
    * @param name the name of the event.
    * @param command the command currently being executed.
    * @param commandNumber the command number.
    * @param downloadItemNumber the download item number, if any.
    * @param changedFile the changed file, if any.
    * @param fileAction the action performed on the file, if any.
    * @param downloaded the change in bytes downloaded, if any.
    * @param fileProgress the progress on the file (in bytes), if any.
    */
   protected void fireBasicUpdateScriptProcessEvent(
      String name, BasicUpdateScriptCommand command, int commandNumber,
      int downloadItemNumber, File changedFile,
      String fileAction, long downloaded, long fileProgress)
   {
      // create event
      BasicUpdateScriptProcessEvent event =
         new BasicUpdateScriptProcessEvent(name, this, command);
      
      if(command != null)
      {
         event.setCommandNumber(commandNumber);
         event.setDownloadItemNumber(downloadItemNumber);
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
    * @param commandNumber the command number.
    * 
    * @return true if the install was successful, false otherwise.
    */
   protected boolean performInstallCommand(
      BasicUpdateScriptCommand command, int commandNumber)
   {
      boolean rval = true;

      File sourceFile = (File)mTempFiles.get(command.getRelativePath());
      File targetFile =
         new File(getWorkingPath() + command.getRelativePath().getPath());

      // fire event
      fireBasicUpdateScriptProcessEvent(
         "fileChanged", command, commandNumber, 0,
         command.getRelativePath(), "backup", 0, 0);
      
      // backup file
      rval &= backupFile(targetFile);
      
      // fire event
      fireBasicUpdateScriptProcessEvent(
         "fileChanged", command, commandNumber, 0,
         command.getRelativePath(), "backup", 0, 100);

      // fire event
      fireBasicUpdateScriptProcessEvent(
         "fileChanged", command, commandNumber, 0,
         command.getRelativePath(), "install", 0, 0);
      
      // copy file
      rval &= copyFile(sourceFile, targetFile);
      
      // fire event
      fireBasicUpdateScriptProcessEvent(
         "fileChanged", command,  commandNumber, 0,
         command.getRelativePath(), "install", 0, 100);
      
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
    * @param commandNumber the command number.
    * @param downloadItemNumber the download item number.
    * @return true if the downloaded file is valid, false if not.
    */
   protected boolean downloadFile(
      BasicUpdateScriptCommand command, int commandNumber,
      int downloadItemNumber)
   {
      boolean rval = false;
      File temp = null;
      
      try
      {
         File destination = command.getRelativePath();
         String md5 = command.getMd5Sum();
         
         // see if the destination can be written to
         if(!destination.exists() || destination.canWrite() &&
            !mCancelProcessing)
         {
            // get a temp file for saving to
            temp = File.createTempFile("update", ".tmp");
            temp.deleteOnExit();
            
            try
            {
               // create file downloader to download file
               FileDownloader downloader = new FileDownloader();
               
               // background execute file downloader
               Object[] params = new Object[]{command.getUrl(), temp};
               MethodInvoker mi =
                  new MethodInvoker(downloader, "downloadFile", params);
               mi.backgroundExecute();
               
               long byteChange = 0;
               long totalBytes = 0;
               long interval = 1;
               while(!downloader.isDownloadingComplete() &&
                     !mi.isInterrupted() && !mCancelProcessing)
               {
                  byteChange = downloader.getBytesDownloaded() - totalBytes;
                  totalBytes = totalBytes + byteChange;
                  
                  // fire file changed event
                  fireBasicUpdateScriptProcessEvent(
                     "fileChanged", command, commandNumber,
                     downloadItemNumber, command.getRelativePath(),
                     "download", byteChange, totalBytes);

                  // join the downloader thread
                  mi.join(interval);
               }
               
               if(mCancelProcessing)
               {
                  mi.interrupt();
                  mi.join();
               }
               else
               {
                  // send final file changed message
                  byteChange = downloader.getBytesDownloaded() - totalBytes;
                  totalBytes = totalBytes + byteChange;
                  
                  // fire file changed event
                  fireBasicUpdateScriptProcessEvent(
                     "fileChanged", command,  commandNumber,
                     downloadItemNumber, command.getRelativePath(),
                     "download", byteChange, totalBytes);
               
                  // check the MD5 sum of the file to see if it matches
                  String tempMD5 = Cryptor.getMD5ChecksumString(temp);
                  if(tempMD5.equals(md5))
                  {
                     // file MD5 matches, if file is valid add to map
                     mTempFiles.put(destination, temp);
                        
                     // fire event
                     fireBasicUpdateScriptProcessEvent(
                        "fileDownloaded", command,  commandNumber,
                        downloadItemNumber, command.getRelativePath(),
                        "completed", 0, 100);
                        
                     rval = true;
                  }
                  else
                  {
                     // fire event
                     fireBasicUpdateScriptProcessEvent(
                        "fileChanged", command, commandNumber,
                        downloadItemNumber, command.getRelativePath(),
                        "failed", 0, 100);
                  }
               }
            }
            catch(Throwable t)
            {
               getLogger().debug(Logger.getStackTrace(t));
               
               // fire event
               fireBasicUpdateScriptProcessEvent(
                  "fileChanged", command, commandNumber,
                  downloadItemNumber, command.getRelativePath(),
                  "failed", 0, 100);
            }
         }
      }
      catch(Throwable t)
      {
         getLogger().debug(Logger.getStackTrace(t));
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
   public synchronized boolean validate()
   {
      boolean rval = true;
      
      // default number of items to 0
      mDownloadItemCount = 0;
      
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
                     mDownloadItemCount++;
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
   public synchronized boolean process()
   {
      boolean rval = false;

      // clear temporary files
      mTempFiles.clear();
      
      Iterator i = mCommands.iterator();
         
      // Perform the download for every file that is needed for installation
      boolean noError = true;
      int commandNumber = 1;
      int downloadItemNumber = 1;
      while(i.hasNext() && noError && !mCancelProcessing)
      {
         BasicUpdateScriptCommand command = (BasicUpdateScriptCommand)i.next();
         if(command.getName().equals("install"))
         {
            // fire event
            fireBasicUpdateScriptProcessEvent(
               "downloadingFile", command, commandNumber, downloadItemNumber, 
               null, null, 0, 0);
            
            noError &= downloadFile(command, commandNumber, downloadItemNumber);
            downloadItemNumber++;
         }
         
         commandNumber++;
      }
      
      // execute each command of the script in order
      i = mCommands.iterator();
      commandNumber = 1;
      while(i.hasNext() && noError && !mCancelProcessing)
      {
         BasicUpdateScriptCommand command = (BasicUpdateScriptCommand)i.next();
         
         if(command.getName().equals("install"))
         {
            noError &= performInstallCommand(command, commandNumber);
         }
         else if(command.getName().equals("delete"))
         {
            // fire event
            fireBasicUpdateScriptProcessEvent(
               "fileChanged", command, commandNumber, 0,
               command.getRelativePath(), "delete", 0, 0);

            // perform delete
            noError &= performDeleteCommand(command);
            
            // fire event
            fireBasicUpdateScriptProcessEvent(
               "fileChanged", command, commandNumber, 0,
               command.getRelativePath(), "delete", 0, 100);
         }
         else if(command.getName().equals("mkdir"))
         {
            // fire event
            fireBasicUpdateScriptProcessEvent(
               "directoryChanged", command, commandNumber, 0,
               command.getRelativePath(), "create", 0, 0);

            // perform make directory
            noError &= performMkdirCommand(command);
            
            // fire event
            fireBasicUpdateScriptProcessEvent(
               "directoryChanged", command, commandNumber, 0,
               command.getRelativePath(), "create", 0, 100);
         }
         else if(command.getName().equals("rmdir"))
         {
            // fire event
            fireBasicUpdateScriptProcessEvent(
               "directoryChanged", command, commandNumber, 0,
               command.getRelativePath(), "delete", 0, 0);

            // perform remove directory
            noError &= performRmdirCommand(command);
            
            // fire event
            fireBasicUpdateScriptProcessEvent(
               "directoryChanged", command, commandNumber, 0,
               command.getRelativePath(), "delete", 0, 100);
         }

         // fire event
         fireBasicUpdateScriptProcessEvent(
            "commandProcessed", command, commandNumber, 0, null, null, 0, 0);

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
   public synchronized boolean revert()
   {
      boolean rval = false;
      
      // move the old files back, installation failed
      boolean isValid = true;
      Iterator i = mTempFiles.values().iterator();
      while(i.hasNext())
      {
         File dest = (File)i.next();
         
         String oldPath = dest.getPath();
         if(oldPath.endsWith(".backup"))
         {
            oldPath = oldPath.substring(0, oldPath.length() - 4);
            File newFile = new File(oldPath);
            newFile.delete();
            isValid &= dest.renameTo(newFile);
         }
      }
      
      rval = isValid;
      
      return rval;
   }
   
   /**
    * Cleans up an installation after this script has been processed.
    * 
    * @return true if cleaned up, false if not.
    */
   public synchronized boolean cleanup()
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
    * Gets the number of download items.
    * 
    * @return the number of download items.
    */
   public int getDownloadItemCount()
   {
      return mDownloadItemCount;
   }
   
   /**
    * Gets the number of commands.
    * 
    * @return the number of commands.
    */
   public int getCommandCount()
   {
      return mCommands.size();
   }
   
   /**
    * Gets the BasicUpdateScriptProcessDelegate.
    * 
    * @return the BasicUpdateScriptProcessDelegate.
    */
   public EventDelegate getBasicUpdateScriptProcessEventDelegate()
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
