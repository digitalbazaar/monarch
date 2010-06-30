/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_FORMAT_MACROS

#include "monarch/test/Test.h"
#include "monarch/test/TestModule.h"
#include "monarch/io/IOMonitor.h"
#include "monarch/io/File.h"
#include "monarch/io/FileInputStream.h"
#include "monarch/io/FileOutputStream.h"
#include "monarch/io/FileList.h"
#include "monarch/io/FilterOutputStream.h"
#include "monarch/io/BitStream.h"
#include "monarch/io/BufferedOutputStream.h"
#include "monarch/io/ByteArrayInputStream.h"
#include "monarch/io/ByteArrayOutputStream.h"
#include "monarch/io/ByteBuffer.h"
#include "monarch/io/IOException.h"
#include "monarch/io/MutatorInputStream.h"
#include "monarch/io/MutatorOutputStream.h"
#include "monarch/io/TruncateInputStream.h"
#include "monarch/modest/Module.h"
#include "monarch/rt/System.h"
#include "monarch/util/StringTools.h"

#include <cstdlib>

using namespace std;
using namespace monarch::test;
using namespace monarch::io;
using namespace monarch::rt;
using namespace monarch::util;

// redefine this here to make testing easier via cpp string concatination
#ifdef WIN32
#define SEP "\\"
#else
#define SEP "/"
#endif

namespace mo_test_io
{

static void runStringEqualityTest(TestRunner& tr)
{
   tr.test("string equality");

   // Note: string length doesn't appear to matter
   string str = "blah";
   uint64_t start, end;

   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; ++i)
   {
      if(str == "");
   }
   end = System::getCurrentMilliseconds();
   printf("String == \"\" time: %" PRIu64 " ms\n", (end - start));

   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; ++i)
   {
      if(str.length() == 0);
   }
   end = System::getCurrentMilliseconds();
   printf("String.length() == 0 time: %" PRIu64 " ms\n", (end - start));

   // Note: test demonstrates that comparing to length is about 6 times faster

   tr.passIfNoException();
}

static void runStringAppendCharTest(TestRunner& tr)
{
   tr.test("string append char");

   // Note: string length doesn't appear to matter
   string str = "blah";
   uint64_t start, end;

   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; ++i)
   {
      if(str.length() == 1 && str[0] == '/');
   }
   end = System::getCurrentMilliseconds();
   printf("String.length() == 1 && str[0] == '/' time: %" PRIu64 " ms\n",
      (end - start));

   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; ++i)
   {
      if(str == "/");
   }
   end = System::getCurrentMilliseconds();
   printf("String == \"/\" time: %" PRIu64 " ms\n", (end - start));

   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; ++i)
   {
      if(strcmp(str.c_str(), "/") == 0);
   }
   end = System::getCurrentMilliseconds();
   printf("strcmp(String.c_str(), \"/\") == 0 time: %" PRIu64 " ms\n",
      (end - start));

   string version = "HTTP/1.0";
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; ++i)
   {
      if(version == "HTTP/1.0");
   }
   end = System::getCurrentMilliseconds();
   printf("String == \"HTTP/1.0\" time: %" PRIu64 " ms\n", (end - start));

   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; ++i)
   {
      if(strcmp(version.c_str(), "HTTP/1.0") == 0);
   }
   end = System::getCurrentMilliseconds();
   printf("strcmp(String.c_str(), \"HTTP/1.0\") == 0 time: %" PRIu64 " ms\n",
      (end - start));

   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 10000; ++i)
   {
      str.append(1, '/');
   }
   end = System::getCurrentMilliseconds();
   printf("String.append(1, '/') time: %" PRIu64 " ms\n", (end - start));

   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 10000; ++i)
   {
      str.append("/");
   }
   end = System::getCurrentMilliseconds();
   printf("String.append(\"/\") time: %" PRIu64 " ms\n", (end - start));

   string space = " ";
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 10000; ++i)
   {
      str.append("this" + space + "is a sentence");
   }
   end = System::getCurrentMilliseconds();
   printf("String inline append time: %" PRIu64 " ms\n", (end - start));

   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 10000; ++i)
   {
      str.append("this");
      str.append(space);
      str.append("is a sentence");
   }
   end = System::getCurrentMilliseconds();
   printf("String multiline append time: %" PRIu64 " ms\n", (end - start));

   tr.passIfNoException();
}

