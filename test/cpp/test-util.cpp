/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */

#include "db/test/Test.h"
#include "db/test/Tester.h"
#include "db/test/TestRunner.h"
#include "db/util/Base64Codec.h"
#include "db/util/Crc16.h"
#include "db/util/StringTools.h"
#include "db/util/Convert.h"
#include "db/util/regex/Pattern.h"
#include "db/util/Date.h"
#include "db/util/StringTokenizer.h"
#include "db/util/UniqueList.h"

#include <cstdlib>
#include <iostream>

using namespace std;
using namespace db::test;
using namespace db::rt;
using namespace db::util;
using namespace db::util::regex;

void runBase64Test(TestRunner& tr)
{
   const char* expected = "YmNkZQ==";

   tr.test("Base64");
   
   char data[] = {'a', 'b', 'c', 'd', 'e'};
   string encoded = Base64Codec::encode(data + 1, 4);
   assert(encoded == expected);
   
   char* decoded;
   unsigned int length;
   Base64Codec::decode(encoded.c_str(), &decoded, length);
   assert(decoded != NULL);
   
   assert(length == 4);
   for(unsigned int i = 0; i < length; i++)
   {
      assert(decoded[i] == data[i + 1]);
   }
   
   string encoded2 = Base64Codec::encode(decoded, 4);
   assertStrCmp(encoded2.c_str(), expected);
   free(decoded);
   
   unsigned int size = 144;
   string large;
   large.append(size, 0x01);
   encoded = Base64Codec::encode(large.c_str(), size);
   Base64Codec::decode(encoded.c_str(), &decoded, length);
   assert(memcmp(decoded, large.c_str(), size) == 0);
   assert(length == size);
   free(decoded);
   
   size = 145;
   large.erase();
   large.append(size, 0x01);
   encoded = Base64Codec::encode(large.c_str(), size);
   Base64Codec::decode(encoded.c_str(), &decoded, length);
   assert(memcmp(decoded, large.c_str(), size) == 0);
   assert(length == size);
   free(decoded);
   
   tr.pass();
}

void runCrcTest(TestRunner& tr)
{
   tr.group("CRC");
   
   unsigned int correctValue = 6013;
   
   tr.test("single value update");   
   Crc16 crc16s;
   crc16s.update(10);
   crc16s.update(20);
   crc16s.update(30);
   crc16s.update(40);
   crc16s.update(50);
   crc16s.update(60);
   crc16s.update(70);
   crc16s.update(80);
   assert(crc16s.getChecksum() == correctValue);
   tr.pass();
   
   tr.test("array update");   
   Crc16 crc16a;
   char b[] = {10, 20, 30, 40, 50, 60, 70, 80};
   crc16a.update(b, 8);
   //cout << "CRC-16=" << crc16.getChecksum() << endl;
   assert(crc16a.getChecksum() == correctValue);
   tr.pass();

   tr.ungroup();
}

