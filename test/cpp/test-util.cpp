/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include <iostream>

#include "db/test/Test.h"
#include "db/test/Tester.h"
#include "db/test/TestRunner.h"
#include "db/util/Base64Codec.h"
#include "db/util/Crc16.h"
#include "db/util/StringTools.h"
#include "db/util/DynamicObject.h"
#include "db/util/DynamicObjectIterator.h"
#include "db/util/Convert.h"
#include "db/util/regex/Pattern.h"
#include "db/util/Date.h"
#include "db/util/StringTokenizer.h"
#include "db/util/UniqueList.h"

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
   
   string hex = "230f";
   assert(Convert::hexToInt(hex.c_str(), hex.length()) == 8975);
   hex = "230F";
   assert(Convert::hexToInt(hex.c_str(), hex.length()) == 8975);
   hex = "230FABCD";
   assert(Convert::hexToInt(hex.c_str(), hex.length()) == 588229581);
   hex = "0";
   assert(Convert::hexToInt(hex.c_str(), hex.length()) == 0);
   
   tr.passIfNoException();
}

void runRegexTest(TestRunner& tr)
{
   tr.test("Regex");
   
   string regex = "[a-z]{3}";
   string str = "abc";
   
   assert(Pattern::match(regex.c_str(), str.c_str()));
   
   cout << endl << "Doing sub-match test..." << endl << endl;
   
   string submatches = "Look for green globs of green matter in green goo.";
   Pattern* p = Pattern::compile("green");
   
   unsigned int start, end;
   unsigned int index = 0;
   while(p->match(submatches.c_str(), index, start, end))
   {
      cout << "Found match at (" << start << ", " << end << ")" << endl;
      cout << "Match=" << submatches.substr(start, end - start) << endl;
      index = end;
   }
   
   delete p;
   
   cout << endl << "Doing replace all test..." << endl << endl;
   
   cout << "change 'green' to 'blue'" << endl;
   cout << submatches << endl;
   StringTools::regexReplaceAll(submatches, "green", "blue");
   cout << submatches << endl;
   
   tr.passIfNoException();
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

void runDynamicObjectTest(TestRunner& tr)
{
   tr.test("DynamicObject");
   
   DynamicObject dyno1;
   dyno1["id"] = 2;
   dyno1["username"] = "testuser1000";
   dyno1["somearray"][0] = "item1";
   dyno1["somearray"][1] = "item2";
   dyno1["somearray"][2] = "item3";
   
   DynamicObject dyno2;
   dyno2["street"] = "1700 Kraft Dr.";
   dyno2["zip"] = "24060";
   
   dyno1["address"] = dyno2;
   
   assert(dyno1["id"]->getInt32() == 2);
   assertStrCmp(dyno1["username"]->getString(), "testuser1000");
   
   assertStrCmp(dyno1["somearray"][0]->getString(), "item1");
   assertStrCmp(dyno1["somearray"][1]->getString(), "item2");
   assertStrCmp(dyno1["somearray"][2]->getString(), "item3");
   
   DynamicObject dyno3 = dyno1["address"];
   assertStrCmp(dyno3["street"]->getString(), "1700 Kraft Dr.");
   assertStrCmp(dyno3["zip"]->getString(), "24060");
   
   DynamicObject dyno4;
   dyno4["whatever"] = "test";
   dyno4["someboolean"] = true;
   assert(dyno4["someboolean"]->getBoolean());
   dyno1["somearray"][3] = dyno4;
   
   dyno1["something"]["strange"] = "tinypayload";
   assertStrCmp(dyno1["something"]["strange"]->getString(), "tinypayload");
   
   DynamicObject dyno5;
   dyno5[0] = "mustard";
   dyno5[1] = "ketchup";
   dyno5[2] = "pickles";
   
   int count = 0;
   DynamicObjectIterator i = dyno5.getIterator();
   while(i->hasNext())
   {
      DynamicObject next = i->next();
      
      if(count == 0)
      {
         assertStrCmp(next->getString(), "mustard");
      }
      else if(count == 1)
      {
         assertStrCmp(next->getString(), "ketchup");
      }
      else if(count == 2)
      {
         assertStrCmp(next->getString(), "pickles");
      }
      
      count++;
   }
   
   DynamicObject dyno6;
   dyno6["eggs"] = "bacon";
   dyno6["milk"] = "yum";
   assertStrCmp(dyno6->removeMember("milk")->getString(), "yum");
   count = 0;
   i = dyno6.getIterator();
   while(i->hasNext())
   {
      DynamicObject next = i->next();
      assertStrCmp(i->getName(), "eggs");
      assertStrCmp(next->getString(), "bacon");
      count++;
   }
   
   assert(count == 1);
   
   // test clone
   dyno1["dyno5"] = dyno5;
   dyno1["dyno6"] = dyno6;
   dyno1["clone"] = dyno1.clone();
   
   DynamicObject clone = dyno1.clone();
   assert(dyno1 == clone);
   
   // test subset
   clone["mrmessy"] = "weirdguy";
   assert(dyno1.isSubset(clone));
   
   // test print out code
   //cout << endl;
   //dumpDynamicObject(dyno1);
   
   tr.pass();
}

void runDynoClearTest(TestRunner& tr)
{
   tr.test("DynamicObject clear");
   
   DynamicObject d;
   
   d = "x";
   assert(d->getType() == String);
   d->clear();
   assert(d->getType() == String);
   assertStrCmp(d->getString(), "");
   
   d = (int)1;
   assert(d->getType() == Int32);
   d->clear();
   assert(d->getType() == Int32);
   assert(d->getInt32() == 0);
   
   d = (unsigned int)1;
   assert(d->getType() == UInt32);
   d->clear();
   assert(d->getType() == UInt32);
   assert(d->getBoolean() == false);
   
   d = (long long)1;
   assert(d->getType() == Int64);
   d->clear();
   assert(d->getType() == Int64);
   assert(d->getInt64() == 0);
   
   d = (unsigned long long)1;
   d->clear();
   assert(d->getType() == UInt64);
   assert(d->getUInt64() == 0);
   
   d = (double)1.0;
   d->clear();
   assert(d->getType() == Double);
   assert(d->getDouble() == 0.0);
   
   d["x"] = 0;
   d->clear();
   assert(d->getType() == Map);
   assert(d->length() == 0);
   
   d[0] = 0;
   d->clear();
   assert(d->getType() == Array);
   assert(d->length() == 0);
   
   tr.passIfNoException();
}

void runDynoConversionTest(TestRunner& tr)
{
   tr.test("DynamicObject conversion");
   
   DynamicObject d;
   d["int"] = 2;
   d["-int"] = -2;
   d["str"] = "hello";
   d["true"] = "true";
   d["false"] = "false";
   
   string s;
   s.clear();
   d["int"]->toString(s);
   assertStrCmp(s.c_str(), "2");

   s.clear();
   d["-int"]->toString(s);
   assertStrCmp(s.c_str(), "-2");

   s.clear();
   d["str"]->toString(s);
   assertStrCmp(s.c_str(), "hello");

   s.clear();
   d["true"]->toString(s);
   assertStrCmp(s.c_str(), "true");

   s.clear();
   d["false"]->toString(s);
   assertStrCmp(s.c_str(), "false");
   
   tr.pass();
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
      setName("Util");
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
      runDynamicObjectTest(tr);
      runDynoClearTest(tr);
      runDynoConversionTest(tr);
      runUniqueListTest(tr);
      return 0;
   }

   /**
    * Runs interactive unit tests.
    */
   virtual int runInteractiveTests(TestRunner& tr)
   {
//      runRegexTest(tr);
//      runDateTest(tr);
      return 0;
   }
};

#ifndef DB_TEST_NO_MAIN
DB_TEST_MAIN(DbUtilTester)
#endif
