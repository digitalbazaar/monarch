/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.util;

import java.util.Comparator;
import java.util.Iterator;
import java.util.Vector;

/**
 * A tree with nodes that can have multiple ordered children. The
 * children of a node are ordered relative to their siblings,
 * not relative to the entire tree. 
 * 
 * @param <T> the type of object to store in this SortedSiblingTree.
 * 
 * @author Dave Longley
 */
public class SortedSiblingTree<T>
{
   /**
    * The base sibling node. This node has no parents and its object is
    * always null.
    */
   protected SiblingNode mRoot;
   
   /**
    * Creates a new sorted sibling tree.
    */
   public SortedSiblingTree()
   {
      mRoot = new SiblingNode(null);
   }
   
   /**
    * Finds a sibling node based on its object.
    * 
    * @param node the node to start searching at.
    * @param object the object to search with.
    * 
    * @return the sibling node or null if no match was found.
    */
   protected SiblingNode findNode(SiblingNode node, T object)
   {
      SiblingNode rval = null;
      
      // if the starting node is null, set it to the root node
      if(node == null)
      {
         node = mRoot;
      }
      
      // check this node
      // FIXME: why does the line below seem to be unnecessarily complicated?
      Object nodeObject = (node.getObject() == null) ? null: node.getObject();
      if(object == nodeObject || (object != null && object.equals(nodeObject)))
      {
         rval = node;
      }
      else
      {
         // check this node's children
         Iterator i = node.getChildren().iterator();
         while(i.hasNext())
         {
            rval = findNode((SiblingNode)i.next(), object);
            if(rval != null)
            {
               break;
            }
         }
      }
      
      return rval;
   }

   /**
    * Finds a sibling node based on its object.
    * 
    * @param object the object to search with.
    * 
    * @return the sibling node or null if no match was found.
    */
   protected SiblingNode findNode(T object)
   {
      return findNode(null, object);
   }
   
   /**
    * Adds a child to a parent.
    * 
    * @param parent the parent object to add the child to.
    * @param child the child object to add.
    * 
    * @return true if the child was added, false if not.
    */
   public boolean add(T parent, T child)
   {
      boolean rval = false;
      
      SiblingNode node = findNode(parent);
      if(node != null)
      {
         rval = node.addChild(new SiblingNode(child));
      }
      
      return rval;
   }
   
   /**
    * Adds a child as a root sibling.
    * 
    * @param child the child object to add.
    * 
    * @return true if the child was added, false if not.
    */
   public boolean add(T child)
   {
      return add(null, child);
   }
   
   /**
    * Removes a child from a parent.
    * 
    * @param parent the parent object to remove the child from.
    * @param child the child object to remove.
    * 
    * @return true if the child was removed, false if not.
    */
   public boolean remove(T parent, T child)
   {
      boolean rval = false;
      
      SiblingNode node = findNode(parent);
      if(node != null)
      {
         SiblingNode sn = findNode(node, child);
         if(sn != null)
         {
            rval = node.removeChild(sn);
         }
      }
      
      return rval;
   }
   
   /**
    * Gets all of the root siblings of this tree.
    * 
    * @return a vector containing the root siblings. Calling "remove" or
    *         "add" on the returned vector will not affect the tree structure.
    */
   public Vector<T> getRootSiblings()
   {
      return getChildren(null);
   }   
   
   /**
    * Gets the children of a parent.
    * 
    * @param parent the parent to get the children of.
    * 
    * @return a vector of the parent's children. Calling "remove" or
    *         "add" on the returned vector will not affect the tree structure.
    */
   public Vector<T> getChildren(T parent)
   {
      Vector<T> children = new Vector<T>();
      
      SiblingNode node = findNode(parent);
      if(node != null)
      {
         for(SiblingNode sn: node.getChildren())
         {
            children.add(sn.getObject());
         }
      }
      
      return children;
   }
   
   /**
    * Returns the total number of entries in this tree.
    * 
    * @return the total number of entries in this tree.
    */
   public int size()
   {
      return mRoot.getDescendantCount();
   }
   
   /**
    * A node in a sorted sibling tree.
    * 
    * @author Dave Longley
    */
   public class SiblingNode
   {
      /**
       * The object stored in this node.
       */
      protected T mObject;
      
      /**
       * The parent of this node. 
       */
      protected SiblingNode mParent;
      
      /**
       * The children of this sibling node.
       */
      protected Vector<SiblingNode> mChildren;
      
      /**
       * Creates a new sibling node.
       * 
       * @param obj the object to store in the node.
       */
      public SiblingNode(T obj)
      {
         mObject = obj;
         mParent = null;
         mChildren = new Vector<SiblingNode>();
      }
      
      /**
       * Gets the object inside this node.
       * 
       * @return the object inside of this node.
       */
      public T getObject()
      {
         return mObject;
      }
      
      /**
       * Sets the parent of this node.
       * 
       * @param parent the parent node of this node.
       */
      public void setParent(SiblingNode parent)
      {
         mParent = parent;
      }
      
      /**
       * Gets the parent of this node.
       * 
       * @return the parent of this node.
       */
      public SiblingNode getParent()
      {
         return mParent;
      }
      
      /**
       * Gets the children of this node.
       * 
       * @return the children of node.
       */
      public Vector<SiblingNode> getChildren()
      {
         return mChildren;
      }
      
      /**
       * Adds a child to this node's children.
       * 
       * @param child the children node to add.
       * 
       * @return true if the child was added, false if not.
       */
      @SuppressWarnings("unchecked")      
      public boolean addChild(SiblingNode child)
      {
         boolean rval = false;
         
         T object = child.getObject();
         if(object instanceof Comparator)
         {
            Comparator<T> c = (Comparator<T>)object;
         
            // add the child according to its order
            Vector<SiblingNode> children = getChildren();
            int size = children.size();
            for(int i = 0; i < size && rval; i++)
            {
               T siblingObject = children.get(i).getObject();
               if(c.compare(object, siblingObject) < 0)
               {
                  children.insertElementAt(child, i);
                  child.setParent(this);
                  rval = true;
               }
            }
            
            // if the child is the largest, add it to the end
            if(size == children.size())
            {
               if(children.add(child))
               {
                  child.setParent(this);
                  rval = true;
               }
            }
         }
         else
         {
            // cannot compare objects, so arbitrarily add
            if(getChildren().add(child))
            {
               child.setParent(this);
               rval = true;
            }
         }
         
         return rval;
      }
      
      /**
       * Removes a child from this node's children.
       * 
       * @param child the node to remove.
       * 
       * @return true if the child was removed, false if not.
       */
      public boolean removeChild(SiblingNode child)
      {
         return getChildren().remove(child);
      }
      
      /**
       * Gets the siblings of this node (includes this node).
       * 
       * @return the siblings of this node (includes this node).
       */
      public Vector<SiblingNode> getSiblings()
      {
         Vector<SiblingNode> siblings = null;
         
         if(mParent != null)
         {
            siblings = mParent.getChildren();
         }
         else
         {
            siblings = new Vector<SiblingNode>();
            siblings.add(this);            
         }
         
         return siblings;
      }
      
      /**
       * Gets the total number of descendants this node has.
       * 
       * @return the total number of descendants this node has.
       */
      public int getDescendantCount()
      {
         int rval = getChildren().size();
         
         Iterator i = getChildren().iterator();
         while(i.hasNext())
         {
            SiblingNode child = (SiblingNode)i.next();
            rval += child.getDescendantCount();
         }
         
         return rval;
      }
   }
}

