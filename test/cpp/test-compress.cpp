/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc. All rights reserved.
 */

#include "db/test/Test.h"
#include "db/test/Tester.h"
#include "db/test/TestRunner.h"
#include "db/io/File.h"
#include "db/io/FileInputStream.h"
#include "db/io/FileOutputStream.h"
#include "db/io/FileList.h"
#include "db/io/OStreamOutputStream.h"
#include "db/io/MutatorInputStream.h"
#include "db/io/MutatorOutputStream.h"
#include "db/compress/deflate/Deflater.h"
#include "db/compress/gzip/Gzipper.h"
#include "db/compress/zip/Zipper.h"

using namespace std;
using namespace db::test;
using namespace db::compress;
using namespace db::compress::deflate;
using namespace db::compress::gzip;
using namespace db::compress::zip;
using namespace db::io;
using namespace db::rt;
using namespace db::util;

#ifdef WIN32
#define TMPDIR "c:/WINDOWS/Temp"
#else
#define TMPDIR "/tmp"
#endif

void runDeflateTest(TestRunner& tr)
{
   tr.group("Deflate");
   
   // create test file
   tr.test("create test file");
   {
      File testFile(TMPDIR "/brump.txt");
      FileOutputStream fos(testFile);
      string content = "brump brump brump 1234 brump brumper";
      for(int i = 0; i < 1000; i++)
      {
         fos.write(content.c_str(), content.length());
      }
      fos.close();
   }
   tr.passIfNoException();
   
   tr.test("raw deflating file via input stream");
   {
      Deflater def;
      def.startDeflating(-1, true);
      assertNoException();
      
      File in(TMPDIR "/brump.txt");
      FileInputStream fis(in);
      File out(TMPDIR "/brump.zip");
      FileOutputStream fos(out);
      
      MutatorInputStream mis(&fis, false, &def, false);
      char b[512];
      int numBytes;
      while((numBytes = mis.read(b, 512)) > 0)
      {
         fos.write(b, numBytes);
      }
      
      fis.close();
      fos.close();
   }
   tr.passIfNoException();
   
   tr.test("raw inflating file via input stream");
   {
      Deflater def;
      def.startInflating(true);
      assertNoException();
      
      File in(TMPDIR "/brump.zip");
      FileInputStream fis(in);
      File out(TMPDIR "/brump2.txt");
      FileOutputStream fos(out);
      
      MutatorInputStream mis(&fis, false, &def, false);
      char b[512];
      int numBytes;
      while((numBytes = mis.read(b, 512)) > 0)
      {
         fos.write(b, numBytes);
      }
      
      fis.close();
      fos.close();
      
      File testFile(TMPDIR "/brump.txt");
      assert(testFile->getLength() == out->getLength());
   }
   tr.passIfNoException();
   
   tr.test("raw deflating file via output stream");
   {
      Deflater def;
      def.startDeflating(-1, true);
      assertNoException();
      
      File in(TMPDIR "/brump.txt");
      FileInputStream fis(in);
      File out(TMPDIR "/brump.zip");
      FileOutputStream fos(out);
      
      MutatorOutputStream mos(&fos, false, &def, false);
      char b[512];
      int numBytes;
      while((numBytes = fis.read(b, 512)) > 0)
      {
         mos.write(b, numBytes);
      }
      
      fis.close();
      mos.close();
   }
   tr.passIfNoException();
   
   tr.test("raw inflating file via output stream");
   {
      Deflater def;
      def.startInflating(true);
      assertNoException();
      
      File in(TMPDIR "/brump.zip");
      FileInputStream fis(in);
      File out(TMPDIR "/brump2.txt");
      FileOutputStream fos(out);
      
      MutatorOutputStream mos(&fos, false, &def, false);
      char b[512];
      int numBytes;
      while((numBytes = fis.read(b, 512)) > 0)
      {
         mos.write(b, numBytes);
      }
      
      fis.close();
      mos.close();
      
      File testFile(TMPDIR "/brump.txt");
      assert(testFile->getLength() == out->getLength());
   }
   tr.passIfNoException();
   
   tr.ungroup();
}

