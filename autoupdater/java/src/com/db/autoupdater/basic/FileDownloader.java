/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.autoupdater.basic;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.net.URL;
import java.net.URLConnection;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * This class is used to download a file and check it against a hash.
 * 
 * @author Dave Longley
 */
public class FileDownloader
{
   /**
    * The number of bytes downloaded so far on the current download.
    */
   protected long mBytesDownloaded;
   
   /**
    * True if downloading has ceased, false otherwise.
    */
   protected boolean mDownloadingComplete;
   
   /**
    * Creates a new FileDownloader.
    */
   public FileDownloader()
   {
      mBytesDownloaded = 0;
      mDownloadingComplete = false;
   }
   
   /**
    * Downloads a file to a temporary file name. The filename will be
    * stored in the temp map in case it is valid and is to be used. If
    * the temp file is invalid, it will be deleted.
    * 
    * @param url the url to download from.
    * @param file the file to save to.
    * @return true if the file downloaded, false if not.
    */
   public boolean downloadFile(URL url, File file)
   {
      boolean rval = false;
      
      try
      {
         mBytesDownloaded = 0;
         mDownloadingComplete = false;
         
         // get a connection for downloading
         URLConnection c = url.openConnection();
         
         FileOutputStream fos = null;
         BufferedInputStream bis = null;
         
         getLogger().debug(getClass(), "Downloading file=" + url);
               
         // create file output stream for writing to temp file
         fos = new FileOutputStream(file);

         // get input stream for reading from connection
         bis = new BufferedInputStream(c.getInputStream());
            
         // read until the stream is empty
         byte[] buffer = new byte[65536];
         int numBytes = -1;
         while((numBytes = bis.read(buffer)) != -1 &&
               !Thread.currentThread().isInterrupted())
         {
            fos.write(buffer, 0, numBytes);

            // increment bytes downloaded
            mBytesDownloaded += numBytes;
         }
         
         // close streams
         bis.close();
         fos.close();
            
         if(bis != null)
         {
            bis.close();
         }

         if(fos != null)
         {
            fos.close();
         }
         
         // file downloaded
         rval = true;
      }
      catch(Throwable t)
      {
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
      
      // downloading is complete
      mDownloadingComplete = true;
      
      return rval;
   }
   
   /**
    * Gets the number of bytes downloaded so far on the current download.
    *
    * @return the number of bytes downloaded so far on the current download.
    */
   public long getBytesDownloaded()
   {
      return mBytesDownloaded;
   }
   
   /**
    * Returns true if this downloader has completed downloading. This method
    * will return true even if the download failed -- all that is required
    * is that downloading has ceased.
    * 
    * @return true if this downloader has ceased downloading.
    */
   public boolean isDownloadingComplete()
   {
      return mDownloadingComplete;
   }
   
   /**
    * Gets the logger for this FileDownloader.
    * 
    * @return the logger for this FileDownloader.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbautoupdater");
   }
}
