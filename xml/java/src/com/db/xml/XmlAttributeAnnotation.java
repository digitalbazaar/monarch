/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.xml;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * An XmlAttributeAnnotation is an annotation that indicates that a particular
 * method returns a value that can be serialized to an XmlAttribute.
 * 
 * This annotation is present at runtime (needed for serialization) and can
 * be applied to a METHOD (a method). The return value of the method will
 * provide the value for the attribute.
 * 
 * @author Dave Longley
 */
@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.METHOD)
public @interface XmlAttributeAnnotation
{
   /**
    * The name of the xml attribute.
    */
   String name();
   
   /**
    * The namespace URI for the xml attribute.
    */
   String namespaceUri() default "";
   
   /**
    * True if the xml attribute should inherit its parent xml element's
    * namespace URI, false if not. 
    */
   boolean inheritNamespaceUri() default true;   
}