void runGzipTest(TestRunner& tr)
{
   tr.group("Gzip");
   
   // create test file
   tr.test("create test file");
   {
      File testFile(TMPDIR "/brump.txt");
      FileOutputStream fos(testFile);
      string content = "brump brump brump 1234 brump brumper";
      for(int i = 0; i < 1000; i++)
      {
         fos.write(content.c_str(), content.length());
      }
      fos.close();
   }
   tr.passIfNoException();
   
   tr.test("compressing file via input stream");
   {
      Gzipper gzipper;
      gzipper.startCompressing();
      assertNoException();
      
      File in(TMPDIR "/brump.txt");
      FileInputStream fis(in);
      File out(TMPDIR "/brump.gz");
      FileOutputStream fos(out);
      
      MutatorInputStream mis(&fis, false, &gzipper, false);
      char b[512];
      int numBytes;
      while((numBytes = mis.read(b, 512)) > 0)
      {
         fos.write(b, numBytes);
      }
      
      fis.close();
      fos.close();
   }
   tr.passIfNoException();
   
   tr.test("decompressing file via input stream");
   {
      Gzipper gzipper;
      gzipper.startDecompressing();
      assertNoException();
      
      File in(TMPDIR "/brump.gz");
      FileInputStream fis(in);
      File out(TMPDIR "/brump2.txt");
      FileOutputStream fos(out);
      
      MutatorInputStream mis(&fis, false, &gzipper, false);
      char b[512];
      int numBytes;
      while((numBytes = mis.read(b, 512)) > 0)
      {
         fos.write(b, numBytes);
      }
      
      fis.close();
      fos.close();
      
      File testFile(TMPDIR "/brump.txt");
      assert(testFile->getLength() == out->getLength());
   }
   tr.passIfNoException();
   
   tr.test("compressing file via output stream");
   {
      Gzipper gzipper;
      gzipper.startCompressing();
      assertNoException();
      
      File in(TMPDIR "/brump.txt");
      FileInputStream fis(in);
      File out(TMPDIR "/brump.gz");
      FileOutputStream fos(out);
      
      MutatorOutputStream mos(&fos, false, &gzipper, false);
      char b[512];
      int numBytes;
      while((numBytes = fis.read(b, 512)) > 0)
      {
         mos.write(b, numBytes);
      }
      
      fis.close();
      mos.close();
   }
   tr.passIfNoException();
   
   tr.test("decompressing file via output stream");
   {
      Gzipper gzipper;
      gzipper.startDecompressing();
      assertNoException();
      
      File in(TMPDIR "/brump.gz");
      FileInputStream fis(in);
      File out(TMPDIR "/brump2.txt");
      FileOutputStream fos(out);
      
      MutatorOutputStream mos(&fos, false, &gzipper, false);
      char b[512];
      int numBytes;
      while((numBytes = fis.read(b, 512)) > 0)
      {
         mos.write(b, numBytes);
      }
      
      fis.close();
      mos.close();
      
      File testFile(TMPDIR "/brump.txt");
      assert(testFile->getLength() == out->getLength());
   }
   tr.passIfNoException();
#if 0
   tr.test("gzip mp3");
   {
      string filename = TMPDIR "/bmtestfile.mp3";
      File file(filename.c_str());
      if(!file.exists())
      {
         string temp = filename;
         temp.append(" does not exist, not running test!");
         tr.warning(temp.c_str());
      }
      else
      {
         FileInputStream fis(file, false);
         File out(TMPDIR "/bmtestfile.gz");
         FileOutputStream fos(out, false);
         
         // create gzipper, buffer
         Gzipper gzipper;
         char b[2048];
         int numBytes;
         
         // start compression
         gzipper.startCompressing();
         assertNoException();
         
         // do compression
         MutatorOutputStream mos(&fos, false, &gzipper, false);
         while((numBytes = fis.read(b, 512)) > 0)
         {
            mos.write(b, numBytes);
         }
         
         // close streams
         fis.close();
         mos.close();
      }
   }
   tr.passIfNoException();
#endif
   
   tr.ungroup();
}

