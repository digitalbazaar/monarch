/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;

import com.db.data.DynamicObject;
import com.db.data.JsonReader;
import com.db.data.JsonWriter;
import com.db.logging.LoggerManager;
import com.db.logging.Logger;

/**
 * The unit tests for JSON-related stuff.
 * 
 * @author Dave Longley
 */
public class UTJson
{
   public static String dynamicObjectToString(
      DynamicObject dyno, boolean compact)
      throws IOException
   {
      JsonWriter jw = new JsonWriter();
      jw.setCompact(compact);
      if(!compact)
      {
         jw.setIndentation(0, 3);
      }
      ByteArrayOutputStream baos = new ByteArrayOutputStream();
      jw.write(dyno, baos);
      return baos.toString("UTF-8");
   }
   
   public static void dumpDynamicObject(DynamicObject dyno, boolean compact)
      throws IOException
   {
      System.out.println(dynamicObjectToString(dyno, compact));
   }
   
   public static void runJsonValidTest() throws IOException
   {
      System.out.println("JSON (Valid)");
      
      String[] tests = 
      {
         "{}",
         "[]",
         " []",
         "[] ",
         " [] ",
         " [ ] ",
         "[true]",
         "[false]",
         "[null]",
         "[ true]",
         "[true ]",
         "[ true ]",
         "[true, true]",
         "[true , true]",
         "[ true , true ]",
         "[0]",
         "[-0]",
         "[0.0]",
         "[-0.0]",
         "[0.0e0]",
         "[0.0e+0]",
         "[0.0e-0]",
         "[1.0]",
         "[-1.0]",
         "[1.1]",
         "[-1.1]",
         "[0,true]",
         "[[]]",
         "[[{}]]",
         "[[],[]]",
         "[[0]]",
         "[\"\"]",
         "[\"s\"]",
         "{\"k\":\"v\"}",
         "{\"k1\":1, \"k2\":2}",
         "{\"k\":[]}",
         "{\"k\":{}}",
         "[\" \\\" \\\\ \\/ \\b \\f \\n \\r \\t\"]",
         "{\"k\":true}",
         "{\"k\":0}",
         "{\"k\":10}",
         "{\"k\":-10}",
         "{\"k\":0.0e+0}",
         "{\"k\":\"v\",\"k2\":true,\"k3\":1000,\"k4\":\"v\"}",
         "[\"\\u0020\"]",
         "[\"\u00c3\u0084 \u00c3\u00a4 \u00c3\u0096 \u00c3\u00b6 \u00c3\u009c \u00c3\u00bc \u00c3\u009f\"]"
      };
      
      JsonReader jr = new JsonReader();
      for(int i = 0; i < tests.length; i++)
      {
         System.out.println(String.format("Parse #%d", i));
         
         DynamicObject d = new DynamicObject();
         jr.start(d);
         jr.read(new ByteArrayInputStream(tests[i].getBytes("UTF-8")));
         jr.finish();
      }
      
      System.out.println("PASS");
   }
   
   public static void runJsonInvalidTest() throws IOException
   {
      System.out.println("JSON (Invalid)");
      
      String[] tests =
      {
         "",
         " ",
         "{",
         "}",
         "[",
         "]",
         "{}{",
         "[][",
         "[tru]",
         "[junk]",
         "[true,]",
         "[true, ]",
         "[,true]",
         "[ ,true]",
         "[0.]",
         "[0.0e]",
         "[0.0e+]",
         "[0.0e-]",
         "[\"\0\"]",
         "[\"\\z\"]",
         "[\"\0\"]",
         "{\"k\":}",
         "{:\"v\"}",
         "{\"k\":1,}",
         "{,\"k\":1}",
         "{null:0}",
         "[\"\n\"]",
         "[\"\t\"]"
      };
      
      JsonReader jr = new JsonReader();
      boolean ex = true;
      for(int i = 0; ex && i < tests.length; i++)
      {
         System.out.println(String.format("Parse #%d", i));
         ex = false;
         
         try
         {
            DynamicObject d = new DynamicObject();
            jr.start(d);
            jr.read(new ByteArrayInputStream(tests[i].getBytes("UTF-8")));
            jr.finish();
         }
         catch(IOException e)
         {
            ex = true;
         }
      }
      
      assert(ex);
      
      System.out.println("PASS");
   }
   
