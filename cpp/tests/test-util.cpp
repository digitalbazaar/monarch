/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */

#include "db/data/json/JsonWriter.h"
#include "db/test/Test.h"
#include "db/test/Tester.h"
#include "db/test/TestRunner.h"
#include "db/util/AnsiEscapeCodes.h"
#include "db/util/Base64Codec.h"
#include "db/util/Convert.h"
#include "db/util/Crc16.h"
#include "db/util/Date.h"
#include "db/util/PathFormatter.h"
#include "db/util/Random.h"
#include "db/util/StringTools.h"
#include "db/util/StringTokenizer.h"
#include "db/util/UniqueList.h"
#include "db/util/regex/Pattern.h"

#include <cstdlib>

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
   //printf("CRC-16=%s\n", crc16.getChecksum().c_str());
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
      Exception::clear();

      // too big
      hex = "876543210";
      assert(!Convert::hexToInt(hex.c_str(), hex.length(), ui));
      assertException();
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

void runRegexTest(TestRunner& tr)
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
      unsigned int _start;
      unsigned int _end;
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

   tr.test("sub-match");
   {
      string submatches = "Look for green globs of green matter in green goo.";
      PatternRef p = Pattern::compile("green");

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
   }
   tr.passIfNoException();

   tr.ungroup();
}

void runStringToolsTest(TestRunner& tr)
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
      assertStrCmp(
         StringTools::format("").c_str(),
         "");
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
            dyno = StringTools::split("", ' ');
            assertDynoCmp(dyno, expected);
         }

         // no splits
         {
            expected[0] = "abc";
            dyno = StringTools::split("abc", ' ');
            assertDynoCmp(dyno, expected);
         }

         // trailing split
         {
            expected[1] = "";
            dyno = StringTools::split("abc.", '.');
            assertDynoCmp(dyno, expected);
         }

         // a few splits
         {
            expected[0] = "a";
            expected[1] = "b";
            expected[2] = "c";
            dyno = StringTools::split("a.b.c", '.');
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

   tr.ungroup();
}

void runDateTest(TestRunner& tr)
{
   tr.group("Date");

   TimeZone gmt = TimeZone::getTimeZone("GMT");
   TimeZone local = TimeZone::getTimeZone();

   tr.test("format and parse");
   {
      Date d;
      string str;
      //d.format(str);
      //d.format(str, "%a, %d %b %Y %H:%M:%S");
      d.format(str, "%a, %d %b %Y %H:%M:%S", &gmt);
      //d.format(str, "%a, %d %b %Y %H:%M:%S", &local);
      //printf("Current Date: %s\n", str.c_str());
      assertNoException();

      // parse date
      Date d2;
      d2.parse(str.c_str(), "%a, %d %b %Y %H:%M:%S", &gmt);
      //d2.parse(str.c_str(), "%a, %d %b %Y %H:%M:%S", &local);
      assertNoException();
      string str2;
      d2.format(str2, "%a, %d %b %Y %H:%M:%S", &gmt);
      //d2.format(str2, "%a, %d %b %Y %H:%M:%S", &local);
      assertNoException();

      //printf("Parsed Date 1: %s\n", str2.c_str());

      // FIXME: parser may have a problem with AM/PM
      // parse date again
      Date d3;
      str = "Thu, 02 Aug 2007 10:30:00";
      d3.parse(str.c_str(), "%a, %d %b %Y %H:%M:%S", &gmt);
      assertNoException();
      string str3;
      //d3.format(str3, "%a, %d %b %Y %H:%M:%S", &gmt);
      d3.format(str3, "%a, %d %b %Y %H:%M:%S", &local);
      assertNoException();

      //printf("Parsed Date 2: %s\n", str3.c_str());
   }
   tr.passIfNoException();

   tr.test("utc datetime");
   {
      Date d;
      string str;
      d.parse("Thu, 02 Aug 2007 10:30:00", "%a, %d %b %Y %H:%M:%S", &gmt);
      assertNoException();

      string utc = d.getUtcDateTime();
      assertStrCmp(utc.c_str(), "2007-08-02 10:30:00");
   }
   tr.passIfNoException();

   tr.ungroup();
}

void runStringTokenizerTest(TestRunner& tr)
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

   tr.ungroup();
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

void runPathFormatterTest(TestRunner& tr)
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