static void runStringCompareTest(TestRunner& tr)
{
   tr.test("string compare");

   string str1 = "blah";
   char str2[] = "blah";
   uint64_t start, end;

   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; ++i)
   {
      if(str1 == "blah");
   }
   end = System::getCurrentMilliseconds();
   printf("std::string compare time: %" PRIu64 " ms\n", (end - start));

   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; ++i)
   {
      if(strcmp(str2, "blah") == 0);
   }
   end = System::getCurrentMilliseconds();
   printf("char* compare time: %" PRIu64 " ms\n", (end - start));

   tr.passIfNoException();
}

static void runMemcpyTest(TestRunner& tr)
{
   tr.test("memcpy timing");

   // Try to time custom vs standard memcpy.

   // NOTE:
   // This test is likely very sensitive to optimizations, particular platforms
   // memcpy implementations, test ordering, cache issues, and so on.

   char* src[1024];
   char* dst[1024];
   int lens[] =
      {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,20,30,40,50,100,200,500,1000,0};
   int loops = 10000000;

   // loop over lengths to check
   for(int n = 0; lens[n] != 0; ++n)
   {
      // test custom memcpy
      uint64_t cust_start;
      uint64_t cust_end;
      {
         cust_start = System::getCurrentMilliseconds();
         for(int i = 0; i < loops; ++i)
         {
            if(n < 10)
            {
               // optimized over memcpy()
               for(int mci = 0; mci < n; ++mci)
               {
                  dst[mci] = src[mci];
               }
            }
            else
            {
               memcpy(dst, src, n);
            }
         }
         cust_end = System::getCurrentMilliseconds();
      }

      // test memcpy
      uint64_t std_start;
      uint64_t std_end;
      {
         std_start = System::getCurrentMilliseconds();
         for(int i = 0; i < loops; ++i)
         {
            memcpy(dst, src, n);
         }
         std_end = System::getCurrentMilliseconds();
      }

      uint64_t cust_dt = (cust_end - cust_start);
      uint64_t std_dt = (std_end - std_start);
      int64_t diff = cust_dt - std_dt;
      printf("n:%d, custom:%" PRIu64 " ms,"
         " std:%" PRIu64 " ms,"
         " diff:%" PRIi64 " ms\n",
         lens[n], cust_dt, std_dt, diff);
   }

   tr.passIfNoException();
}

static void runByteBufferTest(TestRunner& tr)
{
   tr.test("ByteBuffer");

   ByteBuffer b;

   const char* chicken = "chicken";
   const char* t = "T ";
   const char* hate = "hate ";
   b.free();
   b.put(t, strlen(t), true);
   b.put(hate, strlen(hate), true);
   b.put(chicken, strlen(chicken), true);
   b.put("", 1, true);

   // FIXME: this test should be more comprehensive

   assertStrCmp(b.data(), "T hate chicken");

   // this should result in printing out "T hate chicken" still
   b.allocateSpace(10, true);
   sprintf(b.end(), " always");
   char temp[100];
   strncpy(temp, b.data(), b.length());
   assertStrCmp(temp, "T hate chicken");

   // this should now result in printing out "T hate chicken always"
   sprintf(b.end() - 1, " always");
   b.extend(6);
   b.putByte(' ', 1, true);
   b.putByte('t', 1, true);
   b.putByte('r', 1, true);
   b.putByte('u', 1, true);
   b.putByte('e', 1, true);
   b.putByte(0x00, 1, true);
   strncpy(temp, b.data(), b.length());
   assertStrCmp(temp, "T hate chicken always true");

   unsigned char aByte;
   b.getByte(aByte);
   assert(aByte == 'T');

   tr.passIfNoException();
}

static void runByteArrayInputStreamTest(TestRunner& tr)
{
   tr.test("ByteArrayInputStream");

   char html[] = "<html>505 HTTP Version Not Supported</html>";
   ByteArrayInputStream is(html, 43);

   char b[10];
   int numBytes;
   string str;
   while((numBytes = is.read(b, 9)) > 0)
   {
      memset(b + numBytes, 0, 1);
      str.append(b);
   }

   assertStrCmp(str.c_str(), html);

   tr.passIfNoException();
}

