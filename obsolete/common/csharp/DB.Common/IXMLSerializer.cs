/*
 * Copyright (c) 2005 Digital Bazaar, Inc.  All rights reserved.
 */
using System;
using System.Xml;

namespace DB.Common
{
   /// <summary>
   /// An interface that allows an object to be serialized to
   /// and from XML.
   /// </summary>
   ///
   /// <author>Dave Longley</author>
   public interface IXMLSerializer
   {
      /// <summary>
      /// Gets/Sets the serializer options used to configure how to convert
      /// to and convert from xml.
      /// </summary>
      int SerializerOptions
      {
         get;
         set;
      }
      
      /// <summary>
      /// Returns the root tag name for this serializer.
      /// </summary>
      string RootTag
      {
         get;
      }
      
      /// <summary>
      /// This method takes the object representation and creates an
      /// XML-based representation of the object.
      /// </summary>
      ///
      /// <param name="doc">true to create a new document, false not to.</param>
      /// <returns>the XML-based representation of the object.</returns>
      string ConvertToXML(bool doc);
      
      /// <summary>
      /// This method takes the object representation and writes out the
      /// XML-based representation of the object.
      /// </summary>
      ///
      /// <param name="writer">the xml writer used to write to xml.</param>
      void ConvertToXML(XmlWriter writer);
      
      /// <summary>
      /// This method takes XML text (in full document form) and converts
      /// it to it's internal representation.
      /// </summary>
      ///
      /// <param name="xmlText">the xml text document that represents the
      /// object.</param>
      /// <returns>true if successful, false otherwise.</returns>
      bool ConvertFromXML(string xmlText);
      
      /// <summary>
      /// This method takes an XML reader and converts data read from it into
      /// thi object's representation.
      /// </summary>
      ///
      /// <param name="reader">the xml reader that is reading this object's
      /// information.</param>
      /// <returns>true if successful, false otherwise.</returns>
      bool ConvertFromXML(XmlReader reader);
   }
}