void runAnsiEscapeCodeTest(TestRunner& tr)
{
   tr.group("ANSI Escape Codes");

   tr.test("color");
   {
      const char* fg[] = {
         "black   ", DB_ANSI_FG_BLACK,
         "red     ", DB_ANSI_FG_RED,
         "green   ", DB_ANSI_FG_GREEN,
         "yellow  ", DB_ANSI_FG_YELLOW,
         "blue    ", DB_ANSI_FG_BLUE,
         "magenta ", DB_ANSI_FG_MAGENTA,
         "cyan    ", DB_ANSI_FG_CYAN,
         "white   ", DB_ANSI_FG_WHITE,
         NULL
      };
      const char* bg[] = {
         "black   ", DB_ANSI_BG_BLACK,
         "red     ", DB_ANSI_BG_RED,
         "green   ", DB_ANSI_BG_GREEN,
         "yellow  ", DB_ANSI_BG_YELLOW,
         "blue    ", DB_ANSI_BG_BLUE,
         "magenta ", DB_ANSI_BG_MAGENTA,
         "cyan    ", DB_ANSI_BG_CYAN,
         "white   ", DB_ANSI_BG_WHITE,
         NULL
      };
      const char* fg_hi[] = {
         "black   ", DB_ANSI_FG_HI_BLACK,
         "red     ", DB_ANSI_FG_HI_RED,
         "green   ", DB_ANSI_FG_HI_GREEN,
         "yellow  ", DB_ANSI_FG_HI_YELLOW,
         "blue    ", DB_ANSI_FG_HI_BLUE,
         "magenta ", DB_ANSI_FG_HI_MAGENTA,
         "cyan    ", DB_ANSI_FG_HI_CYAN,
         "white   ", DB_ANSI_FG_HI_WHITE,
         NULL
      };
      const char* bg_hi[] = {
         "black   ", DB_ANSI_BG_HI_BLACK,
         "red     ", DB_ANSI_BG_HI_RED,
         "green   ", DB_ANSI_BG_HI_GREEN,
         "yellow  ", DB_ANSI_BG_HI_YELLOW,
         "blue    ", DB_ANSI_BG_HI_BLUE,
         "magenta ", DB_ANSI_BG_HI_MAGENTA,
         "cyan    ", DB_ANSI_BG_HI_CYAN,
         "white   ", DB_ANSI_BG_HI_WHITE,
         NULL
      };

      #define TABLE(txt, fg, bg) \
         printf(txt ":\n"); \
         for(int bgi = 0; bg[bgi] != NULL; bgi += 2) \
         { \
            for(int fgi = 0; fg[fgi] != NULL; fgi += 2) \
            { \
               printf( \
                  DB_ANSI_CSI "%s" DB_ANSI_SEP "%s" DB_ANSI_SGR \
                  "%s" DB_ANSI_OFF, \
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
      #define S DB_ANSI_CSI
      #define E DB_ANSI_SGR TXT DB_ANSI_OFF "\n"
      printf("reset:            " S DB_ANSI_RESET E);
      printf("bold:             " S DB_ANSI_BOLD E);
      printf("faint:            " S DB_ANSI_FAINT E);
      printf("italic:           " S DB_ANSI_ITALIC E);
      printf("underline single: " S DB_ANSI_UNDERLINE_SINGLE E);
      printf("blink slow:       " S DB_ANSI_BLINK_SLOW E);
      printf("blink rapid:      " S DB_ANSI_BLINK_RAPID E);
      printf("negative:         " S DB_ANSI_NEGATIVE E);
      printf("conceal:          " S DB_ANSI_CONCEAL E);
      printf("underline double: " S DB_ANSI_UNDERLINE_DOUBLE E);
      printf("normal:           " S DB_ANSI_NORMAL E);
      printf("underline none:   " S DB_ANSI_UNDERLINE_NONE E);
      printf("blink off:        " S DB_ANSI_BLINK_OFF E);
      printf("positive:         " S DB_ANSI_POSITIVE E);
      printf("reveal:           " S DB_ANSI_REVEAL E);
      #undef TXT
      #undef S
      #undef E
   }
   tr.passIfNoException();

   tr.ungroup();
}

void runRandomTest(TestRunner& tr)
{
   tr.test("Random");
   {
      Random::seed();
      for(int i = 0; i < 10000; i++)
      {
         printf("%llu\n", Random::next(1, 1000000000));
      }
   }
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
      runStringToolsTest(tr);
      runDateTest(tr);
      runPathFormatterTest(tr);
      return 0;
   }

   /**
    * Runs interactive unit tests.
    */
   virtual int runInteractiveTests(TestRunner& tr)
   {
      runAnsiEscapeCodeTest(tr);
      //runRandomTest(tr);
      return 0;
   }
};

db::test::Tester* getDbUtilTester() { return new DbUtilTester(); }


DB_TEST_MAIN(DbUtilTester)
