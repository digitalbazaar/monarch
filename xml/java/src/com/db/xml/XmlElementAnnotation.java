/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.xml;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * An XmlElementAnnotation is an annotation that indicates that a particular
 * method returns data that can be serialized to an XmlElement.
 * 
 * This annotation is present at runtime (needed for serialization) and can
 * be applied to a METHOD (a method). The return value of the method will
 * provide the value for the element.
 * 
 * @author Dave Longley
 */
@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.METHOD)
public @interface XmlElementAnnotation
{
   /**
    * The name of the xml element.
    */
   String name();
   
   /**
    * The namespace URI for the xml element.
    */
   String namespaceUri() default "";
   
   /**
    * True if the xml element should inherit its parent's namespace URI,
    * false if not. 
    */
   boolean inheritNamespaceUri() default true;
   
   /**
    * True if the method's return value is Iterable and its elements
    * should be serialized as children to the xml element.
    */
   boolean serializeIterableAsChildren() default false;
}