static void runByteArrayOutputStreamTest(TestRunner& tr)
{
   tr.test("ByteArrayOutputStream");

   ByteBuffer b;

   ByteArrayOutputStream baos1(&b);
   const char* sentence = "This is a sentence.";
   baos1.write(sentence, strlen(sentence) + 1);

   assertStrCmp(b.data(), sentence);

   const char* t = "T ";
   const char* hate = "hate ";
   const char* chicken = "chicken";
   b.clear();
   b.put(t, strlen(t), true);
   b.put(hate, strlen(hate), true);
   b.put(chicken, strlen(chicken), true);
   b.put("", 1, true);

   assertStrCmp(b.data(), "T hate chicken");

   // trim null-terminator
   b.trim(1);

   // false = turn off resizing buffer
   int length = strlen(sentence) + 1;
   ByteArrayOutputStream baos2(&b, false);
   tr.warning("Add BAOS exception check");
   if(!baos2.write(sentence, length))
   {
      IOException* e = (IOException*)&(*Exception::get());
      //printf("Exception Caught=%s\n", e->getMessage());
      //printf("Written bytes=%d\n", e->getUsedBytes());
      //printf("Unwritten bytes=%d\n", e->getUnusedBytes());
      //printf("Turning on resize and finishing write...\n");

      // turn on resize
      baos2.setResize(true);

      // write remaining bytes
      baos2.write(sentence + e->getUsedBytes(), e->getUnusedBytes());

      // clear exception
      Exception::clear();
   }

   assertStrCmp(b.data(), "T hate chickenThis is a sentence.");

   tr.passIfNoException();
}

static void runBitStreamTest(TestRunner& tr)
{
   tr.group("BitStream");

   tr.test("string conversion");
   {
      BitStream bs;
      const char* bits = "1001010101010110";
      bs.appendFromString(bits, strlen(bits));
      assertStrCmp(bits, bs.toString().c_str());
   }
   tr.passIfNoException();

   tr.test("shift left");
   {
      BitStream bs;
      const char* bits = "1001010101010110";
      bs.appendFromString(bits, strlen(bits));
      bs << 4;
      assertStrCmp("010101010110", bs.toString().c_str());
//      bs << 8;
//      assertStrCmp("01010110", bs.toString().c_str());
   }
   tr.passIfNoException();

   tr.ungroup();
}

#ifdef WIN32

//static int setenv(const char *name, const char *value, int overwrite)
//{
//   return (SetEnvironmentVariable(name, value) != 0 ? 0 : -1);
//}

//static int unsetenv(const char *name)
//{
//   return (SetEnvironmentVariable(name, NULL) != 0 ? 0 : -1);
//}

#endif

