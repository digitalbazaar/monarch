/*
 * Copyright (c) 2005 Digital Bazaar, Inc.  All rights reserved.
 */
using System;
using System.Collections;
using System.IO;
using System.Text;
using System.Xml;

using DB.Common.Logging;

namespace DB.Common
{
   /// <summary>
   /// A ElementReader is a class that allows for easy xml element parsing. It
   /// moves an XmlReader according to xml elements.
   /// </summary>
   /// 
   /// <author>Dave Longley</author>
   public class ElementReader
   {
      /// <summary>
      /// The underlying XmlReader.
      /// </summary>
      protected XmlReader mReader;
      
      /// <summary>
      /// </summary>
      /// 
      /// <param name="reader">the XmlReader to read with.</param>
      public ElementReader(XmlReader reader)
      {
         mReader = reader;
      }
      
      /// <summary>
      /// Moves the internal XmlReader to the specified element or stops
      /// reading if the stop element is found.
      /// </summary>
      /// 
      /// <param name="name">the name of the element to read to.</param>
      /// <param name="stopName">the name of the element to stop reading
      /// at if found.</param>
      /// <returns>true if the element could be read to, false if not.</returns>
      public virtual bool Read(string name, string stopName)
      {
         bool rval = false;
         
         try
         {
            // make sure stop name hasn't already been reached
            if(!(Reader.Name == stopName &&
                 Reader.NodeType == XmlNodeType.EndElement))
            {
               // continue reading while "name" element is not found and while
               // "stopName" element or end element is not found
               Reader.Read();
               while(!(Reader.Name == name &&
                       Reader.NodeType == XmlNodeType.Element) &&
                     !(Reader.Name == stopName &&
                       (Reader.NodeType == XmlNodeType.Element ||
                        Reader.NodeType == XmlNodeType.EndElement)))
               {
                  Reader.Read();
               }
               
               // see if the element was found
               if(Reader.Name == name &&
                  (Reader.NodeType == XmlNodeType.Element ||
                   Reader.NodeType == XmlNodeType.EndElement))
               {
                  rval = true;
               }
            }
         }
         catch(Exception e)
         {
            LoggerManager.Error("dbcommon",
                                "could not read element!,name=" + name);
            LoggerManager.Debug("dbcommon", e.ToString());
         }
         
         return rval;
      }
      
      /// <summary>
      /// Moves the internal XmlReader to the next element or stops
      /// reading if the stop element is found.
      /// </summary>
      /// 
      /// <param name="stopName">the name of the element to stop reading
      /// at if found.</param>
      /// <returns>true if the next element was not the stopping element,
      /// false if not.</returns>
      public virtual bool ReadNext(string stopName)
      {
         bool rval = false;
         
         try
         {
            // make sure stop name hasn't already been reached
            if(!(Reader.Name == stopName &&
                 Reader.NodeType == XmlNodeType.EndElement))
            {
               // continue reading until element that isn't "stopName" is found
               Reader.Read();
               while(Reader.NodeType != XmlNodeType.Element &&
                     !(Reader.Name == stopName &&
                       Reader.NodeType == XmlNodeType.EndElement))
               {
                  Reader.Read();
               }
               
               // see if an element other than stop element was found
               if(Reader.Name != stopName &&
                  Reader.NodeType == XmlNodeType.Element)
               {
                  rval = true;
               }
            }
         }
         catch(Exception e)
         {
            LoggerManager.Error("dbcommon", "could not read next element!");
            LoggerManager.Debug("dbcommon", e.ToString());
         }
         
         return rval;         
      }
      
      /// <summary>
      /// Gets the underlying XmlReader so that its read data can be acquired.
      /// </summary>
      public virtual XmlReader Reader
      {
         get
         {
            return mReader;
         }
      }
      
      /// <summary>
      /// Gets a list of all the child nodes of the passed node that match
      /// the given tag name.
      /// </summary>
      /// 
      /// <param name="node">the node to get the children of.</param>
      /// <param name="name">the tag name for the children.</param>
      /// <returns>an array list with all of the children that matched the
      /// passed name.</returns>
      public static ArrayList GetChildNodes(XmlNode node, string name)
      {
         ArrayList children = new ArrayList();

         // get all child nodes with the specified name
         foreach(XmlNode child in node.ChildNodes)
         {
            if(child.Name == name)
            {
               children.Add(child);
            }
         }
         
         return children;
      }
      
      /// <summary>
      /// Gets the first child of the passed node with the given tag name.
      /// </summary>
      /// 
      /// <param name="node">the node to get the first child of.</param>
      /// <param name="name">the tag name for the child.</param>
      /// <returns>the xml node for the first child that matched the passed
      /// name or null.</returns>
      public static XmlNode GetFirstChild(XmlNode node, string name)
      {
         XmlNode rval = null;

         // get first child node with the specified name
         foreach(XmlNode child in node.ChildNodes)
         {
            if(child.Name == name)
            {
               rval = child;
               break;
            }
         }
         
         return rval;
      }
   }
}
