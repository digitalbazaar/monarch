/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
import java.awt.Dimension;
import java.awt.FontMetrics;
import java.awt.Point;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

//import javax.swing.BorderFactory;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JProgressBar;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.UIManager;
//import javax.swing.border.BevelBorder;

import com.db.event.EventDelegate;
import com.db.event.EventObject;
import com.db.event.ThreadedEventDelegate;
import com.db.gui.ChangeReporter;
import com.db.gui.FastProgressBar;
import com.db.gui.JComponentTable;
import com.db.gui.JComponentTableModel;
import com.db.gui.PositionConstraints;
import com.db.gui.PositionLayout;
import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.util.MethodInvoker;

/**
 * This class is used to test FastProgressBar update speed, positioning
 * and table mechanics.
 * 
 * @author Dave Longley
 * @author Mike Johnson
 */
public class UTFastProgressBar
implements ActionListener
{
   /**
    * The FastProgressBar to test.
    */
   public static FastProgressBar smFastProgressBar;
   
   /**
    * The JProgressBar to compare to.
    */
   public static JProgressBar smJProgressBar;
   
   /**
    * The FastProgressBar in the table.
    */
   public static FastProgressBar smTableFastProgressBar;
   
   /**
    * The button to fire off events to the table progress bar.
    */
   public static JButton smTableTestButton;
   
   /**
    * The main method.
    * 
    * @param args arguments.
    */
   public static void main(String[] args)
   {
      // set up logger
      LoggerManager.setFile("dbgui", "utfastprogressbar.log", false);
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
      JFrame frame = new JFrame("PROGRESS BAR TEST FRAME");
      frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
      
      JPanel contentPanel = new JPanel();
      contentPanel.setSize(500, 500);
      contentPanel.setLayout(new PositionLayout(contentPanel));
      
      // create panel and table
      JPanel panel = createProgressBarPanel();
      JPanel tablePanel = createProgressBarTable();

      // position panel and table
      PositionConstraints panelPC = new PositionConstraints();
      panelPC.location = new Point(0, 0);
      panelPC.size = new Dimension(500, 100);
      
      PositionConstraints tablePanelPC = new PositionConstraints();
      tablePanelPC.location = new Point(0, panelPC.getBottom());
      tablePanelPC.size = new Dimension(500, 400);
      
      // add panel and table
      contentPanel.add(panel, panelPC);
      contentPanel.add(tablePanel, tablePanelPC);
      
      frame.getContentPane().add(contentPanel);
      
      // pack frame
      frame.pack();
      frame.setSize(600, 600);
      frame.setVisible(true);
   }
   
   /**
    * Creates a panel for testing a FastProgressBar.
    * 
    * @return the panel for testing a FastProgressBar.
    */
   public static JPanel createProgressBarPanel()
   {
      JPanel panel = new JPanel();
      
      // set layout
      panel.setSize(500, 100);
      panel.setLayout(new PositionLayout(panel));

      // create fast progress bar
      smFastProgressBar = new FastProgressBar();
      smFastProgressBar.setText("This is a test, joey.");
      smFastProgressBar.setValue(50);
      //smFastProgressBar.setBorder(
         //BorderFactory.createBevelBorder(BevelBorder.LOWERED));
      
      // create button for toggling progress bar indeterminate mode
      JButton indeterminateButton = new JButton("Toggle Indeterminate");
      indeterminateButton.setActionCommand("toggleIndeterminate");
      indeterminateButton.addActionListener(new UTFastProgressBar());
      
      // create button for running an update test
      JButton updateTestButton = new JButton("Run update test");
      updateTestButton.setActionCommand("runUpdateTest");
      updateTestButton.addActionListener(new UTFastProgressBar());
      
      // create regular progress bar
      smJProgressBar = new JProgressBar();
      smJProgressBar.setStringPainted(true);
      smJProgressBar.setString("This is a test, joey.");
      smJProgressBar.setValue(50);
      
      int insets = 5;
      
      // fast progress bar constraints
      PositionConstraints fastProgressBarConstraints =
         new PositionConstraints();
      fastProgressBarConstraints.location = new Point(insets, insets);
      fastProgressBarConstraints.size =
         new Dimension(panel.getWidth() - insets,
                       smFastProgressBar.getPreferredSize().height);
      fastProgressBarConstraints.anchor =
         PositionConstraints.ANCHOR_TOP_LEFT_RIGHT;
      
      // indeterminate button constraints
      PositionConstraints indeterminateButtonConstraints =
         new PositionConstraints();
      indeterminateButtonConstraints.location =
         new Point(panel.getWidth() -
                   indeterminateButton.getPreferredSize().width - insets,
                   fastProgressBarConstraints.getBottom() + insets);
      indeterminateButtonConstraints.size =
         new Dimension(indeterminateButton.getPreferredSize());
      indeterminateButtonConstraints.anchor =
         PositionConstraints.ANCHOR_TOP | PositionConstraints.ANCHOR_RIGHT;
      
      // update test button constraints
      PositionConstraints updateTestButtonConstraints =
         new PositionConstraints();
      updateTestButtonConstraints.location =
         new Point(indeterminateButtonConstraints.getLeft() -
                   updateTestButton.getPreferredSize().width - insets,
                   indeterminateButtonConstraints.getTop());
      updateTestButtonConstraints.size =
         new Dimension(updateTestButton.getPreferredSize());
      updateTestButtonConstraints.anchor =
         PositionConstraints.ANCHOR_TOP | PositionConstraints.ANCHOR_LEFT;
      
      // jprogress bar constraints
      PositionConstraints jProgressBarConstraints =
         new PositionConstraints();
      jProgressBarConstraints.location =
         new Point(insets, indeterminateButtonConstraints.getBottom() + insets);
      jProgressBarConstraints.size =
         new Dimension(panel.getWidth() - insets,
                       smJProgressBar.getPreferredSize().height);
      jProgressBarConstraints.anchor =
         PositionConstraints.ANCHOR_TOP_LEFT_RIGHT;
      
      // add components
      panel.add(smFastProgressBar, fastProgressBarConstraints);
      panel.add(indeterminateButton, indeterminateButtonConstraints);
      panel.add(updateTestButton, updateTestButtonConstraints);
      panel.add(smJProgressBar, jProgressBarConstraints);
      
      return panel;
   }
   
   /**
    * Creates a table containing progress bars and puts it in a panel.
    * 
    * @return a JPanel containing a table full of progress bars.
    */
   public static JPanel createProgressBarTable()
   {
      JPanel panel = new JPanel();
      
      // set layout
      panel.setSize(500, 400);
      panel.setLayout(new PositionLayout(panel));
      
      // make table fast progress bar
      smTableFastProgressBar = new FastProgressBar();
      smTableFastProgressBar.setText("0%");
            
      // table model & table to put progress bar in
      TestTableModel ttm = new TestTableModel();
      ttm.addRow("Col A1", "Col A2", smTableFastProgressBar);
      FastProgressBar barB3 = new FastProgressBar();
      barB3.setValue(33);
      barB3.setText("33%");
      ttm.addRow("Col B1", "Col B2", barB3);
      FastProgressBar barC3 = new FastProgressBar();
      barC3.setValue(66);
      barC3.setText("66%");
      ttm.addRow("Col C1", "Col C2", barC3);
      
      JComponentTable testTable = new JComponentTable(ttm);
      JScrollPane tableScrollPane = new JScrollPane(testTable);

      // button to start test
      smTableTestButton = new JButton("Start");
      smTableTestButton.addActionListener(new UTFastProgressBar());
      smTableTestButton.setActionCommand("table_test_start");
      
      // position constraints
      PositionConstraints scrollPanePC = new PositionConstraints();
      scrollPanePC.location = new Point(0, 0);
      scrollPanePC.size =
         new Dimension(panel.getWidth(), panel.getHeight() - 
               smTableTestButton.getPreferredSize().height - 10);
      scrollPanePC.anchor = PositionConstraints.ANCHOR_ALL;
      
      PositionConstraints buttonPC = new PositionConstraints();
      buttonPC.location = new Point(0, scrollPanePC.getBottom() + 5);
      buttonPC.size = new Dimension(smTableTestButton.getPreferredSize());
      buttonPC.anchor = PositionConstraints.ANCHOR_BOTTOM;
      
      panel.add(tableScrollPane, scrollPanePC);
      panel.add(smTableTestButton, buttonPC);
      
      return panel;
   }
   
   /**
    * Called when an action is performed.
    * 
    * @param e the action event.
    */
   public void actionPerformed(ActionEvent e)
   {
      if(e.getActionCommand().equals("toggleIndeterminate"))
      {
         System.out.println("Toggling Indeterminate...");
         
         smFastProgressBar.setIndeterminate(
            !smFastProgressBar.isIndeterminate());
         
         if(smFastProgressBar.isIndeterminate())
         {
            smFastProgressBar.setText("Indeterminate");
         }
         else
         {
            smFastProgressBar.setText("Determinate");
         }
         
         smJProgressBar.setIndeterminate(
            !smJProgressBar.isIndeterminate());
            
         if(smJProgressBar.isIndeterminate())
         {
            smJProgressBar.setString("Indeterminate");
         }
         else
         {
            smJProgressBar.setString("Determinate");
         }
      }
      else if(e.getActionCommand().equals("runUpdateTest"))
      {
         fireUpdateEvents();
      }
      else if(e.getActionCommand().equals("table_test_start"))
      {
         // disable button
         smTableTestButton.setEnabled(false);
         
         // start the table test in background
         MethodInvoker mi = new MethodInvoker(this, 
               "fireTableUpdateEvents", null);
         mi.backgroundExecute();
      }
   }
   
   /**
    * Fires a bunch of events to update and repaint the fast progress bar.
    */
   public static void fireUpdateEvents()
   {
      // create threaded event delegate
      ThreadedEventDelegate delegate = new ThreadedEventDelegate();
//      delegate.addListener(new UTFastProgressBar(), "handleEvent1");
//      delegate.addListener(new UTFastProgressBar(), "handleEvent1");
//      delegate.addListener(new UTFastProgressBar(), "handleEvent1");
//      delegate.addListener(new UTFastProgressBar(), "handleEvent1");
//      delegate.addListener(new UTFastProgressBar(), "handleEvent1");
//      delegate.addListener(new UTFastProgressBar(), "handleEvent1");
//      delegate.addListener(new UTFastProgressBar(), "handleEvent1");
//      delegate.addListener(new UTFastProgressBar(), "handleEvent1");
//      delegate.addListener(new UTFastProgressBar(), "handleEvent1");
//      delegate.addListener(new UTFastProgressBar(), "handleEvent1");
//      delegate.addListener(new UTFastProgressBar(), "handleEvent1");
//      delegate.addListener(new UTFastProgressBar(), "handleEvent1");
//      delegate.addListener(new UTFastProgressBar(), "handleEvent1");
//      delegate.addListener(new UTFastProgressBar(), "handleEvent1");
//      delegate.addListener(new UTFastProgressBar(), "handleEvent1");
//      delegate.addListener(new UTFastProgressBar(), "handleEvent1");
//      delegate.addListener(new UTFastProgressBar(), "handleEvent1");
//      delegate.addListener(new UTFastProgressBar(), "handleEvent1");
//      delegate.addListener(new UTFastProgressBar(), "handleEvent1");
//      delegate.addListener(new UTFastProgressBar(), "handleEvent1");
//      delegate.addListener(new UTFastProgressBar(), "handleEvent1");
//      delegate.addListener(new UTFastProgressBar(), "handleEvent1");
//      delegate.addListener(new UTFastProgressBar(), "handleEvent1");
//      delegate.addListener(new UTFastProgressBar(), "handleEvent1");
//      delegate.addListener(new UTFastProgressBar(), "handleEvent1");
//      delegate.addListener(new UTFastProgressBar(), "handleEvent1");
//      delegate.addListener(new UTFastProgressBar(), "handleEvent1");
//      delegate.addListener(new UTFastProgressBar(), "handleEvent1");
//      delegate.addListener(new UTFastProgressBar(), "handleEvent1");
//      delegate.addListener(new UTFastProgressBar(), "handleEvent1");
//      delegate.addListener(new UTFastProgressBar(), "handleEvent1");
//      delegate.addListener(new UTFastProgressBar(), "handleEvent1");
//      delegate.addListener(new UTFastProgressBar(), "handleEvent1");
//      delegate.addListener(new UTFastProgressBar(), "handleEvent1");
//      delegate.addListener(new UTFastProgressBar(), "handleEvent1");
      
      delegate.addListener(new UTFastProgressBar(), "updateFastProgressBar");
      //delegate.addListener(new UTFastProgressBar(), "updateJProgressBar");
      
      // set fast progress bar to have a value of zero
      smFastProgressBar.setValue(0);
      
      // fire 100 events to update the progress bar really quickly
      for(int i = 1; i < 100; i++)
      {
         // create update event
         EventObject event = new EventObject("update");
         event.setData("text", "Updating " + (i + 1) + "%");
         event.setData("progress", i + 1);
         
         delegate.fireEvent(event);
         
         // fire event 100 times
//         for(int n = 0; n < 100; n++)
//         {
//            delegate.fireEvent(event);
//         }
      }
   }
   
   /**
    * Fires a bunch of events to update the table's FastProgressBar.
    */
   public static void fireTableUpdateEvents()
   {
      ThreadedEventDelegate delegate = new ThreadedEventDelegate();
      
      delegate.addListener(new UTFastProgressBar(), "updateTableProgressBar");
      
      smFastProgressBar.setValue(0);
      
      // fire 100 events for the bar
      for(int i = 1; i < 100; i++)
      {
         // create update event
         EventObject event = new EventObject("update");
         event.setData("text", "" + (i + 1) + "%");
         event.setData("progress", i + 1);
         
         delegate.fireEvent(event);
         
         try
         {
            Thread.sleep(50);
         }
         catch(Throwable t)
         {
         }
      }
      
      EventObject finished = new EventObject("finished");
      finished.setData("text", "Finished");
      finished.setData("progress", 100);
      
      delegate.fireEvent(finished);
      
      smTableTestButton.setEnabled(true);
   }
   
   /**
    * Updates the fast progress bar.
    * 
    * @param event the event with the data to update the progress bar with.
    */
   public void updateFastProgressBar(EventObject event)
   {
      // get text for progress bar
      String text = event.getDataStringValue("text");
      
      // get progress for progress bar
      int progress = event.getDataIntValue("progress");
      
      // set progress bar text and value
      smFastProgressBar.setText(text);
      smFastProgressBar.setValue(progress);
      
      /*AWTEvent awtEvent =
         Toolkit.getDefaultToolkit().getSystemEventQueue().peekEvent();
      if(event != null)
      {
         System.out.println("EVENT=" + awtEvent.getClass());
         //System.out.println("EVENT_SOURCE=" + (awtEvent.getSource() == this) +
         //      "," + awtEvent.getSource());
      }
      
      if(awtEvent == null)// || awtEvent.getSource() != this)
      {
         // repaint if not processing an event for this component already
         smFastProgressBar.repaint();
      }*/
      
      smFastProgressBar.repaint();
      //smFastProgressBar.updateUI();
   }
   
   /**
    * Does nothing. Is just used to handle events.
    * 
    * @param event the event to do nothing with.
    */
   public void handleEvent1(EventObject event)
   {
      System.out.println();
   }
   
   /**
    * Updates the jprogress bar.
    * 
    * @param event the event with the data to update the progress bar with.
    */
   public void updateJProgressBar(EventObject event)
   {
      // get text for progress bar
      String text = event.getDataStringValue("text");
      
      // get progress for progress bar
      int progress = event.getDataIntValue("progress");
      
      // set progress bar text and value
      smJProgressBar.setString(text);
      smJProgressBar.setValue(progress);
      
      smJProgressBar.repaint();
      //smJProgressBar.updateUI();
   }
   
   /**
    * Updates the table FastProgressBar.
    * 
    * @param event the event with the data to update the progress bar with.
    */
   public void updateTableProgressBar(EventObject event)
   {
      // get text for bar
      String text = event.getDataStringValue("text");
      
      // get progress for bar
      int progress = event.getDataIntValue("progress");
      
      // set new data
      smTableFastProgressBar.setText(text);
      smTableFastProgressBar.setValue(progress);
      
      // repaint bar
      smTableFastProgressBar.fireFastProgressBarChanged();
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
       */
      public TestTableModel()
      {
         // create table headers
         createTableHeaders();
      }
      
      /**
       * Creates the table column headers, and sets the column classes.
       */
      protected void createTableHeaders()
      {
         // set column names, picked from inner row class
         String[] names = {"Text",
                           "Text",
                           "Progress Bar"};
         
         // set column classes, picked from inner row class
         Class[] colClasses = {String.class,
                               String.class,
                               JComponent.class};
         
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
         
         width = fm.stringWidth("Text");
         table.getColumnModel().getColumn(0).setPreferredWidth(width);
         
         width = fm.stringWidth("Text");
         table.getColumnModel().getColumn(1).setPreferredWidth(width);
         
         width = fm.stringWidth("Progress Bar");
         table.getColumnModel().getColumn(2).setPreferredWidth(width);
      }
      
      /**
       * Adds a row.
       * 
       * @param col1 the string in first column.
       * @param col2 the string in second column.
       * @param col3 the fast progress bar in third column.
       */
      public void addRow(String col1, String col2, FastProgressBar col3)
      {
         TestTableRow row = new TestTableRow(col1, col2, col3);
         
         //Vector rowData = new Vector();
         //rowData.add(row);
         
         addValue(row);
         //setRowData(rowData);
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
         TestTableRow tableRow = (TestTableRow)getValueAt(row);
         if(tableRow != null)
         {
            // get specified column
            rval = tableRow.get(column);
         }
         
         return rval;
      }
      
      /**
       * Inner class for table row. Implements ChangeReporter so that 
       * progress bar can update quickly without redrawing entire table.
       * 
       * @author Mike Johnson
       */
      public static class TestTableRow
      implements ChangeReporter
      {
         /**
          * The first column data.
          */
         protected String mCol0;
         
         /**
          * The second column data.
          */
         protected String mCol1;
         
         /**
          * The progress bar that goes in the third column.
          */
         protected FastProgressBar mProgressBar;
         
         /**
          * The change reporter delegate for this row.
          */
         protected EventDelegate mChangeDelegate;
         
         /**
          * Creates a table row using the supplied parameters.
          * 
          * @param col1 a string in the first column.
          * @param col2 a string in the second column.
          * @param col3 a fast progress bar in the third column.
          */
         public TestTableRow(String col1, String col2, FastProgressBar col3)
         {
            mCol0 = col1;
            mCol1 = col2;
            mProgressBar = col3;
            mProgressBar.getChangeDelegate().addListener(this, 
                  "tableRowChanged");
            
            mChangeDelegate = new EventDelegate();
         }
         
         /**
          * Gets the object at the specified column.
          * 
          * @param column the column to get.
          * @return the object in the column, the table row object if 
          *    column does not exist.
          */
         public Object get(int column)
         {
            Object rval = null;
            
            switch(column)
            {
            case 0:
               rval = mCol0;
               break;
            case 1:
               rval = mCol1;
               break;
            case 2:
               rval = mProgressBar;
               break;
            default:
                  rval = this;
               break;
            }
            
            return rval;
         }
         
         /**
          * Gets the change delegate for this change reporter.
          *
          * @return the change delegate.
          */
         public EventDelegate getChangeDelegate()
         {
            return mChangeDelegate;
         }
         
         /**
          * Passes on the change event.
          * 
          * @param event the change event.
          */
         public void tableRowChanged(EventObject event)
         {
            // override source
            EventObject rowEvent = new EventObject("tableRowChanged");
            rowEvent.setData("source", this);
            
            // pass it on
            mChangeDelegate.fireEvent(rowEvent);
         }
      }
   }
}
