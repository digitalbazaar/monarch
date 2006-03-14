/*
 * Copyright (c) 2005 Shepherd Medical Solutions, Inc.  All rights reserved.
 */
import com.sms.common.stream.*;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;

import com.sms.common.stream.IStreamManager;
import com.sms.common.stream.IStreamProcessor;

public class UTInputStreamManager implements IStreamManager, IStreamProcessor
{
   public int manageStreamData(byte[] data, int offset, int length)
   {
      //System.out.println("MANAGIN!");
      
      if(length == 64)
      {
         System.out.println("ok, got 64 bytes");
         return -32;
      }

      //System.out.println("length: " + length);
      //return 0;
      return 64 - length;

//      return (length == 256) ? 0 : 256 - length;
   }

   public byte[] processStreamData(byte[] data, boolean last)
   {
      if(last)
      {
         System.out.println("last chunk in stream read");
      }
      
      return data;
   }
   
   public UTInputStreamManager()
   {
   }
   
   public static void main(String[] args)
   {
      String src = "test/data/test.mp3";
      String dest = "test/data/ismdest.mp3";
      
      File srcFile = new File(src);
      File destFile = new File(dest);
      
      try
      {
         destFile.delete();
      }
      catch(Exception e)
      {
      }
      
      try
      {
         FileInputStream fis = new FileInputStream(srcFile);
         FileOutputStream fos = new FileOutputStream(destFile);
         
         UTInputStreamManager ism = new UTInputStreamManager();
         
         ManagedInputStream mis = new ManagedInputStream(fis, ism, ism);

         System.out.println("starting input stream manager test");
         
         byte[] buffer = new byte[32];
         
         int numBytes = -1;
         long totalBytes = 0;
         while((numBytes = mis.read(buffer)) != -1)
         {
         //   fos.write(buffer, 0, numBytes);
            totalBytes += numBytes;
         }
         
         mis.close();

         System.out.println("TOTAL BYTES/no skip: " + totalBytes);
         
         // TESTING SKIP METHOD
         
         fis = new FileInputStream(srcFile);
         mis = new ManagedInputStream(fis, ism, ism);
         
         totalBytes = 0;
         for(int i = 0; i < 3; i++)
         {
            numBytes = mis.read(buffer);
            if(numBytes != -1)
            {
               totalBytes += numBytes;
               fos.write(buffer, 0, numBytes);
            }
            else
            {
               break;
            }
         }
         
         mis.close();
         
         fis = new FileInputStream(srcFile);
         mis = new ManagedInputStream(fis, ism, ism);
         
         // test skip() method
         System.out.println("skipping: " + totalBytes);
         mis.skip(totalBytes);
         System.out.println("skip complete");
         
         while((numBytes = mis.read(buffer)) != -1)
         {
            fos.write(buffer, 0, numBytes);
            totalBytes += numBytes;
         }
         
         mis.close();
         
         System.out.println("TOTAL BYTES/skip: " + totalBytes);
         
         fos.close();
      }
      catch(Exception e)
      {
         e.printStackTrace();
      }
   }
}
