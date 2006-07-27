/*
 * Copyright (c) 2003-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import java.awt.Component;

import javax.swing.JComponent;
import javax.swing.JTable;
import javax.swing.JTextField;
import javax.swing.AbstractCellEditor;
import javax.swing.table.TableCellEditor;

/**
 * This class allows any JComponent to become a cell editor. So far,
 * only the table cell renderer interfaces have been implemented.
 * 
 * @author Dave Longley
 */
public class JComponentCellEditor extends AbstractCellEditor
                                  implements TableCellEditor
{
   /**
    * The component used to do editing.
    */
   protected JComponent mComponent = null;
   
   /**
    * The table this editor is for.
    */
   protected JTable mTable;
   
   /**
    * Returns true if the cell editor value is valid, false if not. 
    * 
    * @return true if the cell editor value is valid, false if not.
    */
   protected boolean isCellEditorValueValid()
   {
      boolean rval = false;
      
      //Object obj = getCellEditorValue();
      rval = true;
      
      return rval;
   }

   /**
    * Gets the cell editor -- in this case, a JComponent.
    * 
    * @return the JComponent used as a cell editor.
    */
   public Object getCellEditorValue()
   {
      return mComponent;
   }
   
   /**
    * Returns the component used for editing cells. This method is
    * used to configure the editored appropriately before editing.
    *      
    * @param table this parameter is not used in this cell editor.
    * @param value the value of the cell to be edited.
    * @param isSelected true if the cell to be edited is selected,
    *                   otherwise false.
    * @param row the row index of the cell being edited.
    * @param column the column index of the cell being edited.
    *     
    * @return the component used for editing the cell. 
    */    
   public Component getTableCellEditorComponent(
      JTable table, Object value, boolean isSelected, int row, int column)
   {
      mTable = table;
      
      if(value instanceof JComponent)
      {
         mComponent = (JComponent)value;
      }
      else
      {
         mComponent = new JTextField(value.toString());
      }
   
      return mComponent;
   }
   
   /**
    * Calls <code>fireEditingStopped</code> and returns true if edited cell
    * contains a valid value, false if not.
    * 
    * @return true if edited cell contains a valid value, false if not.
    */
   public boolean stopCellEditing()
   { 
      boolean rval = false;
      
      if(isCellEditorValueValid())
      {
         rval = true;
      }
      
      fireEditingStopped();
      
      return rval;
   }
}
