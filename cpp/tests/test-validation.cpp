/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */

#include "monarch/test/Test.h"
#include "monarch/test/TestModule.h"
#include "monarch/rt/DynamicObject.h"
#include "monarch/validation/Validation.h"

#include <cstdio>

using namespace std;
using namespace monarch::test;
using namespace monarch::rt;
namespace v = monarch::validation;

// Use to dump out expected exceptions
#define _dump false

namespace mo_test_validation
{

static void runValidatorTest(TestRunner& tr)
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
      assertExceptionSet();
      if(_dump) dumpException();
      assertStrCmp(
         Exception::get()->getType(), "monarch.validation.ValidationError");
      assertStrCmp(
         Exception::get()->getMessage(),
         "The given object does not meet all of the data validation "
         "requirements. Please examine the error details for more information "
         "about the specific requirements.");
      Exception::clear();
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
      assert(Exception::isSet());
      ExceptionRef e = Exception::get();
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
      tr.test("member");
      DynamicObject dv;
      dv["a"] = 0;
      dv["b"] = true;
      DynamicObject dnv;
      dnv["c"] = false;
      dnv["d"] = "false";

      v::All v0(
         new v::Member("a", "'a' does not exist"),
         new v::Member("b"),
         NULL);
      assert(v0.isValid(dv));
      tr.passIfNoException();

      tr.test("invalid members");
      assert(!v0.isValid(dnv));
      tr.passIfException();
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
      tr.test("array (unordered)");
      DynamicObject dv;
      dv[0] = 0;
      dv[1] = true;
      DynamicObject dnv;
      dnv[0] = false;
      dnv[1] = "false";

      // create with addValidator
      v::Array v0;
      v0.addValidator(-1, new v::Type(Boolean));
      v0.addValidator(-1, new v::Type(Int32));
      assert(v0.isValid(dv));
      tr.passIfNoException();

      tr.test("invalid array (unordered)");
      assert(!v0.isValid(dnv));
      tr.passIfException(_dump);

      tr.test("array (unordered clist)");
      // create with constructor list
      v::Array v1(
         new v::Type(Boolean),
         new v::Type(Int32),
         NULL);
      assert(v1.isValid(dv));
      tr.passIfNoException();

      tr.test("invalid array (unordered clist)");
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

      v::Equals v2("db");
      assert(v2.isValid(dv));
      tr.passIfNoException();

      tr.test("invalid equals");
      assert(!v.isValid(dnv));
      assert(!v2.isValid(dnv));
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
      assert(Exception::isSet());
      ExceptionRef e = Exception::get();
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
      assert(Exception::isSet());
      ExceptionRef e = Exception::get();
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
      tr.test("compare text");

      const char* text = "Apples\nAnd\nOranges\n";

      DynamicObject dv;
      dv["a"] = "Apples\r\nAnd\rOranges\n";
      dv["b"] = "Apples\r\nAnd\r\nOranges\r\n";
      dv["c"] = "Apples\nAnd\nOranges\n";
      DynamicObject dnv;
      dnv["a"] = "Apples\r\rAnd\rOranges\r";
      dnv["b"] = "Apples\r\rAndOranges";

      v::CompareText v(text);
      assert(v.isValid(dv["a"]));
      assert(v.isValid(dv["b"]));
      assert(v.isValid(dv["c"]));
      tr.passIfNoException();

      tr.test("invalid compare text");
      assert(!v.isValid(dnv["a"]));
      assert(!v.isValid(dnv["b"]));
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

   {
      // whitespace test
      DynamicObject dv;
      dv = "test";
      DynamicObject dnvs;
      dnvs = " test";
      DynamicObject dnve;
      dnve = "test ";
      DynamicObject dnvse;
      dnvse = " test ";

      v::Regex v("^[^[:space:]]{1}.*[^[:space:]]{1}$");

      tr.test("ws v");
      assert(v.isValid(dv));
      tr.passIfNoException();

      tr.test("regex(ws start)");
      assert(!v.isValid(dnvs));
      tr.passIfException(_dump);

      tr.test("regex(ws end)");
      assert(!v.isValid(dnve));
      tr.passIfException(_dump);

      tr.test("regex(ws both)");
      assert(!v.isValid(dnvse));
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
         ExceptionRef e = Exception::get();
         assert(e->getDetails()["errors"]->hasMember("m.b"));
         tr.passIfException(_dump);
      }

