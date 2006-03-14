/*
 * Copyright (c) 2005 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.common.gui;

import javax.swing.JTree;
import javax.swing.plaf.basic.BasicTreeUI;
import javax.swing.tree.TreeModel;
import javax.swing.tree.TreeNode;

/**
 * Draws arrows next to tree cells to indicate expanded/collapsed states.
 * 
 * @author Dave Longley
 */
public class ArrowJTree extends JTree
{
   /**
    * Creates a new arrow JTree.
    * 
    * @param root the tree node root.
    */
   public ArrowJTree(TreeNode root)
   {
      super(root);
      
      ((BasicTreeUI)getUI()).setExpandedIcon(null);
      ((BasicTreeUI)getUI()).setCollapsedIcon(null);
      setCellRenderer(new ArrowTreeCellRenderer());
   }
   
   /**
    * Creates a new arrow JTree.
    * 
    * @param tm the tree model for the tree.
    */
   public ArrowJTree(TreeModel tm)
   {
      super(tm);
      
      ((BasicTreeUI)getUI()).setExpandedIcon(null);
      ((BasicTreeUI)getUI()).setCollapsedIcon(null);
      setCellRenderer(new ArrowTreeCellRenderer());
   }
}
