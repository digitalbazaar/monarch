/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */

#include "db/test/Test.h"
#include "db/test/Tester.h"
#include "db/test/TestRunner.h"
#include "db/io/IOMonitor.h"
#include "db/io/File.h"
#include "db/io/FileInputStream.h"
#include "db/io/FileOutputStream.h"
#include "db/io/FileList.h"
#include "db/io/FilterOutputStream.h"
#include "db/io/BitStream.h"
#include "db/io/BufferedOutputStream.h"
#include "db/io/ByteArrayInputStream.h"
#include "db/io/ByteArrayOutputStream.h"
#include "db/io/ByteBuffer.h"
#include "db/io/MutatorInputStream.h"
#include "db/io/MutatorOutputStream.h"
#include "db/io/TruncateInputStream.h"
#include "db/rt/System.h"
#include "db/util/StringTools.h"

#include <iostream>
#include <sstream>
#include <cstdlib>

using namespace std;
using namespace db::test;
using namespace db::io;
using namespace db::rt;
using namespace db::util;

// redefine this here to make testing easier via cpp string concatination
#ifdef WIN32
#define SEP "\\"
#else
#define SEP "/"
#endif

#ifdef WIN32
#define TMPDIR "c:/WINDOWS/Temp"
#else
#define TMPDIR "/tmp"
#endif

void runStringEqualityTest(TestRunner& tr)
{
   cout << "Starting string equality test." << endl << endl;
   
   // Note: string length doesn't appear to matter
   string str = "blah";
   unsigned long long start, end;
   
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; i++)
   {
      if(str == "");
   }
   end = System::getCurrentMilliseconds();
   cout << "String == \"\" time: " << (end - start) << " ms" << endl;
   
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; i++)
   {
      if(str.length() == 0);
   }
   end = System::getCurrentMilliseconds();
   cout << "String.length() == 0 time: " << (end - start) << " ms" << endl;
   
   // Note: test demonstrates that comparing to length is about 6 times faster
   
   cout << endl << "String equality test complete." << endl;
}

void runStringAppendCharTest(TestRunner& tr)
{
   cout << "Starting string append char test." << endl << endl;
   
   // Note: string length doesn't appear to matter
   string str = "blah";
   unsigned long long start, end;
   
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; i++)
   {
      if(str.length() == 1 && str[0] == '/');
   }
   end = System::getCurrentMilliseconds();
   cout << "String.length() == 1 && str[0] == '/' time: " <<
      (end - start) << " ms" << endl;
   
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; i++)
   {
      if(str == "/");
   }
   end = System::getCurrentMilliseconds();
   cout << "String == \"/\" time: " << (end - start) << " ms" << endl;
   
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; i++)
   {
      if(strcmp(str.c_str(), "/") == 0);
   }
   end = System::getCurrentMilliseconds();
   cout << "strcmp(String.c_str(), \"/\") == 0 time: " <<
      (end - start) << " ms" << endl;
   
   string version = "HTTP/1.0";
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; i++)
   {
      if(version == "HTTP/1.0");
   }
   end = System::getCurrentMilliseconds();
   cout << "String == \"HTTP/1.0\" time: " << (end - start) << " ms" << endl;
   
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; i++)
   {
      if(strcmp(version.c_str(), "HTTP/1.0") == 0);
   }
   end = System::getCurrentMilliseconds();
   cout << "strcmp(String.c_str(), \"HTTP/1.0\") == 0 time: " <<
      (end - start) << " ms" << endl;
   
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 10000; i++)
   {
      str.append(1, '/');
   }
   end = System::getCurrentMilliseconds();
   cout << "String.append(1, '/') time: " << (end - start) << " ms" << endl;
   
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 10000; i++)
   {
      str.append("/");
   }
   end = System::getCurrentMilliseconds();
   cout << "String.append(\"/\") time: " << (end - start) << " ms" << endl;
   
   string space = " ";
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 10000; i++)
   {
      str.append("this" + space + "is a sentence");
   }
   end = System::getCurrentMilliseconds();
   cout << "String inline append time: " << (end - start) << " ms" << endl;
   
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 10000; i++)
   {
      str.append("this");
      str.append(space);
      str.append("is a sentence");
   }
   end = System::getCurrentMilliseconds();
   cout << "String multiline append time: " << (end - start) << " ms" << endl;
   
   cout << endl << "String append char test complete." << endl;
}

void runStringCompareTest(TestRunner& tr)
{
   cout << "Starting string compare test." << endl << endl;
   
   string str1 = "blah";
   char str2[] = "blah";
   unsigned long long start, end;
   
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; i++)
   {
      if(str1 == "blah");
   }
   end = System::getCurrentMilliseconds();
   cout << "std::string compare time: " << (end - start) << " ms" << endl;
   
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; i++)
   {
      if(strcmp(str2, "blah") == 0);
   }
   end = System::getCurrentMilliseconds();
   cout << "char* compare time: " << (end - start) << " ms" << endl;
   
   cout << endl << "String compare test complete." << endl;
}

void runByteBufferTest(TestRunner& tr)
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
   sprintf(b.data() + b.length(), " always");
   char temp[100];
   strncpy(temp, b.data(), b.length());
   assertStrCmp(temp, "T hate chicken");
   
   // this should now result in printing out "T hate chicken always"
   sprintf(b.data() + b.length() - 1, " always");
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

void runByteArrayInputStreamTest(TestRunner& tr)
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

