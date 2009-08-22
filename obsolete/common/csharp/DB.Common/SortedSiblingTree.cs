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
   /// A tree with nodes that can have multiple ordered children. The
   /// children of a node are ordered relative to their siblings,
   /// not relative to the entire tree. 
   /// </summary>
   /// 
   /// <author>Dave Longley</author>
   public class SortedSiblingTree
   {
      /// <summary>
      /// The base sibling node. This node has no parents and its object is
      /// always null.
      /// </summary>
      protected SiblingNode mRoot;
      
      /// <summary>
      /// Creates a new sorted sibling tree.
      /// </summary>
      public SortedSiblingTree()
      {
         mRoot = new SiblingNode(null);
      }
      
      /// <summary>
      /// Finds a sibling node based on its data.
      /// </summary>
      /// 
      /// <param name="node">the node to start searching at.</param>
      /// <param name="data">the data to search with.</param>
      /// <returns>the sibling node or null if no match was found.</returns>
      protected virtual SiblingNode FindNode(SiblingNode node, object data)
      {
         SiblingNode rval = null;
         
         // if the starting node is null, set it to the root node
         if(node == null)
         {
            node = mRoot;
         }
         
         // check this node
         object nodeData = (node.Data == null) ? null: node.Data;
         if(data == nodeData || (data != null && data.Equals(nodeData)))
         {
            rval = node;
         }
         else
         {
            // check this node's children
            foreach(SiblingNode child in node)
            {
               rval = FindNode(child, data);
               if(rval != null)
               {
                  break;
               }
            }
         }
         
         return rval;
      }

      /// <summary>
      /// Finds a sibling node based on its object.
      /// </summary>
      /// 
      /// <param name="data">the data to search for.</param>
      /// <returns>the sibling node or null if no match was found.</returns>
      protected virtual SiblingNode FindNode(object data)
      {
         return FindNode(null, data);
      }
      
      /// <summary>
      /// Adds a child to a parent.
      /// </summary>
      /// 
      /// <param name="parent">the parent object to add the child to.</param>
      /// <param name="child">the child object to add.</param>
      /// <returns>true if the child was added, false if not.</returns>
      public virtual bool Add(object parent, object child)
      {
         bool rval = false;
         
         SiblingNode node = FindNode(parent);
         if(node != null)
         {
            rval = node.AddChild(new SiblingNode(child));
         }
         
         return rval;
      }
      
      /// <summary>
      /// Adds a child as a root sibling.
      /// </summary>
      /// 
      /// <param name="child">the child object to add.</param>
      /// <returns>true if the child was added, false if not.</returns>
      public virtual bool Add(object child)
      {
         return Add(null, child);
      }
      
      /// <summary>
      /// Removes a child from a parent.
      /// </summary>
      /// 
      /// <param name="parent">the parent object to remove the child
      /// from.</param>
      /// <param name="child">the child object to remove.</param>
      /// <returns>true if the child was removed, false if not.</returns>
      public virtual bool Remove(object parent, object child)
      {
         bool rval = false;
         
         SiblingNode node = FindNode(parent);
         if(node != null)
         {
            SiblingNode sn = FindNode(node, child);
            if(sn != null)
            {
               rval = node.RemoveChild(sn);
            }
         }
         
         return rval;
      }
      
      /// <summary>
      /// Gets all of the root siblings of this tree.
      /// </summary>
      /// 
      /// <returns>a list containing the root siblings. Calling "remove" or
      /// "add" on the returned list will not affect the tree structure.
      /// </returns>
      public virtual ArrayList RootSiblings
      {
         get
         {
            return GetChildren(null);
         }
      }   
      
      /// <summary>
      /// Gets the children of a parent.
      /// 
      /// <param name="parent">the parent to get the children of.</param>
      /// <returns>a list of the parent's children. Calling "remove" or
      /// "add" on the returned vector will not affect the tree structure.
      /// </returns>
      /// </summary>
      public virtual ArrayList GetChildren(object parent)
      {
         ArrayList children = new ArrayList();
         
         SiblingNode node = FindNode(parent);
         if(node != null)
         {
            foreach(SiblingNode child in node)
            {
               children.Add(child.Data);
            }
         }
         
         return children;
      }
      
      /// <summary>
      /// A node in a sorted sibling tree.
      /// </summary>
      /// 
      /// <author>Dave Longley</author>
      public class SiblingNode : IEnumerable
      {
         /// <summary>
         /// The data stored in this node.
         /// </summary>
         protected Object mData;
         
         /// <summary>
         /// The parent of this node. 
         /// </summary>
         protected SiblingNode mParent;
         
         /// <summary>
         /// The children of this sibling node.
         /// </summary>
         protected ArrayList mChildren;
         
         /// <summary>
         /// Creates a new sibling node.
         /// </summary>
         /// 
         /// <param name="data">the data to store in the node.</param>
         public SiblingNode(object data)
         {
            mData = data;
            mParent = null;
            mChildren = new ArrayList();
         }
         
         /// <summary>
         /// Gets the object inside this node.
         /// </summary>
         /// 
         /// <returns>the data inside of this node.</returns>
         public virtual object Data
         {
            get
            {
               return mData;
            }
         }
         
         /// <summary>
         /// Gets/Sets the parent of this node.
         /// </summary>
         public virtual SiblingNode Parent
         {
            get
            {
               return mParent;
            }
         
            set
            {
               mParent = value;
            }
         }
         
         /// <summary>
         /// Gets the children of this node.
         /// </summary>
         /// 
         /// <returns>the children of node in a list.</returns>
         public virtual ArrayList Children
         {
            get
            {
               return mChildren;
            }
         }
         
         /// <summary>
         /// Gets an enumerator on the children of this node.
         /// </summary>
         ///
         /// <returns>an enumerator on the children of this node.</returns>
         public virtual IEnumerator GetEnumerator()
         {
            return Children.GetEnumerator();
         }
         
         /// <summary>
         /// Returns a child of this node according to its index.
         /// </summary>
         ///
         /// <returns>the child of this node at the specified index.</returns>
         public virtual SiblingNode this[int index]
         {
            get
            {
               return (SiblingNode)Children[index];
            }
         }
         
         /// <summary>
         /// Adds a child to this node's children.
         /// </summary>
         /// 
         /// <param name="child">the children node to add.</param>
         /// <returns>true if the child was added, false if not.</returns>
         public virtual bool AddChild(SiblingNode child)
         {
            bool rval = false;
            
            if(child.Data is IComparer)
            {
               IComparer c = (IComparer)child.Data;
            
               // add the child according to its order
               int count = Children.Count;
               for(int i = 0; i < count; i++)
               {
                  SiblingNode sibling = (SiblingNode)Children[i];
                  if(c.Compare(child.Data, sibling.Data) < 0)
                  {
                     Children.Insert(i, child);
                     child.Parent = this;
                     rval = true;
                     break;
                  }
               }
               
               // if the child is the largest (not added yet),
               // add it to the end
               if(count == Children.Count)
               {
                  if(Children.Add(child) >= 0)
                  {
                     child.Parent = this;
                     rval = true;
                  }
               }
            }
            else
            {
               // cannot compare objects, so arbitrarily add
               if(Children.Add(child) >= 0)
               {
                  child.Parent = this;
                  rval = true;
               }
            }
            
            return rval;
         }
         
         /// <summary>
         /// Removes a child from this node's children.
         /// </summary>
         /// 
         /// <param name="child">the node to remove.</param>
         /// <returns>true if the child was removed, false if not.</returns>
         public virtual bool RemoveChild(SiblingNode child)
         {
            int count = Children.Count;
            Children.Remove(child);
            
            return count < Children.Count;
         }
         
         /// <summary>
         /// Gets the siblings of this node (includes this node).
         /// </summary>
         /// 
         /// <returns>the siblings of this node (includes this node).</returns>
         public virtual ArrayList Siblings
         {
            get
            {
               ArrayList siblings = null;
               
               if(Parent != null)
               {
                  siblings = Parent.Children;
               }
               else
               {
                  siblings = new ArrayList();
                  siblings.Add(this);
               }
               
               return siblings;
            }
         }
      }
   }
}