void runConvertTest(TestRunner& tr)
{
   tr.test("Convert");
   
   // convert to hex
   char data[] = "abcdefghiABCDEFGZXYW0123987{;}*%6,./.12`~";
   string original(data, strlen(data));
   
   //cout << "test data=" << original << endl;
   
   string lowerHex = Convert::bytesToHex(data, strlen(data));
   string upperHex = Convert::bytesToUpperHex(data, strlen(data));
   
   assertStrCmp(lowerHex.c_str(),"616263646566676869414243444546475a585957303132333938377b3b7d2a25362c2e2f2e3132607e");
   assert(lowerHex.length() == 82);
   assertStrCmp(upperHex.c_str(),"616263646566676869414243444546475A585957303132333938377B3B7D2A25362C2E2F2E3132607E");
   assert(upperHex.length() == 82);
   
   char decoded1[lowerHex.length() / 2];
   char decoded2[upperHex.length() / 2];
   
   unsigned int length1;
   unsigned int length2;
   Convert::hexToBytes(lowerHex.c_str(), lowerHex.length(), decoded1, length1);
   Convert::hexToBytes(upperHex.c_str(), upperHex.length(), decoded2, length2);
   
   string ascii1(decoded1, length1);
   string ascii2(decoded2, length2);
   
   assertStrCmp(ascii1.c_str(), data);
   assert(length1 == strlen(data));
   assertStrCmp(ascii2.c_str(), data);
   assert(length2 == strlen(data));
   
   assert(ascii1 == ascii2);
   assert(ascii1 == original);
   
   assertStrCmp(Convert::intToHex(10).c_str(), "0a");
   assertStrCmp(Convert::intToHex(33).c_str(), "21");
   assertStrCmp(Convert::intToHex(100).c_str(), "64");
   assertStrCmp(Convert::intToUpperHex(10).c_str(), "0A");
   assertStrCmp(Convert::intToUpperHex(33).c_str(), "21");
   assertStrCmp(Convert::intToUpperHex(100).c_str(), "64");
   assertStrCmp(Convert::intToHex(8975).c_str(), "230f");
   assertStrCmp(Convert::intToUpperHex(8975).c_str(), "230F");
   assertStrCmp(Convert::intToHex(65537).c_str(), "010001");
   assertStrCmp(Convert::intToUpperHex(65537).c_str(), "010001");
   
   {
      unsigned int ui;
      string hex;
      
      hex = "230f";
      assert(Convert::hexToInt(hex.c_str(), hex.length(), ui));
      assert(ui == 8975);
      
      hex = "230F";
      assert(Convert::hexToInt(hex.c_str(), hex.length(), ui));
      assert(ui == 8975);
      
      hex = "230FABCD";
      assert(Convert::hexToInt(hex.c_str(), hex.length(), ui));
      assert(ui == 588229581);
      
      hex = "0";
      assert(Convert::hexToInt(hex.c_str(), hex.length(), ui));
      assert(ui == 0x0);
      
      hex = "d";
      assert(Convert::hexToInt(hex.c_str(), hex.length(), ui));
      assert(ui == 0xd);
      
      hex = "fab";
      assert(Convert::hexToInt(hex.c_str(), hex.length(), ui));
      assert(ui == 0xfab);
      
      hex = "0141";
      assert(Convert::hexToInt(hex.c_str(), hex.length(), ui));
      assert(ui == 0x141);
      
      // bad hex
      hex = "x";
      assert(!Convert::hexToInt(hex.c_str(), hex.length(), ui));
      assertException();
      Exception::clearLast();

      // too big
      hex = "876543210";
      assert(!Convert::hexToInt(hex.c_str(), hex.length(), ui));
      assertException();
      Exception::clearLast();
   }
   
   {
      unsigned int length;
      string hex;
      char bytes[100]; 

      hex = "0";
      assert(Convert::hexToBytes(hex.c_str(), hex.length(), bytes, length));
      assert(length == 1);
      assert(bytes[0] == 0x0);

      hex = "d";
      assert(Convert::hexToBytes(hex.c_str(), hex.length(), bytes, length));
      assert(length == 1);
      assert(bytes[0] == 0xd);

      hex = "230f";
      assert(Convert::hexToBytes(hex.c_str(), hex.length(), bytes, length));
      assert(length == 2);
      assert(bytes[0] == 0x23);
      assert(bytes[1] == 0x0f);
   }

   tr.passIfNoException();
}

void runRegexTest(TestRunner& tr)
{
   tr.group("Regex");

   {
      tr.test("match");
      assert(Pattern::match("^[a-z]{3}$", "abc"));
      assert(Pattern::match("^[a-zA-Z0-9_]+$", "username"));
      tr.passIfNoException();
   }
   
   {
      tr.test("no match");
      assert(!Pattern::match("^[a-z]{3}$", "abcd"));
      assert(!Pattern::match("^[a-z]{3}$", "ABC"));
      assert(!Pattern::match("^[a-zA-Z0-9_]+$", "user name"));
      tr.passIfNoException();
   }

   {
      tr.test("sub-match");
   
      string submatches = "Look for green globs of green matter in green goo.";
      Pattern* p = Pattern::compile("green");
   
      unsigned int start, end;
      unsigned int index = 0;

      assert(p->match(submatches.c_str(), index, start, end));
      assert(start == 9);
      assert(end == 14);
      assertStrCmp(submatches.substr(start, end - start).c_str(), "green");
      index = end;

      assert(p->match(submatches.c_str(), index, start, end));
      assert(start == 24);
      assert(end == 29);
      assertStrCmp(submatches.substr(start, end - start).c_str(), "green");
      index = end;

      assert(p->match(submatches.c_str(), index, start, end));
      assert(start == 40);
      assert(end == 45);
      assertStrCmp(submatches.substr(start, end - start).c_str(), "green");
      index = end;

      assert(!p->match(submatches.c_str(), index, start, end));
      
      delete p;
      tr.passIfNoException();
   }
   
   {
      tr.test("replace all");
      
      string str = "Look for green globs of green matter in green goo.";
      string exp = "Look for blue globs of blue matter in blue goo.";
      StringTools::regexReplaceAll(str, "green", "blue");
      assertStrCmp(str.c_str(), exp.c_str());
   
      tr.passIfNoException();
   }

   tr.ungroup();
}

