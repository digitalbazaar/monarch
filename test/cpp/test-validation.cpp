/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */

#include "db/test/Test.h"
#include "db/test/Tester.h"
#include "db/test/TestRunner.h"
#include "db/rt/DynamicObject.h"
#include "db/validation/Validation.h"

using namespace std;
using namespace db::test;
using namespace db::rt;
namespace v = db::validation;

#if 0
#define VTESTDUMP dumpException()
#else
#define VTESTDUMP
#endif

void runValidatorTest(TestRunner& tr)
{
   tr.group("Validator");

   tr.test("valid");
   {
      DynamicObject d;
      v::Valid v;
      assert(v.isValid(d));
   }
   tr.passIfNoException();
   
   tr.test("not valid");
   {
      DynamicObject d;
      v::NotValid nv;
      assert(!nv.isValid(d));
      assertException();
      VTESTDUMP;
      assertStrCmp(
         Exception::getLast()->getType(), "db.validation.ValidationError");
      assertStrCmp(
         Exception::getLast()->getMessage(), "Object not valid.");
      Exception::clearLast();
   }
   tr.passIfNoException();
   
   tr.test("map");
   {
      DynamicObject dv;
      dv["i"] = 0;
      dv["b"] = true;
      DynamicObject dnv;
      dnv["i"] = false;
      dnv["b"] = "false";

      // create with addValidator      
      v::Map v0;
      v0.addValidator("i", new v::Type(Int32));
      v0.addValidator("b", new v::Type(Boolean));
      assert(v0.isValid(dv));

      assert(!v0.isValid(dnv));
      assertException();
      VTESTDUMP;
      Exception::clearLast();

      // create with constructor list
      v::Map v1(
         "i", new v::Type(Int32),
         "b", new v::Type(Boolean),
         NULL);
      assert(v1.isValid(dv));

      assert(!v1.isValid(dnv));
      assertException();
      VTESTDUMP;
      Exception::clearLast();
   }
   tr.passIfNoException();
   
   tr.test("types");
   {
      DynamicObject dv;
      dv["int32"] = (int32_t)-123;
      dv["uint32"] = (uint32_t)123;
      dv["int64"] = (int64_t)-123;
      dv["uint64"] = (uint64_t)123;
      dv["double"] = (double)123.0;
      dv["bool"] = true;
      dv["string"] = "string";
      dv["map"]["map"] = true;
      dv["array"][0] = true; 
      DynamicObject dnv;
      dnv["int32"] = false;
      dnv["uint32"] = false;
      dnv["int64"] = false;
      dnv["uint64"] = false;
      dnv["double"] = false;
      dnv["bool"] = "false";
      dnv["string"] = false;
      dnv["map"] = false;
      dnv["array"] = false; 
      
      v::Map v(
         "int32", new v::Type(Int32),
         "uint32", new v::Type(UInt32),
         "int64", new v::Type(Int64),
         "uint64", new v::Type(UInt64),
         "double", new v::Type(Double),
         "bool", new v::Type(Boolean),
         "string", new v::Type(String),
         "array", new v::Type(Array),
         "map", new v::Type(Map),
         NULL);
      assert(v.isValid(dv));

      assert(!v.isValid(dnv));
      assertException();
      VTESTDUMP;
      Exception::clearLast();
   }
   tr.passIfNoException();
   
   tr.test("array");
   {
      DynamicObject dv;
      dv[0] = 0;
      dv[1] = true;
      DynamicObject dnv;
      dnv[0] = false;
      dnv[1] = "false";

      // create with addValidator      
      v::Array v0;
      v0.addValidator(0, new v::Type(Int32));
      v0.addValidator(1, new v::Type(Boolean));
      assert(v0.isValid(dv));

      assert(!v0.isValid(dnv));
      assertException();
      VTESTDUMP;
      Exception::clearLast();

      // create with constructor list
      v::Array v1(
         0, new v::Type(Int32),
         1, new v::Type(Boolean),
         -1);
      assert(v1.isValid(dv));

      assert(!v1.isValid(dnv));
      assertException();
      VTESTDUMP;
      Exception::clearLast();
   }
   tr.passIfNoException();
   
   tr.test("optional");
   {
      DynamicObject d;
      d["present"] = true;
      v::Map v(
         "present", new v::Type(Boolean),
         "missing", new v::Optional(new v::Valid()),
         NULL);
      assert(v.isValid(d));
   }
   tr.passIfNoException();
   
   // trick to test for extra values.  Optional check to see if key is
   // present.  If so, then force not valid.
   tr.test("extra");
   {
      DynamicObject d;
      DynamicObject d2;
      d2["extra"] = true;
      v::Map v(
         "extra", new v::Optional(new v::NotValid()),
         NULL);
      assert(v.isValid(d));
      assert(!v.isValid(d2));
      assertException();
      VTESTDUMP;
      Exception::clearLast();
   }
   tr.passIfNoException();
   
   tr.test("min");
   {
      DynamicObject d;
      d = "1";
      
      v::Min v(0);
      assert(v.isValid(d));
      
      v::Min nv(2);
      assert(!nv.isValid(d));
      assertException();
      VTESTDUMP;
      Exception::clearLast();
   }
   tr.passIfNoException();
   
   tr.test("max");
   {
      DynamicObject d;
      d = "1";
      
      v::Max v(2);
      assert(v.isValid(d));
      
      v::Max nv(0);
      assert(!nv.isValid(d));
      assertException();
      VTESTDUMP;
      Exception::clearLast();
   }
   tr.passIfNoException();
   
   tr.test("equals");
   {
      DynamicObject eq;
      eq = "db";
      DynamicObject dv;
      dv = "db";
      DynamicObject dnv;
      dnv = "db!";
      
      v::Equals v(eq);
      assert(v.isValid(dv));
      
      assert(!v.isValid(dnv));
      assertException();
      VTESTDUMP;
      Exception::clearLast();
   }
   tr.passIfNoException();

   tr.test("all");
   {
      DynamicObject eq;
      eq = 0;
      DynamicObject dv;
      dv = 0;
      DynamicObject dnv;
      dnv = 1;
      
      v::All v(
         new v::Type(Int32),
         new v::Equals(eq),
         NULL);
      assert(v.isValid(dv));

      assert(!v.isValid(dnv));
      assertException();
      VTESTDUMP;
      Exception::clearLast();
   }
   tr.passIfNoException();

   tr.test("any");
   {
      DynamicObject eq0;
      eq0 = 0;
      DynamicObject eq1;
      eq1 = 1;
      DynamicObject dv;
      dv = 1;
      DynamicObject dnv;
      dnv = 2;

      v::Any v(
         new v::Equals(eq0),
         new v::Equals(eq1),
         NULL);
      assert(v.isValid(dv));

      assert(!v.isValid(dnv));
      assertException();
      VTESTDUMP;
      Exception::clearLast();
   }
   tr.passIfNoException();

   tr.test("deep");
   {
      DynamicObject dv;
      dv["parent"]["child"] = "12345678";
      DynamicObject dnv;
      dnv["parent"]["child"] = "1234567";

      v::Map v(
         "parent", new v::Map(
            "child", new v::Min(8),
            NULL),
         NULL);
      assert(v.isValid(dv));

      assert(!v.isValid(dnv));
      assertException();
      VTESTDUMP;
      Exception::clearLast();
   }
   tr.passIfNoException();

   tr.test("each(array)");
   {
      DynamicObject dv;
      dv[0] = "1234";
      dv[1] = "5678";
      DynamicObject dnv;
      dnv[0] = "1234";
      dnv[1] = "567";

      v::Each v(new v::Min(4));
      assert(v.isValid(dv));

      assert(!v.isValid(dnv));
      assertException();
      VTESTDUMP;
      Exception::clearLast();
   }
   tr.passIfNoException();

   tr.test("each(map)");
   {
      DynamicObject dv;
      dv["a"] = "1234";
      dv["b"] = "5678";
      DynamicObject dnv;
      dnv["a"] = "1234";
      dnv["b"] = "567";

      v::Each v(new v::Min(4));
      assert(v.isValid(dv));

      assert(!v.isValid(dnv));
      assertException();
      VTESTDUMP;
      Exception::clearLast();
   }
   tr.passIfNoException();

   tr.test("in(map)");
   {
      DynamicObject vals;
      vals["a"] = true;
      vals["b"] = true;
      vals["c"] = true;
      DynamicObject dv;
      dv = "c";
      DynamicObject dnv;
      dnv = "d";
      
      v::In v(vals);
      assert(v.isValid(dv));

      assert(!v.isValid(dnv));
      assertException();
      VTESTDUMP;
      Exception::clearLast();
   }
   tr.passIfNoException();

   tr.test("in(array)");
   {
      DynamicObject vals;
      vals[0] = "a";
      vals[1] = "b";
      vals[2] = "c";
      DynamicObject dv;
      dv = "c";
      DynamicObject dnv;
      dnv = "d";
      
      v::In v(vals);
      assert(v.isValid(dv));

      assert(!v.isValid(dnv));
      assertException();
      VTESTDUMP;
      Exception::clearLast();
   }
   tr.passIfNoException();
   
   tr.test("compare");
   {
      DynamicObject dv;
      dv["a"] = 0;
      dv["b"] = 0;
      DynamicObject dnv;
      dnv["a"] = 0;
      dnv["b"] = 1;
      
      v::Compare v("a", "b");
      assert(v.isValid(dv));

      assert(!v.isValid(dnv));
      assertException();
      VTESTDUMP;
      Exception::clearLast();
   }
   tr.passIfNoException();
   
   tr.test("register");
   {
      DynamicObject dv;
      dv["username"] = "foobar";
      dv["password"] = "secret";
      dv["password2"] = "secret";
      dv["fullname"] = "Fooish Barlow";
      dv["acceptToS"] = true;
      dv["dob"] = "1985-10-26";
      dv["email"] = "foobar@example.com";
      
      DynamicObject t;
      t = true;

      v::All v(
         new v::Map(
            // FIXME where/how to check/strip whitespace?
            "username", new v::All(
               new v::Type(String),
               new v::Min(6),
               new v::Max(16),
               NULL),
            "password", new v::All(
               new v::Type(String),
               new v::Min(6),
               new v::Max(16),
               NULL),
            "fullname", new v::All(
               new v::Type(String),
               new v::Min(1),
               new v::Max(256),
               NULL),
            "acceptToS", new v::All(
               new v::Type(Boolean),
               new v::Equals(t),
               NULL),
            /*
            "dob", new v::All(
               new v::Date(),
               new v::Age(18),
               NULL),
            "email", new v::Email(...),
            */
            NULL),
         new v::Compare("password", "password2"),
         NULL);
      assert(v.isValid(dv));
   }
   tr.passIfNoException();

   /*
   tr.test("content");
   {
      DynamicObject d;
      d["username"] = "Cookie Monster";
      d["password"] = "C is for Cookie";
      MapValidator v;
      v.setKeyValidator("username", new tor(
         Type(String),
         Type(String),
         NULL));
      v.setKeyValidator("password", String);
      v.addValidator("password", new Password(5));
      assert(v.isValid(d));
   }
   tr.passIfNoException();
   
   tr.test("content");
   {
      DynamicObject d;
      d["val"] = 123;
      d["f"] = 0.123;
      d["password"] = "str";
      Validator v;
      v.addValidator("val", Int32);
      v.addValidator("f", String);
      v.addValidator("password", new Password(5));
      assert(v.isValid(d));
   }
   tr.passIfNoException();
   */
   tr.ungroup();
}

class DbValidationTester : public db::test::Tester
{
public:
   DbValidationTester()
   {
      setName("validation");
   }

   /**
    * Run automatic unit tests.
    */
   virtual int runAutomaticTests(TestRunner& tr)
   {
      runValidatorTest(tr);
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
DB_TEST_MAIN(DbValidationTester)
#endif
