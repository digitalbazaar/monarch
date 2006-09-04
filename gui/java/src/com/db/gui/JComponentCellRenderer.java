/*
 * Copyright (c) 2003-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Graphics;

import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JTable;
import javax.swing.JTree;
import javax.swing.ListCellRenderer;
import javax.swing.UIManager;
import javax.swing.table.TableCellRenderer;

/**
 * This class allows any JComponent to become a cell renderer. So far,
 * only list cell and table cell renderer interfaces have been implemented.
 * 
 * @author Dave Longley
 */
public class JComponentCellRenderer extends JComponent
implements ListCellRenderer, TableCellRenderer
{
   /**
    * The component to paint with.
    */
   protected Component mComponent;

   /**
    * The table being painted, if any.
    */
   protected JTable mTable;
   
   /**
    * Special requirements if the cell renderer is a JTree.
    */
   protected JTree mJTree;
   
   /**
    * The row to be painted.
    */
   protected int mPaintRow;
   
   /**
    * Sets bounds if the cell renderer is a JTree.
    * 
    * @param x the x position.
    * @param y the y position.
    * @param w the width.
    * @param h the height.
    */
   public void setBounds(int x, int y, int w, int h)
   {
      if(mJTree != null)
      {
         // a tree must always be set to the table height
         mJTree.setBounds(x, 0, w, mTable.getHeight());          
      }
      else
      {
         // ensure components are not painted larger than their max size
         //Dimension max = mComponent.getMaximumSize();
         //w = Math.min(max.width, w);
         //h = Math.min(max.height, h);
         
         mComponent.setBounds(x, y, w, h);
      }
   }
   
   /**
    * Paints this renderer.
    * 
    * @param g the graphics to paint with.
    */
   public void paint(Graphics g)
   { 
      if(mJTree != null)
      {
         g.translate(0, -mPaintRow * mJTree.getRowHeight());          
         mJTree.paint(g);
      }
      else
      {
         // paint the component
         mComponent.paint(g);
      }
   }
   
   /**
    * Return a component that has been configured to display the specified
    * value. That component's <code>paint</code> method is then called to
    * "render" the cell.  If it is necessary to compute the dimensions
    * of a list because the list cells do not have a fixed size, this method
    * is called to generate a component on which <code>getPreferredSize</code>
    * can be invoked.
    *
    * @param list The JList we're painting.
    * @param value The value returned by list.getModel().getElementAt(index).
    * @param index The cells index.
    * @param isSelected true if the specified cell was selected.
    * @param hasFocus true if the specified cell has the focus.
    * 
    * @return A component whose paint() method will render the specified value.
    */
   public Component getListCellRendererComponent(
      JList list, Object value, int index, boolean isSelected, boolean hasFocus)
   {
      if(value instanceof JComponent)
      {
         mComponent = (JComponent)value;
      }
      else if(value != null)
      {
         JLabel label = new JLabel(value.toString());
         label.setOpaque(true);
         mComponent = label;
      }
      
      // set colors for component
      if(mComponent instanceof FastProgressBar)
      {
         // fast progress bar is currently a special case
         FastProgressBar fpb = (FastProgressBar)mComponent;
         
         if(isSelected)
         {
            // use selected colors
            fpb.useSelectedColors();
         }
         else
         {
            // reset colors
            fpb.resetColors();
         }
      }
      else
      {
         // for setting the current cell's colors
         Color foreground;
         Color background;

         // if the cell is selected, set colors to list's selection colors
         if(isSelected)
         {
            foreground = list.getSelectionForeground();
            background = list.getSelectionBackground();
         }
         else
         {
            foreground = list.getForeground();
            background = list.getBackground();
         }
      
         // set the background color
         if(mComponent != null)
         {
             mComponent.setForeground(foreground);
             mComponent.setBackground(background);
         }
      }
      
      return mComponent;
   }
         
   /**
    * Returns the component used for drawing the cell. This method is
    * used to configure the renderer appropriately before drawing.
    *      
    * @param table this parameter is not used in this cell renderer.
    * @param value the value of the cell to be rendered.
    * @param isSelected true if the cell to be rendered is selected,
    *                   otherwise false.
    * @param hasFocus if true, render cell appropriately.
    * @param row the row index of the cell being drawn.
    * @param column the column index of the cell being drawn.
    *     
    * @return the component used for drawing the cell. 
    */
   public Component getTableCellRendererComponent(JTable table,
                                                  Object value,
                                                  boolean isSelected,
                                                  boolean hasFocus,
                                                  int row, int column)
   {
      mTable = table;
      mPaintRow = row;
      
      mJTree = null;
      
      if(value instanceof JComponent)
      {
         // special painting required for JTrees
         if(value instanceof JTree)
         {
            mJTree = (JTree)value;
         }

         mComponent = (JComponent)value;
      }
      else if(value != null)
      {
         JLabel label = new JLabel(value.toString());
         label.setOpaque(true);
         
         // restrict label height
         Dimension max = label.getMaximumSize();
         max.height = label.getMinimumSize().height;
         label.setMaximumSize(max);
         mComponent = label;
      }
      
      // set colors for component
      if(mComponent instanceof FastProgressBar)
      {
         // fast progress bar is currently a special case
         FastProgressBar fpb = (FastProgressBar)mComponent;
         
         if(isSelected)
         {
            // use selected colors
            fpb.useSelectedColors();
         }
         else
         {
            // reset colors
            fpb.resetColors();
         }
      }
      else
      {
         // for setting the current cell's colors
         Color foreground;
         Color background;
           
         // if the cell is selected, set colors to table's selection colors
         if(isSelected)
         {
            foreground = table.getSelectionForeground();
            background = table.getSelectionBackground();
         }
         else
         {
            foreground = table.getForeground();
            background = table.getBackground();
         }
           
         // if its editable use the edit colors
         if(hasFocus && table.isCellEditable(row, column))
         {
            foreground = UIManager.getColor("Table.focusCellForeground");
            background = UIManager.getColor("Table.focusCellBackground");
         }
           
         // set the colors
         if(mComponent != null)
         {
            mComponent.setForeground(foreground);
            mComponent.setBackground(background);
         }
      }
        
      return this;
   }
}
