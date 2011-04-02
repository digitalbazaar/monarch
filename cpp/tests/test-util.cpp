/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_FORMAT_MACROS

#include "monarch/data/json/JsonWriter.h"
#include "monarch/test/Test.h"
#include "monarch/test/TestModule.h"
#include "monarch/util/AnsiEscapeCodes.h"
#include "monarch/util/Base64Codec.h"
#include "monarch/util/Convert.h"
#include "monarch/util/Crc16.h"
#include "monarch/util/Date.h"
#include "monarch/util/PathFormatter.h"
#include "monarch/util/Pattern.h"
#include "monarch/util/Random.h"
#include "monarch/util/RateAverager.h"
#include "monarch/util/StringTools.h"
#include "monarch/util/StringTokenizer.h"
#include "monarch/util/Timer.h"
#include "monarch/util/UniqueList.h"

#include <cstdlib>
#include <cstdio>

using namespace std;
using namespace monarch::test;
using namespace monarch::rt;
using namespace monarch::util;

namespace mo_test_util
{

static void runBase64Test(TestRunner& tr)
{
   tr.group("Base64");

   tr.test("basic");
   {
      const char* expected = "YmNkZQ==";

      char data[] = {'a', 'b', 'c', 'd', 'e'};
      string encoded = Base64Codec::encode(data + 1, 4);
      assert(encoded == expected);

      char* decoded;
      unsigned int length;
      Base64Codec::decode(encoded.c_str(), &decoded, length);
      assert(decoded != NULL);

      assert(length == 4);
      for(unsigned int i = 0; i < length; ++i)
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
   }
   tr.pass();

   tr.test("basic2");
   {
      const char* data = "abcdefghijklmnopqrstuvwxyz";
      string enc = Base64Codec::encode(data, strlen(data));
      char* dec;
      unsigned int len;
      Base64Codec::decode(enc.c_str(), &dec, len);
      assert(strlen(data) == len);
      assert(strncmp(data, dec, strlen(data)) == 0);
      free(dec);
   }
   tr.pass();

   tr.test("decall");
   {
      const char* data =
         "abcdefghijklmnopqrstuvwxyz"
         "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
         "0123456789+/";
      char* dec;
      unsigned int len;
      Base64Codec::decode(data, &dec, len);
      string enc = Base64Codec::encode(dec, len);
      assert(enc.length() == strlen(data));
      if(strncmp(data, enc.c_str(), strlen(data)) != 0)
      {
         printf("\ndata[%zd]\n", strlen(data));
         for(size_t i = 0; i < strlen(data); ++i)
         {
            printf("%c", (unsigned char)data[i]);
         }
         printf("\n");
         for(size_t i = 0; i < strlen(data); ++i)
         {
            printf("%x  ", (unsigned char)data[i]);
         }

         printf("\ndec[%d]\n", len);
         for(size_t i = 0; i < len; ++i)
         {
            printf("%x  ", (unsigned char)dec[i]);
         }

         printf("\nenc[%zd]\n", enc.length());
         for(size_t i = 0; i < enc.length(); ++i)
         {
            printf("%c", (unsigned char)enc[i]);
         }
         printf("\n");
         for(size_t i = 0; i < enc.length(); ++i)
         {
            printf("%x  ", (unsigned char)enc[i]);
         }
      }
      assert(strncmp(data, enc.c_str(), strlen(data)) == 0);
      free(dec);
   }
   tr.pass();

   tr.test("symbols");
   {
      const char* data = "/+/+";
      const char* ex = "\xff\xef\xfe";
      char* dec;
      unsigned int len;
      Base64Codec::decode(data, &dec, len);
      assert(len == 3);
      assert(strncmp(dec, ex, strlen(ex)) == 0);
      string enc = Base64Codec::encode(dec, len);
      assert(enc.length() == strlen(data));
      assert(strncmp(data, enc.c_str(), strlen(data)) == 0);
      free(dec);
   }
   tr.pass();

   tr.test("custom symbols");
   {
      const char* data = "$@$@";
      const char* ex = "\xfb\xff\xbf";
      const char* sym = "$@";
      char* dec;
      unsigned int len;
      Base64Codec::decode(data, &dec, len, sym);
      assert(len == 3);
      assert(strncmp(dec, ex, strlen(ex)) == 0);
      string enc = Base64Codec::encode(dec, len, 0, sym);
      assert(enc.length() == strlen(data));
      assert(strncmp(data, enc.c_str(), strlen(data)) == 0);
      free(dec);
   }
   tr.pass();

   tr.test("url safe");
   {
      const char* data = "\xfb\xff\xbf";
      const char* ex = "-_-_";
      string enc = Base64Codec::urlSafeEncode(data, strlen(data));
      assert(strncmp(enc.c_str(), ex, strlen(ex)) == 0);
      char* dec;
      unsigned int len;
      Base64Codec::urlSafeDecode(enc.c_str(), &dec, len);
      assert(len == 3);
      assert(strncmp(dec, data, strlen(data)) == 0);
      free(dec);
   }
   tr.pass();

   tr.ungroup();
}

static void runBase64SpeedTest(TestRunner& tr)
{
   tr.group("Base64Speed");

   printf("Base64 Speed Test\n");

   // setup source
   size_t slen = 1024*1024;
   char src[slen];
   for(size_t i = 0; i < slen; ++i)
   {
      src[i] = random() & 0xff;
   }

   tr.test("random data enc (~3s)");
   {
      uint64_t total_t = 0;
      uint64_t loops = 0;
      printf("Random data encoding (~3s): ");
      fflush(stdout);
      uint64_t start_t = Timer::startTiming();
      while(total_t < 3000)
      {
         string enc = Base64Codec::encode(src, slen);
         total_t = Timer::getMilliseconds(start_t);
         ++loops;
      }
      printf("%9.3f KB/s\n", (slen * loops) / (double)total_t);
      fflush(stdout);
   }
   tr.pass();

   tr.test("random data dec (~3s)");
   {
      // make encoded data
      string enc = Base64Codec::encode(src, slen);
      size_t elen = enc.length();
      uint64_t total_t = 0;
      uint64_t loops = 0;
      printf("Random data decoding (~3s): ");
      fflush(stdout);
      uint64_t start_t = Timer::startTiming();
      while(total_t < 3000)
      {
         char* dec;
         unsigned int dlen;
         Base64Codec::decode(enc.c_str(), &dec, dlen);
         free(dec);
         total_t = Timer::getMilliseconds(start_t);
         ++loops;
      }
      printf("%9.3f KB/s\n", (elen * loops) / (double)total_t);
      fflush(stdout);
   }
   tr.pass();

   tr.ungroup();
}

static void runCrcTest(TestRunner& tr)
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
   //printf("CRC-16=%s\n", crc16.getChecksum().c_str());
   assert(crc16a.getChecksum() == correctValue);
   tr.pass();

