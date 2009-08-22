/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
//import java.awt.Component;
import java.awt.Dimension;
import java.awt.GridLayout;
import java.awt.Point;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
//import java.util.Iterator;
//import java.util.Vector;

//import javax.swing.AbstractListModel;
import javax.swing.BorderFactory;
import javax.swing.DefaultListModel;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
//import javax.swing.ListCellRenderer;
import javax.swing.UIManager;

import com.db.gui.JComponentCellRenderer;
import com.db.gui.PositionConstraints;
import com.db.gui.PositionLayout;
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
      LoggerManager.setFile("dbgui", "utcustomerrenderers.log");
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
      panel.setLayout(new PositionLayout(panel, 500, 500));
      
      // create list model
      DefaultListModel model = new DefaultListModel();
      
      // create list
      JList list = new JList(model);
      
      // set custom renderer
      list.setCellRenderer(new JComponentCellRenderer());
      
      // create panel to put in list
      JPanel listPanel = createJListTestPanel2();
      
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
    * Creates a panel to place in a JList as a test.
    * 
    * @return the panel to render in a JList as a test.
    */
   public static JPanel createJListTestPanel1()
   {
      // create panel to put in list
      JPanel panel = new JPanel();
      panel.setLayout(new GridLayout(2, 1));
      
      // add button to list panel
      JButton button = new JButton("button1");
      button.addActionListener(new UTCustomRenderers());
      panel.add(button);
      
      // add label to list panel
      JLabel label = new JLabel("label1");
      panel.add(label);
      
      return panel;
   }
   
   /**
    * Creates a panel to place in a JList as a test.
    * 
    * @return the panel to render in a JList as a test.
    */
   public static JPanel createJListTestPanel2()
   {
      JPanel panel = new JPanel();
      
      // set title and filename
      //panel.setOpaque(false);
      panel.setBorder(BorderFactory.createEtchedBorder());
      panel.setLayout(new PositionLayout(panel, 600, 200));
      
      // create detail labels
      JLabel title = new JLabel("Some Title");
      //JLabel filename = new JLabel("");
      
      PositionConstraints titlePC = new PositionConstraints();
      titlePC.size = new Dimension(title.getPreferredSize());
      titlePC.location = new Point();
      
      // add labels to panel
      panel.add(title, titlePC);
      
      // multi-ware, display with multi ware info
      
      JPanel dwPanel = new JPanel();
      dwPanel.setBorder(BorderFactory.createTitledBorder("Contents:"));
      dwPanel.setLayout(new PositionLayout(dwPanel, 0, 0));
      
      PositionConstraints dwPanelPC = new PositionConstraints();
      dwPanelPC.size = new Dimension(PositionConstraints.HORIZONTAL_FILL, 0);
      dwPanelPC.location = new Point(0, titlePC.getBottom());
      
      // add dw panel
      panel.add(dwPanel, dwPanelPC);
      
      Point bottom = new Point(0, 0);
      
      // display inner digital works
      int count = 1;
      for(int i = 0; i < count; i++)
      {
         // create a new inner ware panel
         JPanel warePanel = new JPanel();
         warePanel.setLayout(new PositionLayout(warePanel, 500, 500));
         
         JLabel wareTitle = new JLabel("Inner Title " + i);
         JLabel wareFile = new JLabel("InnerFile" + i);
         
         PositionConstraints dwTitlePC = new PositionConstraints();
         dwTitlePC.size = new Dimension(wareTitle.getPreferredSize());
         dwTitlePC.location = new Point();
         
         PositionConstraints dwFilePC = new PositionConstraints();
         dwFilePC.size = new Dimension(wareFile.getPreferredSize());
         dwTitlePC.location = new Point(0, 
               wareTitle.getPreferredSize().height);
         
         // add ware elements to ware panel
         warePanel.add(wareTitle, dwTitlePC);
         warePanel.add(wareFile, dwFilePC);
         
         PositionConstraints warePanelPC = new PositionConstraints();
         // FIXME: what is this "* 2" for?
         warePanelPC.size =
            new Dimension(PositionConstraints.HORIZONTAL_FILL,
                          wareTitle.getPreferredSize().height * 2);
         warePanelPC.location = new Point(0, bottom.y);
         
         // move the bottom point down
         bottom.x = 0;
         bottom.y = warePanelPC.getBottom();
         
         // add ware panel to the bottom of the dwPanel
         
         dwPanelPC.size = new Dimension(PositionConstraints.HORIZONTAL_FILL,
               dwPanelPC.size.height + warePanelPC.size.height);
         ((PositionLayout)panel.getLayout()).setConstraints(dwPanel, dwPanelPC);
         
         System.out.println("warePanelPC=" + warePanelPC);
         
         dwPanel.add(warePanel, warePanelPC);
      }
      
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
