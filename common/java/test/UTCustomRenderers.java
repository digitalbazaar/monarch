/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
import java.awt.Component;
import java.awt.Dimension;
import java.awt.GridLayout;
import java.awt.Point;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Vector;

import javax.swing.AbstractListModel;
import javax.swing.DefaultListModel;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.ListCellRenderer;
import javax.swing.UIManager;

import com.db.common.gui.JComponentCellRenderer;
import com.db.common.gui.PositionConstraints;
import com.db.common.gui.PositionLayout;
import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * This class is used to test custom renderers.
 * 
 * @author Dave Longley
 */
public class UTCustomRenderers implements ActionListener
{
   /**
    * The main method.
    * 
    * @param args arguments.
    */
   public static void main(String[] args)
   {
      // set up logger
      LoggerManager.setFile("dbcommon", "utcustomerrenderers.log", false);
      LoggerManager.setConsoleVerbosity("dbcommon", Logger.DEBUG_VERBOSITY);
      
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
      JPanel panel = createListCellPanel();
      frame.getContentPane().add(panel);
      
      // pack frame
      frame.pack();
      frame.setSize(600, 600);
      frame.setVisible(true);
   }
   
   /**
    * Creates a panel for testing a list cell renderer.
    * 
    * @return the panel for testing a list cell renderer.
    */
   public static JPanel createListCellPanel()
   {
      JPanel panel = new JPanel();
      
      // set layout
      panel.setSize(500, 500);
      panel.setLayout(new PositionLayout(panel));
      
      // create list model
      DefaultListModel model = new DefaultListModel();
      
      // create list
      JList list = new JList(model);
      
      // set custom renderer
      list.setCellRenderer(new JComponentCellRenderer());
      
      // create panel to put in list
      JPanel listPanel = new JPanel();
      listPanel.setLayout(new GridLayout(2, 1));
      
      // add button to list panel
      JButton button = new JButton("button1");
      button.addActionListener(new UTCustomRenderers());
      listPanel.add(button);
      
      // add label to list panel
      JLabel label = new JLabel("label1");
      listPanel.add(label);
      
      // add list panel to list model
      model.addElement(listPanel);
      
      // create scroll pane for list
      JScrollPane scrollPane = new JScrollPane(list);
      
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
    * Called when an action is performed.
    * 
    * @param e the action event.
    */
   public void actionPerformed(ActionEvent e)
   {
      System.out.println("ACTION PERFORMED!");
   }
}
