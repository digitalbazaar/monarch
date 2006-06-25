/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
import java.awt.Dimension;
import java.awt.Point;

import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JTextField;
import javax.swing.UIManager;

import com.db.event.EventObject;
import com.db.gui.PositionConstraints;
import com.db.gui.PositionLayout;
import com.db.gui.wizard.Wizard;
import com.db.gui.wizard.WizardBuilder;
import com.db.gui.wizard.WizardFrame;
import com.db.gui.wizard.WizardPage;
import com.db.gui.wizard.WizardPagePool;
import com.db.gui.wizard.WizardPageSelector;
import com.db.gui.wizard.WizardPageView;
import com.db.gui.wizard.WizardTask;
import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * This class is used to test Wizard functionality and display.
 * 
 * @author Dave Longley
 */
public class UTWizard
{
   /**
    * The main method.
    * 
    * @param args arguments.
    */
   public static void main(String[] args)
   {
      // set up logger
      LoggerManager.setFile("dbgui", "utwizard.log", false);
      LoggerManager.setFile("dbutil", "utwizard.log");
      LoggerManager.setConsoleVerbosity("dbgui", Logger.DEBUG_VERBOSITY);
      LoggerManager.setConsoleVerbosity("dbutil", Logger.ERROR_VERBOSITY);
      
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
      
      // create wizard builder to create wizard
      TestWizardBuilder builder = new TestWizardBuilder();
      Wizard testWizard = builder.createWizard();
      
      // listen for wizard start
      testWizard.getWizardStartedEventDelegate().addListener(
         UTWizard.class, "wizardStarted");

      // listen for wizard finish
      testWizard.getWizardFinishedEventDelegate().addListener(
         UTWizard.class, "wizardFinished");
      
      // listen for wizard cancellation
      testWizard.getWizardCancelledEventDelegate().addListener(
         UTWizard.class, "wizardCancelled");
      
      // create wizard frame
      WizardFrame frame = new WizardFrame(testWizard);
      frame.setTitle("Test Wizard Frame");
      
      // set close operation
      frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
      
      // start wizard
      testWizard.startWizard();
   }
   
   /**
    * Called when the wizard is started.
    * 
    * @param event the wizard started event.
    */
   public static void wizardStarted(EventObject event)
   {
      System.out.println("Wizard Started.");
   }
   
   /**
    * Called when the wizard is finished.
    * 
    * @param event the wizard finished event.
    */
   public static void wizardFinished(EventObject event)
   {
      System.out.println("Wizard Finished.");
   }
   
   /**
    * Called when the wizard is cancelled.
    * 
    * @param event the wizard cancelled event.
    */
   public static void wizardCancelled(EventObject event)
   {
      System.out.println("Wizard Cancelled.");
   }
   
   /**
    * The test wizard builder.
    * 
    * @author Dave Longley
    */
   public static class TestWizardBuilder implements WizardBuilder
   {
      /**
       * Creates a new TestWizardBuilder.
       */
      public TestWizardBuilder()
      {
      }
      
      /**
       * Creates the task for a Wizard.
       * 
       * @return the task for a Wizard.
       */
      public WizardTask createTask()
      {
         // create TestWizardTask
         return new TestWizardTask();
      }
      
      /**
       * Creates the page pool for a Wizard.
       * 
       * @param task the wizard task for the pages.
       * 
       * @return the page pool for a Wizard.
       */
      public WizardPagePool createPagePool(WizardTask task)
      {
         // create page pool
         WizardPagePool pagePool = new WizardPagePool(); 
         
         // create page 1
         TestWizardPage page1 = new TestWizardPage("page1", task);
         
         // create page 2
         TestWizardPage page2 = new TestWizardPage("page2", task);
         
         // create page 3
         TestWizardPage page3 = new TestWizardPage("page3", task);
         
         // add pages
         pagePool.addPage(page1);
         pagePool.addPage(page2);
         pagePool.addPage(page3);
         
         // return page pool
         return pagePool;
      }
      
      /**
       * Creates the page selector for a Wizard.
       * 
       * @param pagePool the page pool for the wizard.
       * 
       * @return the page selector for a Wizard.
       */
      public WizardPageSelector createPageSelector(WizardPagePool pagePool)
      {
         // create TestWizardPageSelector
         return new TestWizardPageSelector();
      }
      
      /**
       * Creates a new Wizard.
       * 
       * @return a new Wizard.
       */
      public Wizard createWizard()      
      {
         // create a new task
         WizardTask task = createTask();
         
         // create a new page pool for the task
         WizardPagePool pagePool = createPagePool(task);
         
         // create a page selector
         WizardPageSelector pageSelector = createPageSelector(pagePool);
         
         // create wizard
         Wizard wizard = new Wizard(task, pagePool, pageSelector);
         
         // return wizard
         return wizard;
      }
   }
   
   /**
    * The test wizard page selector.
    * 
    * @author Dave Longley
    */
   public static class TestWizardPageSelector implements WizardPageSelector
   {
      /**
       * Creates a new TestWizardPageSelector.
       */
      public TestWizardPageSelector()
      {
      }
      
      /**
       * Gets the first wizard page in a given wizard page pool.
       * 
       * @param pagePool the wizard page pool to get the first page in.
       * 
       * @return the first wizard page in the given pool.
       */
      public WizardPage getFirstPage(WizardPagePool pagePool)
      {
         return pagePool.getPage("page1");
      }
      
