/*
 * Copyright (c) 2003-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import java.awt.Dimension;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;

import javax.swing.JComponent;
import javax.swing.JTable;
import javax.swing.table.JTableHeader;
import javax.swing.table.TableModel;

import com.db.event.EventDelegate;
import com.db.event.EventObject;

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
    * Whether or not this table is editable.
    */
   protected boolean mEditable;
   
   /**
    * An event delegate for firing double click events.
    */
   protected EventDelegate mDoubleClickEventDelegate;
   
   /**
    * Creates a new JComponentTable using the passed JComponentTableModel.
    * 
    * @param jctm the JComponentTableModel to use.
    */
   public JComponentTable(JComponentTableModel jctm)
   {
      super(jctm);
      
      // create the double click event delegate
      mDoubleClickEventDelegate = new EventDelegate();

      setEditable(true);
      
      setupTableHeader();
      
      // set default editors for JComponents
      setDefaultEditor(JComponent.class, new JComponentCellEditor());
      setDefaultRenderer(JComponent.class, new JComponentCellRenderer());
      
      // set table column header renderers
      jctm.setTableColumnHeaderRenderers(
         this, (JComponentTableHeader)getTableHeader());
      
      // set table dimensions
      jctm.setTableDimensions(this);
      
      // add mouse adapter as listener
      addMouseListener(new JComponentTableMouseAdapter(this));
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
    * Fires a double click event.
    * 
    * @param row the selected row.
    */
   public void fireDoubleClickEvent(int row)
   {
      // create event
      EventObject event = new EventObject("tableDoubleClicked");
      
      event.setData("table", this);
      event.setDataKeyMessage("table",
         "The JComponentTable that was double clicked.");
      
      event.setData("row", row);
      event.setDataKeyMessage("row",
         "The row index (int) of the selected row.");
      
      // fire the event
      getDoubleClickEventDelegate().fireEvent(event);
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
    * Sets whether or not this table is editable.
    * 
    * @param editable true to make this table editable, false not to.
    */
   public void setEditable(boolean editable)
   {
      mEditable = editable;
   }
   
   /**
    * Returns whether or not this table is editable.
    * 
    * @return true if this table is editable, false if not. 
    */
   public boolean isEditable()
   {
      return mEditable;
   }
   
   /**
    * Returns true if the cell at <code>row</code> and <code>column</code>
    * is editable. Otherwise, invoking <code>setValueAt</code> on the cell
    * will have no effect.
    * <p>
    * <b>Note</b>: The column is specified in the table view's display
    *              order, and not in the <code>TableModel</code>'s column
    *              order. This is an important distinction because as the
    *              user rearranges the columns in the table,
    *              the column at a given index in the view will change.
    *              Meanwhile the user's actions never affect the model's
    *              column ordering.
    *
    * @param row the row whose value is to be queried.
    * @param column the column whose value is to be queried.
    * 
    * @return true if the cell is editable.
    */
   public boolean isCellEditable(int row, int column)   
   {
      boolean rval = false;
      
      if(isEditable())
      {
         rval = super.isCellEditable(row, column);
      }
      else
      {
         rval = false;
      }
      
      return rval;
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
   
   /**
    * Gets the double click event delegate.
    * 
    * @return the double click event delegate.
    */
   public EventDelegate getDoubleClickEventDelegate()
   {
      return mDoubleClickEventDelegate;
   }
   
   /**
    * A mouse adapter for handling double clicks on the table.
    * 
    * @author Dave Longley
    */
   public class JComponentTableMouseAdapter extends MouseAdapter
   {
      /**
       * The table this mouse adapter is for.
       */
      protected JComponentTable mTable;
      
      /**
       * Creates a new JComponentTableMouseAdapter.
       * 
       * @param table the table this mouse adapter is for.
       */
      public JComponentTableMouseAdapter(JComponentTable table)
      {
         // store table
         mTable = table;
      }
      
      /**
       * Called when the mouse is clicked on the table.
       * 
       * @param e the mouse event.
       */
      public void mouseClicked(MouseEvent e)
      {
         // handle double click
         if(e.getClickCount() == 2)
         {
            // get selected row
            int row = mTable.getSelectedRow();
            
            // if a row was selected, fire an event
            if(row != -1)
            {
               mTable.fireDoubleClickEvent(row);
            }
         }
      }
   }
}
