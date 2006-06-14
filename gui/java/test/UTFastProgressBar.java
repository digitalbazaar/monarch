/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
import java.awt.Dimension;
import java.awt.Point;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

//import javax.swing.BorderFactory;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JProgressBar;
import javax.swing.UIManager;
//import javax.swing.border.BevelBorder;

import com.db.gui.FastProgressBar;
import com.db.gui.PositionConstraints;
import com.db.gui.PositionLayout;
import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * This class is used to test a JComponentTable.
 * 
 * @author Dave Longley
 */
public class UTFastProgressBar implements ActionListener
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
      JFrame frame = new JFrame("TEST FRAME");
      frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
      
      // create and add panel
      JPanel panel = createProgressBarPanel();
      frame.getContentPane().add(panel);
      
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
      panel.setSize(500, 500);
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
      panel.add(smJProgressBar, jProgressBarConstraints);
      
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
   }
}
