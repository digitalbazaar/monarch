/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
import java.awt.GridLayout;

import javax.swing.DefaultListModel;
import javax.swing.JFrame;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.JSplitPane;
import javax.swing.UIManager;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * This class is used to test drag 'n drop.
 * 
 * @author Dave Longley
 */
public class UTDragNDrop
{
   /**
    * The main method.
    * 
    * @param args arguments.
    */
   public static void main(String[] args)
   {
      // set up logger
      LoggerManager.setFile("dbgui", "utdragndrop.log", false);
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
      JPanel panel = createDragNDropPanel();
      frame.getContentPane().add(panel);
      
      // pack frame
      frame.pack();
      frame.setSize(600, 600);
      frame.setLocationRelativeTo(null);
      frame.setVisible(true);
   }
   
   /**
    * Creates a panel for testing drag 'n drop.
    * 
    * @return the panel for testing drag 'n drop.
    */
   public static JPanel createDragNDropPanel()
   {
      JPanel panel = new JPanel();
      
      // set layout
      panel.setLayout(new GridLayout(1, 1));
      
      // create list1
      DefaultListModel model1 = new DefaultListModel();
      JList list1 = new JList(model1);
      
      // create list2
      DefaultListModel model2 = new DefaultListModel();
      JList list2 = new JList(model2);
      
      // create split pane
      JSplitPane splitPane = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT);
      splitPane.setLeftComponent(list1);
      splitPane.setRightComponent(list2);
      splitPane.setResizeWeight(0.5D);

      // add components
      panel.add(splitPane);
      
      return panel;
   }
}
