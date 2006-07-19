/*
 * Copyright (c) 2006 Digital Bazar, Inc.  All rights reserved.
 */
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.util.zip.DeflaterOutputStream;
import java.util.zip.GZIPInputStream;
import java.util.zip.GZIPOutputStream;
import java.util.zip.InflaterInputStream;

import com.db.stream.DeflaterInputStream;
import com.db.stream.GZipInputStream;
import com.db.stream.InflaterOutputStream;
import com.db.stream.UnGZipOutputStream;

/**
 * This class is used to test GZIP capability.
 *
 * @author Dave Longley
 */
public class UTGZip
{
   /**
    * A deflate test using Java's built in deflate and inflate classes.
    */
   public static void builtInDeflateTest()
   {
      System.out.println("Built-in Java Deflate test:");
      
      try
      {
         // the content to deflate
         String content = "GZIP GZIP GZIP GZIP GZIP GZIP GZIP GZIP.";
         
         // input stream to read the content with
         ByteArrayInputStream bais = new ByteArrayInputStream(
            content.getBytes());
         
         // file to gzip to
         FileOutputStream fos = new FileOutputStream("test.zip");

         // deflater
         DeflaterOutputStream dos = new DeflaterOutputStream(fos);
         
         // read until content is zipped
         int numBytes = -1;
         byte[] b = new byte[2048];
         while((numBytes = bais.read(b)) != -1)
         {
            // write content out
            dos.write(b, 0, numBytes);
         }
         
         // close streams
         bais.close();
         dos.close();
         
         // read deflated content
         FileInputStream fis = new FileInputStream("test.zip");
         
         // inflater
         InflaterInputStream iis = new InflaterInputStream(fis);
         
         // output stream to write content with
         ByteArrayOutputStream baos = new ByteArrayOutputStream();
         
         // read until content is inflated
         numBytes = -1;
         while((numBytes = iis.read(b)) != -1)
         {
            // write content out
            baos.write(b, 0, numBytes);
         }
         
         // close inflater
         iis.close();
         
         // get content
         String inflatedContent = baos.toString();
         
         System.out.println("Inflated content='" + inflatedContent + "'");
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
   
   /**
    * A deflate test using Java's built in deflate class and com.db.stream's
    * inflate class.
    */
   public static void javaDeflaterDBInflaterTest()
   {
      System.out.println("Java Deflate DB Inflate test:");
      
      try
      {
         // the content to deflate
         String content = "GZIP GZIP GZIP GZIP GZIP GZIP GZIP GZIP.";
         
         // input stream to read the content with
         ByteArrayInputStream bais = new ByteArrayInputStream(
            content.getBytes());
         
         // file to gzip to
         FileOutputStream fos = new FileOutputStream("test.zip");

         // deflater
         DeflaterOutputStream dos = new DeflaterOutputStream(fos);
         
         // read until content is zipped
         long totalRead = 0;
         int numBytes = -1;
         byte[] b = new byte[2048];
         while((numBytes = bais.read(b)) != -1)
         {
            totalRead += numBytes;
            
            // write content out
            dos.write(b, 0, numBytes);
         }
         
         // close streams
         bais.close();
         dos.close();
         
         System.out.println("total read=" + totalRead);
         
         // read deflated content
         FileInputStream fis = new FileInputStream("test.zip");
         
         // output stream to write content with
         ByteArrayOutputStream baos = new ByteArrayOutputStream();
         
         // inflater
         InflaterOutputStream ios = new InflaterOutputStream(baos);
         
         // read until content is inflated
         numBytes = -1;
         while((numBytes = fis.read(b)) != -1)
         {
            // write content out
            ios.write(b, 0, numBytes);
         }
         
         // close streams
         fis.close();
         ios.close();
         
         // get content
         String inflatedContent = baos.toString();
         
         System.out.println("Inflated content='" + inflatedContent + "'");
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
   
   /**
    * A deflate test using DB's deflater and Java's built in inflater.
    */
   public static void dBDeflaterJavaInflaterTest()
   {
      System.out.println("DB Deflate Java Inflate test:");
      
      try
      {
         // the content to deflate
         String content = "GZIP GZIP GZIP GZIP GZIP GZIP GZIP GZIP.";
         
         // input stream to read the content with
         ByteArrayInputStream bais = new ByteArrayInputStream(
            content.getBytes());
         
         // deflater
         DeflaterInputStream dis = new DeflaterInputStream(bais);
         
         // file to gzip to
         FileOutputStream fos = new FileOutputStream("test.zip");
         
         // read until content is zipped
         int numBytes = -1;
         byte[] b = new byte[2048];
         while((numBytes = dis.read(b)) != -1)
         {
            // write content out
            fos.write(b, 0, numBytes);
         }
         
         // close streams
         dis.close();
         fos.close();
         
         // read deflated content
         FileInputStream fis = new FileInputStream("test.zip");
         
         // inflater
         InflaterInputStream iis = new InflaterInputStream(fis);
         
         // output stream to write content with
         ByteArrayOutputStream baos = new ByteArrayOutputStream();
         
         // read until content is inflated
         numBytes = -1;
         while((numBytes = iis.read(b)) != -1)
         {
            // write content out
            baos.write(b, 0, numBytes);
         }
         
         // close inflater
         iis.close();
         
         // get content
         String inflatedContent = baos.toString();
         
         System.out.println("Inflated content='" + inflatedContent + "'");
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }  
   
   /**
    * A deflate test using all com.db.stream code.
    */
   public static void dBDeflateTest()
   {
      System.out.println("DB Deflate test:");
      
      try
      {
         // the content to deflate
         String content = "GZIP GZIP GZIP GZIP GZIP GZIP GZIP GZIP.";
         
         // input stream to read the content with
         ByteArrayInputStream bais = new ByteArrayInputStream(
            content.getBytes());
         
         // deflater
         DeflaterInputStream dis = new DeflaterInputStream(bais);
         
         // file to gzip to
         FileOutputStream fos = new FileOutputStream("test.zip");

         // read until content is zipped
         long totalRead = 0;
         int numBytes = -1;
         byte[] b = new byte[2048];
         while((numBytes = dis.read(b)) != -1)
         {
            totalRead += numBytes;
            
            // write content out
            fos.write(b, 0, numBytes);
         }
         
         // close streams
         dis.close();
         fos.close();
         
         System.out.println("total read=" + totalRead);
         
         // read deflated content
         FileInputStream fis = new FileInputStream("test.zip");
         
         // output stream to write content with
         ByteArrayOutputStream baos = new ByteArrayOutputStream();
         
         // inflater
         InflaterOutputStream ios = new InflaterOutputStream(baos);
         
         // read until content is inflated
         numBytes = -1;
         while((numBytes = fis.read(b)) != -1)
         {
            // write content out
            ios.write(b, 0, numBytes);
         }
         
         // close streams
         fis.close();
         ios.close();
         
         // get content
         String inflatedContent = baos.toString();
         
         System.out.println("Inflated content='" + inflatedContent + "'");
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }   
   
   /**
    * A gzip test using Java's built in GZIP classes.
    */
   public static void builtInGZIPTest()
   {
      System.out.println("Built-in Java GZIP test:");
      
      try
      {
         // the content to gzip
         String content = "GZIP GZIP GZIP GZIP GZIP GZIP GZIP GZIP.";
         
         // input stream to read the content with
         ByteArrayInputStream bais = new ByteArrayInputStream(
            content.getBytes());
         
         // file to gzip to
         FileOutputStream fos = new FileOutputStream("test.zip");

         // gzipper
         GZIPOutputStream gzos = new GZIPOutputStream(fos);
         
         // read until content is zipped
         int numBytes = -1;
         byte[] b = new byte[2048];
         while((numBytes = bais.read(b)) != -1)
         {
            // write content out
            gzos.write(b, 0, numBytes);
         }
         
         // close streams
         bais.close();
         gzos.close();
         
         System.out.println("total written=" + new File("test.zip").length());
         
         // read zipped content
         FileInputStream fis = new FileInputStream("test.zip");
         
         // ungzipper
         GZIPInputStream gzis = new GZIPInputStream(fis);
         
         // output stream to write content with
         ByteArrayOutputStream baos = new ByteArrayOutputStream();
         
         // read until content is unzipped
         numBytes = -1;
         while((numBytes = gzis.read(b)) != -1)
         {
            // write content out
            baos.write(b, 0, numBytes);
         }
         
         // close ungzipper
         gzis.close();
         
         // get content
         String unzippedContent = baos.toString();
         
         System.out.println("Unzipped content='" + unzippedContent + "'");
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
   
   /**
    * A test using Java's built-in gzipper and com.db.stream's ungzipper.
    */
   public static void javaGZipperDBUnGZipperTest()
   {
      System.out.println("Java GZIP, DB UN-GZIP Test:");
      
      try
      {
         // the content to gzip
         String content = "GZIP GZIP GZIP GZIP GZIP GZIP GZIP GZIP.";
         
         // input stream to read the content with
         ByteArrayInputStream bais = new ByteArrayInputStream(
            content.getBytes());
         
         // file to gzip to
         FileOutputStream fos = new FileOutputStream("test.zip");

         // gzipper
         GZIPOutputStream gzos = new GZIPOutputStream(fos);
         
         // read until content is zipped
         int numBytes = -1;
         byte[] b = new byte[1];
         while((numBytes = bais.read(b)) != -1)
         {
            // write content out
            gzos.write(b, 0, numBytes);
         }
         
         // close streams
         bais.close();
         gzos.close();
         
         System.out.println("total written=" + new File("test.zip").length()); 
         
         // read zipped content
         FileInputStream fis = new FileInputStream("test.zip");
         
         // output stream to write content with
         ByteArrayOutputStream baos = new ByteArrayOutputStream();

         // ungzipper
         UnGZipOutputStream ungzos = new UnGZipOutputStream(baos); 
         
         // read until content is unzipped
         numBytes = -1;
         b = new byte[1];
         while((numBytes = fis.read(b)) != -1)
         {
            // write content out
            ungzos.write(b, 0, numBytes);
         }
         
         // close file input stream
         fis.close();
         
         // close ungzipper
         ungzos.close();
         
         // get content
         String unzippedContent = baos.toString();
         
         System.out.println("Unzipped content='" + unzippedContent + "'");
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
   
   /**
    * A test using com.db.stream's gzipper and Java's built-in ungzipper.
    */
   public static void dBGZipperjavaUnGZipperTest()
   {
      System.out.println("DB GZIP, Java UN-GZIP Test:");
      
      try
      {
         // the content to gzip
         String content = "GZIP GZIP GZIP GZIP GZIP GZIP GZIP GZIP.";
         
         // input stream to read the content with
         ByteArrayInputStream bais = new ByteArrayInputStream(
            content.getBytes());
         
         // gzipper
         GZipInputStream gzis = new GZipInputStream(bais);

         // file to gzip to
         FileOutputStream fos = new FileOutputStream("test.zip");

         // read until content is zipped
         long totalWritten = 0;
         int numBytes = -1;
         byte[] b = new byte[1];
         while((numBytes = gzis.read(b)) != -1)
         {
            // write content out
            fos.write(b, 0, numBytes);
            totalWritten += numBytes;
         }
         
         // close streams
         gzis.close();
         fos.close();
         
         System.out.println("total written=" + totalWritten);
         
         // read zipped content
         FileInputStream fis = new FileInputStream("test.zip");
         
         // ungzipper
         GZIPInputStream ungzis = new GZIPInputStream(fis);
         
         // output stream to write content with
         ByteArrayOutputStream baos = new ByteArrayOutputStream();
         
         // read until content is unzipped
         numBytes = -1;
         while((numBytes = ungzis.read(b)) != -1)
         {
            // write content out
            baos.write(b, 0, numBytes);
         }
         
         // close ungzipper
         ungzis.close();
         
         // get content
         String unzippedContent = baos.toString();
         
         System.out.println("Unzipped content='" + unzippedContent + "'");
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
   
   /**
    * The main method that runs the gzip test.
    * 
    * @param args the args for the main method.
    */
   public static void main(String[] args)
   {
      // run the built-in deflate test
      //builtInDeflateTest();
      
      //System.out.println();
      
      // run the java deflater, db inflater test
      //javaDeflaterDBInflaterTest();
      
      //System.out.println();
      
      // run the db deflater, java inflater test
      //dBDeflaterJavaInflaterTest();
      
      //System.out.println();
      
      // run all db deflation/inflation code
      //dBDeflateTest();
      
      //System.out.println();
      
      // run the built-in gzip test
      //builtInGZIPTest();
      
      //System.out.println();
      
      // run the java gzipper, db ungzipper test
      //javaGZipperDBUnGZipperTest();
      
      //System.out.println();
      
      // run the db gzipper, java ungzipper test
      dBGZipperjavaUnGZipperTest();
   }
}