      /**
       * Gets the next wizard page in a given wizard page pool, given the
       * current page.
       * 
       * @param pagePool the wizard page pool to get the next page in.
       * @param current the current wizard page or null if there no
       *                current page.
       *
       * @return the next wizard page.
       */
      public WizardPage getNextPage(WizardPagePool pagePool, WizardPage current)
      {
         WizardPage rval = null;
         
         if(current != null)
         {
            if(current.getName().equals("page1"))
            {
               rval = pagePool.getPage("page2");
            }
            else if(current.getName().equals("page2"))
            {
               rval = pagePool.getPage("page3");
            }
         }
         
         return rval;
      }
      
      /**
       * Gets the final wizard page in a given wizard page pool.
       * 
       * @param pagePool the wizard page pool to get the final page in.
       * 
       * @return the final wizard page in the given pool.
       */
      public WizardPage getFinalPage(WizardPagePool pagePool)
      {
         return pagePool.getPage("page3");
      }
   }
   
   /**
    * The test wizard task.
    * 
    * @author Dave Longley
    */
   public static class TestWizardTask implements WizardTask
   {
      /**
       * The option that are selected by the wizard. 
       */
      protected String[] mOptions = new String[3];

      /**
       * Creates a new TestWizardTask.
       */
      public TestWizardTask()
      {
         mOptions = new String[3];
         
         mOptions[0] = "";
         mOptions[1] = "";
         mOptions[2] = "";
      }
      
      /**
       * Starts this wizard task.
       */
      public void start()
      {
         System.out.println("Started TestWizardTask.");
      }
      
      /**
       * Finishes this wizard task.
       */
      public void finish()
      {
         System.out.println("Finished TestWizardTask.");
      }
      
      /**
       * Cancels this wizard task.
       */
      public void cancel()      
      {
         System.out.println("Cancelled TestWizardTask.");
      }
      
      /**
       * Sets an option.
       * 
       * @param option the selected option.
       * @param index which number option it is.
       */
      public void setOption(String option, int index)
      {
         mOptions[index] = option;
      }
      
      /**
       * Gets an option.
       * 
       * @param index the number option to retrieve.
       * 
       * @return an option.
       */
      public String getOption(int index)
      {
         return mOptions[index];
      }
   }
   
   /**
    * The test wizard page.
    * 
    * @author Dave Longley
    */
   public static class TestWizardPage extends WizardPage
   {
      /**
       * Creates a new TestWizardPage.
       * 
       * @param name the wizard page name (used to uniquely identify this page).
       * @param task the wizard task this page will work on. 
       */
      public TestWizardPage(String name, WizardTask task)
      {
         super(name, task);
      }
      
      /**
       * Creates the view for this page.
       * 
       * @return the view for this page.
       */
      protected WizardPageView createView()
      {
         return new TestWizardPageView(this);
      }
      
      /**
       * Checks all of the data that a wizard page contains for errors.
       * 
       * This method is called before writing the data to the WizardTask. 
       * 
       * @param task the WizardTask.
       * 
       * @return true if the page is valid, false otherwise.
       */
      public boolean validate(WizardTask task)
      {
         boolean rval = false;
         
         // clear errors
         clearErrors();
         
         // get the view
         TestWizardPageView view = (TestWizardPageView)getView();
         
         // make sure the view has an option set
         if(!view.getOption().equals(""))
         {
            // option set
            rval = true;
         }
         else
         {
            // add errors
            addError("You must enter some text into the text field.");
         }
         
         return rval;
      }
      
      /**
       * Updates the WizardTask with the data on this page.
       * 
       * This method is called after validate() and before proceeding to the
       * next step in a wizard.
       *
       * @param task the WizardTask to update. 
       */
      public void updateWizardTask(WizardTask task)
      {
         // cast the wizard task
         TestWizardTask testWizardTask = (TestWizardTask)task;
         
         int index = 0;
         if(getName().equals("page2"))
         {
            index = 1;
         }
         else if(getName().equals("page3"))
         {
            index = 2;
         }
         
         // get the view
         TestWizardPageView view = (TestWizardPageView)getView();
         
         // set the option
         testWizardTask.setOption(view.getOption(), index);
      }
   }
   
   /**
    * The test wizard page view.
    * 
    * @author Dave Longley
    */
   public static class TestWizardPageView extends WizardPageView
   {
      /**
       * The text field for option selection.
       */
      protected JTextField mTextField;
      
      /**
       * Creates a new TestWizardPageView.
       * 
       * @param page the wizard page this view is for.
       */
      public TestWizardPageView(WizardPage page)
      {
         super(page);
         
         // sets up this view
         setupView();
      }
      
      /**
       * Sets up this view.
       */
      protected void setupView()
      {
         // set layout
         setSize(500, 500);
         setLayout(new PositionLayout(this));
         
         // create JLabel
         JLabel label = new JLabel(getPage().getName());
         label.setHorizontalAlignment(JLabel.CENTER);
         
         // create a JTextField
         mTextField = new JTextField();
         
         // create the label constraints
         PositionConstraints labelConstraints = new PositionConstraints();
         labelConstraints.location = new Point(5, 5);
         labelConstraints.size = new Dimension(
            getWidth() - 10, label.getPreferredSize().height);
         labelConstraints.anchor =
            PositionConstraints.ANCHOR_TOP_LEFT_RIGHT;
         
         // create the text field constraints
         PositionConstraints textFieldConstraints = new PositionConstraints();
         textFieldConstraints.location =
            new Point(5, labelConstraints.getBottom() + 10);
         textFieldConstraints.size = new Dimension(
            getWidth() - 10, mTextField.getPreferredSize().height);
         textFieldConstraints.anchor =
            PositionConstraints.ANCHOR_TOP_LEFT_RIGHT;
         
         // add components to the page
         add(label, labelConstraints);
         add(mTextField, textFieldConstraints);
      }
      
      /**
       * Gets the option set by this view.
       * 
       * @return the option set by this view.
       */
      public String getOption()
      {
         return mTextField.getText();
      }
   }
}