void runDateTest(TestRunner& tr)
{
   cout << "Starting Date test." << endl << endl;
   
   TimeZone gmt = TimeZone::getTimeZone("GMT");
   TimeZone local = TimeZone::getTimeZone();
   
   Date d;
   string str;
   //d.format(str);
   //d.format(str, "E EEEE d dd M MMMM MM yy w ww yyyy a", "java");
   //d.format(str, "EEEE, MMMM dd yyyy hh:mm:ss a", "java");
   //d.format(str, "EEE, MMMM dd yyyy hh:mm:ss a", "java", &local);
   //d.format(str, "EEE, d MMM yyyy HH:mm:ss", "java", &gmt);
   //d.format(str, "%a, %d %b %Y %H:%M:%S");
   d.format(str, "%a, %d %b %Y %H:%M:%S", "c", &gmt);
   //d.format(str, "%a, %d %b %Y %H:%M:%S", "c", &local);
   
   cout << "Current Date: " << str << endl;
   
   // parse date
   Date d2;
   d2.parse(str, "%a, %d %b %Y %H:%M:%S", "c", &gmt);
   //d2.parse(str, "%a, %d %b %Y %H:%M:%S", "c", &local);
   string str2;
   d2.format(str2, "%a, %d %b %Y %H:%M:%S", "c", &gmt);
   //d2.format(str2, "%a, %d %b %Y %H:%M:%S", "c", &local);
   
   cout << "Parsed Date 1: " << str2 << endl;
   
//   // FIXME: parser may have a problem with AM/PM
   // parse date again
   Date d3;
   str = "Thu, 02 Aug 2007 10:30:00";
   d3.parse(str, "%a, %d %b %Y %H:%M:%S", "c", &gmt);
   string str3;
   //d3.format(str3, "%a, %d %b %Y %H:%M:%S", "c", &gmt);
   d3.format(str3, "%a, %d %b %Y %H:%M:%S", "c", &local);
   
   cout << "Parsed Date 2: " << str3 << endl;
   
   cout << endl << "Date test complete." << endl;
}

void runStringTokenizerTest(TestRunner& tr)
{
   tr.test("StringTokenizer");
   
   const char* str = "This is a test of the StringTokenizer class.";
   
   /*
   StringTokenizer st0(str, ' ');
   while(st0.hasNextToken())
   {
      cout << "token='" << st0.nextToken() << "'" << endl;
   }
   */
   StringTokenizer st(str, ' ');
   #define NT(str) \
      do { \
         assert(st.hasNextToken()); \
         assertStrCmp(st.nextToken(), str); \
      } while(0)
   NT("This");
   NT("is");
   NT("a");
   NT("test");
   NT("of");
   NT("the");
   NT("StringTokenizer");
   NT("class.");
   assert(!st.hasNextToken());
   #undef NT
   
   tr.passIfNoException();
}

void runUniqueListTest(TestRunner& tr)
{
   tr.test("UniqueList");
   
   UniqueList<int> list;
   
   list.add(5);
   list.add(6);
   list.add(7);
   list.add(5);
   
   Iterator<int>* i = list.getIterator();
   /*
   while(i->hasNext())
   {
      cout << "element=" << i->next() << endl;
   }
   */
   assert(i->hasNext());
   assert(i->next() == 5);
   assert(i->hasNext());
   assert(i->next() == 6);
   assert(i->hasNext());
   assert(i->next() == 7);
   assert(!i->hasNext());
   delete i;
   
   list.remove(5);
   
   i = list.getIterator();
   /*
   while(i->hasNext())
   {
      cout << "element=" << i->next() << endl;
   }
   */
   assert(i->hasNext());
   assert(i->next() == 6);
   assert(i->hasNext());
   assert(i->next() == 7);
   assert(!i->hasNext());
   delete i;
   
   list.clear();
   
   i = list.getIterator();
   assert(!i->hasNext());
   delete i;

   tr.passIfNoException();
}

class DbUtilTester : public db::test::Tester
{
public:
   DbUtilTester()
   {
      setName("util");
   }

   /**
    * Run automatic unit tests.
    */
   virtual int runAutomaticTests(TestRunner& tr)
   {
      runBase64Test(tr);
      runCrcTest(tr);
      runConvertTest(tr);
      runStringTokenizerTest(tr);
      runUniqueListTest(tr);
      runRegexTest(tr);
      return 0;
   }

   /**
    * Runs interactive unit tests.
    */
   virtual int runInteractiveTests(TestRunner& tr)
   {
//      runDateTest(tr);
      return 0;
   }
};

#ifndef DB_TEST_NO_MAIN
DB_TEST_MAIN(DbUtilTester)
#endif