static void runFileTest(TestRunner& tr)
{
   tr.group("File");

   File cdir(".");

   string tmpFilename;
   assert(File::getTemporaryDirectory(tmpFilename));

   string tmpFilenameA = tmpFilename;
   tmpFilenameA.append(File::NAME_SEPARATOR);
   tmpFilenameA.append("MONARCHTEST_a.txt");

   string tmpFilenameB = tmpFilename;
   tmpFilenameB.append(File::NAME_SEPARATOR);
   tmpFilenameB.append("dir");
   tmpFilenameB.append(File::NAME_SEPARATOR);
   tmpFilenameB.append("..");
   tmpFilenameB.append(File::NAME_SEPARATOR);
   tmpFilenameB.append("file.txt");

   string expectTmpFilenameB = tmpFilename;
   expectTmpFilenameB.append(File::NAME_SEPARATOR);
   expectTmpFilenameB.append("file.txt");

   string tmpFilenameC = tmpFilename;
   tmpFilenameC.append(File::NAME_SEPARATOR);
   tmpFilenameC.append("MONARCHTEST_c.txt");

   File tmp(tmpFilename.c_str());
   File tmpFileA(tmpFilenameA.c_str());
   File tmpFileB(tmpFilenameB.c_str());
   File tmpFileC(tmpFilenameC.c_str());
   File junk("../../foo/../junk238jflk38sjf.txt");
   string np;

   tr.test("absolute paths");
   {
      assertStrCmp(tmp->getAbsolutePath(), tmpFilename.c_str());
      assertStrCmp(tmpFileA->getAbsolutePath(), tmpFilenameA.c_str());
      assertStrCmp(tmpFileB->getAbsolutePath(), expectTmpFilenameB.c_str());
   }
   tr.passIfNoException();

   tr.test("normalization (invalid)");
   {
      File::normalizePath("../../foo/../junk238jflk38sjf.txt", np);
   }
   tr.passIfException();

   tr.test("normalization (valid)");
   {
      File::normalizePath(File::NAME_SEPARATOR, np);
      assertStrCmp(np.c_str(), File::NAME_SEPARATOR);

#ifdef WIN32
      File::normalizePath("C:/windows/temp/dir/../file.txt", np);
      assertStrCmp(np.c_str(), "C:\\windows\\temp\\file.txt");

      File::normalizePath("C:/windows/temp/./dir/../file.txt", np);
      assertStrCmp(np.c_str(), "C:\\windows\\temp\\file.txt");

      File::normalizePath("C:/windows/temp/../../file.txt", np);
      assertStrCmp(np.c_str(), "C:\\file.txt");
#else
      File::normalizePath("/../../foo/../junk238jflk38sjf.txt", np);
      assertStrCmp(np.c_str(), "/junk238jflk38sjf.txt");

      File::normalizePath("/tmp/dir/../file.txt", np);
      assertStrCmp(np.c_str(), "/tmp/file.txt");

      File::normalizePath("/tmp/./dir/../file.txt", np);
      assertStrCmp(np.c_str(), "/tmp/file.txt");

      File::normalizePath("/tmp/../../file.txt", np);
      assertStrCmp(np.c_str(), "/file.txt");
#endif
   }
   tr.passIfNoException();

   tr.test("readable curdir");
   {
      assert(cdir->isReadable());
   }
   tr.passIfNoException();

   tr.test("not readable junk");
   {
      assert(!junk->isReadable());
   }
   tr.passIfNoException();

   tr.test("writable curdir");
   {
      assert(cdir->isWritable());
   }
   tr.passIfNoException();

   tr.test("directory containment");
   {
      assert(tmp->contains(tmpFileA));
      assert(!tmpFileA->contains(tmp));
   }
   tr.passIfNoException();

   tr.test("directory list");
   {
      File dir(tmpFilename.c_str());
      FileList files;
      dir->listFiles(files);

      //printf("/tmp contains %i files\n", files.count())";

      assert(files->count() > 0);
   }
   tr.passIfNoException();

   tr.test("get type");
   {
      File dir(tmpFilename.c_str());
      FileList files;
      dir->listFiles(files);
      IteratorRef<File> i = files->getIterator();
      while(i->hasNext())
      {
         File& file = i->next();
         const char* type;
         switch(file->getType())
         {
            case FileImpl::RegularFile:
               type = "Regular File";
               break;
            case FileImpl::Directory:
               type = "Directory";
               break;
            case FileImpl::SymbolicLink:
               type = "Symbolic Link";
               break;
            default:
               type = "Unknown";
               break;
         }
         //printf("Name: '%s', Type: '%s'\n", file->getAbsolutePath(), type);
      }
   }
   tr.passIfNoException();

   tr.test("create");
   {
      tmpFileA->create();
      assert(tmpFileA->exists());
   }
   tr.passIfNoException();

   tr.test("rename");
   {
      bool renamed;
      renamed = tmpFileA->rename(tmpFileC);
      assertNoExceptionSet();
      assert(renamed);

      bool exists;
      exists = tmpFileA->exists();
      assertNoExceptionSet();
      assert(!exists);

      exists = tmpFileC->exists();
      assertNoExceptionSet();
      assert(exists);
   }
   tr.passIfNoException();

   tr.test("expandUser");
   {
#ifdef WIN32
      const char* HOMEDRIVE = getenv("HOMEDRIVE");
      const char* HOMEPATH = getenv("HOMEPATH");
      const char* USERPROFILE = getenv("USERPROFILE");
      assert(HOMEDRIVE != NULL);
      assert(HOMEPATH != NULL);
      assert(USERPROFILE != NULL);
#else
      const char* oldHOME = getenv("HOME");
      assert(oldHOME != NULL);
#endif

      // test failure with unset env vars
#ifdef WIN32
      //unsetenv("HOMEPATH");
      // can not unset HOMEPATH/HOMEDRIVE on Windows
#else
      unsetenv("HOME");
      {
         string path;
         assert(!File::expandUser("~/", path));
         assertExceptionSet();
         Exception::clear();
      }
#endif

#ifdef WIN32
      // can not set HOME* vars on Windows so use real values
      char homestr[strlen(HOMEDRIVE) + strlen(HOMEPATH) + 1];
      sprintf(homestr, "%s%s", HOMEDRIVE, HOMEPATH);
      char profilestr[strlen(USERPROFILE) + 1];
      sprintf(profilestr, "%s", USERPROFILE);
#else
      // set env vars to known state for this test
      const char* homestr = "/home/test";
      setenv("HOME", "/home/test", 1);
#endif

      {
         string path;
         assert(File::expandUser("~", path));
         assertStrCmp(path.c_str(), homestr);
      }

      {
         string path;
         assert(File::expandUser("~/", path));
         string tmp;
         tmp.append(homestr);
         tmp.append(SEP);
         assertStrCmp(path.c_str(), tmp.c_str());
      }

      {
         string path;
         assert(File::expandUser("~/foo.txt", path));
         string tmp;
         tmp.append(homestr);
         tmp.append(SEP "foo.txt");
         assertStrCmp(path.c_str(), tmp.c_str());
      }

#ifdef WIN32
      // Windows only tests
      {
         string path;
         assert(File::expandUser("%HOMEDRIVE%%HOMEPATH%", path));
         string tmp;
         tmp.append(HOMEDRIVE);
         tmp.append(HOMEPATH);
         assertStrCmp(path.c_str(), tmp.c_str());
      }

      {
         string path;
         assert(File::expandUser("%HOMEDRIVE%%HOMEPATH%/foo.txt", path));
         string tmp;
         tmp.append(HOMEDRIVE);
         tmp.append(HOMEPATH);
         tmp.append(SEP "foo.txt");
         assertStrCmp(path.c_str(), tmp.c_str());
      }

      {
         string path;
         assert(File::expandUser("%USERPROFILE%", path));
         string tmp;
         tmp.append(USERPROFILE);
         assertStrCmp(path.c_str(), tmp.c_str());
      }

      {
         string path;
         assert(File::expandUser("%USERPROFILE%/foo.txt", path));
         string tmp;
         tmp.append(USERPROFILE);
         tmp.append(SEP "foo.txt");
         assertStrCmp(path.c_str(), tmp.c_str());
      }

      {
         string path;
         assert(File::expandUser("%HOMEDRIVE%/foo.txt", path));
         string tmp;
         tmp.append(HOMEDRIVE);
         tmp.append(SEP "foo.txt");
         assertStrCmp(path.c_str(), tmp.c_str());
      }

      {
         string path;
         assert(File::expandUser("%HOMEPATH%/foo.txt", path));
         string tmp;
         tmp.append(HOMEPATH);
         tmp.append(SEP "foo.txt");
         assertStrCmp(path.c_str(), tmp.c_str());
      }
#endif

      {
         string path;
         assert(!File::expandUser("~user/foo.txt", path));
         assertExceptionSet();
         Exception::clear();
      }

      {
         string path;
         assert(!File::expandUser("~user/foo.txt", path));
         assertExceptionSet();
         Exception::clear();
      }

      {
         string path;
         assert(File::expandUser("/root/path", path));
         string tmp(SEP "root" SEP "path");
         assertStrCmp(path.c_str(), tmp.c_str());
      }

      {
         string path;
         assert(File::expandUser("rel/path", path));
         string tmp("rel" SEP "path");
         assertStrCmp(path.c_str(), tmp.c_str());
      }

      {
         string path;
         assert(File::expandUser("", path));
         assertStrCmp(path.c_str(), "");
      }

      {
         string path;
         assert(File::expandUser(".", path));
         assertStrCmp(path.c_str(), ".");
      }

      // restore old env
#ifdef WIN32
      // can not change HOME* env vars so nothing to cleanup
#else
      setenv("HOME", oldHOME, 1);
#endif
   }
   tr.passIfNoException();

   tr.test("split,splitext,dir,base");
   {
      {
         string dir, base;
         File::split("", dir, base);
         assertStrCmp(dir.c_str(), "");
         assertStrCmp(base.c_str(), "");
      }

      {
         string dir, base;
         File::split("/", dir, base);
         assertStrCmp(dir.c_str(), SEP);
         assertStrCmp(base.c_str(), "");
      }

      {
         string dir, base;
         File::split("base", dir, base);
         assertStrCmp(dir.c_str(), "");
         assertStrCmp(base.c_str(), "base");
      }

      {
         string dir, base;
         File::split("/base", dir, base);
         assertStrCmp(dir.c_str(), SEP);
         assertStrCmp(base.c_str(), "base");
      }

      {
         string dir, base;
         File::split("/dir/", dir, base);
         assertStrCmp(dir.c_str(), SEP "dir");
         assertStrCmp(base.c_str(), "");
      }

      {
         string dir, base;
         File::split("dir/", dir, base);
         assertStrCmp(dir.c_str(), "dir");
         assertStrCmp(base.c_str(), "");
      }

      {
         string dir, base;
         File::split("/dir/base", dir, base);
         assertStrCmp(dir.c_str(), SEP "dir");
         assertStrCmp(base.c_str(), "base");
      }

      {
         string dir, base;
         File::split("dir/base", dir, base);
         assertStrCmp(dir.c_str(), "dir");
         assertStrCmp(base.c_str(), "base");
      }

      {
         string dir, base;
         File::split("/dir1/dir2/base", dir, base);
         assertStrCmp(dir.c_str(), SEP "dir1" SEP "dir2");
         assertStrCmp(base.c_str(), "base");
      }

      {
         string dir, base;
         File::split("////", dir, base);
         assertStrCmp(dir.c_str(), SEP);
         assertStrCmp(base.c_str(), "");
      }

      {
         string root, ext;
         File::splitext("root.ext", root, ext);
         assertStrCmp(root.c_str(), "root");
         assertStrCmp(ext.c_str(), ".ext");
      }

      {
         string root, ext;
         File::splitext("root", root, ext);
         assertStrCmp(root.c_str(), "root");
         assertStrCmp(ext.c_str(), "");
      }

      {
         string root, ext;
         File::splitext(".ext", root, ext);
         assertStrCmp(root.c_str(), "");
         assertStrCmp(ext.c_str(), ".ext");
      }

      {
         string root, ext;
         File::splitext("", root, ext);
         assertStrCmp(root.c_str(), "");
         assertStrCmp(ext.c_str(), "");
      }

      {
         string root, ext;
         File::splitext("root.", root, ext);
         assertStrCmp(root.c_str(), "root");
         assertStrCmp(ext.c_str(), ".");
      }

      {
         assertStrCmp(File::dirname("/dir1/dir2/base").c_str(),
            SEP "dir1" SEP "dir2");
      }

      {
         assertStrCmp(File::basename("/dir1/dir2/base").c_str(), "base");
      }
   }
   tr.passIfNoException();

   tr.test("isPathAbsolute");
   {
      assert(File::isPathAbsolute("/"));
      assert(File::isPathAbsolute("/x"));
      assert(!File::isPathAbsolute(NULL));
      assert(!File::isPathAbsolute(""));
      assert(!File::isPathAbsolute("x"));
#ifdef WIN32
      assert(File::isPathAbsolute("/"));
      assert(File::isPathAbsolute("/abc"));
      assert(File::isPathAbsolute("\\"));
      assert(File::isPathAbsolute("\\abc"));
      assert(File::isPathAbsolute("a:"));
      assert(File::isPathAbsolute("A:\\"));
      assert(File::isPathAbsolute("A:\\abc"));
      assert(File::isPathAbsolute("A:/"));
      assert(File::isPathAbsolute("A:/abc"));
      assert(File::isPathAbsolute("c:/"));
      assert(File::isPathAbsolute("c:/abc"));
      assert(File::isPathAbsolute("z:/"));
      assert(File::isPathAbsolute("z:/abc"));
      assert(!File::isPathAbsolute("a:abc"));
#endif
   }
   tr.passIfNoException();

   tr.test("isPathRoot");
   {
      assert(File::isPathRoot("/"));
      assert(!File::isPathRoot("/x"));
      assert(!File::isPathRoot(NULL));
      assert(!File::isPathRoot(""));
      assert(!File::isPathRoot("x"));
#ifdef WIN32
      assert(File::isPathRoot("/"));
      assert(File::isPathRoot("\\"));
      assert(File::isPathRoot("a:"));
      assert(File::isPathRoot("A:\\"));
      assert(File::isPathRoot("A:/"));
      assert(File::isPathRoot("c:/"));
      assert(File::isPathRoot("z:/"));
#endif
   }
   tr.passIfNoException();

   tr.test("join");
   {
      {
         string path = File::join("", "");
         assertStrCmp(path.c_str(), "");
      }

      {
         string path = File::join(SEP, "");
         assertStrCmp(path.c_str(), SEP);
      }

      {
         string path = File::join("", SEP);
         assertStrCmp(path.c_str(), SEP);
      }

      {
         string path = File::join("a", "");
         assertStrCmp(path.c_str(), "a");
      }

      {
         string path = File::join("a", "b");
         assertStrCmp(path.c_str(), "a" SEP "b");
      }

      {
         string path1 = File::join(SEP, "a");
         string path2 = File::join(path1.c_str(), "b");
         assertStrCmp(path2.c_str(), SEP "a" SEP "b");
      }

      {
         string path = File::join(SEP "a", SEP "b");
         assertStrCmp(path.c_str(), SEP "a" SEP "b");
      }

      {
         string path = File::join("a" SEP, SEP "b" SEP);
         assertStrCmp(path.c_str(), "a" SEP "b" SEP);
      }

      {
         string path = File::join(SEP "a" SEP, SEP "b" SEP);
         assertStrCmp(path.c_str(), SEP "a" SEP "b" SEP);
      }

      {
         string path1 = File::join("a", "");
         string path2 = File::join(path1.c_str(), "b");
         assertStrCmp(path2.c_str(), "a" SEP "b");
      }

      {
         string path = File::join("", "a");
         assertStrCmp(path.c_str(), "a");
      }

      {
         string path = File::join("a", "");
         assertStrCmp(path.c_str(), "a");
      }

      {
         string path1 = File::join("", "a");
         string path2 = File::join(path1.c_str(), "");
         assertStrCmp(path2.c_str(), "a");
      }
   }
   tr.passIfNoException();
#if 0
   tr.test("mkdirs");
   {
      File file("/tmp/bmtest/foo/bar/testfile.txt");
      file->mkdirs();
   }
   tr.passIfNoException();
#endif

   tr.test("tmp file");
   {
      File file = File::createTempFile("tmp.");
      assert(!file.isNull());
      string path = file->getAbsolutePath();
      printf("CREATED TEMP FILE: '%s'... ", path.c_str());
      assert(file->exists());
      file.setNull();
      file = File(path.c_str());
      assert(!file->exists());
   }
   tr.passIfNoException();

   tr.test("create unique file");
   {
      File file = File::createTempFile("tmp.");
      assert(!file.isNull());
      string path = file->getAbsolutePath();
      file->remove();
      assert(file->createUnique());
      file->setRemoveOnCleanup(true);
   }
   tr.passIfNoException();

   tr.test("create unique file twice failure");
   {
      File file = File::createTempFile("tmp.");
      assert(!file.isNull());
      string path = file->getAbsolutePath();
      file->remove();
      assert(file->createUnique());
      file->setRemoveOnCleanup(true);
      assertNoExceptionSet();
      file->createUnique();
   }
   tr.passIfException();

   tr.test("parse path");
   {
#if WIN32
#else
      FileList files = File::parsePath("/tmp/a:/tmp/b\\:c::/tmp/d\\\\:/tmp/e");
      IteratorRef<File> i = files->getIterator();
      assert(i->hasNext());
      assertStrCmp(i->next()->getAbsolutePath(), "/tmp/a");
      assert(i->hasNext());
      assertStrCmp(i->next()->getAbsolutePath(), "/tmp/b\\:c");
      assert(i->hasNext());
      assertStrCmp(i->next()->getAbsolutePath(), "/tmp/d\\\\");
      assert(i->hasNext());
      assertStrCmp(i->next()->getAbsolutePath(), "/tmp/e");
      assert(!i->hasNext());
#endif
   }
   tr.passIfNoException();

   tr.ungroup();
}

