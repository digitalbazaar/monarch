/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
import java.util.Vector;

import com.db.logging.LoggerManager;
import com.db.logging.Logger;
import com.db.xml.XmlAttributeAnnotation;
import com.db.xml.XmlElement;
import com.db.xml.XmlElementAnnotation;
import com.db.xml.XmlSerializableAnnotation;
import com.db.xml.XmlSerializer;

/**
 * A unit test for the XmlSerializer class.
 * 
 * @author Dave Longley
 */
public class UTXmlSerializer
{
   /**
    * Runs the XmlSerializer test.
    * 
    * @param args the args.
    */
   public static void main(String[] args)
   {
      try
      {
         // set up logger
         LoggerManager.setFile("dbxml", "ut-filelock.log", false);
         LoggerManager.setFileVerbosity("dbxml", Logger.DETAIL_VERBOSITY);
         LoggerManager.setConsoleVerbosity("dbxml", Logger.ERROR_VERBOSITY);
         
         // create test objects
         Test1 test1 = new Test1();
         Test2 test2 = new Test2();
         Test3 test3 = new Test3();
         
         // create an XmlSerializer
         XmlSerializer serializer = new XmlSerializer();
         XmlElement element1 = serializer.serialize(test1);
         XmlElement element2 = serializer.serialize(test2, element1);
         element1.addChild(element2);
         XmlElement element3 = serializer.serialize(test3);
         
         //System.out.println("xml1=\n" + element1.convertToXml());
         //System.out.println("\nxml2=\n" + element2.convertToXml());
         System.out.println("xml3=\n" + element3.convertToXml());
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
   
   /**
    * A test XmlSerializable class.
    * 
    * @author Dave Longley
    */
   @XmlSerializableAnnotation(rootElementName = "test")
   public static class Test1
   {
      /**
       * Creates a new Test.
       */
      public Test1()
      {
      }
      
      /**
       * Returns the "foo" attribute for the test xml element.
       * 
       * @return the "foo" attribute for the test xml element.
       */
      @XmlAttributeAnnotation(name = "foo")
      public String getFooAttribute()
      {
         return "bar";
      }
      
      /**
       * Returns the value for the test xml element.
       * 
       * @return the value for the test xml element.
       */
      @XmlElementAnnotation(name = "test")
      public String getValue()
      {
         return "mydata";
      }
   }
   
   /**
    * A test XmlSerializable class.
    * 
    * @author Dave Longley
    */
   @XmlSerializableAnnotation(
      rootElementName = "test", namespaceUri = "urn:test2")
   public static class Test2
   {
      /**
       * Creates a new Test.
       */
      public Test2()
      {
      }
      
      /**
       * Returns the "xmlns:myprefix" attribute for the test xml element.
       * 
       * @return the "xmlns:myprefix" attribute for the test xml element.
       */
      @XmlAttributeAnnotation(name = "xmlns:myprefix")
      public String getMyPrefixAttribute()
      {
         return "urn:test2";
      }
      
      /**
       * Returns the "xmlns:myprefix" attribute for the test xml element.
       * 
       * @return the "xmlns:myprefix" attribute for the test xml element.
       */
      @XmlAttributeAnnotation(name = "xmlns:myprefixtwo")
      public String getMyPrefix2Attribute()
      {
         return "urn:whatever";
      }      
      
      /**
       * Returns the "foo" attribute for the test xml element.
       * 
       * @return the "foo" attribute for the test xml element.
       */
      @XmlAttributeAnnotation(name = "foo", namespaceUri = "urn:whatever")
      public String getFooAttribute()
      {
         return "bar";
      }
      
      /**
       * Returns the child element for the test xml element.
       * 
       * @return the child element for the test xml element.
       */
      @XmlElementAnnotation(name = "child")
      public String getValue()
      {
         return "mydata";
      }
   }
   
   /**
    * A test XmlSerializable class.
    * 
    * @author Dave Longley
    */
   @XmlSerializableAnnotation(
      rootElementName = "test", namespaceUri = "urn:test3")
   public static class Test3
   {
      /**
       * A vector Test1s.
       */
      protected Vector<Test1> mTest1s;
      
      /**
       * Creates a new Test.
       */
      public Test3()
      {
         mTest1s = new Vector<Test1>();
         
         mTest1s.add(new Test1());
         mTest1s.add(new Test1());
         mTest1s.add(new Test1());
      }
      
      /**
       * Returns the "foo" attribute for the test xml element.
       * 
       * @return the "foo" attribute for the test xml element.
       */
      @XmlAttributeAnnotation(name = "foo")
      public String getFooAttribute()
      {
         return "bar";
      }
      
      /**
       * Returns the child element for the test xml element.
       * 
       * @return the child element for the test xml element.
       */
      @XmlElementAnnotation(name = "child")
      public String getValue()
      {
         return "mydata";
      }
      
      /**
       * Returns the vector of test1s as children of the root element.
       * 
       * @return the vector of test1s as children of the root element.
       */
      @XmlElementAnnotation(
         name = "test", namespaceUri = "urn:test3",
         serializeIterableAsChildren = true)
      public Vector<Test1> getTest1sAsRootChildren()
      {
         return mTest1s;
      }

      /**
       * Returns the vector of test1s.
       * 
       * @return the vector of test1s.
       */
      @XmlElementAnnotation(
         name = "kid", serializeIterableAsChildren = true)
      public Vector<Test1> getTest1sAsKids()
      {
         return mTest1s;
      }      
   }
}