      {
         tr.test("invalid a.0");
         DynamicObject dnv = dv.clone();
         dnv["a"][0] = false;
         assert(!v.isValid(dnv));
         ExceptionRef e = Exception::get();
         assert(e->getDetails()["errors"]->hasMember("a[0]"));
         tr.passIfException(_dump);
      }

      {
         tr.test("invalid m2.m.b");
         DynamicObject dnv = dv.clone();
         dnv["m2"]["m"]["b"] = false;
         assert(!v.isValid(dnv));
         ExceptionRef e = Exception::get();
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
   tr.ungroup();

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

static void runValidatorFactoryTest(TestRunner& tr)
{
   tr.group("ValidatorFactory");

   tr.test("Type");
   {
      v::ValidatorFactory vf;

      // load definitions
      DynamicObject types;
      types->append("String");
      types->append("UInt32");
      types->append("Int32");
      types->append("UInt64");
      types->append("Int64");
      types->append("Boolean");
      types->append("Map");
      types->append("Array");
      DynamicObjectIterator i = types.getIterator();
      while(i->hasNext())
      {
         const char* next = i->next();

         DynamicObject def;
         def["type"] = next;
         def["extends"] = "Type";
         def["def"] = next;

         assertNoException(
            vf.loadValidatorDefinitions(def));
      }

      // test string
      {
         DynamicObject value;
         value = "a string";
         v::ValidatorRef val = vf.createValidator("String");
         assertNoExceptionSet();
         assertNoException(val->isValid(value));
      }
      // test uint32
      {
         DynamicObject value;
         value = (uint32_t)1;
         v::ValidatorRef val = vf.createValidator("UInt32");
         assertNoExceptionSet();
         assertNoException(val->isValid(value));
      }
      // test int32
      {
         DynamicObject value;
         value = (int32_t)1;
         v::ValidatorRef val = vf.createValidator("Int32");
         assertNoExceptionSet();
         assertNoException(val->isValid(value));
      }
      // test uint64
      {
         DynamicObject value;
         value = (uint64_t)1;
         v::ValidatorRef val = vf.createValidator("UInt64");
         assertNoExceptionSet();
         assertNoException(val->isValid(value));
      }
      // test int64
      {
         DynamicObject value;
         value = (int64_t)1;
         v::ValidatorRef val = vf.createValidator("Int64");
         assertNoExceptionSet();
         assertNoException(val->isValid(value));
      }
      // test boolean
      {
         DynamicObject value;
         value = true;
         v::ValidatorRef val = vf.createValidator("Boolean");
         assertNoExceptionSet();
         assertNoException(val->isValid(value));
      }
      // test map
      {
         DynamicObject value;
         value->setType(Map);
         v::ValidatorRef val = vf.createValidator("Map");
         assertNoExceptionSet();
         assertNoException(val->isValid(value));
      }
      // test array
      {
         DynamicObject value;
         value->setType(Array);
         v::ValidatorRef val = vf.createValidator("Array");
         assertNoExceptionSet();
         assertNoException(val->isValid(value));
      }
   }
   tr.passIfNoException();

   tr.test("Regex");
   {
      v::ValidatorFactory vf;

      DynamicObject def;
      def["type"] = "test";
      def["extends"] = "Regex";
      def["def"] = "^(true|false)$";

      assertNoException(
         vf.loadValidatorDefinitions(def));

      DynamicObject value;
      value = "true";
      v::ValidatorRef val = vf.createValidator("test");
      assertNoExceptionSet();
      assertNoException(val->isValid(value));
   }
   tr.passIfNoException();

   tr.test("Null");
   {
      v::ValidatorFactory vf;

      DynamicObject def;
      def["type"] = "test";
      def["extends"] = "Null";

      assertNoException(
         vf.loadValidatorDefinitions(def));

      DynamicObject value(NULL);
      v::ValidatorRef val = vf.createValidator("test");
      assertNoExceptionSet();
      assertNoException(val->isValid(value));
   }
   tr.passIfNoException();

   tr.test("NotValid");
   {
      v::ValidatorFactory vf;

      DynamicObject def;
      def["type"] = "test";
      def["extends"] = "NotValid";
      def["error"] = "This won't pass no matter what.";

      assertNoException(
         vf.loadValidatorDefinitions(def));

      DynamicObject value;
      v::ValidatorRef val = vf.createValidator("test");
      assertNoExceptionSet();
      assertException(val->isValid(value));
   }
   tr.passIfException();

   tr.test("NotCompare");
   {
      v::ValidatorFactory vf;

      DynamicObject def;
      def["type"] = "test";
      def["extends"] = "NotCompare";
      def["def"]["key1"] = "password";
      def["def"]["key2"] = "passwordConfirm";

      assertNoException(
         vf.loadValidatorDefinitions(def));

      DynamicObject value;
      value["password"] = "password";
      value["passwordConfirm"] = "i messed up while typing";
      v::ValidatorRef val = vf.createValidator("test");
      assertNoExceptionSet();
      assertNoException(val->isValid(value));
   }
   tr.passIfNoException();

   tr.test("Compare");
   {
      v::ValidatorFactory vf;

      DynamicObject def;
      def["type"] = "test";
      def["extends"] = "Compare";
      def["def"]["key1"] = "password";
      def["def"]["key2"] = "passwordConfirm";

      assertNoException(
         vf.loadValidatorDefinitions(def));

      DynamicObject value;
      value["password"] = "password";
      value["passwordConfirm"] = "password";
      v::ValidatorRef val = vf.createValidator("test");
      assertNoExceptionSet();
      assertNoException(val->isValid(value));
   }
   tr.passIfNoException();

   tr.test("Not");
   {
      v::ValidatorFactory vf;

      DynamicObject def;
      def["type"] = "test";
      def["extends"] = "Not";
      def["def"]["type"] = "NotValid";

      assertNoException(
         vf.loadValidatorDefinitions(def));

      DynamicObject value;
      v::ValidatorRef val = vf.createValidator("test");
      assertNoExceptionSet();
      assertNoException(val->isValid(value));
   }
   tr.passIfNoException();

   tr.test("Min");
   {
      v::ValidatorFactory vf;

      DynamicObject def;
      def["type"] = "test";
      def["extends"] = "Min";
      def["def"] = 1;

      assertNoException(
         vf.loadValidatorDefinitions(def));

      DynamicObject value;
      value = "a";
      v::ValidatorRef val = vf.createValidator("test");
      assertNoExceptionSet();
      assertNoException(val->isValid(value));
   }
   tr.passIfNoException();

   tr.test("Max");
   {
      v::ValidatorFactory vf;

      DynamicObject def;
      def["type"] = "test";
      def["extends"] = "Max";
      def["def"] = 2;

      assertNoException(
         vf.loadValidatorDefinitions(def));

      DynamicObject value;
      value = "a";
      v::ValidatorRef val = vf.createValidator("test");
      assertNoExceptionSet();
      assertNoException(val->isValid(value));
   }
   tr.passIfNoException();

   tr.test("Member");
   {
      v::ValidatorFactory vf;

      DynamicObject def;
      def["type"] = "test";
      def["extends"] = "Member";
      def["def"] = "foo";

      assertNoException(
         vf.loadValidatorDefinitions(def));

      DynamicObject value;
      value["foo"] = "bar";
      v::ValidatorRef val = vf.createValidator("test");
      assertNoExceptionSet();
      assertNoException(val->isValid(value));
   }
   tr.passIfNoException();

   tr.test("Int");
   {
      {
         v::ValidatorFactory vf;
         DynamicObject def;
         def["type"] = "test";
         def["extends"] = "Int";
         def["def"]["type"] = "Positive";
         assertNoException(
            vf.loadValidatorDefinitions(def));

         DynamicObject value;
         value = 1;
         v::ValidatorRef val = vf.createValidator("test");
         assertNoExceptionSet();
         assertNoException(val->isValid(value));
      }
      {
         v::ValidatorFactory vf;
         DynamicObject def;
         def["type"] = "test";
         def["extends"] = "Int";
         def["def"]["type"] = "Negative";
         assertNoException(
            vf.loadValidatorDefinitions(def));

         DynamicObject value;
         value = "-1";
         v::ValidatorRef val = vf.createValidator("test");
         assertNoExceptionSet();
         assertNoException(val->isValid(value));
      }
      {
         v::ValidatorFactory vf;
         DynamicObject def;
         def["type"] = "test";
         def["extends"] = "Int";
         def["def"]["type"] = "NonPositive";
         assertNoException(
            vf.loadValidatorDefinitions(def));

         DynamicObject value;
         value = -1;
         v::ValidatorRef val = vf.createValidator("test");
         assertNoExceptionSet();
         assertNoException(val->isValid(value));
      }
      {
         v::ValidatorFactory vf;
         DynamicObject def;
         def["type"] = "test";
         def["extends"] = "Int";
         def["def"]["type"] = "NonNegative";
         assertNoException(
            vf.loadValidatorDefinitions(def));

         DynamicObject value;
         value = "0";
         v::ValidatorRef val = vf.createValidator("test");
         assertNoExceptionSet();
         assertNoException(val->isValid(value));
      }
      {
         v::ValidatorFactory vf;
         DynamicObject def;
         def["type"] = "test";
         def["extends"] = "Int";
         def["def"]["type"] = "Zero";
         assertNoException(
            vf.loadValidatorDefinitions(def));

         DynamicObject value;
         value = 0;
         v::ValidatorRef val = vf.createValidator("test");
         assertNoExceptionSet();
         assertNoException(val->isValid(value));
      }
      {
         v::ValidatorFactory vf;
         DynamicObject def;
         def["type"] = "test";
         def["extends"] = "Int";
         def["def"]["min"] = 0;
         def["def"]["max"] = 10;
         assertNoException(
            vf.loadValidatorDefinitions(def));

         DynamicObject value;
         value = 5;
         v::ValidatorRef val = vf.createValidator("test");
         assertNoExceptionSet();
         assertNoException(val->isValid(value));
      }
   }
   tr.passIfNoException();

   tr.test("In");
   {
      v::ValidatorFactory vf;

      DynamicObject def;
      def["type"] = "test";
      def["extends"] = "In";
      def["def"]->append("foo");

      assertNoException(
         vf.loadValidatorDefinitions(def));

      DynamicObject value;
      value = "foo";
      v::ValidatorRef val = vf.createValidator("test");
      assertNoExceptionSet();
      assertNoException(val->isValid(value));
   }
   tr.passIfNoException();

   tr.test("Equals");
   {
      v::ValidatorFactory vf;

      DynamicObject def;
      def["type"] = "test";
      def["extends"] = "Equals";
      def["def"] = "foo";

      assertNoException(
         vf.loadValidatorDefinitions(def));

      DynamicObject value;
      value = "foo";
      v::ValidatorRef val = vf.createValidator("test");
      assertNoExceptionSet();
      assertNoException(val->isValid(value));
   }
   tr.passIfNoException();

   tr.test("Each");
   {
      v::ValidatorFactory vf;

      DynamicObject def;
      def["type"] = "test";
      def["extends"] = "Each";
      def["def"]["type"] = "Equals";
      def["def"]["def"] = "foo";

      assertNoException(
         vf.loadValidatorDefinitions(def));

      DynamicObject value;
      value->append("foo");
      value->append("foo");
      v::ValidatorRef val = vf.createValidator("test");
      assertNoExceptionSet();
      assertNoException(val->isValid(value));
   }
   tr.passIfNoException();

   tr.test("Map");
   {
      v::ValidatorFactory vf;

      DynamicObject def;
      def["type"] = "test";
      def["extends"] = "Map";
      def["def"]["foo"]["type"] = "Equals";
      def["def"]["foo"]["def"] = "bar";
      def["def"]["bar"]["type"] = "Equals";
      def["def"]["bar"]["def"] = 10;

      assertNoException(
         vf.loadValidatorDefinitions(def));

      DynamicObject value;
      value["foo"] = "bar";
      value["bar"] = 10;
      v::ValidatorRef val = vf.createValidator("test");
      assertNoExceptionSet();
      assertNoException(val->isValid(value));
   }
   tr.passIfNoException();

   tr.test("Array");
   {
      {
         v::ValidatorFactory vf;
         DynamicObject def;
         def["type"] = "test";
         def["extends"] = "Array";
         {
            DynamicObject& d = def["def"]->append();
            d["index"] = 0;
            d["type"] = "Equals";
            d["def"] = "foo";
         }
         assertNoException(
            vf.loadValidatorDefinitions(def));

         DynamicObject value;
         value->append("foo");
         v::ValidatorRef val = vf.createValidator("test");
         assertNoExceptionSet();
         assertNoException(val->isValid(value));
      }
      {
         v::ValidatorFactory vf;
         DynamicObject def;
         def["type"] = "test";
         def["extends"] = "Array";
         {
            DynamicObject& d = def["def"]->append();
            d["type"] = "Equals";
            d["def"] = "bar";
         }

         assertNoException(
            vf.loadValidatorDefinitions(def));

         DynamicObject value;
         value->append("bar");
         value->append("foo");
         v::ValidatorRef val = vf.createValidator("test");
         assertNoExceptionSet();
         assertNoException(val->isValid(value));
      }
   }
   tr.passIfNoException();

   tr.test("Any");
   {
      v::ValidatorFactory vf;

      DynamicObject def;
      def["type"] = "test";
      def["extends"] = "Any";
      {
         DynamicObject& d = def["def"]->append();
         d["type"] = "Equals";
         d["def"] = "bar";
      }
      {
         DynamicObject& d = def["def"]->append();
         d["type"] = "Equals";
         d["def"] = "foo";
      }

      assertNoException(
         vf.loadValidatorDefinitions(def));

      DynamicObject value;
      value = "foo";
      v::ValidatorRef val = vf.createValidator("test");
      assertNoExceptionSet();
      assertNoException(val->isValid(value));
   }
   tr.passIfNoException();

   tr.test("All");
   {
      v::ValidatorFactory vf;

      DynamicObject def;
      def["type"] = "test";
      def["extends"] = "All";
      {
         DynamicObject& d = def["def"]->append();
         d["type"] = "Map";
         d["def"]["foo"]["type"] = "Equals";
         d["def"]["foo"]["def"] = "bar";
      }
      {
         DynamicObject& d = def["def"]->append();
         d["type"] = "Map";
         d["def"]["bar"]["type"] = "Equals";
         d["def"]["bar"]["def"] = "foo";
      }

      assertNoException(
         vf.loadValidatorDefinitions(def));

      DynamicObject value;
      value["foo"] = "bar";
      value["bar"] = "foo";
      v::ValidatorRef val = vf.createValidator("test");
      assertNoExceptionSet();
      assertNoException(val->isValid(value));
   }
   tr.passIfNoException();

   tr.test("Custom");
   {
      v::ValidatorFactory vf;

      {
         DynamicObject def;
         def["type"] = "custom";
         def["extends"] = "All";
         {
            DynamicObject& d = def["def"]->append();
            d["type"] = "Map";
            d["def"]["foo"]["type"] = "Equals";
            d["def"]["foo"]["def"] = "bar";
         }
         {
            DynamicObject& d = def["def"]->append();
            d["type"] = "Map";
            d["def"]["bar"]["type"] = "Equals";
            d["def"]["bar"]["def"] = "foo";
         }

         assertNoException(
            vf.loadValidatorDefinitions(def));
      }

      {
         DynamicObject def;
         def["type"] = "test";
         def["extends"] = "All";
         {
            DynamicObject& d = def["def"]->append();
            d["type"] = "custom";
         }
         {
            DynamicObject& d = def["def"]->append();
            d["type"] = "Map";
            d["def"]["hello"]["type"] = "Equals";
            d["def"]["hello"]["def"] = "world";
         }

         assertNoException(
            vf.loadValidatorDefinitions(def));
      }

      DynamicObject value;
      value["foo"] = "bar";
      value["bar"] = "foo";
      value["hello"] = "world";
      v::ValidatorRef val = vf.createValidator("test");
      assertNoExceptionSet();
      assertNoException(val->isValid(value));
   }
   tr.passIfNoException();

   tr.test("Custom - dependency not met");
   {
      v::ValidatorFactory vf;

      {
         DynamicObject def;
         def["type"] = "custom";
         def["extends"] = "All";
         {
            DynamicObject& d = def["def"]->append();
            d["type"] = "Map";
            d["def"]["foo"]["type"] = "Equals";
            d["def"]["foo"]["def"] = "bar";
         }
         {
            DynamicObject& d = def["def"]->append();
            d["type"] = "Map";
            d["def"]["bar"]["type"] = "Equals";
            d["def"]["bar"]["def"] = "foo";
         }

         assertNoException(
            vf.loadValidatorDefinitions(def));
      }

      {
         DynamicObject def;
         def["type"] = "test";
         def["extends"] = "All";
         {
            DynamicObject& d = def["def"]->append();
            d["type"] = "I DONT EXIST";
         }
         {
            DynamicObject& d = def["def"]->append();
            d["type"] = "Map";
            d["def"]["hello"]["type"] = "Equals";
            d["def"]["hello"]["def"] = "world";
         }

         assertException(
            vf.loadValidatorDefinitions(def));
      }
   }
   tr.passIfException();

   tr.ungroup();
}

static void runAnyExceptionsTest(TestRunner& tr)
{
   tr.group("Any validator exceptions");

   v::Any v(
      new v::Map(
         "dog", new v::Equals("woof"),
         "cat", new v::Equals("meow"),
         NULL),
      new v::Int(v::Int::Positive),
      NULL);

   tr.test("map fail 1");
   {
      DynamicObject mapFail;
      mapFail["dog"] = "bowwow";
      mapFail["cat"] = "meow";

      assert(!v.isValid(mapFail));
      printf("\nShould be able to tell that only \"dog\" was incorrect.\n");
      DynamicObject ex = Exception::getAsDynamicObject();
      dumpDynamicObject(ex);
      printf("\nIt looks like this for only a map validator (w/o the Any):\n");

      Exception::clear();
      v::Map v2(
         "dog", new v::Equals("woof"),
         "cat", new v::Equals("meow"),
         NULL);
      assert(!v2.isValid(mapFail));
      ex = Exception::getAsDynamicObject();
      dumpDynamicObject(ex);
   }
   tr.passIfException();

   tr.test("map fail 2");
   {
      DynamicObject mapFail;
      mapFail["dog"] = "woof";

      assert(!v.isValid(mapFail));
      printf("\nShould be able to tell that only \"cat\" was missing.\n");
      ExceptionRef e = Exception::get();
      DynamicObject ex = Exception::convertToDynamicObject(e);
      dumpDynamicObject(ex);
      printf("\nIt looks like this for only a map validator (w/o the Any):\n");

      Exception::clear();
      v::Map v2(
         "dog", new v::Equals("woof"),
         "cat", new v::Equals("meow"),
         NULL);
      assert(!v2.isValid(mapFail));
      e = Exception::get();
      ex = Exception::convertToDynamicObject(e);
      dumpDynamicObject(ex);
   }
   tr.passIfException();

   tr.test("int fail 1");
   {
      DynamicObject intFail;
      intFail = 0;

      assert(!v.isValid(intFail));
      printf("\nShould be able to tell that only integer wasn't positive.\n");
      ExceptionRef e = Exception::get();
      DynamicObject ex = Exception::convertToDynamicObject(e);
      dumpDynamicObject(ex);
      printf("\nIt looks like this for only an int validator (w/o the Any):\n");

      Exception::clear();
      v::Int v2(v::Int::Positive);
      assert(!v2.isValid(intFail));
      e = Exception::get();
      ex = Exception::convertToDynamicObject(e);
      dumpDynamicObject(ex);
   }
   tr.passIfException();

   tr.ungroup();
}

#undef _dump

static bool run(TestRunner& tr)
{
   if(tr.isDefaultEnabled())
   {
      runValidatorTest(tr);
      runValidatorFactoryTest(tr);
   }
   if(tr.isTestEnabled("any-exception"))
   {
      runAnyExceptionsTest(tr);
   }
   if(tr.isTestEnabled("validator-factory"))
   {
      runValidatorFactoryTest(tr);
   }
   return true;
}

} // end namespace

MO_TEST_MODULE_FN(
   "monarch.tests.validation.test", "1.0", mo_test_validation::run)
