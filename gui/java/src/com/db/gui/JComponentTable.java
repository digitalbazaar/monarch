/*
 * Copyright (c) 2003-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import java.awt.Dimension;

import javax.swing.JComponent;
import javax.swing.JTable;
import javax.swing.table.JTableHeader;
import javax.swing.table.TableModel;

/**
 * Table that allows JComponents to be displayed in its cells and header.
 * 
 * Sorting is also supported.
 * 
 * @author Dave Longley
 */
public class JComponentTable extends JTable
{
   /**
    * Creates a new JComponentTable using the passed JComponentTableModel.
    * 
    * @param jctm the JComponentTableModel to use.
    */
   public JComponentTable(JComponentTableModel jctm)
   {
      super(jctm);
      
      setupTableHeader();
      
      // set default editors for JComponents
      setDefaultEditor(JComponent.class, new JComponentCellEditor());
      setDefaultRenderer(JComponent.class, new JComponentCellRenderer());
      
      // set table dimensions
      jctm.setTableDimensions(this);
   }
   
   /**
    * Sets up the table header.
    */
   protected void setupTableHeader()
   {
      // setup the table header
      JComponentTableHeader header =
         new JComponentTableHeader(getColumnModel());
      setTableHeader(header);
   }
   
   /**
    * Sets the data model for this table to <code>newModel</code> and registers
    * with it for listener notifications from the new data model.
    *
    * @param dataModel the new data source for this table.
    */
   public void setModel(TableModel dataModel)
   {
      super.setModel(dataModel);
      
      JTableHeader header = getTableHeader();
      if(header != null)
      {
         header.setColumnModel(getColumnModel());
      }
      
      if(dataModel instanceof JComponentTableModel)
      {
         JComponentTableModel jctm = (JComponentTableModel)dataModel;
         jctm.setScrollResizingOn(this, true);
      }
   }
   
   /**
    * If the preferredSize has been set to a non-null value just returns it.
    * If the UI delegate's getPreferredSize method returns a non null value 
    * then return that; otherwise defer to the component's layout manager.
    *
    * @return the value of the preferredSize property
    */
   public Dimension getPreferredSize()
   {
      // keep preferred height
      Dimension size = super.getPreferredSize();
      
      // add up preferred width
      size.width = this.getColumnModel().getTotalColumnWidth();
      
      return size;
   }
}