   tr.ungroup();
}

static void runConvertTest(TestRunner& tr)
{
   tr.test("Convert");

   // convert to hex
   char data[] = "abcdefghiABCDEFGZXYW0123987{;}*%6,./.12`~";
   string original(data, strlen(data));

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
      assertExceptionSet();
      Exception::clear();

      // too big
      hex = "876543210";
      assert(!Convert::hexToInt(hex.c_str(), hex.length(), ui));
      assertExceptionSet();
      Exception::clear();
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

static void runRegexTest(TestRunner& tr)
{
   tr.group("Regex");

   tr.test("match");
   {
      assert(Pattern::match("^[a-z]{3}$", "abc"));
      assert(Pattern::match("^[a-zA-Z0-9_]+$", "username"));
      assert(Pattern::match("^.*$", ".123-a0"));
      assert(Pattern::match("^[0-9]{3}$", "123"));
      assert(Pattern::match("^[[:digit:]]{3}$", "123"));
      assert(Pattern::match("^\\.[0-9]{3}$", ".123"));
      assert(Pattern::match("^\\.[0-9]{3}-[a-z]{1}$", ".123-a"));
      assert(Pattern::match("^\\.[0-9]{3}-[a-z]{1}[0-9]+$", ".123-a0"));
      const char* pat = "^\\.[0-9]{3}(-[a-z]{1}[0-9]+)?(\\.gz)?$";
      assert(Pattern::match(pat, ".123"));
      assert(Pattern::match(pat, ".123.gz"));
      assert(Pattern::match(pat, ".123-a5"));
      assert(Pattern::match(pat, ".123-b50"));
      assert(Pattern::match(pat, ".123-b50.gz"));
   }
   tr.passIfNoException();

   tr.test("compiled match");
   {
      int _start;
      int _end;
      {
         PatternRef pat = Pattern::compile("moo");
         assert(pat->match("moo", 0, _start, _end));
      }
      {
         PatternRef pat = Pattern::compile("^.*$");
         assert(pat->match("anything", 0, _start, _end));
      }
      {
         PatternRef pat = Pattern::compile("^[0-9]+$");
         assert(!pat->match("", 0, _start, _end));
         assert(!pat->match("abc", 0, _start, _end));
         assert(pat->match("0123", 0, _start, _end));
      }
      {
         PatternRef pat = Pattern::compile("^[0-9]+$");
         assert(!pat->match("abc", 0, _start, _end));
         assert(pat->match("123", 0, _start, _end));
         assert(pat->match("abc123" + 3, 0, _start, _end));
         // Note offset does _not_ work this way:
         //assert(pat->match("abc123", 3, _start, _end));
      }
   }
   tr.passIfNoException();

   tr.test("no match");
   {
      assert(!Pattern::match("^[a-z]{3}$", "abcd"));
      assert(!Pattern::match("^[a-z]{3}$", "ABC"));
      assert(!Pattern::match("^[a-zA-Z0-9_]+$", "user name"));
   }
   tr.passIfNoException();

   tr.test("sub-match string");
   {
      string submatches = "Look for green globs of green matter in green goo.";
      PatternRef p = Pattern::compile("green", true, true);
      assertNoExceptionSet();

      int start, end;
      int index = 0;

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
   }
   tr.passIfNoException();

   tr.test("subexpression matches");
   {
      PatternRef p = Pattern::compile("^foo(.*)bar(.*)foo(.*)bar$", true, true);
      assertNoExceptionSet();

      DynamicObject matches;
      assert(p->getSubMatches("fooABCbarDEFfooGHIbar", matches));
      //dumpDynamicObject(matches);

      DynamicObject expect;
      expect[0] = "fooABCbarDEFfooGHIbar";
      expect[1] = "ABC";
      expect[2] = "DEF";
      expect[3] = "GHI";

      assertDynoCmp(expect, matches);
   }
   tr.passIfNoException();

   tr.test("split (simple)");
   {
      PatternRef p = Pattern::compile("([^ ]+)", true, true);
      assertNoExceptionSet();

      DynamicObject matches;
      assert(p->split("foo1 foo2", matches));
      //dumpDynamicObject(matches);

      DynamicObject expect;
      expect[0] = "foo1";
      expect[1] = "foo2";

      assertDynoCmp(expect, matches);
   }
   tr.passIfNoException();

   tr.test("split (key=value)");
   {
      PatternRef p =
         Pattern::compile("([[:alnum:]_]+)=([[:alnum:]_]+)", true, true);
      assertNoExceptionSet();

      DynamicObject matches;
      assert(p->split("foo1=bar1, foo2=bar2", matches));
      //dumpDynamicObject(matches);

      DynamicObject expect;
      expect[0] = "foo1";
      expect[1] = "bar1";
      expect[2] = "foo2";
      expect[3] = "bar2";

      assertDynoCmp(expect, matches);
   }
   tr.passIfNoException();

   tr.test("split (key=\"value\")");
   {
      PatternRef p =
         Pattern::compile("([[:alnum:]_]+)=\"([^\"]+)\"", true, true);
      assertNoExceptionSet();

      DynamicObject matches;
      assert(p->split("foo1=\"bar1\", foo2=\"bar2\"", matches));
      //dumpDynamicObject(matches);

      DynamicObject expect;
      expect[0] = "foo1";
      expect[1] = "bar1";
      expect[2] = "foo2";
      expect[3] = "bar2";

      assertDynoCmp(expect, matches);
   }
   tr.passIfNoException();

   tr.test("url rewrite");
   {
      PatternRef p = Pattern::compile("^/~([^/]+)/?(.*)$", true, true);
      assert(!p.isNull());

      DynamicObject matches;
      assert(p->getSubMatches("/~foo", matches));

      string output = StringTools::format("/u/%s/%s",
         matches[1]->getString(), matches[2]->getString());
      string expect = "/u/foo/";
      assertStrCmp(expect.c_str(), output.c_str());
   }
   tr.passIfNoException();

   tr.ungroup();
}

static void runStringToolsTest(TestRunner& tr)
{
   tr.group("StringTools");

   tr.test("trim");
   {
      string str;
      string trimmed;

      str = "";
      trimmed = StringTools::trim(str);
      assertStrCmp(trimmed.c_str(), "");

      str = "a";
      trimmed = StringTools::trim(str);
      assertStrCmp(trimmed.c_str(), "a");

      str = " a";
      trimmed = StringTools::trim(str);
      assertStrCmp(trimmed.c_str(), "a");

      str = "a ";
      trimmed = StringTools::trim(str);
      assertStrCmp(trimmed.c_str(), "a");

      str = " a ";
      trimmed = StringTools::trim(str);
      assertStrCmp(trimmed.c_str(), "a");

      str = " a b ";
      trimmed = StringTools::trim(str);
      assertStrCmp(trimmed.c_str(), "a b");

      str = " a b ";
      trimmed = StringTools::trim(str, " b");
      assertStrCmp(trimmed.c_str(), "a");

      str = " a b ";
      trimmed = StringTools::trim(str, " ab");
      assertStrCmp(trimmed.c_str(), "");
   }
   tr.passIfNoException();

   tr.test("replace");
   {
      // FIXME: add replace() tests
   }
   tr.passIfNoException();

   tr.test("replace all");
   {

      string str = "Look for green globs of green matter in green goo.";
      string exp = "Look for blue globs of blue matter in blue goo.";
      StringTools::regexReplaceAll(str, "green", "blue");
      assertStrCmp(str.c_str(), exp.c_str());
   }
   tr.passIfNoException();

   tr.test("format");
   {
      // NOTE: zero-length format string will just cause a compiler warning
      //assertStrCmp(
      //   StringTools::format("").c_str(),
      //   "");
      assertStrCmp(
         StringTools::format(" ").c_str(),
         " ");
      assertStrCmp(
         StringTools::format("%s", "123").c_str(),
         "123");
      assertStrCmp(
         StringTools::format("%d", 123).c_str(),
         "123");
      assertStrCmp(
         StringTools::format("%d-%s", 123, "123").c_str(),
         "123-123");
   }
   tr.passIfNoException();

   tr.test("split");
   {
      {
         DynamicObject dyno;
         DynamicObject expected;
         expected->setType(Array);

         // empty str
         {
            expected[0] = "";
            dyno = StringTools::split("", " ");
            assertDynoCmp(dyno, expected);
         }

         // no splits
         {
            expected[0] = "abc";
            dyno = StringTools::split("abc", " ");
            assertDynoCmp(dyno, expected);
         }

         // trailing split
         {
            expected[1] = "";
            dyno = StringTools::split("abc.", ".");
            assertDynoCmp(dyno, expected);
         }

         // a few splits
         {
            expected[0] = "a";
            expected[1] = "b";
            expected[2] = "c";
            dyno = StringTools::split("a.b.c", ".");
            assertDynoCmp(dyno, expected);
         }

         // longer delimiter
         {
            expected[0] = "a";
            expected[1] = "b";
            expected[2] = "c";
            dyno = StringTools::split("a123b123c", "123");
            assertDynoCmp(dyno, expected);
            dyno = StringTools::split("a...b...c", "...");
            assertDynoCmp(dyno, expected);
         }

         // longer delimiter empty strings
         {
            expected[0] = "";
            expected[1] = "";
            expected[2] = "";
            dyno = StringTools::split("......", "...");
            assertDynoCmp(dyno, expected);
            dyno = StringTools::split("123123", "123");
            assertDynoCmp(dyno, expected);
         }
      }
   }
   tr.passIfNoException();

   tr.test("join");
   {
      DynamicObject dyno;
      dyno->setType(Array);
      string str;

      // no elements
      {
         str = StringTools::join(dyno, "");
         assertStrCmp(str.c_str(), "");

         str = StringTools::join(dyno, ".");
         assertStrCmp(str.c_str(), "");
      }

      // one element
      {
         dyno[0] = "a";
         str = StringTools::join(dyno, "");
         assertStrCmp(str.c_str(), "a");

         str = StringTools::join(dyno, ".");
         assertStrCmp(str.c_str(), "a");
      }

      // two elements
      {
         dyno[1] = "b";
         str = StringTools::join(dyno, "");
         assertStrCmp(str.c_str(), "ab");

         str = StringTools::join(dyno, ".");
         assertStrCmp(str.c_str(), "a.b");
      }

      // many elements
      {
         dyno[2] = "c";
         str = StringTools::join(dyno, "");
         assertStrCmp(str.c_str(), "abc");

         str = StringTools::join(dyno, ".");
         assertStrCmp(str.c_str(), "a.b.c");
      }
   }
   tr.passIfNoException();

   tr.test("regex rewrite");
   {
      const char* regex = "foo(.*)bar";
      string input = "fooABCbar";
      StringTools::regexRewrite(input, regex, "moo$1bar\\$\\\\");
      const char* expect = "mooABCbar$\\";
      assertStrCmp(expect, input.c_str());
   }
   tr.passIfNoException();

   tr.ungroup();
}

static void runDateTest(TestRunner& tr)
{
   tr.group("Date");

   TimeZone gmt = TimeZone::getTimeZone("GMT");
   TimeZone local = TimeZone::getTimeZone();

   tr.test("current local and utc date");
   {
      Date d;
      printf("\nlocal date: %s", d.toString().c_str());
      printf("\nUTC date: %s\n... ", d.getUtcDateTime().c_str());
   }
   tr.passIfNoException();

   tr.test("format and parse");
   {
      // format now
      Date d;
      string str1;
      d.format(str1, "%a, %d %b %Y %H:%M:%S", &gmt);

      // parse and reformat now
      Date d2;
      assertNoException(
         d2.parse(str1.c_str(), "%a, %d %b %Y %H:%M:%S", &gmt));
      string str2;
      d2.format(str2, "%a, %d %b %Y %H:%M:%S", &gmt);

      // compare
      assertStrCmp(str1.c_str(), str2.c_str());
   }
   tr.passIfNoException();

   tr.test("broken-down time");
   {
      const char* testDate = "Thu, 02 Aug 2007 10:30:00";

      // parse local date
      Date d;
      assertNoException(
         d.parse(testDate, "%a, %d %b %Y %H:%M:%S"));

      assertIntCmp(d.second(), 0);
      assertIntCmp(d.minute(), 30);
      assertIntCmp(d.hour(), 10);
      assertIntCmp(d.day(), 2);
      assertIntCmp(d.month(), 7);
      assertIntCmp(d.year(), 2007);
      assertIntCmp(d.yearSince1900(), 107);
   }
   tr.passIfNoException();

   tr.test("timezones");
   {
      const char* testDate = "Thu, 02 Aug 2007 10:30:00";

      // compare 3 timezones
      TimeZone est = TimeZone::getTimeZone("EST");
      TimeZone pst = TimeZone::getTimeZone("PST");
      Date estDate;
      assertNoException(
         estDate.parse(testDate, "%a, %d %b %Y %H:%M:%S", &est));
      Date pstDate;
      assertNoException(
         pstDate.parse(testDate, "%a, %d %b %Y %H:%M:%S", &pst));
      Date gmtDate;
      assertNoException(
         gmtDate.parse(testDate, "%a, %d %b %Y %H:%M:%S", &gmt));

      // normalize all dates to GMT for comparison:

      // difference of 5 hours between EST and GMT, so 10:30 EST would be
      // 5 hours after than GMT 10:30, subtract to go back
      estDate.addSeconds(-5 * 60 * 60);

      // difference of 5 hours between EST and GMT, 8 between PST and GMT,
      // so 10:30 PST would be 8 hours after than GMT 10:30, subtract to
      // go back
      pstDate.addSeconds(-8 * 60 * 60);

      string est1 = estDate.toString("%a, %d %b %Y %H:%M:%S", &est);
      string est2 = pstDate.toString("%a, %d %b %Y %H:%M:%S", &est);
      string est3 = gmtDate.toString("%a, %d %b %Y %H:%M:%S", &est);
      string pst1 = estDate.toString("%a, %d %b %Y %H:%M:%S", &pst);
      string pst2 = pstDate.toString("%a, %d %b %Y %H:%M:%S", &pst);
      string pst3 = gmtDate.toString("%a, %d %b %Y %H:%M:%S", &pst);
      string gmt1 = gmtDate.toString("%a, %d %b %Y %H:%M:%S", &gmt);
      string gmt2 = estDate.toString("%a, %d %b %Y %H:%M:%S", &gmt);
      string gmt3 = pstDate.toString("%a, %d %b %Y %H:%M:%S", &gmt);

      assertStrCmp(est1.c_str(), est2.c_str());
      assertStrCmp(est2.c_str(), est3.c_str());
      assertStrCmp(pst1.c_str(), pst2.c_str());
      assertStrCmp(pst2.c_str(), pst3.c_str());
      assertStrCmp(gmt1.c_str(), gmt2.c_str());
      assertStrCmp(gmt2.c_str(), gmt3.c_str());
   }
   tr.passIfNoException();

   tr.test("utc datetime");
   {
      // get utc date
      {
         Date d;
         string str;
         assertNoException(
            d.parse(
               "Thu, 02 Aug 2007 10:30:00", "%a, %d %b %Y %H:%M:%S", &gmt));

         string utc = d.getUtcDateTime();
         assertStrCmp(utc.c_str(), "2007-08-02 10:30:00");
      }

      {
         time_t seconds = 1293840000UL;
         const char* utcDatetime = "2011-01-01T00:00:00Z";
         Date d1;
         assert(d1.parseUtcDateTime(utcDatetime, true));
         assertStrCmp(utcDatetime, d1.getUtcDateTime(true).c_str());
         assert(d1.getSeconds() == seconds);
      }

      // hour before start of DST (EST => EDT), 2011
      {
         time_t seconds = 1299996000UL;
         const char* utcDatetime = "2011-03-13T06:00:00Z";
         Date d1;
         assert(d1.parseUtcDateTime(utcDatetime, true));
         assertStrCmp(utcDatetime, d1.getUtcDateTime(true).c_str());
         assert(d1.getSeconds() == seconds);
      }

      // start of DST (EST => EDT), 2011
      {
         time_t seconds = 1299999600UL;
         const char* utcDatetime = "2011-03-13T07:00:00Z";
         Date d1;
         assert(d1.parseUtcDateTime(utcDatetime, true));
         assertStrCmp(utcDatetime, d1.getUtcDateTime(true).c_str());
         assert(d1.getSeconds() == seconds);
      }

      // hour before end of DST (EDT => EST), 2011
      {
         time_t seconds = 1320555600UL;
         const char* utcDatetime = "2011-11-06T05:00:00Z";
         Date d1;
         assert(d1.parseUtcDateTime(utcDatetime, true));
         assertStrCmp(utcDatetime, d1.getUtcDateTime(true).c_str());
         assert(d1.getSeconds() == seconds);
      }

      // end of DST (EST => EDT), 2011
      {
         time_t seconds = 1320559200UL;
         const char* utcDatetime = "2011-11-06T06:00:00Z";
         Date d1;
         assert(d1.parseUtcDateTime(utcDatetime, true));
         assertStrCmp(utcDatetime, d1.getUtcDateTime(true).c_str());
         assert(d1.getSeconds() == seconds);
      }
   }
   tr.passIfNoException();

   tr.test("parse json-ld dates");
   {
      const char* testDate = "2007-08-02T10:30:00Z";

      Date d;
      assertNoException(
         d.parseUtcDateTime(testDate, true));

      string jsonld = d.getUtcDateTime(true);
      jsonld.append("^^<xsd:dateTime>");

      assertNoException(
         d.parse(jsonld.c_str(), "%Y-%m-%dT%H:%M:%SZ", &gmt));
      assertNoException(
         d.parse(jsonld.c_str(), "%Y-%m-%dT%H:%M:%SZ^^<xsd:dateTime>", &gmt));

      assertStrCmp(d.getUtcDateTime(true).c_str(), testDate);
   }
   tr.passIfNoException();

   tr.ungroup();
}

static void runStringTokenizerTest(TestRunner& tr)
{
   tr.group("StringTokenizer");

   tr.test("tokenize forward");
   {
      const char* str = "This is a test of the StringTokenizer class.";

      /*
      StringTokenizer st0(str, ' ');
      while(st0.hasNextToken())
      {
         printf("token='%s'\n", st0.nextToken());
      }
      */
      StringTokenizer st(str, ' ', true);
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
   }
   tr.passIfNoException();

   tr.test("tokenize backward");
   {
      const char* str = "This is a test of the StringTokenizer class.";

      StringTokenizer st(str, ' ', false);
      #define NT(str) \
         do { \
            assert(st.hasPreviousToken()); \
            assertStrCmp(st.previousToken(), str); \
         } while(0)
      NT("class.");
      NT("StringTokenizer");
      NT("the");
      NT("of");
      NT("test");
      NT("a");
      NT("is");
      NT("This");
      assert(!st.hasPreviousToken());
      #undef NT
   }
   tr.passIfNoException();

   tr.test("tokenize forward w/long delimiter");
   {
      const char* str =
         "These_x_words_x_are_x_delimited_x_by_x_x_x_surrounded_x_by_x_"
         "underscores.";

      /*
      StringTokenizer st0(str, "_x_");
      while(st0.hasNextToken())
      {
         printf("token='%s'\n", st0.nextToken());
      }
      */
      StringTokenizer st(str, "_x_", true);
      #define NT(str) \
         do { \
            assert(st.hasNextToken()); \
            assertStrCmp(st.nextToken(), str); \
         } while(0)
      NT("These");
      NT("words");
      NT("are");
      NT("delimited");
      NT("by");
      NT("x");
      NT("surrounded");
      NT("by");
      NT("underscores.");
      assert(!st.hasNextToken());
      #undef NT
   }
   tr.passIfNoException();

   tr.test("tokenize backward w/long delimiter");
   {
      const char* str =
         "These_x_words_x_are_x_delimited_x_by_x_x_x_surrounded_x_by_x_"
         "underscores.";

      StringTokenizer st(str, "_x_", false);
      #define NT(str) \
         do { \
            assert(st.hasPreviousToken()); \
            assertStrCmp(st.previousToken(), str); \
         } while(0)
      NT("underscores.");
      NT("by");
      NT("surrounded");
      NT("x");
      NT("by");
      NT("delimited");
      NT("are");
      NT("words");
      NT("These");
      assert(!st.hasPreviousToken());
      #undef NT
   }
   tr.passIfNoException();

   tr.test("get first token");
   {
      const char* str = "This is a test of the StringTokenizer class.";
      StringTokenizer st(str, ' ', true);
      assertStrCmp(st.getToken(0), "This");
   }
   tr.passIfNoException();

   tr.test("get second token");
   {
      const char* str = "This is a test of the StringTokenizer class.";
      StringTokenizer st(str, ' ', true);
      assertStrCmp(st.getToken(1), "is");
   }
   tr.passIfNoException();

   tr.test("get last token");
   {
      const char* str = "This is a test of the StringTokenizer class.";
      StringTokenizer st(str, ' ', false);
      assertStrCmp(st.getToken(-1), "class.");
   }
   tr.passIfNoException();

   tr.test("get second to last token");
   {
      const char* str = "This is a test of the StringTokenizer class.";
      StringTokenizer st(str, ' ', false);
      assertStrCmp(st.getToken(-2), "StringTokenizer");
   }
   tr.passIfNoException();

   tr.test("empty str, empty token");
   {
      StringTokenizer st("", '/');
      assert(st.hasNextToken());
      assertStrCmp(st.nextToken(), "");
      assert(!st.hasNextToken());
   }
   tr.passIfNoException();

   tr.test("simple str, same token");
   {
      StringTokenizer st("/", '/');
      assert(st.hasNextToken());
      assertStrCmp(st.nextToken(), "");
      assert(st.hasNextToken());
      assertStrCmp(st.nextToken(), "");
      assert(!st.hasNextToken());
   }
   tr.passIfNoException();

   tr.test("simple str, diff token");
   {
      StringTokenizer st("/", 'X');
      assert(st.hasNextToken());
      assertStrCmp(st.nextToken(), "/");
      assert(!st.hasNextToken());
   }
   tr.passIfNoException();

   tr.ungroup();
}

static void runUniqueListTest(TestRunner& tr)
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
      printf("element=%d\n", i->next());
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
      printf("element=%d\n", i->next());
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

static void runPathFormatterTest(TestRunner& tr)
{
   tr.test("PathFormatter");
   {
      string s1 = "?/\\*:|\"<>+[]";
      string s2 = "____________";
      assertStrCmp(PathFormatter::formatFilename(s1).c_str(), s2.c_str());
   }
   {
      string s1 = "abcABC123!@#$%^&()~,. ?/\\*:|\"<>+[]";
      string s2 = "abcABC123!@#$%^&()~,. ____________";
      assertStrCmp(PathFormatter::formatFilename(s1).c_str(), s2.c_str());
   }
   tr.passIfNoException();
}

static void runRateAveragerTest(TestRunner& tr)
{
   tr.test("RateAverager 10 items/sec");
   {
      RateAverager ra;
      uint64_t start = System::getCurrentMilliseconds();
      Thread::sleep(900);
      ra.addItems(5, start);
      Thread::sleep(100);
      ra.addItems(5, start);
      printf("cur=%1.4f i/s,tot=%1.4f i/s ... ",
         ra.getItemsPerSecond(),
         ra.getTotalItemsPerSecond());
   }
   tr.passIfNoException();

   tr.test("RateAverager 10 items/sec again");
   {
      RateAverager ra;
      uint64_t start = System::getCurrentMilliseconds();
      ra.addItems(5, start);
      Thread::sleep(900);
      start = System::getCurrentMilliseconds();
      Thread::sleep(100);
      ra.addItems(5, start);
      printf("cur=%1.4f i/s,tot=%1.4f i/s ... ",
         ra.getItemsPerSecond(),
         ra.getTotalItemsPerSecond());
   }
   tr.passIfNoException();
}

static void runAnsiEscapeCodeTest(TestRunner& tr)
{
   tr.group("ANSI Escape Codes");

   tr.test("color");
   {
      const char* fg[] = {
         "black   ", MO_ANSI_FG_BLACK,
         "red     ", MO_ANSI_FG_RED,
         "green   ", MO_ANSI_FG_GREEN,
         "yellow  ", MO_ANSI_FG_YELLOW,
         "blue    ", MO_ANSI_FG_BLUE,
         "magenta ", MO_ANSI_FG_MAGENTA,
         "cyan    ", MO_ANSI_FG_CYAN,
         "white   ", MO_ANSI_FG_WHITE,
         NULL
      };
      const char* bg[] = {
         "black   ", MO_ANSI_BG_BLACK,
         "red     ", MO_ANSI_BG_RED,
         "green   ", MO_ANSI_BG_GREEN,
         "yellow  ", MO_ANSI_BG_YELLOW,
         "blue    ", MO_ANSI_BG_BLUE,
         "magenta ", MO_ANSI_BG_MAGENTA,
         "cyan    ", MO_ANSI_BG_CYAN,
         "white   ", MO_ANSI_BG_WHITE,
         NULL
      };
      const char* fg_hi[] = {
         "black   ", MO_ANSI_FG_HI_BLACK,
         "red     ", MO_ANSI_FG_HI_RED,
         "green   ", MO_ANSI_FG_HI_GREEN,
         "yellow  ", MO_ANSI_FG_HI_YELLOW,
         "blue    ", MO_ANSI_FG_HI_BLUE,
         "magenta ", MO_ANSI_FG_HI_MAGENTA,
         "cyan    ", MO_ANSI_FG_HI_CYAN,
         "white   ", MO_ANSI_FG_HI_WHITE,
         NULL
      };
      const char* bg_hi[] = {
         "black   ", MO_ANSI_BG_HI_BLACK,
         "red     ", MO_ANSI_BG_HI_RED,
         "green   ", MO_ANSI_BG_HI_GREEN,
         "yellow  ", MO_ANSI_BG_HI_YELLOW,
         "blue    ", MO_ANSI_BG_HI_BLUE,
         "magenta ", MO_ANSI_BG_HI_MAGENTA,
         "cyan    ", MO_ANSI_BG_HI_CYAN,
         "white   ", MO_ANSI_BG_HI_WHITE,
         NULL
      };

      #define TABLE(txt, fg, bg) \
         printf(txt ":\n"); \
         for(int bgi = 0; bg[bgi] != NULL; bgi += 2) \
         { \
            for(int fgi = 0; fg[fgi] != NULL; fgi += 2) \
            { \
               printf( \
                  MO_ANSI_CSI "%s" MO_ANSI_SEP "%s" MO_ANSI_SGR \
                  "%s" MO_ANSI_OFF, \
                  fg[fgi+1], bg[bgi+1], fg[fgi]); \
            } \
            printf("\n"); \
         }
      printf("\n");
      TABLE("normal fg & normal bg", fg,    bg)
      TABLE("normal fg & high bg",   fg,    bg_hi)
      TABLE("high fg & normal bg",   fg_hi, bg)
      TABLE("high fg & high bg",     fg_hi, bg_hi)
      #undef TABLE

      #define TXT "Digital Bazaar, Inc."
      #define S MO_ANSI_CSI
      #define E MO_ANSI_SGR TXT MO_ANSI_OFF "\n"
      printf("reset:            " S MO_ANSI_RESET E);
      printf("bold:             " S MO_ANSI_BOLD E);
      printf("faint:            " S MO_ANSI_FAINT E);
      printf("italic:           " S MO_ANSI_ITALIC E);
      printf("underline single: " S MO_ANSI_UNDERLINE_SINGLE E);
      printf("blink slow:       " S MO_ANSI_BLINK_SLOW E);
      printf("blink rapid:      " S MO_ANSI_BLINK_RAPID E);
      printf("negative:         " S MO_ANSI_NEGATIVE E);
      printf("conceal:          " S MO_ANSI_CONCEAL E);
      printf("underline double: " S MO_ANSI_UNDERLINE_DOUBLE E);
      printf("normal:           " S MO_ANSI_NORMAL E);
      printf("underline none:   " S MO_ANSI_UNDERLINE_NONE E);
      printf("blink off:        " S MO_ANSI_BLINK_OFF E);
      printf("positive:         " S MO_ANSI_POSITIVE E);
      printf("reveal:           " S MO_ANSI_REVEAL E);
      #undef TXT
      #undef S
      #undef E
   }
   tr.passIfNoException();

   tr.ungroup();
}

static void runRandomTest(TestRunner& tr)
{
   tr.test("Random");
   {
      Random::seed();
      for(int i = 0; i < 10000; ++i)
      {
         printf("%" PRIu64 "\n", Random::next(1, 1000000000));
      }
   }
   tr.passIfNoException();
}

static bool run(TestRunner& tr)
{
   if(tr.isDefaultEnabled())
   {
      runBase64Test(tr);
      runCrcTest(tr);
      runConvertTest(tr);
      runStringTokenizerTest(tr);
      runUniqueListTest(tr);
      runRegexTest(tr);
      runStringToolsTest(tr);
      runDateTest(tr);
      runPathFormatterTest(tr);
   }
   if(tr.isTestEnabled("date"))
   {
      Date now;
      TimeZone local = TimeZone::getTimeZone();
      printf("Current date: %s\n", now.toString().c_str());
      printf("Current UTC date: %s\n", now.getUtcDateTime().c_str());
      printf("Local time zone minutes west: %d\n", (int)local.getMinutesWest());
   }
   if(tr.isTestEnabled("ansi-escape-codes"))
   {
      runAnsiEscapeCodeTest(tr);
   }
   if(tr.isTestEnabled("random"))
   {
      runRandomTest(tr);
   }
   if(tr.isTestEnabled("rate-averager"))
   {
      runRateAveragerTest(tr);
   }
   if(tr.isTestEnabled("base64-speed"))
   {
      runBase64SpeedTest(tr);
   }

   return true;
}

} // end namespace

MO_TEST_MODULE_FN("monarch.tests.util.test", "1.0", mo_test_util::run)