void runByteArrayOutputStreamTest(TestRunner& tr)
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
      IOException* e = (IOException*)&(*Exception::getLast());
      //cout << "Exception Caught=" << e->getMessage() << endl;
      //cout << "Written bytes=" << e->getUsedBytes() << endl;
      //cout << "Unwritten bytes=" << e->getUnusedBytes() << endl;
      //cout << "Turning on resize and finishing write..." << endl;
      
      // turn on resize
      baos2.setResize(true);
      
      // write remaining bytes
      baos2.write(sentence + e->getUsedBytes(), e->getUnusedBytes());
      
      // clear exception
      Exception::clearLast();
   }
   
   assertStrCmp(b.data(), "T hate chickenThis is a sentence.");
   
   tr.passIfNoException();
}

void runBitStreamTest(TestRunner& tr)
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

void runFileTest(TestRunner& tr)
{
   tr.group("File");
   
   File cdir(".");
   
#ifdef WIN32
   const char* name = ".";
   const char* tmpFilename = "C:\\windows\\temp";
   const char* tmpFilenameA = "C:\\windows\\temp\\DBCORETEST_a.txt";
   const char* tmpFilenameB = "C:\\windows\\temp\\dir\\..\\file.txt";
   const char* expectTmpFilenameB = "C:\\windows\\temp\\file.txt";
   const char* tmpFilenameC = "C:\\windows\\temp\\DBCORETEST_c.txt";
#else
   const char* name = "/tmp";
   const char* tmpFilename = "/tmp";
   const char* tmpFilenameA = "/tmp/DBCORETEST_a.txt";
   const char* tmpFilenameB = "/tmp/dir/../file.txt";
   const char* expectTmpFilenameB = "/tmp/file.txt";
   const char* tmpFilenameC = "/tmp/DBCORETEST_c.txt";
#endif
   
   File tmp(tmpFilename);
   File tmpFileA(tmpFilenameA);
   File tmpFileB(tmpFilenameB);
   File tmpFileC(tmpFilenameC);
   File junk("../../foo/../junk238jflk38sjf.txt");
   string np;
   
   tr.test("absolute paths");
   {
      assertStrCmp(tmp->getAbsolutePath(), tmpFilename);
      assertStrCmp(tmpFileA->getAbsolutePath(), tmpFilenameA);
      assertStrCmp(tmpFileB->getAbsolutePath(), expectTmpFilenameB);
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
      File dir(name);
      FileList files;
      dir->listFiles(files);
      
      //printf("/tmp contains %i files\n", files.count())";
      
      assert(files->count() > 0);
   }
   tr.passIfNoException();
   
   tr.test("get type");
   {
      File dir(name);
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
      tmpFileA->rename(tmpFileC);
      assert(!tmpFileA->exists());
      assert(tmpFileC->exists());
   }
   tr.passIfNoException();
   
   tr.test("expandUser");
   {
#ifdef WIN32
      const char* HOMEDRIVE = getenv("HOMEDRIVE");
      const char* HOMEPATH = getenv("HOMEPATH");
      assert(HOMEDRIVE != NULL);
      assert(HOMEPATH != NULL);
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
         assertException();
         Exception::clearLast();
      }
#endif

#ifdef WIN32
      // can not set HOME* vars on Windows so use real values 
      char homestr[strlen(HOMEDRIVE) + strlen(HOMEPATH) + 1];
      sprintf(homestr, "%s%s", HOMEDRIVE, HOMEPATH);
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
         assert(File::expandUser("%HOMEDRIVE%%HOMEPATH%/foo.txt", path));
         string tmp;
         tmp.append(HOMEDRIVE);
         tmp.append(HOMEPATH);
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
         assertException();
         Exception::clearLast();
      }
      
      {
         string path;
         assert(!File::expandUser("~user/foo.txt", path));
         assertException();
         Exception::clearLast();
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
   tr.ungroup();
}

void runFileInputStreamTest(TestRunner& tr)
{
   tr.group("FileInputStream");
   
   File temp(TMPDIR "/fistestoutput.txt");
   FileOutputStream fos(temp);
   const char* content =
      "This is for testing the skip method for a file input stream.";
   fos.write(content, strlen(content));
   fos.close();
   
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
   }
   tr.passIfNoException();
   
   tr.ungroup();
}

void runTruncateInputStreamTest(TestRunner& tr)
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

void runIOMonitorTest(TestRunner& tr)
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

class DbIoTester : public db::test::Tester
{
public:
   DbIoTester()
   {
      setName("io");
   }

   /**
    * Run automatic unit tests.
    */
   virtual int runAutomaticTests(TestRunner& tr)
   {
      runByteBufferTest(tr);
      runByteArrayInputStreamTest(tr);
      runByteArrayOutputStreamTest(tr);
      runBitStreamTest(tr);
      runFileTest(tr);
      runFileInputStreamTest(tr);
      runTruncateInputStreamTest(tr);
      //runIOMonitorTest(tr);
      return 0;
   }

   /**
    * Runs interactive unit tests.
    */
   virtual int runInteractiveTests(TestRunner& tr)
   {
      runStringEqualityTest(tr);
      runStringAppendCharTest(tr);
      runStringCompareTest(tr);
      return 0;
   }
};

#undef SEP
#undef TMPDIR

#ifndef DB_TEST_NO_MAIN
DB_TEST_MAIN(DbIoTester)
#endif