static void runFileInputStreamTest(TestRunner& tr)
{
   tr.group("FileInputStream");

   File temp = File::createTempFile("fistestoutput");
   FileOutputStream fos(temp);
   const char* content =
      "This is for testing the skip method for a file input stream.";
   int contentLength = strlen(content);
   fos.write(content, contentLength);
   fos.close();

   tr.test("read");
   {
      FileInputStream fis(temp);
      char b[100];
      int numBytes = fis.read(b, 100);
      assert(numBytes == contentLength);
      b[numBytes] = 0;
      assertStrCmp(b, content);
      fis.close();
   }
   tr.passIfNoException();

   tr.test("read small");
   {
      FileInputStream fis(temp);
      char b[7];
      int numBytes;
      int total = 0;
      string input;
      while((numBytes = fis.read(b, 7)) > 0)
      {
         input.append(b, numBytes);
         total += numBytes;
      }
      assert(total == contentLength);
      assertStrCmp(input.c_str(), content);
      fis.close();
   }
   tr.passIfNoException();

   tr.test("read line");
   {
      File temp2 = File::createTempFile("fistestlines");
      FileOutputStream fos(temp2);
      const char* content2 = "Line 1\nLine 2\nLine 3\nLine 4";
      fos.write(content2, strlen(content2));
      fos.close();

      FileInputStream fis(temp2);
      string line;
      int count = 1;
      while((fis.readLine(line) > 0))
      {
         //printf("line=%s\n", line.c_str());
         switch(count)
         {
            case 1:
               assertStrCmp(line.c_str(), "Line 1");
               break;
            case 2:
               assertStrCmp(line.c_str(), "Line 2");
               break;
            case 3:
               assertStrCmp(line.c_str(), "Line 3");
               break;
            case 4:
               assertStrCmp(line.c_str(), "Line 4");
               break;
            default:
               assert(0);
         }
         ++count;
      }

      fis.close();
   }
   tr.passIfNoException();

   tr.test("not found");
   {
      File temp = File::createTempFile("dumb");
      temp->remove();
      FileInputStream fis(temp);
      char b[100];
      fis.read(b, 100);
      //dumpException();
      fis.close();
   }
   tr.passIfException();

   tr.test("skip");
   {
      FileInputStream fis(temp);
      char b[10];

      assert(fis.read(b, 4) == 4);
      b[4] = 0;
      assertStrCmp(b, "This");

      assert(fis.skip(4) == 4);

      assert(fis.read(b, 3) == 3);
      b[3] = 0;
      assertStrCmp(b, "for");

      assert(fis.skip(18) == 18);

      assert(fis.read(b, 6) == 6);
      b[6] = 0;
      assertStrCmp(b, "method");

      assert(fis.skip(12) == 12);

      assert(fis.read(b, 5) == 5);
      b[5] = 0;
      assertStrCmp(b, "input");

      assert(fis.skip(10) == 8);

      assert(fis.read(b, 3) == 0);

      fis.close();
   }
   tr.passIfNoException();

   tr.ungroup();
}

