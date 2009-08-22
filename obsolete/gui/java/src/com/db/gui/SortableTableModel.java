/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

/**
 * Interface for a sortable table model.
 * 
 * @author Dave Longley.
 */
public interface SortableTableModel
{
   /**
    * Sorts a column.
    * 
    * @param column the column to sort.
    * @param ascending true to sort ascending, false to sort descending.
    */
   public void sortColumn(int column, boolean ascending);
   
   /**
    * Compares two row objects on a column. Returns -1 if the first object
    * is less than the second, 0 if they are equal, and 1 if the first
    * object is greater than the second.
    * 
    * @param column the column to compare data in.
    * @param row1 the first row.
    * @param row2 the second row.
    * @return -1 if the first object is less than the second, 0 if the
    *         first is equal to the second, and 1 if the first is greater.
    */
   public int compareColumnData(int column, int row1, int row2);  
}
