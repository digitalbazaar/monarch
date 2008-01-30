/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
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

void runDeflateTest(TestRunner& tr)
{
   tr.group("Deflate");
   
   // create test file
   tr.test("create test file");
   {
      File testFile("/tmp/brump.txt");
      FileOutputStream fos(&testFile);
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
      
      File in("/tmp/brump.txt");
      FileInputStream fis(&in);
      File out("/tmp/brump.zip");
      FileOutputStream fos(&out);
      
      MutatorInputStream mis(&fis, &def, false);
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
      
      File in("/tmp/brump.zip");
      FileInputStream fis(&in);
      File out("/tmp/brump2.txt");
      FileOutputStream fos(&out);
      
      MutatorInputStream mis(&fis, &def, false);
      char b[512];
      int numBytes;
      while((numBytes = mis.read(b, 512)) > 0)
      {
         fos.write(b, numBytes);
      }
      
      fis.close();
      fos.close();
      
      File testFile("/tmp/brump.txt");
      assert(testFile.getLength() == out.getLength());
   }
   tr.passIfNoException();
   
   tr.test("raw deflating file via output stream");
   {
      Deflater def;
      def.startDeflating(-1, true);
      assertNoException();
      
      File in("/tmp/brump.txt");
      FileInputStream fis(&in);
      File out("/tmp/brump.zip");
      FileOutputStream fos(&out);
      
      MutatorOutputStream mos(&fos, &def, false);
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
      
      File in("/tmp/brump.zip");
      FileInputStream fis(&in);
      File out("/tmp/brump2.txt");
      FileOutputStream fos(&out);
      
      MutatorOutputStream mos(&fos, &def, false);
      char b[512];
      int numBytes;
      while((numBytes = fis.read(b, 512)) > 0)
      {
         mos.write(b, numBytes);
      }
      
      fis.close();
      mos.close();
      
      File testFile("/tmp/brump.txt");
      assert(testFile.getLength() == out.getLength());
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
      File testFile("/tmp/brump.txt");
      FileOutputStream fos(&testFile);
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
      
      File in("/tmp/brump.txt");
      FileInputStream fis(&in);
      File out("/tmp/brump.gz");
      FileOutputStream fos(&out);
      
      MutatorInputStream mis(&fis, &gzipper, false);
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
      
      File in("/tmp/brump.gz");
      FileInputStream fis(&in);
      File out("/tmp/brump2.txt");
      FileOutputStream fos(&out);
      
      MutatorInputStream mis(&fis, &gzipper, false);
      char b[512];
      int numBytes;
      while((numBytes = mis.read(b, 512)) > 0)
      {
         fos.write(b, numBytes);
      }
      
      fis.close();
      fos.close();
      
      File testFile("/tmp/brump.txt");
      assert(testFile.getLength() == out.getLength());
   }
   tr.passIfNoException();
   
   tr.test("compressing file via output stream");
   {
      Gzipper gzipper;
      gzipper.startCompressing();
      assertNoException();
      
      File in("/tmp/brump.txt");
      FileInputStream fis(&in);
      File out("/tmp/brump.gz");
      FileOutputStream fos(&out);
      
      MutatorOutputStream mos(&fos, &gzipper, false);
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
      
      File in("/tmp/brump.gz");
      FileInputStream fis(&in);
      File out("/tmp/brump2.txt");
      FileOutputStream fos(&out);
      
      MutatorOutputStream mos(&fos, &gzipper, false);
      char b[512];
      int numBytes;
      while((numBytes = fis.read(b, 512)) > 0)
      {
         mos.write(b, numBytes);
      }
      
      fis.close();
      mos.close();
      
      File testFile("/tmp/brump.txt");
      assert(testFile.getLength() == out.getLength());
   }
   tr.passIfNoException();
   
   tr.ungroup();
}

void runZipTest(TestRunner& tr)
{
   tr.group("Zip");
   
   // create test files
   tr.test("create test files");
   {
      {
         File testFile("/tmp/brump-a.txt");
         FileOutputStream fos(&testFile);
         string content = "brump brump brump 1234 brump brumper";
         for(int i = 0; i < 500; i++)
         {
            fos.write(content.c_str(), content.length());
         }
         fos.close();
      }
      
      {
         File testFile("/tmp/brump-b.txt");
         FileOutputStream fos(&testFile);
         string content = "brump brump brump 1234 brump brumper";
         for(int i = 0; i < 1000; i++)
         {
            fos.write(content.c_str(), content.length());
         }
         fos.close();
      }
      
      {
         File testFile("/tmp/brump-c.txt");
         FileOutputStream fos(&testFile);
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
      File f1("/tmp/brump-a.txt");
      File f2("/tmp/brump-b.txt");
      File f3("/tmp/brump-c.txt");
      
      FileList fl(false);
      fl.add(&f1);
      fl.add(&f2);
      fl.add(&f3);
      
      File out("/tmp/brump-zipped.zip");
      
      Zipper zipper;
      zipper.zip(&fl, &out);
   }
   tr.passIfNoException();
   
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

#ifndef DB_TEST_NO_MAIN
DB_TEST_MAIN(DbCompressTester)
#endif
