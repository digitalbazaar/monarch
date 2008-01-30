/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include <iostream>
#include <sstream>

#include "db/test/Test.h"
#include "db/test/Tester.h"
#include "db/test/TestRunner.h"
#include "db/io/File.h"
#include "db/io/FileInputStream.h"
#include "db/io/FileOutputStream.h"
#include "db/io/FileList.h"
#include "db/io/FilterOutputStream.h"
#include "db/io/ByteArrayInputStream.h"
#include "db/io/ByteArrayOutputStream.h"
#include "db/io/ByteBuffer.h"
#include "db/io/BufferedOutputStream.h"
#include "db/io/MutatorInputStream.h"
#include "db/io/MutatorOutputStream.h"
#include "db/rt/System.h"

using namespace std;
using namespace db::test;
using namespace db::io;
using namespace db::rt;
using namespace db::util;

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
   b.put(' ', true);
   b.put('t', true);
   b.put('r', true);
   b.put('u', true);
   b.put('e', true);
   b.put(0x00, true);
   strncpy(temp, b.data(), b.length());
   assertStrCmp(temp, "T hate chicken always true");
   
   unsigned char aByte;
   b.get(aByte);
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
      IOException* e = (IOException*)Exception::getLast();
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

void runFileTest(TestRunner& tr)
{
   const char* name = "/tmp";

   File cdir(".");
   File tmp("/tmp");   
   File a("/tmp/a.txt");
   File b("../../foo/../junk238jflk38sjf.txt");
   string np;

   tr.test("File/normalization");
   
   File::normalizePath(&b, np);
   cout << np << "... ";

   tr.passIfNoException();

   tr.test("File/readable #1");
   
   File::normalizePath(&cdir, np);
   
   cout << np << " should be readable...";
   
   assert(cdir.isReadable());
   tr.passIfNoException();

   tr.test("File/readable #2");

   File::normalizePath(&b, np);
   cout << np << " should not be readable...";
   
   assert(b.isReadable() == false);
   tr.passIfNoException();

   tr.test("File/writable");

   File::normalizePath(&cdir, np);
   cout << np << " should be writable...";
   assert(cdir.isWritable());
   tr.passIfNoException();

   tr.test("File/directory containment");
   
   assert(tmp.contains(&a));
   assert(a.contains(&tmp) == false);
   
   tr.passIfNoException();
   tr.test("File/directory list");
   
   File dir(name);
   FileList files(true);
   dir.listFiles(&files);
   
   cout << "/tmp contains " << files.count() << " files...";
   
   assert(files.count() > 0);
   
   tr.passIfNoException();
   tr.test("File/get type");
   
   Iterator<File*>* i = files.getIterator();
   while(i->hasNext())
   {
      File* file = i->next();
      const char* type;
      switch(file->getType())
      {
         case File::RegularFile:
            type = "Regular File";
            break;
         case File::Directory:
            type = "Directory";
            break;
         case File::SymbolicLink:
            type = "Symbolic Link";
            break;
         default:
            type = "Unknown";
            break;
      }
      //cout << "Name: '" << file->getName() << "', Type: " << type << endl;
   }
   delete i;
   
   tr.passIfNoException();
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
      runFileTest(tr);
      return 0;
   }

   /**
    * Runs interactive unit tests.
    */
   virtual int runInteractiveTests(TestRunner& tr)
   {
//      runStringEqualityTest(tr);
//      runStringAppendCharTest(tr);
//      runStringCompareTest(tr);
      return 0;
   }
};

#ifndef DB_TEST_NO_MAIN
DB_TEST_MAIN(DbIoTester)
#endif
