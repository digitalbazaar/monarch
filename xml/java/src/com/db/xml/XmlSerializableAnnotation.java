/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.xml;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * An XmlSerializableAnnotation is an annotation that indicates that a
 * particular class, interface, or enum can be serialized to an XmlElement.
 * 
 * This annotation is present at runtime (needed for serialization) and can
 * be applied to a TYPE (a class, interface, or enum) or a METHOD (a method).
 * 
 * @author Dave Longley
 */
@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.TYPE)
public @interface XmlSerializableAnnotation
{
   /**
    * The name of the root xml element.
    */
   String rootElementName();
   
   /**
    * The namespace URI for the root xml element.
    */
   String namespaceUri() default "";
   
   /**
    * True if the xml element should inherit its parent's namespace URI,
    * false if not. 
    */
   boolean inheritNamespaceUri() default true;
}
