/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_FORMAT_MACROS

#include "monarch/test/Test.h"
#include "monarch/test/TestModule.h"
#include "monarch/io/File.h"
#include "monarch/io/FileInputStream.h"
#include "monarch/io/FileOutputStream.h"
#include "monarch/io/FileList.h"
#include "monarch/io/OStreamOutputStream.h"
#include "monarch/io/MutatorInputStream.h"
#include "monarch/io/MutatorOutputStream.h"
#include "monarch/compress/deflate/Deflater.h"
#include "monarch/compress/gzip/Gzipper.h"
#include "monarch/compress/zip/Zipper.h"
#include "monarch/crypto/MessageDigest.h"

using namespace std;
using namespace monarch::test;
using namespace monarch::compress;
using namespace monarch::compress::deflate;
using namespace monarch::compress::gzip;
using namespace monarch::compress::zip;
using namespace monarch::crypto;
using namespace monarch::io;
using namespace monarch::rt;
using namespace monarch::util;

namespace mo_test_compress
{

static void runDeflateTest(TestRunner& tr)
{
   tr.group("Deflate");

   File original = File::createTempFile("deflate-original");
   File deflated = File::createTempFile("deflate-deflated");
   File inflated = File::createTempFile("deflate-inflated");

   // create input test file
   tr.test("create test file");
   {
      FileOutputStream fos(original);
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

      FileInputStream fis(original);
      FileOutputStream fos(deflated);

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

      FileInputStream fis(deflated);
      FileOutputStream fos(inflated);

      MutatorInputStream mis(&fis, false, &def, false);
      char b[512];
      int numBytes;
      while((numBytes = mis.read(b, 512)) > 0)
      {
         fos.write(b, numBytes);
      }

      fis.close();
      fos.close();

      MessageDigest md1("SHA1");
      MessageDigest md2("SHA1");
      assert(md1.digestFile(original));
      assert(md2.digestFile(inflated));
      assert(md1.getDigest() == md2.getDigest());
   }
   tr.passIfNoException();

   tr.test("raw deflating file via output stream");
   {
      Deflater def;
      def.startDeflating(-1, true);
      assertNoException();

      FileInputStream fis(original);
      FileOutputStream fos(deflated);

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

      FileInputStream fis(deflated);
      FileOutputStream fos(inflated);

      MutatorOutputStream mos(&fos, false, &def, false);
      char b[512];
      int numBytes;
      while((numBytes = fis.read(b, 512)) > 0)
      {
         mos.write(b, numBytes);
      }

      fis.close();
      mos.close();

      MessageDigest md1("SHA1");
      MessageDigest md2("SHA1");
      assert(md1.digestFile(original));
      assert(md2.digestFile(inflated));
      assert(md1.getDigest() == md2.getDigest());
   }
   tr.passIfNoException();

   tr.ungroup();
}

static void runGzipTest(TestRunner& tr)
{
   tr.group("Gzip");

   File original = File::createTempFile("gzip-original");
   File compressed = File::createTempFile("gzip-compressed");
   File decompressed = File::createTempFile("gzip-decompressed");

   // create test file
   tr.test("create test file");
   {
      FileOutputStream fos(original);
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

      FileInputStream fis(original);
      FileOutputStream fos(compressed);

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

      FileInputStream fis(compressed);
      FileOutputStream fos(decompressed);

      MutatorInputStream mis(&fis, false, &gzipper, false);
      char b[512];
      int numBytes;
      while((numBytes = mis.read(b, 512)) > 0)
      {
         fos.write(b, numBytes);
      }

      fis.close();
      fos.close();

      MessageDigest md1("SHA1");
      MessageDigest md2("SHA1");
      assert(md1.digestFile(original));
      assert(md2.digestFile(decompressed));
      assert(md1.getDigest() == md2.getDigest());
   }
   tr.passIfNoException();

   tr.test("compressing file via output stream");
   {
      Gzipper gzipper;
      gzipper.startCompressing();
      assertNoException();

      FileInputStream fis(original);
      FileOutputStream fos(compressed);

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

      FileInputStream fis(compressed);
      FileOutputStream fos(decompressed);

      MutatorOutputStream mos(&fos, false, &gzipper, false);
      char b[512];
      int numBytes;
      while((numBytes = fis.read(b, 512)) > 0)
      {
         mos.write(b, numBytes);
      }

      fis.close();
      mos.close();

      MessageDigest md1("SHA1");
      MessageDigest md2("SHA1");
      assert(md1.digestFile(original));
      assert(md2.digestFile(decompressed));
      assert(md1.getDigest() == md2.getDigest());
   }
   tr.passIfNoException();
#if 0
   tr.test("gzip mp3");
   {
      string filename = "/tmp/bmtestfile.mp3";
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
         File out("/tmp/bmtestfile.gz");
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

static void runZipTest(TestRunner& tr)
{
   tr.group("Zip");

   File a = File::createTempFile("a");
   File b = File::createTempFile("b");
   File c = File::createTempFile("c");

   // create test files
   tr.test("create test files");
   {
      {
         FileOutputStream fos(a);
         string content = "brump brump brump 1234 brump brumper";
         for(int i = 0; i < 500; i++)
         {
            fos.write(content.c_str(), content.length());
         }
         fos.close();
      }

      {
         FileOutputStream fos(b);
         string content = "brump brump brump 1234 brump brumper";
         for(int i = 0; i < 1000; i++)
         {
            fos.write(content.c_str(), content.length());
         }
         fos.close();
      }

      {
         FileOutputStream fos(c);
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
      FileList fl;
      fl->add(a);
      fl->add(b);
      fl->add(c);

      File out = File::createTempFile("zipped");
      Zipper zipper;
      zipper.zip(fl, out);
   }
   tr.passIfNoException();

   tr.test("archive-only files");
   {
      Zipper zipper;

      ZipEntry ze1;
      ze1->setFilename("brump-a.txt");
      ze1->setInputFile(a);
      ze1->disableCompression(true);
      zipper.addEntry(ze1);

      ZipEntry ze2;
      ze2->setFilename("brump-b.txt");
      ze2->setInputFile(b);
      ze2->disableCompression(true);
      zipper.addEntry(ze2);

      ZipEntry ze3;
      ze3->setFilename("brump-c.txt");
      ze3->setInputFile(c);
      ze3->disableCompression(true);
      zipper.addEntry(ze3);

      // get size estimate
      uint64_t outputSize = zipper.getEstimatedArchiveSize();

      File out = File::createTempFile("archived");
      FileOutputStream fos(out);
      char b[2048];
      int numBytes;
      while(zipper.hasNextEntry())
      {
         ZipEntry next = zipper.nextEntry();

         // write entry
         if(zipper.writeEntry(next, &fos))
         {
            // write data for entry
            FileInputStream fis(next->getInputFile());
            bool success = true;
            while(success && (numBytes = fis.read(b, 2048)) > 0)
            {
               success = zipper.write(b, numBytes, &fos);
            }
            assertNoException();

            // close input stream
            fis.close();
         }
      }
      assertNoException();

      // finish zip archive, close output stream
      zipper.finish(&fos);
      fos.close();

      // check size estimate
      assert((uint64_t)out->getLength() == outputSize);
   }
   tr.passIfNoException();
#if 0
   tr.test("zip mp3");
   {
      string filename = "/tmp/bmtestfile.mp3";
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
         File out("/tmp/bmtestfile.zip");
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
   tr.test("archive-only mp3");
   {
      string filename = "/tmp/bmtestfile.mp3";
      File file(filename.c_str());
      if(!file->exists())
      {
         string temp = filename;
         temp.append(" does not exist, not running test!");
         tr.warning(temp.c_str());
      }
      else
      {
         File out("/tmp/bmtestfile.zip");
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
         //printf("Estimated total size: %" PRIu64 "\n", totalSize);

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

static bool run(TestRunner& tr)
{
   if(tr.isDefaultEnabled())
   {
      runDeflateTest(tr);
      runGzipTest(tr);
      runZipTest(tr);
   }
   return true;
}

} // end namespace

MO_TEST_MODULE_FN("monarch.tests.compress.test", "1.0", mo_test_compress::run)