void runZipTest(TestRunner& tr)
{
   tr.group("Zip");
   
   // create test files
   tr.test("create test files");
   {
      {
         File testFile(TMPDIR "/brump-a.txt");
         FileOutputStream fos(testFile);
         string content = "brump brump brump 1234 brump brumper";
         for(int i = 0; i < 500; i++)
         {
            fos.write(content.c_str(), content.length());
         }
         fos.close();
      }
      
      {
         File testFile(TMPDIR "/brump-b.txt");
         FileOutputStream fos(testFile);
         string content = "brump brump brump 1234 brump brumper";
         for(int i = 0; i < 1000; i++)
         {
            fos.write(content.c_str(), content.length());
         }
         fos.close();
      }
      
      {
         File testFile(TMPDIR "/brump-c.txt");
         FileOutputStream fos(testFile);
         string content = "brump brump brump 1234 brump brumper";
         for(int i = 0; i < 1500; i++)
         {
            fos.write(content.c_str(), content.length());
         }
         fos.close();
      }
   }
   tr.passIfNoException();
   
   tr.test("zip files");
   {
      File f1(TMPDIR "/brump-a.txt");
      File f2(TMPDIR "/brump-b.txt");
      File f3(TMPDIR "/brump-c.txt");
      
      FileList fl;
      fl->add(f1);
      fl->add(f2);
      fl->add(f3);
      
      File out(TMPDIR "/brump-zipped.zip");
      
      Zipper zipper;
      zipper.zip(fl, out);
   }
   tr.passIfNoException();
#if 0
   tr.test("zip mp3");
   {
      string filename = TMPDIR "/bmtestfile.mp3";
      File file(filename.c_str());
      if(!file->exists())
      {
         string temp = filename;
         temp.append(" does not exist, not running test!");
         tr.warning(temp.c_str());
      }
      else
      {
         FileInputStream fis(file);
         File out(TMPDIR "/bmtestfile.zip");
         FileOutputStream fos(out, false);
         
         // create zipper, buffer
         Zipper zipper;
         char b[2048];
         int numBytes;
         
         // create zip entry, set file name
         ZipEntry ze;
         ze->setFilename("bmtestfile-unzipped.mp3");
         
         // write entry
         if(zipper.writeEntry(ze, &fos))
         {
            // write data for entry
            bool success = true;
            while(success && (numBytes = fis.read(b, 2048)) > 0)
            {
               success = zipper.write(b, numBytes, &fos);
            }
         }
         
         // close input stream
         fis.close();
         
         // finish zip archive, close output stream
         zipper.finish(&fos);
         fos.close();
      }
   }
   tr.passIfNoException();
#endif
#if 0
   tr.test("zip non-compressed mp3");
   {
      string filename = TMPDIR "/bmtestfile.mp3";
      File file(filename.c_str());
      if(!file->exists())
      {
         string temp = filename;
         temp.append(" does not exist, not running test!");
         tr.warning(temp.c_str());
      }
      else
      {
         File out(TMPDIR "/bmtestfile.zip");
         FileOutputStream fos(out, false);
         
         // create zipper, buffer
         Zipper zipper;
         char b[2048];
         int numBytes;
         
         // create zip entry, set file name
         ZipEntry ze;
         ze->setFilename("bmtestfile-unzipped.mp3");
         ze->disableCompression(true);
         ze->setInputFile(file);
         zipper.addEntry(ze);
         
         //uint64_t totalSize = zipper.getEstimatedArchiveSize();
         //printf("Estimated total size: %llu\n", totalSize);
         
         while(zipper.hasNextEntry())
         {
            // write next entry
            ZipEntry next = zipper.nextEntry();
            if(zipper.writeEntry(next, &fos))
            {
               // write data for entry
               FileInputStream fis(ze->getInputFile());
               bool success = true;
               while(success && (numBytes = fis.read(b, 2048)) > 0)
               {
                  success = zipper.write(b, numBytes, &fos);
               }
               
               // close input stream
               fis.close();
            }
         }
         
         // finish zip archive, close output stream
         zipper.finish(&fos);
         fos.close();
      }
   }
   tr.passIfNoException();
#endif
   tr.ungroup();
}

class DbCompressTester : public db::test::Tester
{
public:
   DbCompressTester()
   {
      setName("compress");
   }
   
   /**
    * Run automatic unit tests.
    */
   virtual int runAutomaticTests(TestRunner& tr)
   {
      // db::compress tests
      runDeflateTest(tr);
      runGzipTest(tr);
      runZipTest(tr);
      return 0;
   }

   /**
    * Runs interactive unit tests.
    */
   virtual int runInteractiveTests(TestRunner& tr)
   {
      return 0;
   }
};

#undef TMPDIR

#ifndef DB_TEST_NO_MAIN
DB_TEST_MAIN(DbCompressTester)
#endif