   public static void runJsonDJDTest() throws IOException
   {
      System.out.println("JSON (Dyno->JSON->Dyno)");
      
      DynamicObject dyno0 = new DynamicObject();
      dyno0.set("email", "example@example.com");
      dyno0.set("AIM", "example");
      dumpDynamicObject(dyno0, true);
      
      DynamicObject dyno1 = new DynamicObject();
      dyno1.set("id", 2);
      dyno1.set("-id", -2);
      dyno1.get("floats").set(0, 0.0);
      dyno1.get("floats").set(1, -0.0);
      dyno1.get("floats").set(2, 1.0);
      dyno1.get("floats").set(3, -1.0);
      dyno1.get("floats").set(4, 1.23456789);
      dyno1.get("floats").set(5, -1.23456789);
      dyno1.set("username", "testuser1000");
      dyno1.set("l33t", true);
      dyno1.set("luser", false);
      dyno1.get("somearray").set(0, "item1");
      dyno1.get("somearray").set(1, "item2");
      dyno1.get("somearray").set(2, "item3");
      dyno1.get("somearray").set(3, dyno0);
      dyno1.set("contact", dyno0);
      
      DynamicObject[] dynos =
      {
         dyno0,
         dyno1
      };
      
      JsonWriter jw = new JsonWriter();
      JsonReader jr = new JsonReader();
      for(int i = 0; i < dynos.length; i++)
      {
         System.out.println(String.format("Verify #%d", i));
         
         DynamicObject d = dynos[i];
         //dumpDynamicObject(d, false);
         
         ByteArrayOutputStream baos = new ByteArrayOutputStream();
         jw.setCompact(true);
         jw.write(d, baos);
         baos.reset();
         
         jw.setCompact(false);
         jw.setIndentation(0, 3);
         jw.write(d, baos);
         ByteArrayInputStream bais =
            new ByteArrayInputStream(baos.toByteArray());
         DynamicObject dr = new DynamicObject();
         jr.start(dr);
         jr.read(bais);
         jr.finish();
      }
      
      System.out.println("PASS");
   }
   
   public static void runJsonVerifyDJDTest() throws IOException
   {
      System.out.println("JSON (Verify Dyno->JSON->Dyno)");
      
      int tdcount = 0;
      DynamicObject td = new DynamicObject();
      td.get(tdcount).get("dyno").setType(DynamicObject.Type.Map);
      td.get(tdcount++).set("JSON", "{}");
      td.get(tdcount).get("dyno").setType(DynamicObject.Type.Array);
      td.get(tdcount++).set("JSON", "[]");
      td.get(tdcount).get("dyno").set(0, true);
      td.get(tdcount++).set("JSON", "[true]");
      td.get(tdcount).get("dyno").set("k", "v");
      td.get(tdcount++).set("JSON", "{\"k\":\"v\"}");
      td.get(tdcount).get("dyno").set(0, 0);
      td.get(tdcount++).set("JSON", "[0]");
      td.get(tdcount).get("dyno").set(0, "\n");
      td.get(tdcount++).set("JSON", "[\"\\n\"]");
      td.get(tdcount).get("dyno").set(0, td.get(0).get("dyno"));
      td.get(tdcount++).set("JSON", "[{}]");
      td.get(tdcount).get("dyno").set(0, -1);
      td.get(tdcount++).set("JSON", "[-1]");
      td.get(tdcount).get("dyno").set(0, (DynamicObject)null);
      td.get(tdcount++).set("JSON", "[null]");
      td.get(tdcount).get("dyno").set("k", 0);
      td.get(tdcount++).set("JSON", "{\"k\":0}");
      td.get(tdcount).get("dyno").set("k", 10);
      td.get(tdcount++).set("JSON", "{\"k\":10}");
      td.get(tdcount).get("dyno").set("k", -10);
      td.get(tdcount++).set("JSON", "{\"k\":-10}");
      td.get(tdcount).get("dyno").set(0, "\u0001");
      td.get(tdcount++).set("JSON", "[\"\\u0001\"]");
      // test if UTF-16 C escapes translate into a UTF-8 JSON string
      td.get(tdcount).get("dyno").set(0,
         "\u040e \u045e \u0404 \u0454 \u0490 \u0491");
      td.get(tdcount++).set("JSON",
         "[\"\u00d0\u008e \u00d1\u009e \u00d0\u0084 \u00d1\u0094 \u00d2\u0090 \u00d2\u0091\"]");
      
      JsonWriter jw = new JsonWriter();
      JsonReader jr = new JsonReader();
      for(int i = 0; i < tdcount; i++)
      {
         System.out.println(String.format("Verify #%d", i));
         
         DynamicObject d = td.get(i).get("dyno");
         String s = td.get(i).get("JSON").getString();
         
         ByteArrayOutputStream baos = new ByteArrayOutputStream();
         jw.setCompact(true);
         jw.write(d, baos);
         
         // verify written string
         assert(s.length() == baos.size());
         assert(s.equals(baos.toString("UTF-8")));
         
         ByteArrayInputStream bais =
            new ByteArrayInputStream(baos.toByteArray());
         DynamicObject dr = new DynamicObject();
         jr.start(dr);
         jr.read(bais);
         jr.finish();
         
         assert(d.equals(dr));
      }
      
      System.out.println("PASS");
   }
   
   /**
    * Runs the JSON unit tests.
    * 
    * @param args the args.
    */
   public static void main(String[] args)
   {
      try
      {
         // set up logger
         LoggerManager.setFile("dbdata", "ut-json.log", false);
         LoggerManager.setFileVerbosity("dbdata", Logger.DETAIL_VERBOSITY);
         LoggerManager.setConsoleVerbosity("dbdata", Logger.ERROR_VERBOSITY);
         
         runJsonValidTest();
         runJsonInvalidTest();
         runJsonDJDTest();
         runJsonVerifyDJDTest();
         
         System.out.println("Tests passed.");
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
}