static void runTruncateInputStreamTest(TestRunner& tr)
{
   tr.group("TruncateInputStream");

   tr.test("truncate");
   {
      const char* test = "this is a test";
      ByteBuffer b;
      b.put(test, strlen(test), true);

      char buf[100];
      ByteArrayInputStream bais(&b);
      assert(bais.read(buf, 1) == 1);
      TruncateInputStream tis(3, &bais, false);
      assert(tis.read(buf, 100) == 3);
      assert(strncmp(buf, "his", 3) == 0);
   }
   tr.passIfNoException();

   tr.ungroup();
}

class ReadWatcher
{
public:
   ReadWatcher() {};
   virtual ~ReadWatcher() {};

   virtual void readUpdated(int fd, int events)
   {
      printf("FD '%d' is readable!\n", fd);
   }
};

static void runIOMonitorTest(TestRunner& tr)
{
   tr.group("IOMonitor");

   tr.test("watch read");
   {
      ReadWatcher rw;
      IOWatcherRef w = new IOEventDelegate<ReadWatcher>(
         &rw, &ReadWatcher::readUpdated);

      // FIXME:
      //FILE* fp = fopen("/tmp/testfile.txt", "rw");
      //assert(fp != NULL);
      //int fd = fileno(fp);
      int fd = 1;

      IOMonitor iom;
      iom.addWatcher(fd, 0, w);

      // FIXME: do stuff

      //fclose(fp);
   }
   tr.passIfNoException();

   tr.ungroup();
}

#undef SEP

static bool run(TestRunner& tr)
{
   if(tr.isDefaultEnabled())
   {
      runByteBufferTest(tr);
      runByteArrayInputStreamTest(tr);
      runByteArrayOutputStreamTest(tr);
      runBitStreamTest(tr);
      runFileTest(tr);
      runFileInputStreamTest(tr);
      runTruncateInputStreamTest(tr);
   }
   if(tr.isTestEnabled("timing"))
   {
      runStringEqualityTest(tr);
      runStringAppendCharTest(tr);
      runStringCompareTest(tr);
   }
   if(tr.isTestEnabled("memcpy-timing"))
   {
      runMemcpyTest(tr);
   }
   if(tr.isTestEnabled("io-monitor"))
   {
      runIOMonitorTest(tr);
   }

   return true;
}

} // end namespace

MO_TEST_MODULE_FN("monarch.tests.io.test", "1.0", mo_test_io::run)
