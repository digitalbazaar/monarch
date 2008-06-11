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

// Use to dump out expected exceptions 
#define _dump false

void runValidatorTest(TestRunner& tr)
{
   tr.group("Validator");

   {
      tr.test("valid");
      DynamicObject d;
      v::Valid v;
      assert(v.isValid(d));
      tr.passIfNoException();
   }
   
   {
      tr.test("not valid");
      DynamicObject d;
      v::NotValid nv;
      assert(!nv.isValid(d));
      assertException();
      if(_dump) dumpException();
      assertStrCmp(
         Exception::getLast()->getType(), "db.validation.ValidationError");
      assertStrCmp(
         Exception::getLast()->getMessage(), "Object not valid.");
      Exception::clearLast();
      tr.passIfNoException();
   }
   
   {
      tr.test("map (addv)");
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
      tr.passIfNoException();

      tr.test("invalid map (addv)");
      assert(!v0.isValid(dnv));
      assert(Exception::hasLast());
      ExceptionRef e = Exception::getLast();
      assert(e->getDetails()->hasMember("errors"));
      assert(e->getDetails()["errors"]->length() == 2);
      assert(e->getDetails()["errors"]->hasMember("i"));
      assert(e->getDetails()["errors"]->hasMember("b"));
      tr.passIfException(_dump);

      tr.test("map (clist)");
      // create with constructor list
      v::Map v1(
         "i", new v::Type(Int32),
         "b", new v::Type(Boolean),
         NULL);
      assert(v1.isValid(dv));
      tr.passIfNoException();

      tr.test("invalid map (clist)");
      assert(!v1.isValid(dnv));
      tr.passIfException(_dump);
   }
   
   {
      tr.test("types");
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
      tr.passIfNoException();
      
      tr.test("invalid types");
      assert(!v.isValid(dnv));
      tr.passIfException(_dump);
   }
   
   {
      tr.test("array (addv)");
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
      tr.passIfNoException();

      tr.test("invalid array (addv)");
      assert(!v0.isValid(dnv));
      tr.passIfException(_dump);

      tr.test("array (clist)");
      // create with constructor list
      v::Array v1(
         0, new v::Type(Int32),
         1, new v::Type(Boolean),
         -1);
      assert(v1.isValid(dv));
      tr.passIfNoException();

      tr.test("invalid array (clist)");
      assert(!v1.isValid(dnv));
      tr.passIfException(_dump);
   }
   
   {
      tr.test("optional");
      DynamicObject d;
      d["present"] = true;
      v::Map v(
         "present", new v::Type(Boolean),
         "missing", new v::Optional(new v::Valid()),
         NULL);
      assert(v.isValid(d));
      tr.passIfNoException();
   }
   
   // trick to test for extra values.  Optional check to see if key is
   // present.  If so, then force not valid.
   {
      tr.test("extra");
      DynamicObject d;
      d->setType(Map);
      DynamicObject d2;
      d2["extra"] = true;
      v::Map v(
         "extra", new v::Optional(new v::NotValid()),
         NULL);
      assert(v.isValid(d));
      tr.passIfNoException();
      
      tr.test("invalid extra");
      assert(!v.isValid(d2));
      tr.passIfException(_dump);
   }
   
   {
      tr.test("min");
      DynamicObject d;
      d = "1";
      
      v::Min v(0);
      assert(v.isValid(d));
      tr.passIfNoException();
      
      tr.test("invalid min");
      v::Min nv(2);
      assert(!nv.isValid(d));
      tr.passIfException(_dump);
   }
   
   {
      tr.test("max");
      DynamicObject d;
      d = "1";
      
      v::Max v(2);
      assert(v.isValid(d));
      tr.passIfNoException();
      
      tr.test("invalid max");
      v::Max nv(0);
      assert(!nv.isValid(d));
      tr.passIfException(_dump);
   }
   
   {
      DynamicObject d;
      
      tr.test("not");
      v::Not v(new v::NotValid());
      assert(v.isValid(d));
      tr.passIfNoException();
      
      tr.test("invalid not");
      v::Not nv(new v::Valid());
      assert(!nv.isValid(d));
      tr.passIfException(_dump);
   }
   
   {
      tr.test("equals");
      DynamicObject eq;
      eq = "db";
      DynamicObject dv;
      dv = "db";
      DynamicObject dnv;
      dnv = "db!";
      
      v::Equals v(eq);
      assert(v.isValid(dv));
      tr.passIfNoException();
      
      tr.test("invalid equals");
      assert(!v.isValid(dnv));
      tr.passIfException(_dump);
   }

   {
      tr.test("all");
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
      tr.passIfNoException();

      tr.test("invalid all");
      assert(!v.isValid(dnv));
      tr.passIfException(_dump);
   }

   {
      tr.test("any");
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
      tr.passIfNoException();

      tr.test("invalid any");
      assert(!v.isValid(dnv));
      tr.passIfException(_dump);
   }

   {
      DynamicObject dv0;
      dv0["q"] = "12";
      DynamicObject dnv0;
      dnv0["q"] = "";
      DynamicObject dnv1;
      dnv1["q"] = "12345";
      DynamicObject dnv2;
      // dnv2 empty
      DynamicObject dnv3;
      dnv3["q"] = "a";
      
      v::Any v(
         new v::Map(
            "q", new v::All(
               new v::Type(String),
               new v::Min(2, "q 2 short."),
               new v::Max(4, "q 2 long."),
               NULL),
            NULL),
         NULL);

      tr.test("any+map+all (valid q)");
      assert(v.isValid(dv0));
      tr.passIfNoException();

      tr.test("invalid any+map+all (short q)");
      assert(!v.isValid(dnv0));
      tr.passIfException(_dump);

      tr.test("invalid any+map+all (long q)");
      assert(!v.isValid(dnv1));
      tr.passIfException(_dump);

      tr.test("invalid any+map+all (empty)");
      assert(!v.isValid(dnv2));
      tr.passIfException(_dump);

      tr.test("invalid any+map+all (\"a\")");
      assert(!v.isValid(dnv3));
      tr.passIfException(_dump);
   }

   {
      tr.test("deep");
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
      tr.passIfNoException();

      tr.test("invalid deep");
      assert(!v.isValid(dnv));
      tr.passIfException(_dump);
   }

   {
      tr.test("each(array)");
      DynamicObject dv;
      dv[0] = "1234";
      dv[1] = "5678";
      dv[2] = "9012";
      DynamicObject dnv;
      dnv[0] = "1234";
      dnv[1] = "567";
      dnv[2] = "901";

      v::Each v(new v::Min(4));
      assert(v.isValid(dv));
      tr.passIfNoException();

      tr.test("invalid each(array)");
      assert(!v.isValid(dnv));
      assert(Exception::hasLast());
      ExceptionRef e = Exception::getLast();
      assert(e->getDetails()->hasMember("errors"));
      assert(e->getDetails()["errors"]->length() == 2);
      tr.passIfException(_dump);
   }

   {
      tr.test("each(map)");
      DynamicObject dv;
      dv["a"] = "1234";
      dv["b"] = "5678";
      dv["c"] = "9012";
      DynamicObject dnv;
      dnv["a"] = "1234";
      dnv["b"] = "567";
      dnv["c"] = "901";

      v::Each v(new v::Min(4));
      assert(v.isValid(dv));
      tr.passIfNoException();

      tr.test("invalid each(map)");
      assert(!v.isValid(dnv));
      assert(Exception::hasLast());
      ExceptionRef e = Exception::getLast();
      assert(e->getDetails()->hasMember("errors"));
      assert(e->getDetails()["errors"]->length() == 2);
      assert(e->getDetails()["errors"]->hasMember("b"));
      assert(e->getDetails()["errors"]->hasMember("c"));
      tr.passIfException(_dump);
   }

   {
      tr.test("in(map)");
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
      tr.passIfNoException();

      tr.test("invalid in(map)");
      assert(!v.isValid(dnv));
      tr.passIfException(_dump);
   }

   {
      tr.test("in(array)");
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
      tr.passIfNoException();

      tr.test("invalid in(array)");
      assert(!v.isValid(dnv));
      tr.passIfException(_dump);
   }
   
   {
      DynamicObject dv0;
      dv0 = 0;
      DynamicObject dv0s;
      dv0s = "0";
      DynamicObject dvu;
      dvu = 2;
      DynamicObject dvus;
      dvus = "2";
      DynamicObject dvs;
      dvs = -2;
      DynamicObject dvss;
      dvss = "-2";
      DynamicObject dnv;
      dnv = "x";
      
      // default, any int
      v::Int v0;
      // uint64_t
      v::Int vu(UInt64);
      // int64_t
      v::Int vs(Int64);
      // [-1, 1]
      v::Int vm(-1, 1);
      // [-1, 1]
      v::Int vm2(-2, 2);
      // >= 0
      v::Int vp(v::Int::NonNegative);
      // < 0
      v::Int vn(v::Int::Negative);
      
      tr.test("int");
      assert(v0.isValid(dv0));
      assert(vu.isValid(dv0));
      assert(vs.isValid(dv0));
      assert(vm.isValid(dv0));
      assert(vp.isValid(dv0));

      assert(v0.isValid(dv0s));
      assert(vu.isValid(dv0s));
      assert(vs.isValid(dv0s));
      assert(vm.isValid(dv0s));
      assert(vp.isValid(dv0s));

      assert(v0.isValid(dvu));
      assert(vu.isValid(dvu));
      assert(vs.isValid(dvu));
      assert(vp.isValid(dvu));
      
      assert(v0.isValid(dvus));
      assert(vu.isValid(dvus));
      assert(vs.isValid(dvus));
      assert(vp.isValid(dvus));
      
      assert(v0.isValid(dvs));
      assert(vs.isValid(dvs));
      assert(vn.isValid(dvs));
      
      assert(v0.isValid(dvss));
      assert(vs.isValid(dvss));
      assert(vn.isValid(dvss));
      
      assert(vm2.isValid(dvs));
      assert(vm2.isValid(dvu));
      tr.passIfNoException();

      tr.test("invalid int (string)");
      assert(!v0.isValid(dnv));
      tr.passIfException(_dump);
      
      tr.test("invalid int (min int)");
      assert(!vm.isValid(dvs));
      tr.passIfException(_dump);
      
      tr.test("invalid int (min string)");
      assert(!vm.isValid(dvss));
      tr.passIfException(_dump);
      
      tr.test("invalid int (max int)");
      assert(!vm.isValid(dvu));
      tr.passIfException(_dump);
      
      tr.test("invalid int (max string)");
      assert(!vm.isValid(dvus));
      tr.passIfException(_dump);
      
      tr.test("invalid int (not positive)");
      assert(!vp.isValid(dvs));
      tr.passIfException(_dump);
      
      tr.test("invalid int (not negative (0))");
      assert(!vn.isValid(dv0));
      tr.passIfException(_dump);
      
      tr.test("invalid int (not negative)");
      assert(!vn.isValid(dvu));
      tr.passIfException(_dump);
   }
   
   {
      tr.test("compare");
      DynamicObject dv;
      dv["a"] = 0;
      dv["b"] = 0;
      DynamicObject dnv;
      dnv["a"] = 0;
      dnv["b"] = 1;
      
      v::Compare v("a", "b");
      assert(v.isValid(dv));
      tr.passIfNoException();

      tr.test("invalid compare");
      assert(!v.isValid(dnv));
      tr.passIfException(_dump);
   }
   
   {
      tr.test("regex");
      DynamicObject dv;
      dv = "username";
      DynamicObject dnv;
      dnv = "user name";
      DynamicObject dnv2;
      dnv2 = 123;
      
      v::Regex v("^[a-zA-Z0-9_]+$");
      assert(v.isValid(dv));
      tr.passIfNoException();

      tr.test("invalid regex");
      assert(!v.isValid(dnv));
      tr.passIfException(_dump);

      tr.test("invalid regex (num)");
      assert(!v.isValid(dnv2));
      tr.passIfException(_dump);
   }
   
   {
      DynamicObject dv;
      dv = "bitmunk.com";
      DynamicObject dnv;
      dnv = "bitmunkxcom";
      
      v::Regex v("bitmunk.com$");
      tr.test("regex(dot)");
      assert(v.isValid(dv));
      tr.passIfNoException();

      tr.test("regex(dot)");
      assert(v.isValid(dnv));
      tr.passIfNoException();

      v::Regex ve("bitmunk\\.com$");
      tr.test("regex(escape dot)");
      assert(ve.isValid(dv));
      tr.passIfNoException();

      tr.test("regex(escape dot)");
      assert(!ve.isValid(dnv));
      tr.passIfException(_dump);
   }
   
   tr.group("register");
   {
      tr.test("init");
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
               new v::Min(6, "Username too short!"),
               new v::Max(16, "Username too long!"),
               NULL),
            "password", new v::All(
               new v::Type(String),
               new v::Min(6, "Password too short!"),
               new v::Max(16, "Password too long!"),
               NULL),
            "fullname", new v::All(
               new v::Type(String),
               new v::Min(1, "Full name too short!"),
               new v::Max(256, "Full name too long!"),
               NULL),
            "acceptToS", new v::All(
               new v::Type(Boolean),
               new v::Equals(t, "You must accept the Terms of Service!"),
               NULL),
            "email", new v::All(
               new v::Regex(
                  "^([a-zA-Z0-9_\\.\\-\\+])+\\@(([a-zA-Z0-9\\-])+\\.)+([a-zA-Z0-9]{2,4})+$",
                  "Invalid email format!"),
               new v::Not(new v::Regex(
                  "@bitmunk\\.com$"),
                  "Invalid email domain!"),
               NULL),
            /*
            "dob", new v::All(
               new v::Date(),
               new v::Age(18),
               NULL),
            "email", new v::Email(...),
            */
            NULL),
         new v::Compare("password", "password2", "Passwords do not match!"),
         NULL);
      tr.passIfNoException();
      
      {
         tr.test("valid");
         assert(v.isValid(dv));
         tr.passIfNoException();
      }
      
      {
         tr.test("invalid username type");
         DynamicObject dnv = dv.clone();
         dnv["username"] = false;
         assert(!v.isValid(dnv));
         tr.passIfException(_dump);
      }

      {
         tr.test("short username");
         DynamicObject dnv = dv.clone();
         dnv["username"] = "x";
         assert(!v.isValid(dnv));
         tr.passIfException(_dump);
      }

      {
         tr.test("long username");
         DynamicObject dnv = dv.clone();
         dnv["username"] = "01234567890123456";
         assert(!v.isValid(dnv));
         tr.passIfException(_dump);
      }

      // skipping password and fullname checking (same as username)
      
      {
         tr.test("no tos");
         DynamicObject dnv = dv.clone();
         dnv["acceptToS"] = false;
         assert(!v.isValid(dnv));
         tr.passIfException(_dump);
      }

      {
         tr.test("empty email");
         DynamicObject dnv = dv.clone();
         dnv["email"] = "";
         assert(!v.isValid(dnv));
         tr.passIfException(_dump);
      }

      {
         tr.test("no email domain");
         DynamicObject dnv = dv.clone();
         dnv["email"] = "joe";
         assert(!v.isValid(dnv));
         tr.passIfException(_dump);
      }

      {
         tr.test("junk email");
         DynamicObject dnv = dv.clone();
         dnv["email"] = "junk@email";
         assert(!v.isValid(dnv));
         tr.passIfException(_dump);
      }

      {
         tr.test("@bitmunk.com email");
         DynamicObject dnv = dv.clone();
         dnv["email"] = "liar@bitmunk.com";
         assert(!v.isValid(dnv));
         tr.passIfException(_dump);
      }

      {
         tr.test("invalid password2");
         DynamicObject dnv = dv.clone();
         dnv["password2"] = false;
         assert(!v.isValid(dnv));
         tr.passIfException(_dump);
      }
   }
   tr.ungroup();

   tr.group("sub map/array types");
   {
      tr.test("init");
      DynamicObject dvm;
      dvm["m"]["x"] = true;
      
      DynamicObject dva;
      dva["a"][0] = true;
      
      DynamicObject t;
      t = true;

      v::Map vm(
         "m", new v::Map(
            "x", new v::Equals(t),
            NULL),
         NULL);
      
      v::Map va(
         "a", new v::Array(0, new v::Equals(t), -1),
         NULL);
      tr.passIfNoException();
      
      {
         tr.test("valid");
         assert(vm.isValid(dvm));
         assert(va.isValid(dva));
         tr.passIfNoException();
      }
      
      {
         tr.test("invalid m");
         DynamicObject dnv = dvm.clone();
         dnv["m"] = false;
         assert(!vm.isValid(dnv));
         tr.passIfException(_dump);
      }
      
      {
         tr.test("invalid a");
         DynamicObject dnv = dvm.clone();
         dnv["a"] = false;
         assert(!va.isValid(dnv));
         tr.passIfException(_dump);
      }
   }
   tr.ungroup();

   tr.group("error paths");
   {
      tr.test("init");
      DynamicObject dv;
      dv["b"] = true;
      dv["m"]["b"] = true;
      dv["a"][0] = true;
      dv["m2"]["m"]["b"] = true;
      
      DynamicObject t;
      t = true;

      v::Map v(
         "b", new v::Equals(t),
         "m", new v::Map(
            "b", new v::Equals(t),
            NULL),
         "a", new v::Array(
            0, new v::Equals(t),
            -1),
         "m2", new v::Map(
            "m", new v::Map(
               "b", new v::Equals(t),
               NULL),
            NULL),
         NULL);
      tr.passIfNoException();
      
      {
         tr.test("valid");
         assert(v.isValid(dv));
         tr.passIfNoException();
      }
      
      {
         tr.test("invalid b");
         DynamicObject dnv = dv.clone();
         dnv["b"] = false;
         assert(!v.isValid(dnv));
         tr.passIfException(_dump);
      }
      
      {
         tr.test("invalid m");
         DynamicObject dnv = dv.clone();
         dnv["m"] = false;
         assert(!v.isValid(dnv));
         tr.passIfException(_dump);
      }
      
      {
         tr.test("invalid m.b");
         DynamicObject dnv = dv.clone();
         dnv["m"]["b"] = false;
         assert(!v.isValid(dnv));
         ExceptionRef e = Exception::getLast();
         assert(e->getDetails()["errors"]->hasMember("m.b"));
         tr.passIfException(_dump);
      }
      
      {
         tr.test("invalid a.0");
         DynamicObject dnv = dv.clone();
         dnv["a"][0] = false;
         assert(!v.isValid(dnv));
         ExceptionRef e = Exception::getLast();
         assert(e->getDetails()["errors"]->hasMember("a[0]"));
         tr.passIfException(_dump);
      }
      
      {
         tr.test("invalid m2.m.b");
         DynamicObject dnv = dv.clone();
         dnv["m2"]["m"]["b"] = false;
         assert(!v.isValid(dnv));
         ExceptionRef e = Exception::getLast();
         assert(e->getDetails()["errors"]->hasMember("m2.m.b"));
         tr.passIfException(_dump);
      }
   }
   tr.ungroup();

   tr.group("null objects");
   {
      tr.test("init");
      DynamicObject nullObject;
      nullObject.setNull();

      v::Map v("a", new v::Valid(), NULL);
      tr.passIfNoException();
      
      {
         tr.test("simple");
         DynamicObject nullObject;
         nullObject.setNull();

         assert(!v.isValid(nullObject));
         tr.passIfException(_dump);
      }
   }
   
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

#undef _dump

#ifndef DB_TEST_NO_MAIN
DB_TEST_MAIN(DbValidationTester)
#endif
