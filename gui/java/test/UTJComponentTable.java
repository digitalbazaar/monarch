/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
import java.awt.Dimension;
import java.awt.FontMetrics;
import java.awt.Point;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Vector;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.UIManager;

import com.db.gui.JComponentTable;
import com.db.gui.JComponentTableHeader;
import com.db.gui.JComponentTableModel;
import com.db.gui.PositionConstraints;
import com.db.gui.PositionLayout;
import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * This class is used to test a JComponentTable.
 * 
 * @author Dave Longley
 */
public class UTJComponentTable implements ActionListener
{
   /**
    * The main method.
    * 
    * @param args arguments.
    */
   public static void main(String[] args)
   {
      // set up logger
      LoggerManager.setFile("dbcommon", "utjcomponenttable.log", false);
      LoggerManager.setFile("dbgui", "utjcomponenttable.log");
      LoggerManager.setConsoleVerbosity("dbgui", Logger.DEBUG_VERBOSITY);
      
      try
      {
         UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
         //UIManager.setLookAndFeel(
         //      UIManager.getCrossPlatformLookAndFeelClassName());
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
      
      // create frame
      JFrame frame = new JFrame("TEST FRAME");
      frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
      
      // create and add panel
      JPanel panel = createTablePanel();
      frame.getContentPane().add(panel);
      
      // pack frame
      frame.pack();
      frame.setSize(600, 600);
      frame.setVisible(true);
   }
   
   /**
    * Creates a panel for testing a JComponentTable.
    * 
    * @return the panel for testing a JComponentTable.
    */
   public static JPanel createTablePanel()
   {
      JPanel panel = new JPanel();
      
      // set layout
      panel.setLayout(new PositionLayout(panel, 500, 500));
      
      // create table rows
      Vector<Object> tableRows = new Vector<Object>();
      
      Vector<String> row0 = new Vector<String>();
      row0.add("row0");
      row0.add("row0");
      row0.add("row0");
      tableRows.add(row0);

      Vector<String> row1 = new Vector<String>();
      row1.add("row1");
      row1.add("row1");
      row1.add("row1");
      tableRows.add(row1);
      
      Vector<String> row2 = new Vector<String>();
      row2.add("row2");
      row2.add("row2");
      row2.add("row2");
      tableRows.add(row2);

      Vector<String> row3 = new Vector<String>();
      row3.add("row3");
      row3.add("row3");
      row3.add("row3");
      tableRows.add(row3);
      
      Vector<String> row4 = new Vector<String>();
      row4.add("row4");
      row4.add("row4");
      row4.add("row4");
      tableRows.add(row4);

      Vector<String> row5 = new Vector<String>();
      row5.add("row5");
      row5.add("row5");
      row5.add("row5");
      tableRows.add(row5);

      // create table model
      TestTableModel model = new TestTableModel(tableRows);
      
      // create table
      JComponentTable table = new JComponentTable(model);
      ((JComponentTableHeader)table.getTableHeader()).setColumnHeaderRenderer(
         "First Column", new JButton("First Column"));
      
      // create scroll pane for table
      JScrollPane scrollPane = new JScrollPane(table);
      
      // scroll pane constraints
      PositionConstraints scrollPaneConstraints = new PositionConstraints();
      scrollPaneConstraints.location = new Point(0, 0);
      scrollPaneConstraints.size =
         new Dimension(panel.getWidth(), panel.getHeight());
      scrollPaneConstraints.anchor = PositionConstraints.ANCHOR_ALL;
      
      // add scroll pane
      panel.add(scrollPane, scrollPaneConstraints);
      
      return panel;
   }
   
   /**
    * A JComponent table model for testing.
    * 
    * @author Dave Longley
    */
   public static class TestTableModel extends JComponentTableModel
   {
      /**
       * Creates a new TestTableModel.
       * 
       * @param tableRows a vector of the table rows.
       */
      public TestTableModel(Vector<Object> tableRows)
      {
         // set row data
         setRowData(tableRows);
         
         // create table headers
         createTableHeaders();
      }
      
      /**
       * Creates the table column headers, and sets the column classes.
       */
      protected void createTableHeaders()
      {
         // set column names, picked from inner row class
         String[] names = {"First Column",
                           "Second Column",
                           "Third Column"};
         
         // set column classes, picked from inner row class
         Class[] colClasses = {String.class,
                               String.class,
                               String.class};
         
         setColumnNames(names);
         setColumnClasses(colClasses);
      }
      
      /**
       * Sets the preferred table dimensions for rows and columns.
       * 
       * Overloaded to change preferred width for list sellers column.
       *  
       * @param table the table to alter.
       */
      public void setTableDimensions(JTable table)
      {
         int width = 0;
         FontMetrics fm = table.getFontMetrics(table.getFont());
         
         width = fm.stringWidth("First Column");
         table.getColumnModel().getColumn(0).setPreferredWidth(width);
         
         width = fm.stringWidth("Second Column");
         table.getColumnModel().getColumn(1).setPreferredWidth(width);
         
         width = fm.stringWidth("Third Column");
         table.getColumnModel().getColumn(2).setPreferredWidth(width);
      }
      
      /**
       * Returns the value at the given row, column.
       *
       * @param row the row.
       * @param column the column.
       * @return the object at the specified row, column.
       */
      public Object getValueAt(int row, int column)
      {
         Object rval = null;
         
         // get specified row
         Vector tableRow = (Vector)getValueAt(row);
         if(tableRow != null)
         {
            // get specified column
            rval = tableRow.get(column);
         }
         
         return rval;
      }
   }
   
   /**
    * Called when an action is performed.
    * 
    * @param e the action event.
    */
   public void actionPerformed(ActionEvent e)
   {
      System.out.println("ACTION PERFORMED!");
   }
}
