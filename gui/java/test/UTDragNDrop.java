/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
import java.awt.Component;
import java.awt.GridLayout;
import java.awt.Image;
import java.awt.Point;
import java.awt.dnd.DnDConstants;

import javax.swing.DefaultListModel;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.JSplitPane;
import javax.swing.ListModel;
import javax.swing.UIManager;

import com.db.gui.ComponentImageCreator;
import com.db.gui.DragImageProvider;
import com.db.gui.DraggableObjectAcceptor;
import com.db.gui.DraggableObjectDestination;
import com.db.gui.DraggableObjectPool;
import com.db.gui.DraggableObjectSource;
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
      model1.addElement("DRAG AND DROP ME 1");
      model1.addElement("DRAG AND DROP ME 2");
      model1.addElement("DRAG AND DROP ME 3");
      model1.addElement("DRAG AND DROP ME 4");
      DnDList list1 = new DnDList(model1);
      
      // create list2
      DefaultListModel model2 = new DefaultListModel();
      DnDList list2 = new DnDList(model2);
      
      // create split pane
      JSplitPane splitPane = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT);
      splitPane.setLeftComponent(list1);
      splitPane.setRightComponent(list2);
      splitPane.setResizeWeight(0.5D);

      // add components
      panel.add(splitPane);
      
      return panel;
   }
   
   /**
    * A drag 'n drop JList.
    * 
    * @author Dave Longley
    */
   public static class DnDList extends JList
   implements DraggableObjectAcceptor, DraggableObjectPool
   {
      /**
       * A draggable object source for this list.
       */
      protected DraggableObjectSource mDragSource;
      
      /**
       * A draggable object destination for this list.
       */
      protected DraggableObjectDestination mDragDestination;
      
      /**
       * Creates a new DnDList.
       * 
       * @param model the model for the list.
       */
      public DnDList(ListModel model)
      {
         super(model);
         
         // create TestDragImageProvider
         TestDragImageProvider imageProvider = new TestDragImageProvider();
         
         // create the drag source
         mDragSource = new DraggableObjectSource(
            this, DnDConstants.ACTION_MOVE, this, imageProvider);
         
         // create the drop destination
         mDragDestination = new DraggableObjectDestination(
            this, DnDConstants.ACTION_MOVE, this, imageProvider);
      }
      
      /**
       * Returns true if the passed object can be accepted by the
       * given destination component.
       * 
       * @param obj the object to transfer.
       * @param action the drag 'n drop action:
       * 
       * DnDConstants.ACTION_NONE,
       * DnDConstants.ACTION_COPY,
       * DnDConstants.ACTION_MOVE,
       * DnDConstants.ACTION_COPY_OR_MOVE
       * 
       * @param destination the destination component.
       * @param location the current location of the drag.
       * 
       * @return true if the passed object can be accepted, false if not.
       */
      public boolean canAcceptDraggableObject(
         Object obj, int action, Component destination, Point location)
      {
         boolean rval = false;
         
         if(action == DnDConstants.ACTION_MOVE) 
         {
            // if the action is a move, it can be done
            rval = true;
         }
         
         return rval;
      }
      
      /**
       * Accepts the passed object for the given destination component
       * when the object is dropped.
       * 
       * @param obj the object to accept.
       * @param action the drag 'n drop action:
       * 
       * DnDConstants.ACTION_NONE,
       * DnDConstants.ACTION_COPY,
       * DnDConstants.ACTION_MOVE,
       * DnDConstants.ACTION_COPY_OR_MOVE
       * 
       * @param destination the destination component.
       * @param location the location of the drop.
       */
      public void acceptDraggableObject(
         Object obj, int action, Component destination, Point location)      
      {
         // the object should be an array of selected objects
         Object[] selected = (Object[])obj;
         
         // assume the action is a move

         // get the list model
         DefaultListModel model = (DefaultListModel)getModel();
         
         // add the objects to the model
         for(int i = 0; i < selected.length; i++)
         {
            model.addElement(selected[i]);
         }
      }
      
      /**
       * Gets an object that is to be dragged (and potentially dropped) from
       * a DraggableObjectSource.
       * 
       * @param source the component to that is being dragged from.
       * 
       * @return an object to be dragged from a DraggableObjectSource or null
       *         if no object can currently be dragged.
       */
      public Object getDraggableObject(Component source)
      {
         // get the currently selected objects from the list
         Object rval = null;
         
         Object[] selected = getSelectedValues();
         if(selected.length > 0)
         {
            rval = selected;
         }
         
         return rval;
      }
      
      /**
       * Called when a draggable Object from this pool is dropped. The
       * dropped object may or may not need to be removed from this pool
       * depending on the drag 'n drop action. 
       * 
       * @param obj the draggable object that was dropped.
       * @param action the drag 'n drop action:
       * 
       * DnDConstants.ACTION_NONE,
       * DnDConstants.ACTION_COPY,
       * DnDConstants.ACTION_MOVE,
       * DnDConstants.ACTION_COPY_OR_MOVE
       *  
       * @param source the component that the draggable Object was dragged from.
       */
      public void draggableObjectDropped(
         Object obj, int action, Component source)      
      {
         // the object should be an array of selected objects
         Object[] selected = (Object[])obj;
         
         // assume the action is a move

         // get the list model
         DefaultListModel model = (DefaultListModel)getModel();
         
         // remove the objects from the model
         for(int i = 0; i < selected.length; i++)
         {
            model.removeElement(selected[i]);
         }         
      }
   }
   
   /**
    * A TestDragImageProvider.
    * 
    * @author Dave Longley
    */
   public static class TestDragImageProvider implements DragImageProvider
   {
      /**
       * The image to provide.
       */
      protected Image mImage;
      
      /**
       * The current object being dragged.
       */
      protected Object mObject;
      
      /**
       * Creates a new TestDragImageProvider.
       */
      public TestDragImageProvider()
      {
         mImage = null;
         mObject = null;
      }
      
      /**
       * Gets the drag image. This is the image to display while dragging.
       * 
       * @param obj the object being dragged.
       * @param action the drag 'n drop action:
       * 
       * DnDConstants.ACTION_NONE,
       * DnDConstants.ACTION_COPY,
       * DnDConstants.ACTION_MOVE,
       * DnDConstants.ACTION_COPY_OR_MOVE
       * 
       * @param component the component the drag is over
       *                  (either a source or destination).
       *  
       * @return the drag image.
       */
      public Image getDragImage(Object obj, int action, Component component)
      {
         if(mObject != obj)
         {
            // store the object being dragged
            mObject = obj;
            
            // consider the object an array of selected strings
            Object[] selected = (Object[])obj;
            
            // get the first selected object as a string
            String text = (String)selected[0];
            
            // create the image to provide
            JLabel label = new JLabel(text);
            mImage = ComponentImageCreator.createComponentImage(
               label, label.getPreferredSize(), 1.0F);
         }
         
         return mImage;
      }
      
      /**
       * Gets the drag image offset. This is the offset, from the cursor,
       * to display the drag image at.
       * 
       * @param obj the object being dragged.
       * @param action the drag 'n drop action:
       * 
       * DnDConstants.ACTION_NONE,
       * DnDConstants.ACTION_COPY,
       * DnDConstants.ACTION_MOVE,
       * DnDConstants.ACTION_COPY_OR_MOVE
       * 
       * @param component the component the drag is over
       *                  (either a source or destination).
       * 
       * @return the drag image offset.
       */
      public Point getDragImageOffset(
         Object obj, int action, Component component)    
      {
         return new Point(10, -5);
      }
   }
}
