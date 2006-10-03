/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import java.awt.Component;
import java.awt.FontMetrics;
import java.awt.event.ComponentEvent;
import java.awt.event.ComponentListener;
import java.util.Collections;
import java.util.Comparator;
import java.util.Iterator;
import java.util.HashMap;
import java.util.Vector;

import javax.swing.JComponent;
import javax.swing.JTable;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.TableColumnModel;

import com.db.event.EventObject;
import com.db.logging.LoggerManager;

/**
 * This is a simple table model that can be easily extended.
 * 
 * @author Dave Longley
 * @author Mike Johnson
 */
public abstract class JComponentTableModel
extends AbstractTableModel
implements SortableTableModel, ComponentListener
{
   /**
    * Column names for any table model.
    */
   protected String[] mColumnNames;
   
   /**
    * Column classes for any table model.
    */
   protected Class<? extends Object>[] mColumnClasses;
   
   /**
    * The row data.
    */
   protected Vector<Object> mRows;
   
   /**
    * Table that is to be resized.
    */
   protected JTable mResizeTable;
   
   /**
    * The old table width.
    */
   protected int mOldWidth = -1;
   
   /**
    * The old table height.
    */
   protected int mOldHeight = -1;
   
   /**
    * The column widths of the table.
    */
   protected HashMap<Integer, Integer> mColWidths;
   
   /**
    * The total preferred width of the table.
    */
   protected int mTotalPrefWidth;
   
   /**
    * The column sorter for this table.
    */
   protected ColumnSorter mColumnSorter;
   
   /**
    * Creates a default simple table model.
    */
   public JComponentTableModel()
   {
      super();
      
      mRows = new Vector<Object>();
      
      // create the column sorter for this table
      mColumnSorter = createColumnSorter();
   }
   
   /**
    * Sets the row data.
    * 
    * @param rowData the row data to use.
    */
   protected void setRowData(Vector<Object> rowData)
   {
      mRows = rowData;
      
      Iterator rowIndex = rowData.iterator();
      while(rowIndex.hasNext())
      {
         Object row = rowIndex.next();
         if(row instanceof ChangeReporter)
         {
            ChangeReporter cr = (ChangeReporter)row;
            cr.getChangeDelegate().addListener(this, "stateChanged");
         }
      }
      
      // resort column
      getColumnSorter().resort();
   }
   
   /**
    * Gets all of the rows as a vector.
    * 
    * @return the row data as a vector.
    */
   protected Vector<? extends Object> getRowData()
   {
      return mRows;
   }
   
   /**
    * Creates the column sorter for this model.
    *
    * @return the column sorter for this model.
    */
   protected ColumnSorter createColumnSorter()
   {
      return new ColumnSorter(this);
   }
   
   /**
    * Gets the column sorter for this model.
    * 
    * @return the column sorter for this model.
    */
   protected ColumnSorter getColumnSorter()
   {
      return mColumnSorter;
   }
   
   /**
    * Gets the preferred width for components in a column.
    *
    * @param table the table the column is in.
    * @param col the column to get the preferred width for.
    * @return the preferred width for the column.
    */
   protected int getPreferredColumnWidth(JTable table, int col)
   {
      int rval = 0;
      
      try
      {
         // check actual data for preferred widths
         int width = 0;
         int rows = getRowCount();
         for(int i = 0; i < rows; i++)
         {
            Object obj = getValueAt(i, col);
            if(obj instanceof JComponent)
            {
               JComponent component = (JComponent)obj;
               width = component.getPreferredSize().width;
            }
            else
            {
               FontMetrics fm = table.getFontMetrics(table.getFont());
               width = fm.stringWidth(obj.toString());
            }
            
            if(width > rval)
            {
               rval = width;
            }
         }
         
         // check column classes for preferred widths
         if(col < mColumnClasses.length)
         {
            Class c = mColumnClasses[col];
            Object obj = c.newInstance();
            if(obj instanceof JComponent)
            {
               JComponent component = (JComponent)obj;
               width = component.getPreferredSize().width;
            }
            else
            {
               FontMetrics fm = table.getFontMetrics(table.getFont());
               width = fm.stringWidth(obj.toString());
            }
            
            if(width > rval)
            {
               rval = width;
            }            
         }
      }
      catch(Throwable t)
      {
         //LoggerManager.debug("dbgui", LoggerManager.getStackTrace(t));
      }
      
      LoggerManager.getLogger("dbdata").debug(getClass(),
         "col=" + col + ",preferred width=" + rval);
      
      return rval;
   }
    
   /**
    * Sets the column names for this model.
    * 
    * @param colNames the column names for this model.
    */
   public void setColumnNames(String[] colNames)
   {
      mColumnNames = colNames;
   }
   
   /**
    * Gets the column names for this model.
    * 
    * @return the column names for this model.
    */
   public String[] getColumnNames()
   {
      return mColumnNames;
   }
   
   /**
    * Gets a column name.
    * 
    * @param column the column index.
    * 
    * @return the name of the column.
    */
   @Override
   public String getColumnName(int column)
   {
      String colName = "NONE";
      
      if(column >= 0 && column < getColumnCount())
      {
         colName = mColumnNames[column];
      }
      
      return colName;
   }
   
   /**
    * Sets the column classes for this model.
    * 
    * @param colClasses the column classes for this model.
    */
   public void setColumnClasses(Class<?>[] colClasses)
   {
      mColumnClasses = colClasses;
   }
   
   /**
    * Gets the column classes for this model.
    * 
    * @return the column classes for this model.
    */
   public Class[] getColumnClasses()
   {
      return mColumnClasses;
   }
   
   /**
    * Gets the column class for a column.
    * 
    * @param column the column index.
    * 
    * @return the class for the column.
    */
   @Override
   public Class<? extends Object> getColumnClass(int column)
   {
      Class<? extends Object> c = Object.class;
      
      if(column >= 0 && column < mColumnClasses.length)
      {
         c = mColumnClasses[column];
      }
      
      return c;
   }
   
   /**
    * Allows cells to be edited so that components can be inserted
    * into the table.
    *
    * @param row the row of the cell.
    * @param column the column of the cell.
    * 
    * @return return true if the class is JComponent for the cell.
    */
   @Override
   public boolean isCellEditable(int row, int column)
   {
      boolean rval = false;
      
      if(getColumnClass(column) == JComponent.class)
      {
         rval = true;
      }
      
      return rval;
   }   
   
   /**
    * Returns the column count for this model.
    * 
    * @return the column count for this model.
    */
   public int getColumnCount()
   {
      return mColumnClasses.length;
   }
   
   /**
    * Sets up the table column header renderers for the table header.
    *  
    * @param table the table to alter.
    * @param header the table's header.
    */
   public void setTableColumnHeaderRenderers(
      JComponentTable table, JComponentTableHeader header)
   {
      // do nothing by default
   }
   
   /**
    * Sets the preferred table dimensions for rows and columns.
    *  
    * @param table the table to alter.
    */
   public void setTableDimensions(JTable table)
   {
      // setup the preferred column widths
      TableColumnModel tcm = table.getColumnModel();

      int count = getColumnCount();
      for(int i = 0; i < count; i++)
      {
         int width = getPreferredColumnWidth(table, i);
         tcm.getColumn(i).setPreferredWidth(width);
      }
   }
   
   /**
    * Call this function after adding a table to a scrolling container
    * if you want the table to be resized according to the parent
    * container's dimensions.
    * 
    * @param table the table you want to modify.
    * @param on true if you want to turn column resizing on, false if not. 
    */
   public void setScrollResizingOn(JTable table, boolean on)
   {
      Component parent = table.getParent();

      if(on)
      {
         if(parent != null)
         {
            mResizeTable = table;
            parent.getParent().addComponentListener(this);
            
            mColWidths = new HashMap<Integer, Integer>();
            mTotalPrefWidth = 0;
            TableColumnModel tcm = mResizeTable.getColumnModel();
            for(int i = 0; i < tcm.getColumnCount(); i++)
            {
               int width = tcm.getColumn(i).getPreferredWidth();
               mTotalPrefWidth += width;
               mColWidths.put(new Integer(i), new Integer(width));
            }
         }
      }
      else if(parent != null)
      {
         mResizeTable = null;
         parent.getParent().removeComponentListener(this);
      }
   }
   
   /**
    * Notifies all listeners that a value has been updated.
    * 
    * @param obj the value that has been updated.
    */
   public void fireTableValueUpdated(Object obj)
   {
      int row = getRow(obj);
      if(row != -1)
      {
         fireTableRowsUpdated(row, row);
      }
   }
   
   /**
    * Inserts an object at the given row.
    * 
    * @param obj the object to insert.
    * @param row the row to insert at.
    */
   public void insertValueAt(Object obj, int row)
   {
      mRows.insertElementAt(obj, row);
      
      if(obj instanceof ChangeReporter)
      {
         ChangeReporter cr = (ChangeReporter)obj;
         cr.getChangeDelegate().addListener(this, "stateChanged");
      }
      
      fireTableRowsInserted(row, row);
   }
   
   /**
    * Sets the object at a certain row.
    * 
    * @param obj the object to insert.
    * @param row the row to set.
    */
   public void setValueAt(Object obj, int row)
   {
      if(row < mRows.size())
      {
         mRows.removeElementAt(row);
         fireTableRowsDeleted(row, row);
      }
      
      if(row <= mRows.size())
      {
         mRows.insertElementAt(obj, row);
         
         if(obj instanceof ChangeReporter)
         {
            ChangeReporter cr = (ChangeReporter)obj;
            cr.getChangeDelegate().addListener(this, "stateChanged");
         }
         
         fireTableRowsInserted(row, row);
      }
   }
   
   /**
    * Gets the object for a particular row.
    * 
    * @param row the row to get the object for.
    * @return the object retrieved or null.
    */
   public synchronized Object getValueAt(int row)
   {
      Object obj = null;
      
      if(mRows.size() > row)
      {
         obj = mRows.get(row);
      }
      
      return obj;
   }
   
   /**
    * Gets the row the passed object is at.
    * 
    * @param obj the value to look for.
    * @return the row the passed object is at or -1.
    */
   public int getRow(Object obj)
   {
      int row = -1;
      
      Iterator i = mRows.iterator();
      for(int count = 0; i.hasNext(); count++)
      {
         Object o = i.next();
         if(o.equals(obj))
         {
            row = count;
            break;
         }
      }
      
      return row;
   }
    
   /**
    * Adds a new row to the table using the passed object.
    * 
    * @param obj the object to add.
    * @return true if successfully added, false if not.
    */
   public boolean addValue(Object obj)
   {
      boolean rval = false;
      
      if(mRows.add(obj))
      {
         if(obj instanceof ChangeReporter)
         {
            ChangeReporter cr = (ChangeReporter)obj;
            cr.getChangeDelegate().addListener(this, "stateChanged");
         }
         
         // resort column
         getColumnSorter().resort();
         
         rval = true;
         
         fireTableDataChanged();
      }
      
      return rval;
   }
   
   /**
    * Removes a row from the table where the row matches
    * the object.
    * 
    * @param obj the object to remove.
    * @return true if a row was removed, false if not.
    */
   public boolean removeValue(Object obj)
   {
      boolean rval = false;
      
      if(mRows.remove(obj))
      {
         rval = true;
         fireTableDataChanged();
      }
      
      return rval;
   }
   
   /**
    * Removes the passed rows from the table.
    * 
    * @param rows the rows to remove.
    */
   public void removeRows(int[] rows)
   {
      boolean changed = false;
      
      Vector<Object> objects = new Vector<Object>();
      for(int i = 0; i < rows.length; i++)
      {
         objects.add(getValueAt(rows[i]));
      }

      Iterator i = objects.iterator();
      while(i.hasNext())
      {
         if(mRows.remove(i.next()))
         {
            changed = true;
         }
      }
      
      if(changed)
      {
         fireTableDataChanged();
      }
   }

   /**
    * Removes a row from the table.
    * 
    * @param row the row to remove.
    * @return true if a row was removed, false if not.
    */
   public boolean removeRow(int row)
   {
      boolean rval = false;
      
      Object obj = getValueAt(row);
      rval = removeValue(obj);
      
      return rval;
   }
   
   /**
    * Gets a row from the table.
    * 
    * Same as getValueAt(row).
    * 
    * @param row the row to get.
    * 
    * @return the row.
    */
   public Object getRow(int row)
   {
      return getValueAt(row);
   }
   
   /**
    * Removes all of the rows from the table.
    */
   public void clear()
   {
      mRows.clear();
      fireTableDataChanged();
   }
   
   /**
    * Gets the row count for this model.
    * 
    * @return the number of rows currently represented by this model.
    */
   public int getRowCount()
   {
      return mRows.size();
   }
   
   /*
    * Methods for component listener.
    */
   public void componentHidden(ComponentEvent e) {}
   public void componentMoved(ComponentEvent e) {}
   
   public void componentShown(ComponentEvent e)
   {
      resizeColumns();
   }
   
   public void componentResized(ComponentEvent e)
   {
      resizeColumns();
   }
   
   /**
    * Forces a resize of the columns. This method will only work if
    * the table the model is for is in a scrollable pane and scroll
    * resizing has been turned on.
    */
   public void resizeColumns()
   {
      Component parent = mResizeTable.getParent();
      
      // if height has changed
      if(parent != null &&
         (mOldWidth != parent.getWidth() || mOldHeight != parent.getHeight()))
      {
         mOldWidth = parent.getWidth();
         mOldHeight = parent.getHeight();
         
         TableColumnModel tcm = mResizeTable.getColumnModel();

         // get the average amount to add to each column
         double avg = 0;
         if(mTotalPrefWidth < mOldWidth)
         {
            avg = ((double)(mOldWidth - mTotalPrefWidth)) /
                  tcm.getColumnCount();
         }
         
         int add = (int)avg;
         int totalWidth = 0;
         int resizable = -1;
         
         for(int i = 0; i < tcm.getColumnCount(); i++)
         {
            if(tcm.getColumn(i).getResizable())
            {
               Integer width = mColWidths.get(i);
               if(width != null)
               {
                  tcm.getColumn(i).setPreferredWidth(width.intValue() + add);
                  totalWidth += width.intValue() + add;
               }
               else
               {
                  int w = tcm.getColumn(i).getPreferredWidth();
                  mColWidths.put(new Integer(i), new Integer(w));
                  tcm.getColumn(i).setPreferredWidth(w + add);
                  totalWidth += w + add;
               }
               
               if(resizable == -1)
               {
                  resizable = i;
               }
            }
            else
            {
               totalWidth += tcm.getColumn(i).getPreferredWidth();
            }
         }
         
         // add the remaining width to the first resizable column
         if(totalWidth != mOldWidth && resizable != -1)
         {
            add = mOldWidth - totalWidth;
            int w = tcm.getColumn(resizable).getPreferredWidth();
            tcm.getColumn(resizable).setPreferredWidth(w + add);
         }
      }
      
      fireTableDataChanged();
   }
   
   /**
    * Called when row data changes.
    * 
    * @param event the event identifying the row that changed.
    */
   public void stateChanged(EventObject event)
   {
      // get the index of the changed row
      int index = getRowData().indexOf(event.getData("source"));
      if(index != -1)
      {
         // fire rows updated message
         fireTableRowsUpdated(index, index);
      }
   }
   
   /**
    * Sorts a column.
    * 
    * @param column the column to sort.
    * @param ascending true to sort ascending, false to sort descending.
    */
   public void sortColumn(int column, boolean ascending)
   {
      // sort the row data
      getColumnSorter().sort(column, ascending);
      
      if(getRowCount() > 0)
      {
         // all rows have been updated
         fireTableRowsUpdated(0, getRowCount());
      }
   }
   
   /**
    * Compares two row objects on a column. Returns -1 if the first object
    * is less than the second, 0 if they are equal, and 1 if the first
    * object is greater than the second.
    * 
    * This is the method you should overload if you want to do
    * special comparisons.
    * 
    * @param column the column to compare data in.
    * @param row1 the first row.
    * @param row2 the second row.
    * @return -1 if the first object is less than the second, 0 if the
    *         first is equal to the second, and 1 if the first is greater.
    */
   @SuppressWarnings("unchecked")
   public int compareColumnData(int column, int row1, int row2)
   {
      int rval = 0;
      
      Object obj1 = getValueAt(row1, column);
      Object obj2 = getValueAt(row2, column);
      
      if(obj1.equals(obj2))
      {
         rval = 0;
      }
      else if(obj1 instanceof Comparable)
      {
         Comparable<Object> c1 = (Comparable<Object>)obj1;
         rval = c1.compareTo(obj2);
      }
      else if(obj1 instanceof Comparator)
      {
         Comparator<Object> c1 = (Comparator<Object>)obj1;
         rval = c1.compare(obj1, obj2);
      }
      else if(obj1 instanceof Number)
      {
         Number number1 = (Number)obj1;
         Number number2 = (Number)obj2;
         
         rval = (number1.doubleValue() < number2.doubleValue()) ? -1 : 1; 
      }
      else
      {
         String s1 = "" + obj1;
         String s2 = "" + obj2;
         
         rval = s1.compareTo(s2);
      }
      
      if(rval != 0)
      {
         rval = (rval < 0) ? -1 : 1;
      }
      
      return rval;
   }
   
   /**
    * The column sorter.
    */
   public class ColumnSorter implements Comparator
   {
      /**
       * The table model being sorted.
       */
      protected JComponentTableModel mTableModel;
      
      /**
       * The column to sort.
       */
      protected int mColumn;
      
      /**
       * Whether or not to use ascending sorting.
       */
      protected boolean mAscending;
      
      /**
       * Creates a new ColumnSorter.
       * 
       * @param model the model to sort.
       */
      public ColumnSorter(JComponentTableModel model)
      {
         // store model
         mTableModel = model;
         
         // column -1 by default
         mColumn = -1;
         
         // ascending by default
         mAscending = true;
      }
      
      /**
       * Sorts row data in a given column.
       * 
       * @param column the column to sort according to.
       * @param ascending true to sort ascending, false to sort descending.
       */
      @SuppressWarnings("unchecked")
      public void sort(int column, boolean ascending)
      {
         mColumn = column;
         mAscending = ascending;
         Collections.sort(mTableModel.getRowData(), this);
      }
      
      /**
       * Resorts using the last sort parameters. If no column was previously
       * sorted than no sorting will take place.
       */
      public void resort()
      {
         if(mColumn != -1)
         {
            sort(mColumn, mAscending);
         }
      }
      
      /**
       * Compares two objects. Returns -1 if the first object
       * is less than the second, 0 if they are equal, and 1 if the first
       * object is greater than the second.
       * 
       * @param o1 the first object to compare.
       * @param o2 the second object to compare.
       * @return -1 if the first object is less than the second, 0 if the
       *         first is equal to the second, and 1 if the first is greater.
       */      
      public int compare(Object o1, Object o2)  
      {
         int rval = 0;
         
         int index1 = getRowData().indexOf(o1);
         int index2 = getRowData().indexOf(o2);
         
         rval = mTableModel.compareColumnData(mColumn, index1, index2);
         
         if(!mAscending)
         {
            // swap values
            if(rval != 0)
            {
               rval = (rval < 0) ? 1 : -1;
            }
         }
         
         return rval;
      }
      
      /**
       * Gets the column that was last sorted.
       * 
       * @return the column that was last sorted.
       */
      public int getSortedColumn()
      {
         return mColumn;
      }
   }
}
