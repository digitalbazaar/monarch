/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import java.awt.CardLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.GridLayout;
import java.awt.Insets;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.awt.event.MouseWheelEvent;
import java.awt.event.MouseWheelListener;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Set;
import java.util.Vector;

import javax.swing.Box;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JViewport;
import javax.swing.UIManager;
import javax.swing.border.AbstractBorder;
import javax.swing.border.Border;
import javax.swing.plaf.basic.BasicArrowButton;

/**
 * A tab panel that allows any arbitrary component to be put into the
 * tab area of a tab.
 * 
 * @author Dave Longley
 */
public class TabPanel extends JPanel
{
   /**
    * The left scroll button.
    */
   protected JButton mLeftScrollButton;
   
   /**
    * The right scroll button.
    */
   protected JButton mRightScrollButton;

   /**
    * The panel for the tab area.
    */
   protected JPanel mTabAreaPanel;
   
   /**
    * The scroll pane for the tab area panel.
    */
   protected JScrollPane mTabAreaScrollPane;

   /**
    * The leading tab area. That is, the tab area that is first
    * in the tab area viewport.
    */
   protected Component mLeadingTabArea;
   
   /**
    * The tab area insets.
    */
   protected Insets mTabAreaInsets;
   
   /**
    * The maximum height for tab areas.
    */
   protected int mMaxTabAreaHeight;
   
   /**
    * A map of tab area components to tab content components.
    */
   protected HashMap mAreaToContent;
   
   /**
    * A map of tab content to tab area components.
    */
   protected HashMap mContentToArea;
   
   /**
    * A map of tab area parents to tab areas.
    */
   protected HashMap mTabAreaParentToTabArea;
   
   /**
    * The panel for the tab content.
    */
   protected JPanel mTabContentPanel;
   
   /**
    * The tab content insets.
    */
   protected Insets mTabContentInsets;   
   
   /**
    * A hash map of ids (for the card layout) to the appropriate
    * content parent panels.
    */
   protected HashMap mIdToContentParent;
   
   /**
    * A hash map of content parent panels to the appropriate
    * id (for the card layout).
    */
   protected HashMap mContentParentToId;
   
   /**
    * A hash map of content parent panels to their content children.
    */
   protected HashMap mContentParentToContent;
   
   /**
    * A mapping of tab content to flash thread.
    */
   protected HashMap mFlashThreads;
   
   /**
    * The currently selected content component, if any.
    */
   protected Component mSelectedContent;
   
   /**
    * The "no selection" tab content component.
    */
   protected Component mNoSelectionContent;
   
   /**
    * The tab panel listeners.
    */
   protected Vector mTabPanelListeners;
   
   /**
    * The tab panel handler.
    */
   protected TabPanelHandler mTabPanelHandler;
   
   /**
    * The scroll controls handler.
    */
   protected ScrollControlsHandler mScrollControlsHandler;
   
   /**
    * The currently mouse entered tab area. Often null. 
    */
   protected Component mMouseEnteredTabArea;
   
   /**
    * An ordered list of the selected tabs. 
    */
   protected Vector mSelectedTabs;
   
   /**
    * The tab selection policy for this panel.
    */
   protected int mTabSelectionPolicy;
   
   /**
    * A tab selection policy. If this panel uses this policy, then
    * whenever a selected tab is removed, no other tab will be selected.
    */
   public static final int NO_TAB_SELECTION_POLICY = 0;
   
   /**
    * A tab selection policy. If this panel uses this policy, then
    * whenever a selected tab is removed, the tab that was selected
    * before it will be selected. If there was no previously selected tab,
    * then the previous tab (usually to the left) will be selected (if
    * there is one). If there are no previous tabs to select, then the
    * next tab (usually to the right) will be selected. If there are
    * no tabs to select, then none will be selected.
    */
   public static final int SELECT_PREVIOUSLY_SELECTED_TAB_POLICY = 1;
   
   /**
    * A tab selection policy. If this panel uses this policy, then
    * whenever a selected tab is removed, the previous tab (usually to
    * the left) will be selected, if there is a previous tab. If there is
    * no previous tab, then the next tab (usually to the right) will be
    * selected. If there are no tabs to select, then none will be selected. 
    */
   public static final int SELECT_PREVIOUS_TAB_POLICY = 2;
   
   /**
    * A tab selection policy. If this panel uses this policy, then
    * whenever a selected tab is removed, the next tab (usually to
    * the right) will be selected, if there is a next tab. If there is
    * no next tab, then the previous tab (usually to the left) will be
    * selected. If there are no tabs to select, then none will be selected. 
    */
   public static final int SELECT_NEXT_TAB_POLICY = 3;   
   
   /**
    * Creates a new TabPanel.
    */
   public TabPanel()
   {
      setup();
   }
   
   /**
    * Called when a new tab is added to the tab panel to notify listeners.
    * 
    * @param content the content component that was added to the tab panel.
    */
   protected void fireTabAdded(Component content)
   {
      Iterator i = mTabPanelListeners.iterator();
      while(i.hasNext())
      {
         TabPanelListener tpl = (TabPanelListener)i.next();
         tpl.tabAdded(content);
      }
   }
   
   /**
    * Called when a new tab is remove from the tab panel to notify listeners.
    * 
    * @param content the content component that was removed from the tab panel.
    */
   protected void fireTabRemoved(Component content)
   {
      Iterator i = mTabPanelListeners.iterator();
      while(i.hasNext())
      {
         TabPanelListener tpl = (TabPanelListener)i.next();
         tpl.tabRemoved(content);
      }
   }
   
   /**
    * Called when the tab selection changes in the tab panel to
    * notify listeners.
    * 
    * @param oldSelection the old tab selection (can be null).
    * @param newSelection the new tab selection.
    */
   protected void fireTabSelectionChanged(Component oldSelection,
                                          Component newSelection)
   {
      Iterator i = mTabPanelListeners.iterator();
      while(i.hasNext())
      {
         TabPanelListener tpl = (TabPanelListener)i.next();
         tpl.tabSelectionChanged(oldSelection, newSelection);
      }
   }
   
   /**
    * Gets the tab panel handler.
    * 
    * @return the tab panel handler.
    */
   protected TabPanelHandler getTabPanelHandler()
   {
      if(mTabPanelHandler == null)
      {
         mTabPanelHandler = new TabPanelHandler();
      }
      
      return mTabPanelHandler;
   }
   
   /**
    * Gets the scroll controls handler.
    * 
    * @return the scroll controls handler.
    */
   protected ScrollControlsHandler getScrollControlsHandler()
   {
      if(mScrollControlsHandler == null)
      {
         mScrollControlsHandler = new ScrollControlsHandler();
      }
      
      return mScrollControlsHandler;
   }   
   
   /**
    * Sets up the tab panel.
    */
   protected void setup()
   {
      // create hash tables
      mAreaToContent = new HashMap();
      mContentToArea = new HashMap();
      mTabAreaParentToTabArea = new HashMap();
      mIdToContentParent = new HashMap();
      mContentParentToId = new HashMap();
      mContentParentToContent = new HashMap();
      mFlashThreads = new HashMap();
      
      // create listener collections
      mTabPanelListeners = new Vector();
      
      // set tab area insets
      mTabAreaInsets = new Insets(7, 7, 3, 7);
      
      // set tab content insets
      mTabContentInsets = new Insets(4, 3, 3, 3);
      
      // set the background color
      setBackground(getShadowColor());//getContentAreaColor());
      
      // make double buffered
      setDoubleBuffered(true);
      
      // set layout
      super.setLayout(new PositionLayout(this, 500, 500));
      
      // create tab scroll buttons
      createLeftTabScrollButton();
      createRightTabScrollButton();

      // setup tab area panel and tab content panel
      setupTabAreaPanel();
      setupTabContentPanel();
      
      // left button constraints
      PositionConstraints leftButtonConstraints = new PositionConstraints();
      leftButtonConstraints.location = new Point(0, 0);
      leftButtonConstraints.size =
         new Dimension(mLeftScrollButton.getPreferredSize());
      
      // right button constraints
      PositionConstraints rightButtonConstraints = new PositionConstraints();
      rightButtonConstraints.location =
         new Point(getWidth() - mRightScrollButton.getPreferredSize().width, 0);
      rightButtonConstraints.size =
         new Dimension(mRightScrollButton.getPreferredSize());
      rightButtonConstraints.anchor =
         PositionConstraints.ANCHOR_TOP | PositionConstraints.ANCHOR_RIGHT;
      
      // tab area pane constraints
      PositionConstraints tabAreaPaneConstraints = new PositionConstraints();
      tabAreaPaneConstraints.location =
         new Point(leftButtonConstraints.getRight(), 0);
      tabAreaPaneConstraints.size =
         new Dimension(rightButtonConstraints.getLeft() -
                       tabAreaPaneConstraints.getLeft(),
                       rightButtonConstraints.size.height);
      tabAreaPaneConstraints.anchor = PositionConstraints.ANCHOR_TOP_LEFT_RIGHT;
      
      // tab content pane constraints
      PositionConstraints tabContentPaneConstraints = new PositionConstraints();
      tabContentPaneConstraints.location =
         new Point(0, leftButtonConstraints.getBottom());
      tabContentPaneConstraints.size =
         new Dimension(getWidth(),
                       getHeight() - tabContentPaneConstraints.getTop());
      tabContentPaneConstraints.anchor = PositionConstraints.ANCHOR_ALL;
      
      super.add(mLeftScrollButton, leftButtonConstraints);
      super.add(mTabAreaScrollPane, tabAreaPaneConstraints);
      super.add(mRightScrollButton, rightButtonConstraints);
      super.add(mTabContentPanel, tabContentPaneConstraints);

      // create vector for keeping track of selected tabs
      mSelectedTabs = new Vector();
      
      // set the default tab selection policy
      mTabSelectionPolicy = SELECT_PREVIOUSLY_SELECTED_TAB_POLICY;
   }
   
   /**
    * Gets the content area color.
    * 
    * @return the content area color.
    */
   protected Color getContentAreaColor()
   {
      Color rval = null;
      
      if(!UIManager.getLookAndFeel().isNativeLookAndFeel())
      {
         rval = UIManager.getColor("TabbedPane.contentAreaColor");         
      }
      else
      {
         rval = UIManager.getColor("control");         
      }
      
      if(rval == null)
      {
         rval = Color.gray;
      }
      
      return rval;
   }
   
   /**
    * Gets the light highlight color.
    * 
    * @return the light highlight color.
    */
   protected Color getLightHighlightColor()
   {
      Color rval = null;
      
      if(!UIManager.getLookAndFeel().isNativeLookAndFeel())
      {
         rval = UIManager.getColor("TabbedPane.light");        
      }
      else
      {
         rval = UIManager.getColor("controlLtHighlight");         
      }
      
      if(rval == null)
      {
         rval = Color.lightGray.brighter();
      }
      
      return rval;
   }
   
   /**
    * Gets the highlight color.
    * 
    * @return the highlight color.
    */
   protected Color getHighlightColor()
   {
      Color rval = null;
      
      if(!UIManager.getLookAndFeel().isNativeLookAndFeel())
      {
         rval = UIManager.getColor("TabbedPane.highlight");         
      }
      else
      {
         rval = UIManager.getColor("controlHighlight");         
      }
      
      if(rval == null)
      {
         rval = Color.lightGray;
      }
      
      return rval;
   }   
   
   /**
    * Gets the shadow color.
    * 
    * @return the shadow color.
    */
   protected Color getShadowColor()
   {
      Color rval = null;
      
      if(!UIManager.getLookAndFeel().isNativeLookAndFeel())
      {
         rval = UIManager.getColor("TabbedPane.shadow");         
      }
      else
      {
         rval = UIManager.getColor("controlShadow");         
      }
      
      if(rval == null)
      {
         rval = Color.darkGray;
      }
      
      return rval;
   }
   
   /**
    * Gets the dark shadow color.
    * 
    * @return the dark shadow color.
    */
   protected Color getDarkShadowColor()
   {
      Color rval = null;
      
      if(!UIManager.getLookAndFeel().isNativeLookAndFeel())
      {
         rval = UIManager.getColor("TabbedPane.darkShadow");         
      }
      else
      {
         rval = UIManager.getColor("controlDkShadow");         
      }
      
      if(rval == null)
      {
         rval = Color.darkGray.darker();
      }
      
      return rval;
   }
   
   /**
    * Gets the focus color.
    * 
    * @return the focus color.
    */
   protected Color getFocusColor()
   {
      Color rval = null;
      
      if(!UIManager.getLookAndFeel().isNativeLookAndFeel())
      {
         rval = UIManager.getColor("TabbedPane.focus");         
      }
      else
      {
         rval = UIManager.getColor("textHighlight");         
      }
      
      if(rval == null)
      {
         rval = Color.lightGray;
      }
      
      return rval;
   }    
   
   /**
    * Gets the selected color.
    * 
    * @return the selected color.
    */
   protected Color getSelectedColor()
   {
      Color rval = null;
      
      if(!UIManager.getLookAndFeel().isNativeLookAndFeel())
      {
         rval = UIManager.getColor("TabbedPane.selected");         
      }
      else
      {
         rval = UIManager.getColor("controlHighlight");         
      }
      
      if(rval == null)
      {
         rval = Color.lightGray;
      }
      
      return rval;
   }
   
   /**
    * Gets the default flash color.
    * 
    * @return the default flash color.
    */
   protected Color getFlashingColor()
   {
      return getFocusColor();
   }   
   
   /**
    * Gets the default flash interval (in milliseconds).
    * 
    * @return the default flash interval in milliseconds.
    */
   protected long getFlashInterval()
   {
      return 750;
   }
   
   /**
    * Gets the unselected color.
    * 
    * @return the unselected color.
    */
   protected Color getUnselectedColor()
   {
      Color c = getSelectedColor();
      double scale = 0.9;
      int r = Math.min(255, (int)Math.round(c.getRed() * scale));
      int g = Math.min(255, (int)Math.round(c.getGreen() * scale));
      int b = Math.min(255, (int)Math.round(c.getBlue() * scale));
      
      c = new Color(r, g, b);

      return c;
   }
   
   /**
    * Gets the unselected highlight color.
    * 
    * @return the unselected highlight color.
    */
   protected Color getUnselectedHighlightColor()
   {
      Color c = getUnselectedColor();
      double scale = 1.05;
      int r = Math.min(255, (int)Math.round(c.getRed() * scale));
      int g = Math.min(255, (int)Math.round(c.getGreen() * scale));
      int b = Math.min(255, (int)Math.round(c.getBlue() * scale));
      
      c = new Color(r, g, b);
       
      return c;
   }

   /**
    * Ensures that this tab panel is valid.
    */
   protected void ensureValid()
   {
      if(!isValid())
      {
         validate();
      }
   }
   
   /**
    * Creates the left tab scroll button.
    */
   protected void createLeftTabScrollButton()
   {
      // scroll button facing left (WEST)
      mLeftScrollButton =
         new BasicArrowButton(BasicArrowButton.WEST,
               getSelectedColor(), getShadowColor(),
               getDarkShadowColor(), getHighlightColor());
      
      // add action listener
      mLeftScrollButton.setActionCommand("scroll_left");
      mLeftScrollButton.addActionListener(getScrollControlsHandler());
      mLeftScrollButton.addMouseListener(getScrollControlsHandler());
   }
   
   /**
    * Creates the right tab scroll button.
    */
   protected void createRightTabScrollButton()
   {
      // scroll button facing right (EAST)
      mRightScrollButton =
         new BasicArrowButton(BasicArrowButton.EAST,
               getSelectedColor(), getShadowColor(),
               getDarkShadowColor(), getHighlightColor());      
      
      // add action listener
      mRightScrollButton.setActionCommand("scroll_right");
      mRightScrollButton.addActionListener(getScrollControlsHandler());
      mRightScrollButton.addMouseListener(getScrollControlsHandler());
   }
   
   /**
    * Sets up the tab area.
    */
   protected void setupTabAreaPanel()
   {
      mTabAreaPanel = new JPanel();
      
      // set layout
      mTabAreaPanel.setLayout(new PositionLayout(mTabAreaPanel, 500, 500));
      
      // tab area panel is transparent
      mTabAreaPanel.setOpaque(false);
      
      // add transparent filler
      JComponent filler = (JComponent)Box.createGlue();
      filler.setOpaque(false);
      mTabAreaPanel.add(filler);
      
      // create tab area scroll pane
      mTabAreaScrollPane = new JScrollPane(mTabAreaPanel);
      mTabAreaScrollPane.setOpaque(false);
      mTabAreaScrollPane.getViewport().setOpaque(false);
      mTabAreaScrollPane.setAutoscrolls(true);
      mTabAreaScrollPane.getViewport().setScrollMode(
            JViewport.BLIT_SCROLL_MODE);//.SIMPLE_SCROLL_MODE);
      
      // draw no scroll bars
      mTabAreaScrollPane.setHorizontalScrollBarPolicy(
            JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
      mTabAreaScrollPane.setVerticalScrollBarPolicy(
            JScrollPane.VERTICAL_SCROLLBAR_NEVER);
      
      // no border
      mTabAreaScrollPane.setBorder(null);
      
      // add listeners
      mTabAreaScrollPane.addMouseWheelListener(getTabPanelHandler());

      // set default max tab area height
      mMaxTabAreaHeight = 0;
   }
   
   /**
    * Sets up the tab content panel.
    */
   protected void setupTabContentPanel()
   {
      mTabContentPanel = new JPanel();
      mTabContentPanel.setBackground(getSelectedColor());
      
      CardLayout cl = new CardLayout();
      mTabContentPanel.setLayout(cl);

      // create no selection content
      mNoSelectionContent = new JPanel();
      mNoSelectionContent.setBackground(getSelectedColor());
      
      // create a parent container to put the content component in
      Container parent = createTabContentParent(mNoSelectionContent);
      
      // add the tab content component to the parent
      parent.add(mNoSelectionContent);
      
      // get an id for the parent
      String id = "tabcontent_none";

      // update hash tables
      mIdToContentParent.put(id, parent);
      mContentParentToId.put(parent, id);
      mContentParentToContent.put(parent, mNoSelectionContent);
      
      // add the content parent panel
      cl.addLayoutComponent(parent, id);
      mTabContentPanel.add(parent, id, -1);
      
      // set the selection to no selection
      mSelectedContent = mNoSelectionContent;
      
      // show no selection content
      cl.show(mTabContentPanel, id);
   }
   
   /**
    * Gets the tab area panel.
    *
    * @return the tab area panel.
    */
   protected JPanel getTabAreaPanel()
   {
      return mTabAreaPanel;
   }
   
   /**
    * Gets an iterator over all of the tab areas.
    * 
    * @return an iterator over all of the tab areas.
    */
   protected Iterator getTabAreaIterator()
   {
      return mTabAreaParentToTabArea.values().iterator();      
   }

   /**
    * Gets the tab content panel.
    *
    * @return the tab content panel.
    */
   protected JPanel getTabContentPanel()
   {
      return mTabContentPanel;
   }
   
   /**
    * Gets an iterator over all of the tab content components.
    * 
    * @return an iterator over all of the tab content components.
    */
   protected Iterator getTabContentIterator()
   {
      return mContentParentToContent.values().iterator();     
   }
   
   /**
    * Gets the tab area viewport.
    * 
    * @return the tab area viewport.
    */
   protected JViewport getTabAreaViewport()
   {
      return mTabAreaScrollPane.getViewport();
   }
   
   /**
    * Gets the tab area viewport view position.
    * 
    * @return the tab area viewport view position.
    */
   protected Point getTabAreaViewPosition()
   {
      return getTabAreaViewport().getViewPosition();      
   }
   
   /**
    * Checks a tab index.
    *
    * @param index the tab index.
    * @return true if the tab index is valid, false if not.
    */
   protected boolean checkIndex(int index)
   {
      boolean rval = false;
      
      if(index >= 0 && index < getTabCount())
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Gets the index of a tab content component.
    * 
    * @param content the tab content component to get the index for.
    * @return the index of the tab content or -1 if no match found.
    */
   protected int getTabContentIndex(Component content)
   {
      int index = -1;
      
      int count = getTabCount();
      for(int i = 0; i < count; i++)
      {
         Component parent = getTabContentPanel().getComponent(i);
         Component c = getTabContentFromParent(parent);
         if(content == c)
         {
            index = i;
            break;
         }
      }
      
      return index;
   }
   
   /**
    * Gets a tab content component from its parent.
    * 
    * @param parent the parent of the tab content.
    * @return the tab content, if any.
    */
   protected Component getTabContentFromParent(Component parent)
   {
      return (Component)mContentParentToContent.get(parent);
   }   
   
   /**
    * Gets a tab area from its parent.
    * 
    * @param parent the parent of the tab area.
    * @return the tab area, if any.
    */
   protected Component getTabAreaFromParent(Component parent)
   {
      return (Component)mTabAreaParentToTabArea.get(parent);
   }
   
   /**
    * Gets the tab area for any child of a tab area.
    * 
    * @param c the component to get the tab area of, if one exists.
    * @return the tab area, if one exists, else null. 
    */
   protected Component getTabAreaFromChild(Component c)
   {
      // This code will run very slowly for very nested child components
      // but these cases should be few -- in any case, we should probably
      // try to speed up this algorithm.
      Component tabArea = null;
      
      if(c != null)
      {
         // get a set of all tab area parents
         Set parents = mTabAreaParentToTabArea.keySet();
         
         // get base parent
         boolean found = false;
         while(!found && c.getParent() != null && c != c.getParent())
         {
            c = c.getParent();

            // see if the component is in the set
            found = parents.contains(c);
         }
         
         // should have the parent now
         tabArea = getTabAreaFromParent(c);
      }
      
      return tabArea;
   }
   
   /**
    * Gets the leading tab area index.
    * 
    * @return the leading tab area index.
    */
   protected int getLeadingTabAreaIndex()
   {
      return getTabAreaIndex(getLeadingTabArea());
   }
   
   /**
    * Gets a unique id for a content component.
    *
    * @return the unique id for a content component.
    */
   protected String generateTabContentId()
   {
      String id = "tabcontent_";
      
      int i = 0;
      while(mIdToContentParent.get(id + i) != null)
      {
         i++;
      }
      
      id += i;
      
      return id;
   }
   
   /**
    * Gets the content parent for a given id.
    * 
    * @param id the id to get the content parent for.
    * @return the content parent or null, if there is no match.
    */
   protected Component getContentParent(String id)
   {
      return (Component)mIdToContentParent.get(id);
   }

   /**
    * Gets the id of a content parent.
    * 
    * @param content the content to get the parent id for.
    * @return the id of the a content parent or null, if there is none.
    */
   protected String getContentParentId(Component content)
   {
      String id = null;
      
      if(content != null)
      {
         Component parent = content.getParent();
         if(parent != null)
         {
            id = (String)mContentParentToId.get(parent);
         }
      }
      
      return id;
   }
   
   /**
    * Gets the card layout for the tab content panel.
    * 
    * @return the card layout for the tab content panel.
    */
   protected CardLayout getTabContentPanelLayout()
   {
      CardLayout cl = (CardLayout)getTabContentPanel().getLayout();
      return cl;
   }
   
   /**
    * Updates the constraints for a particular tab area.
    * 
    * This method can be overridden to set the constraints for specialized
    * tab areas.
    * 
    * @param tabArea the tab area to update.
    */
   protected void updateTabAreaConstraints(Component tabArea)
   {
   }

   /**
    * Updates the constraints for a particular tab area parent.
    * 
    * This method can be overridden to set the constraints for specialized
    * tab area parents.
    * 
    * @param parent the tab area parent to update.
    * @param layout the layout for the tab area panel.
    * @param constraints the constraints for the parent.
    * @param prevConstraints the constraints of the previous tab area parent.
    * @param insets the tab area insets for this panel.
    */
   protected void updateTabAreaParentConstraints(
      Component parent, PositionLayout layout,
      PositionConstraints constraints, PositionConstraints prevConstraints,
      Insets insets)
   {
      Component tabArea = getTabAreaFromParent(parent);
      if(tabArea != null)
      {
         // update tab area constraints
         updateTabAreaConstraints(tabArea);
         
         // update location
         constraints.location.x = prevConstraints.getRight();
         
         // update width
         constraints.size.width =
            tabArea.getPreferredSize().width + insets.left + insets.right;
         
         // update height
         constraints.size.height =
            mMaxTabAreaHeight + insets.top + insets.bottom;
         
         // set new constraints
         layout.setConstraints(parent, constraints);
         
         // update previous constraints
         prevConstraints = constraints;
      }
   }
   
   /**
    * Updates the constraints for the components in this panel. 
    */
   protected void updateComponentConstraints()
   {
      // get tree lock while updating constraints
      synchronized(getTreeLock())
      {
         // get tab area insets
         Insets insets = getTabAreaInsets();
         
         // get layout for tab area panel
         PositionLayout layout =
            (PositionLayout)getTabAreaPanel().getLayout();
         PositionConstraints constraints;

         // set constraints for tab area parents
         PositionConstraints prevConstraints = new PositionConstraints();
         int count = getTabAreaPanel().getComponentCount();
         for(int i = 0; i < count; i++)
         {
            Component parent = getTabAreaPanel().getComponent(i);

            // get the parent's constraints
            constraints = layout.getConstraints(parent);
            
            // update the parent's constraints
            updateTabAreaParentConstraints(
               parent, layout, constraints, prevConstraints, insets);
            
            // set previous constraints
            prevConstraints = constraints;
         }
         
         // get position layout for main panel
         layout = (PositionLayout)getLayout();
         
         // update tab area scroll pane constraints
         constraints = layout.getConstraints(mTabAreaScrollPane);
         
         // update height
         int newHeight = mMaxTabAreaHeight + insets.top + insets.bottom; 
         int diff = constraints.size.height - newHeight;
         constraints.size.height = newHeight;
            
         // set new constraints
         layout.setConstraints(mTabAreaScrollPane, constraints);
         
         // update left scroll button constraints
         constraints = layout.getConstraints(mLeftScrollButton);
         constraints.size.height = newHeight;
         
         // update right scroll button constraints
         constraints = layout.getConstraints(mRightScrollButton);
         constraints.size.height = newHeight;
         
         // update tab content panel constraints
         constraints = layout.getConstraints(getTabContentPanel());
         constraints.location.y -= diff;
         constraints.size.height += diff;
         layout.setConstraints(getTabContentPanel(), constraints);
      }
   }
   
   /**
    * Adds listeners to a component and all of its children.
    * 
    * @param c the component to add listeners to.
    */
   protected void addListenersToComponent(Component c)
   {
      // add listeners to component and all its children
      c.addMouseListener(getTabPanelHandler());
      c.addMouseMotionListener(getTabPanelHandler());
      
      if(c instanceof Container)
      {
         Container container = (Container)c;
         int count = container.getComponentCount();
         for(int i = 0; i < count; i++)
         {
            Component child = container.getComponent(i);
            addListenersToComponent(child);
         }
      }
   }
   
   /**
    * Removes listeners from a component and all of its children.
    * 
    * @param c the component to remove listeners from.
    */
   protected void removeListenersFromComponent(Component c)
   {
      // remove listeners from component and all of its children
      c.removeMouseListener(getTabPanelHandler());
      c.removeMouseMotionListener(getTabPanelHandler());
      
      if(c instanceof Container)
      {
         Container container = (Container)c;
         int count = container.getComponentCount();
         for(int i = 0; i < count; i++)
         {
            Component child = container.getComponent(i);
            removeListenersFromComponent(child);
         }
      }
   }
   
   /**
    * Creates a new tab area border. Overload this class to
    * change the borders on tab areas.
    * 
    * @return the new tab area border.
    */
   protected Border createTabAreaParentBorder()
   {
      // set a border to be painted later
      Insets insets = getTabAreaInsets();
      Border b = new TabAreaParentBorder(insets);

      return b;
   }
   
   /**
    * Creates a new tab area parent container.
    * 
    * @param tabArea the tab area to create the parent for.
    * @return the new tab area parent container.
    */
   protected Container createTabAreaParent(Component tabArea)
   {
      // create a parent to put the tab area in
      JPanel parent = new JPanel();
      parent.setLayout(new GridLayout(1, 1));
      parent.setBorder(createTabAreaParentBorder());
      
      return parent;
   }
   
   /**
    * Adds a tab area component at a given index.
    * 
    * @param tabArea the tab area component to add.
    * @param index the index to add the tab area at.
    * @return true if the tab area was added, false if not.
    */
   protected synchronized boolean addTabArea(Component tabArea, int index)
   {
      boolean rval = false;

      // ensure index is valid, before filler
      int count = getTabAreaPanel().getComponentCount();
      if(index == count)
      {
         index = -1;
      }
      
      if(index >= count || index <= -1)
      {
         index = count - 1;
      }
      
      // do not add null tab areas
      if(tabArea != null)
      {
         rval = true;
         
         // create a parent to put the tab area in
         Container parent = createTabAreaParent(tabArea);
         
         // set the background
         parent.setBackground(getUnselectedColor());
         
         // add the tab area component to the parent
         parent.add(tabArea);
         
         // update max tab area height as necessary
         if(tabArea.getPreferredSize().height > mMaxTabAreaHeight)
         {
            mMaxTabAreaHeight = tabArea.getPreferredSize().height;
         }
         
         // add the panel for the tab area component
         getTabAreaPanel().add(parent, new PositionConstraints(), index);
         
         // add map entry
         mTabAreaParentToTabArea.put(parent, tabArea);
         
         // add listeners
         parent.addMouseListener(getTabPanelHandler());
         addListenersToComponent(tabArea);
      }
      
      return rval;       
   }

   /**
    * Adds a tab area component.
    * 
    * @param tabArea the tab area component to add.
    * @return true if the tab area was added, false if not.
    */
   protected boolean addTabArea(Component tabArea)
   {
      boolean rval = false;
      
      // add tab area panel (-1 means append)
      rval = addTabArea(tabArea, -1);
      
      return rval;
   }
   
   /**
    * Removes a tab area component.
    * 
    * @param tabArea the tab area component to remove.
    * @return true if the tab area was removed, false if not.
    */
   protected synchronized boolean removeTabArea(Component tabArea)
   {
      boolean rval = false;
      
      // get the parent for the tab area
      Component parent = tabArea.getParent();
      if(parent != null)
      {
         rval = true;
         
         // remove listeners
         parent.removeMouseListener(getTabPanelHandler());
         removeListenersFromComponent(tabArea);
         
         Component content = getTabContent(tabArea);
         
         // remove map entries
         mTabAreaParentToTabArea.remove(parent);
         mAreaToContent.remove(tabArea);
         mContentToArea.remove(content);
         
         TabAreaFlashThread taft = getTabAreaFlashThread(content);
         if(taft != null)
         {
            taft.stopFlashing();
            mFlashThreads.remove(content);
         }
         
         // remove the tab area component parent
         getTabAreaPanel().remove(parent);
         
         // recalculate max tab area height
         mMaxTabAreaHeight = 0;
         int count = getTabAreaPanel().getComponentCount();
         for(int i = 0; i < count; i++)
         {
            Component tempParent = getTabAreaPanel().getComponent(i);
            Component tempTabArea = getTabAreaFromParent(tempParent);
            if(tempTabArea != null)
            {
               if(tempTabArea.getPreferredSize().height > mMaxTabAreaHeight)
               {
                  mMaxTabAreaHeight = tempTabArea.getPreferredSize().height;
               }
            }
         }
         
         // update component constraints
         updateComponentConstraints();
      }
      
      return rval;
   }
   
   /**
    * Creates a new tab content border. Overload this class to
    * change the borders on tab contents.
    * 
    * @return the new tab content border.
    */
   protected Border createTabContentParentBorder()
   {
      // set a border to be painted later
      Insets insets = getTabContentInsets();
      Border b = new TabContentParentBorder(insets);

      return b;
   }
   
   /**
    * Creates a new tab content parent container.
    * 
    * @param content the tab content to create the parent for.
    * @return the new tab content parent container.
    */
   protected Container createTabContentParent(Component content)
   {
      // create a parent to put the tab content in
      JPanel parent = new JPanel();
      parent.setOpaque(false);
      parent.setLayout(new GridLayout(1, 1));
      parent.setBorder(createTabContentParentBorder());
      
      return parent;
   }   
   
   /**
    * Adds a tab content component at the given index.
    * 
    * @param content the tab content component to add.
    * @param index the index to add the tab content component at.
    * @return true if the tab content was added, false if not.
    */
   protected synchronized boolean addTabContent(Component content, int index)
   {
      boolean rval = false;
      
      // ensure index is valid, before filler (no selection content)
      int count = getTabContentPanel().getComponentCount();
      if(index == count)
      {
         index = -1;
      }
      
      if(index >= count || index <= -1)
      {
         index = count - 1;
      }
      
      // if this tab has already been added, do not re-add it
      String id = getContentParentId(content);
      if(id == null)
      {
         rval = true;
            
         // create a parent container to put the tab content component in
         Container parent = createTabContentParent(content);
         
         // add the tab content component to the parent
         parent.add(content);
         
         // get an id for the parent
         id = generateTabContentId();

         // update hash tables
         mIdToContentParent.put(id, parent);
         mContentParentToId.put(parent, id);
         mContentParentToContent.put(parent, content);
         
         // add the content parent panel
         CardLayout cl = getTabContentPanelLayout();
         cl.addLayoutComponent(parent, id);
         getTabContentPanel().add(parent, id, index);
      }
      
      return rval;
   }
   
   /**
    * Adds a tab content component.
    * 
    * @param content the tab content component to add.
    * @return true if the tab content was added, false if not.
    */
   protected synchronized boolean addTabContent(Component content)
   {
      boolean rval = false;
      
      // add tab content (-1 means append)
      rval = addTabContent(content, -1);
      
      return rval;
   }
   
   /**
    * Removes a tab content component.
    * 
    * @param content the tab content component to remove.
    * @return true if the tab content was removed, false if not.
    */
   protected synchronized boolean removeTabContent(Component content)
   {
      boolean rval = false;
      
      // get the content parent's id
      String id = getContentParentId(content);
      if(id != null)
      {
         rval = true;
         
         // get the content component's parent
         Component parent = content.getParent();
         
         // clean up hash tables
         mIdToContentParent.remove(id);
         mContentParentToId.remove(parent);
         mContentParentToContent.remove(parent);
         
         // remove content
         CardLayout cl = getTabContentPanelLayout();
         cl.removeLayoutComponent(parent);
         getTabContentPanel().remove(parent);
         
         // remove the tab content from the selected tabs list
         removeTabFromSelectedTabOrder(content);
      }
      
      return rval;
   }
   
   /**
    * Updates the selected tab content order.
    * 
    * @param content the latest tab content that was selected.
    */
   protected synchronized void updateSelectedTabOrder(Component content)
   {
      if(content != null)
      {
         // remove the passed tab from the selected tabs list
         removeTabFromSelectedTabOrder(content);
         
         // append the selected tab to the selected tabs list
         mSelectedTabs.add(content);
      }
   }
   
   /**
    * Removes a tab from the selected tab content ordered list.
    * 
    * @param content the tab content to remove from the ordered list.
    */
   protected synchronized void removeTabFromSelectedTabOrder(Component content)
   {
      Iterator i = mSelectedTabs.iterator();
      while(i.hasNext())
      {
         Component c = (Component)i.next();
         
         // remove tab content from list if it matches the passed content
         if(c == content)
         {
            i.remove();
         }
      }
   }
   
   /**
    * Gets the next tab to select based on the current tab selection policy.
    * 
    * @param index the old selected tab index.
    * @return the next tab to select.
    */
   protected Component getNextTabSelection(int index)
   {
      Component rval = null;
      
      // get the next selection based on the tab selection policy
      if(getTabSelectionPolicy() != NO_TAB_SELECTION_POLICY)
      {
         if(getTabSelectionPolicy() == SELECT_NEXT_TAB_POLICY)
         {
            // get the next tab
            Component tabArea = getTabArea(index);
            if(tabArea != null)
            {
               rval = getTabContent(tabArea);
            }
            else
            {
               // get the previous tab
               tabArea = getTabArea(index - 1);
               if(tabArea != null)
               {
                  rval = getTabContent(tabArea);
               }
            }
         }
         else
         {
            if(getTabSelectionPolicy() == SELECT_PREVIOUSLY_SELECTED_TAB_POLICY)
            {
               // get the last selected tab in the selected tabs list
               if(mSelectedTabs.size() > 0)
               {
                  rval = (Component)mSelectedTabs.lastElement();
               }
            }
            
            if(rval == null ||
               getTabSelectionPolicy() == SELECT_PREVIOUS_TAB_POLICY)
            {
               // get the previous tab
               Component tabArea = getTabArea(index - 1);
               if(tabArea != null)
               {
                  rval = getTabContent(tabArea);
               }
               else
               {
                  // get the next tab
                  tabArea = getTabArea(index);
                  if(tabArea != null)
                  {
                     rval = getTabContent(tabArea);
                  }
               }
            }
         }
      }
      
      return rval;
   }
   
   /**
    * Selects a tab content component.
    * 
    * @param content the tab content component to select.
    * @return true if the content is selected, false if not.
    */
   protected synchronized boolean selectTabContent(Component content)
   {
      boolean rval = false;
      
      String id = null;

      // if no selection, user "no selection" content id
      if(content == null)
      {
         rval = true;
         id = getContentParentId(mNoSelectionContent);
      }
      else
      {
         id = getContentParentId(content);
      }
      
      if(id != null)
      {
         // get the old selection
         Component oldSelected = getSelected();
         
         rval = true;

         // save the new selection
         mSelectedContent = content;
            
         // update selected tab content order
         updateSelectedTabOrder(content);
            
         // set the old selected back to unselected color
         if(oldSelected != null)
         {
            Component tabArea = getTabArea(oldSelected);
            if(tabArea != null)
            {
               tabArea.getParent().setBackground(getUnselectedColor());
            }
         }
            
         // set the new selection to the selected color
         Component tabArea = getSelectedTabArea();
         if(tabArea != null)
         {
            tabArea.getParent().setBackground(getSelectedColor());
         }
            
         // show the new selection
         CardLayout cl = getTabContentPanelLayout();
         cl.show(getTabContentPanel(), id);
      }
      
      return rval;
   }
   
   /**
    * Gets the selected tab area.
    * 
    * @return the selected tab area, if any, else return null.
    */
   protected Component getSelectedTabArea()
   {
      Component selectedTabArea = null;
      
      // get the selected tab area
      Component content = getSelected();
      selectedTabArea = getTabArea(content);
      
      return selectedTabArea;
   }
   
   /**
    * Gets the last tab area.
    * 
    * @return the last tab area or null if there is none.
    */
   protected Component getLastTabArea()
   {
      return getTabArea(getTabCount() - 1);
   }
   
   /**
    * Scrolls to a tab area.
    * 
    * @param tabArea the tabarea to scroll to.
    * @return true if scrolled, false if not.
    */
   protected boolean scrollToTabArea(Component tabArea)
   {
      boolean rval = false;
      
      if(tabArea != null)
      {
         rval = true;
         
         // ensure this tab panel is valid
         ensureValid();
         
         // scroll the viewport to the parent panel of the tab area
         Component parent = tabArea.getParent();
         getTabAreaViewport().setViewPosition(parent.getLocation());
         
         // set the leading tab area
         mLeadingTabArea = tabArea;
         
         // revalidate and repaint
         invalidate();
         validate();
         repaint();
      }
      
      return rval;
   }
   
   /**
    * Scrolls until a tab area is visible.
    * 
    * @param tabArea the tabarea to scroll to.
    * @return true if scrolled, false if not.
    */
   protected boolean scrollToVisibleTabArea(Component tabArea)
   {
      boolean rval = false;
      
      if(tabArea != null)
      {
         // ensure this tab panel is valid
         ensureValid();
         
         // get the view
         Rectangle view = getTabAreaViewport().getViewRect();
         
         // get the tab area's parent and rectangle
         Component parent = tabArea.getParent();
         Rectangle pRect = parent.getBounds();
         
         // only scroll if the parent isn't visible
         if(pRect.x < view.x || (pRect.x + pRect.width) > (view.x + view.width))
         {
            rval = true;
            
            if(pRect.x < view.x)
            {
               // set the view position to the tab area parent
               getTabAreaViewport().setViewPosition(parent.getLocation());
            }
            else
            {
               // translate parent by view
               pRect.x -= view.x;
               
               // set the view position to the tab area parent
               getTabAreaViewport().scrollRectToVisible(pRect);
            }
            
            // determine and set the leading tab area
            parent = getTabAreaPanel().getComponentAt(getTabAreaViewPosition());
            Component c = getTabAreaFromParent(parent);
            if(c != null)
            {
               int x = getTabAreaViewPosition().x;
               if(parent.getX() < x)
               {
                  int index = getTabAreaIndex(c);
                  c = getTabArea(index + 1);
               }
               
               if(c != null)
               {
                  mLeadingTabArea = c;
               }
            }
         }
      }
      
      return rval;
   }   
   
   /**
    * Gets the tab area insets.
    * 
    * @return the tab area insets.
    */
   protected Insets getTabAreaInsets()
   {
      return mTabAreaInsets;
   }
   
   /**
    * Gets the tab content insets.
    * 
    * @return the tab content insets.
    */
   protected Insets getTabContentInsets()
   {
      return mTabContentInsets;
   }
   
   /**
    * Moves a tab to a new index.
    * 
    * @param content the tab content component to move.
    * @param index the index to move it to.
    * @return true if moved successfully, false if not. 
    */
   protected boolean moveTab(Component content, int index)
   {
      boolean rval = false;
      
      // ensure index is valid
      int count = getTabCount();
      if(index >= count || index < -1)
      {
         index = -1;
      }
      
      // don't move null content
      if(content != null)
      {
         // save the tab area and tab area index
         Component tabArea = getTabArea(content);
         int oldIndex = getTabAreaIndex(tabArea);
         
         // don't move content if indices are the same or old index is invalid
         if(index != oldIndex && oldIndex != -1)
         {
            rval = true;

            // get the flash thread, if there is one
            TabAreaFlashThread taft = getTabAreaFlashThread(content);
            boolean flash =
               (taft != null && taft.isFlashing() && !taft.stopWhenSelected());
            
            // remove the content
            removeTabAreaAndContent(content);

            // modify insertion index if it was effected by removal
            if(index > (oldIndex + 1))
            {
               index--;
            }
         
            // add the content at the new index
            addTabAreaAndContent(tabArea, content, index);
            
            // start flashing tab again, if appropriate
            if(flash)
            {
               startFlashingTab(content, taft.getFlashColor(),
                                taft.getFlashInterval(),
                                taft.stopWhenSelected());
            }
         }
      }
      
      return rval;
   }
   
   /**
    * Swaps two tabs.
    * 
    * @param content1 the content of the first tab.
    * @param content2 the content of the second tab.
    * @return true if the tabs were swapped, false if not.
    */
   protected boolean swapTabs(Component content1, Component content2)
   {
      boolean rval = false;
      
      // don't swap the same tab or null contents
      if(content1 != content2 && content1 != null && content2 != null)
      {
         rval = true;
         
         // save the first tab area and tab area index
         Component tabArea1 = getTabArea(content1);
         int index1 = getTabAreaIndex(tabArea1);
         
         // remove the first content
         removeTabAreaAndContent(content1);
         
         // get the tab area and index of the second content
         Component tabArea2 = getTabArea(content2);
         int index2 = getTabAreaIndex(tabArea2);

         // add the first content in place of the second content
         addTabAreaAndContent(tabArea1, content1, index2);
         
         // remove the second content
         removeTabAreaAndContent(content2);
         
         // add the second content in place of the first content
         addTabAreaAndContent(tabArea2, content2, index1);
      }
      
      return rval;
   }
   
   /**
    * Adds new tab area and tab content combo to the tab panel.
    * 
    * @param tabArea the component to display in the tab area. 
    * @param content the component to display in the content area.
    * @param index the index to add the tab area and tab content.
    * @return true if the tab area and tab content were added. 
    */
   protected boolean addTabAreaAndContent(Component tabArea, Component content,
                                          int index)
   {
      boolean rval = false;
      
      // if this tab has already been added, do not re-add it
      String id = getContentParentId(content);
      if(id == null)
      {
         // add the tab area component
         if(addTabArea(tabArea, index))
         {
            // add the tab content component
            rval = addTabContent(content, index);
            
            // map the tab area to the content and vice versa
            mAreaToContent.put(tabArea, content);
            mContentToArea.put(content, tabArea);
            
            // set leading tab area if this is the first tab
            if(getTabCount() == 1)
            {
               mLeadingTabArea = tabArea;
            }
         }
      }
      
      return rval;
   }
   
   /**
    * Removes a tab area and tab content combo from the tab panel.
    * 
    * @param content the tab content component to remove (the associated
    *                tab area will be looked up).
    * @return true if the tab area and content were removed, false if not.
    */
   protected boolean removeTabAreaAndContent(Component content)
   {
      boolean rval = false;
      
      if(content != null)
      {
         // get the tab area
         Component tabArea = getTabArea(content);

         // remove the tab area component
         if(removeTabArea(tabArea))
         {
            // remove the content
            rval = removeTabContent(content);
         }
      }
      
      return rval;
   }
   
   /**
    * Gets the tab area that the mouse has entered.
    * 
    * @return the tab area that the mouse has entered, null if none.
    */
   protected Component getMouseEnteredTabArea()
   {
      return mMouseEnteredTabArea;
   }
   
   /**
    * Overridden to update component constraints.
    *  
    * Validates this container and all of its subcomponents.
    * 
    * The <code>validate</code> method is used to cause a container
    * to lay out its subcomponents again. It should be invoked when
    * this container's subcomponents are modified (added to or
    * removed from the container, or layout-related information
    * changed) after the container has been displayed.
    */   
   public void validate()
   {
      // update component constraints
      updateComponentConstraints();
      
      // validate
      super.validate();
   }
   
   /**
    * Gets the tab area flash thread for a tab.
    * 
    * @param content the content of the tab to get the flash thread for.
    * @return the tab area flash thread, or null if one does not exist.
    */
   public TabAreaFlashThread getTabAreaFlashThread(Component content)
   {
      return (TabAreaFlashThread)mFlashThreads.get(content);      
   }
   
   /**
    * Adds a tab to the tab panel.
    * 
    * @param tabArea the component to display in the tab area. 
    * @param content the component to display in the content area.
    */
   public void addTab(Component tabArea, Component content)
   {
      // add the tab (-1 means append)
      addTab(tabArea, content, -1);
   }
   
   /**
    * Adds a tab to the tab panel at a specified index. An index
    * of -1 will append the tab.
    * 
    * @param tabArea the component to display in the tab area. 
    * @param content the component to display in the content area.
    * @param index the index to add the tab at. 
    */
   public void addTab(Component tabArea, Component content, int index)
   {
      // add tab area and content
      if(addTabAreaAndContent(tabArea, content, index))
      {
         // revalidate, repaint
         invalidate();
         validate();
         repaint();
         
         // fire added event
         fireTabAdded(content);
      }
   }   
   
   /**
    * Removes a tab from the tab panel.
    * 
    * @param content the content component that was displayed in the tab.
    */
   public void removeTab(Component content)
   {
      // get the tab content index
      int index = getTabContentIndex(content);
      
      if(removeTabAreaAndContent(content))
      {
         // set selection if this tab was the selected one
         Component selected = getSelected();
         if(selected == content)
         {
            // set the new selection based on the tab selection policy
            setSelected(getNextTabSelection(index));
         }
         
         // revalidate, repaint
         invalidate();
         validate();
         repaint();
         
         // fire removed event
         fireTabRemoved(content);
      }
   }
   
   /**
    * Sets the tab area for an existing tab.
    * 
    * @param content the tab content of the tab to change.  
    * @param tabArea the new tab area.
    */
   public synchronized void setTabArea(Component content, Component tabArea)
   {
      // get the old tab area
      Component oldTabArea = getTabArea(content);
      if(oldTabArea != null)
      {
         // save current selection
         boolean isSelected = (getSelectedTabArea() == oldTabArea);

         // get the old tab area index
         int index = getTabAreaIndex(oldTabArea);
         
         // save background
         Color background = oldTabArea.getParent().getBackground();
         
         // remove the old tab area
         removeTabArea(oldTabArea);
         
         // add the new tab area in its place
         addTabArea(tabArea, index);
         
         // map the tab area to the content and vice versa
         mAreaToContent.put(tabArea, content);
         mContentToArea.put(content, tabArea);

         // set background
         tabArea.getParent().setBackground(background);
         
         // reselect tab area if appropriate
         if(isSelected)
         {
            selectTabContent(content);
         }
         
         // revalidate, repaint
         invalidate();
         validate();
         repaint();
      }
   }
   
   /**
    * Gets the tab area component for a given tab content component.
    * 
    * @param content the content component.
    * @return the tab area component, or null if there is none.
    */
   public Component getTabArea(Component content)
   {
      return (Component)mContentToArea.get(content);
   }
   
   /**
    * Sets the tab area for an existing tab.
    * 
    * @param index the index of the tab content of the tab to change.  
    * @param tabArea the new tab area.
    */
   public synchronized void setTabArea(int index, Component tabArea)   
   {
      setTabArea(getTabArea(index), tabArea);
   }
   
   /**
    * Gets a tab area based on its index.
    * 
    * @param index the index of the tab area.
    * @return the tab area for the index or null if no match is found.
    */
   public Component getTabArea(int index)
   {
      Component tabArea = null;
      
      int count = getTabAreaPanel().getComponentCount();
      if(index >= 0 && index < count)
      {
         Component parent = getTabAreaPanel().getComponent(index);
         tabArea = getTabAreaFromParent(parent);
      }
      
      return tabArea;
   }
   
   /**
    * Gets the index of a tab area.
    * 
    * @param tabArea the tab area to get the index for.
    * @return the index of the tab area or -1 if no match found.
    */
   public int getTabAreaIndex(Component tabArea)
   {
      int index = -1;
      
      int count = getTabCount();
      for(int i = 0; i < count; i++)
      {
         Component parent = getTabAreaPanel().getComponent(i);
         Component c = getTabAreaFromParent(parent);
         if(tabArea == c)
         {
            index = i;
            break;
         }
      }
      
      return index;
   }
   
   /**
    * Gets the leading tab area. That is, the tab area that is first
    * in the scrolling tab area viewport.
    * 
    * @return the leading tab area.
    */
   public Component getLeadingTabArea() 
   {
      return mLeadingTabArea;
   }

   /**
    * Sets the tab content for an existing tab.
    * 
    * @param oldContent the old content for the tab.
    * @param newContent the new content for the tab.
    */
   public synchronized void setTabContent(
      Component oldContent, Component newContent)
   {
      // ensure we aren't just replacing with the same tab content
      if(oldContent != newContent)
      {
         // get the old tab index and tab area
         int index = getTabContentIndex(oldContent);
         Component tabArea = getTabArea(oldContent);
         if(index != -1 && tabArea != null)
         {
            // remove the old tab content
            removeTabContent(oldContent);
            
            // add the new tab content in its place
            addTabContent(newContent, index);
            
            // remove the old content map entry
            mContentToArea.remove(oldContent);
            
            // map the tab area to the new content and vice versa
            mAreaToContent.put(tabArea, newContent);
            mContentToArea.put(newContent, tabArea);
            
            // set selection if appropriate
            if(getSelected() == oldContent)
            {
               selectTabContent(newContent);
            }
            
            // revalidate, repaint
            invalidate();
            validate();
            repaint();
         }
      }      
   }
   
   /**
    * Gets the tab content for a given tab area.
    * 
    * @param tabArea the tab area to get the tab content for.
    * @return the tab content, or null, if no match is found.
    */
   public Component getTabContent(Component tabArea)
   {
      return (Component)mAreaToContent.get(tabArea);
   }
   
   /**
    * Sets the tab content for an existing tab.
    * 
    * @param index the index for the tab content.
    * @param newContent the new content for the tab.
    */
   public synchronized void setTabContent(
      int index, Component newContent)   
   {
      setTabContent(getTabContent(index), newContent);
   }
   
   /**
    * Gets a tab content component based on its index.
    * 
    * @param index the index of the tab content.
    * @return the tab content for the index or null if no match is found.
    */
   public Component getTabContent(int index)
   {
      Component content = null;
      
      int count = getTabContentPanel().getComponentCount();
      if(index >= 0 && index < count)
      {
         Component parent = getTabContentPanel().getComponent(index);
         content = getTabContentFromParent(parent);
      }
      
      return content;
   }
   
   /**
    * Adds a tab panel listener.
    * 
    * @param tpl the tab panel listener to add.
    */
   public void addTabPanelListener(TabPanelListener tpl)
   {
      mTabPanelListeners.add(tpl);
   }
   
   /**
    * Removes a tab panel listener.
    * 
    * @param tpl the tab panel listener to remove.
    */
   public void removeTabPanelListener(TabPanelListener tpl)
   {
      mTabPanelListeners.remove(tpl);
   }
   
   /**
    * Sets the selected content component.
    * 
    * @param content the content component to set as selected.
    */
   public void setSelected(Component content)
   {
      // get the old selection
      Component oldSelection = getSelected();
      
      if(selectTabContent(content))
      {
         // scroll to selection
         scrollToVisibleContent(content);
         
         // fire selected event
         if(oldSelection != content)
         {
            fireTabSelectionChanged(oldSelection, content);
         }
      }
      else if(oldSelection == content)
      {
         // scroll to selection
         scrollToVisibleContent(content);
      }
      
      invalidate();
      validate();
      repaint();
   }
   
   /**
    * Sets the selected index.
    * 
    * @param index the index of the content component to select.
    */
   public void setSelectedIndex(int index)
   {
      setSelected(getTabContent(index));
   }
   
   /**
    * Gets the selected content component.
    * 
    * @return the selected content component, or null, if none selected.
    */
   public Component getSelected()
   {
      return mSelectedContent;
   }
   
   /**
    * Gets the selected content component index.
    * 
    * @return the selected content component index or -1 if no selection.
    */
   public int getSelectedIndex()
   {
      return getIndex(getSelected());
   }
   
   /**
    * Returns true if the passed tab content component is selected,
    * false if not.
    * 
    * @param content the tab content component to check for selection.
    * @return true if the content is selected, false if not.
    */
   public boolean isSelected(Component content)
   {
      boolean rval = false;
      
      if(content == getSelected())
      {
         rval = true;
      }
      
      return rval;
   }   
   
   /**
    * Gets the index of a content component.
    * 
    * @param content the content component to get the index of.
    * @return the index of the content component or -1 if no match found.
    */
   public int getIndex(Component content)
   {
      int index = -1;
      
      if(content != null)
      {
         int count = getTabContentPanel().getComponentCount();
         for(int i = 0; i < count; i++)
         {
            Component parent = getTabContentPanel().getComponent(i);
            if(parent instanceof JPanel)
            {
               JPanel panel = (JPanel)parent;
               Component c = panel.getComponent(0);
               if(c == content)
               {
                  index = i;
                  break;
               }
            }
         }
      }
      
      return index;
   }
   
   /**
    * Gets the number of tabs in this tab panel.
    *
    * @return the number of tabs in the tab panel.
    */
   public int getTabCount()
   {
      return mTabContentPanel.getComponentCount() - 1;
   }
   
   /**
    * Determines if there is a tab for the passed content component.
    * 
    * @param content the content component to check for.
    * @return true if there is a tab for the content component, false if not. 
    */
   public boolean containsContent(Component content)
   {
      boolean rval = false;
      
      String id = getContentParentId(content);
      rval = (id != null);
      
      return rval;
   }
   
   /**
    * Scrolls until the passed content component is visible.
    * 
    * @param content the content component to scroll to.
    * @return true if scrolled to visible content, false if not.
    */
   public boolean scrollToVisibleContent(Component content)
   {
      boolean rval = false;
      
      if(containsContent(content))
      {
         // get the tab area
         Component tabArea = getTabArea(content);
         if(scrollToVisibleTabArea(tabArea))
         {
            // revalidate and repaint
            invalidate();
            validate();
            repaint();
         }
      }
      
      return rval;
   }
   
   /**
    * Scrolls the tab area to the left one tab.
    */
   public void scrollTabsLeft()
   {
      // get the index of the leading tab area
      int index = getLeadingTabAreaIndex();

      // index of 0 means you cannot scroll left any more
      if(index != 0)
      {
         Component tabArea = getTabArea(index - 1);
         scrollToTabArea(tabArea);
      }
   }
   
   /**
    * Scrolls the tab area to the right one tab.
    */
   public void scrollTabsRight()
   {
      // get the index of the leading tab area
      int index = getLeadingTabAreaIndex();
      
      // get the tab count
      int count = getTabCount();

      // index == (count - 1) means you cannot scroll right any more
      if(index != (count - 1))
      {
         // see if last tab is already viewable
         Component tabArea = getLastTabArea();
         if(tabArea != null)
         {
            // get parent
            Component parent = tabArea.getParent();
            
            // get view
            Rectangle view = getTabAreaViewport().getViewRect();

            // only scroll if last parent isn't visible
            if((parent.getX() + parent.getWidth()) > (view.x + view.width))
            {
               // get the next tab area
               tabArea = getTabArea(index + 1);
               scrollToTabArea(tabArea);
            }
         }
      }
   }
   
   /**
    * Sets the tab selection policy. This is the policy that is used to
    * determine which tab to select once a selected tab has been removed.
    * 
    * @param policy the tab selection policy to use.
    */
   public void setTabSelectionPolicy(int policy)
   {
      mTabSelectionPolicy = policy;
   }
   
   /**
    * Gets the tab selection policy. This is the policy that is used
    * to determine which tab to select once a selected tab has been removed.
    * 
    * @return the tab selection policy.
    */
   public int getTabSelectionPolicy()
   {
      return mTabSelectionPolicy;
   }
   
   /**
    * Returns true if the tab at the given index is currently flashing.
    * 
    * @param index the index of the tab to inspect.
    * @return true if the tab at the given index is currently flashing.
    */
   public boolean isTabFlashing(int index)
   {
      Component content = getTabContent(index);
      return isTabFlashing(content);
   }
   
   /**
    * Starts flashing the tab area of a tab with the default flash color and
    * interval.
    * 
    * @param index the index of the tab to flash.
    * @param stopWhenSelected true to stop flashing upon selection, false to
    *                         keep flashing until instructed to stop.
    */
   public void startFlashingTab(int index, boolean stopWhenSelected)
   {
      startFlashingTab(index, getFlashingColor(), getFlashInterval(),
                       stopWhenSelected);
   }
   
   /**
    * Starts flashing the tab area of a tab.
    * 
    * @param index the index of the tab to flash.
    * @param color the color to flash the tab with.
    * @param interval how often to flash the tab (in milliseconds).
    * @param stopWhenSelected true to stop flashing upon selection, false to
    *                         keep flashing until instructed to stop.
    */
   public void startFlashingTab(int index, Color color, long interval,
                                boolean stopWhenSelected)
   {
      Component content = getTabContent(index);
      startFlashingTab(content, color, interval, stopWhenSelected);
   }
   
   /**
    * Stops flashing the tab area of a tab.
    * 
    * @param index the index of the tab to stop flashing.
    */
   public void stopFlashing(int index)
   {
      // get the content
      Component content = getTabContent(index);
      stopFlashingTab(content);
   }
   
   /**
    * Returns true if the tab with the given content is currently flashing.
    * 
    * @param content the content of the tab to inspect.
    * @return true if the tab with the given content is currently flashing.
    */
   public boolean isTabFlashing(Component content)
   {
      boolean rval = false;
      
      TabAreaFlashThread taft = getTabAreaFlashThread(content);
      if(taft != null)
      {      
         rval = taft.isFlashing();
      }
      
      return rval;
   }
   
   /**
    * Starts flashing the tab area of a tab with the default flash color and
    * interval.
    * 
    * @param content the tab content of the tab to flash.
    * @param stopWhenSelected true to stop flashing upon selection, false to
    *                         keep flashing until instructed to stop.
    */
   public void startFlashingTab(Component content, boolean stopWhenSelected)
   {
      startFlashingTab(content, getFlashingColor(), getFlashInterval(),
                       stopWhenSelected);
   }

   /**
    * Starts flashing the tab area of a tab.
    * 
    * @param content the tab content of the tab to flash.
    * @param color the color to flash the tab with.
    * @param interval how often to flash the tab (in milliseconds).
    * @param stopWhenSelected true to stop flashing upon selection, false to
    *                         keep flashing until instructed to stop.
    */
   public void startFlashingTab(Component content, Color color, long interval,
                                boolean stopWhenSelected)
   {
      // make sure thread isn't already flashing
      TabAreaFlashThread taft = getTabAreaFlashThread(content);
      if(taft == null || !taft.isFlashing())
      {
         Component tabArea = getTabArea(content);
         if(tabArea != null)
         {
            taft = new TabAreaFlashThread(this, tabArea, content);
            mFlashThreads.put(content, taft);
            taft.startFlashing(color, interval, stopWhenSelected);
         }
      }
   }
   
   /**
    * Stops flashing the tab area of a tab.
    * 
    * @param content the tab content of the tab to flash.
    */
   public void stopFlashingTab(Component content)
   {
      // get the flash thread
      TabAreaFlashThread taft = getTabAreaFlashThread(content);
      if(taft != null)
      {
         // stop flashing
         taft.stopFlashing();
      }
   }
   
   /**
    * An opaque border with insets.
    * 
    * @author Dave Longley
    */
   public abstract class AbstractInsetBorder extends AbstractBorder
   {
      /**
       * Left inset.
       */
      protected int mLeft;
      
      /**
       * Right inset.
       */
      protected int mRight;
      
      /**
       * Top inset.
       */
      protected int mTop;
      
      /**
       * Bottom inset.
       */
      protected int mBottom;

      /**
       * Creates a border with the specified insets.
       * 
       * @param top the top inset of the border.
       * @param left the left inset of the border.
       * @param bottom the bottom inset of the border.
       * @param right the right inset of the border.
       */
      public AbstractInsetBorder(int top, int left, int bottom, int right)
      {
          mTop = top; 
          mRight = right;
          mBottom = bottom;
          mLeft = left;
      }

      /**
       * Creates a border with the specified insets.
       * 
       * @param borderInsets the insets of the border.
       */
      public AbstractInsetBorder(Insets borderInsets)
      {
          mTop = borderInsets.top; 
          mRight = borderInsets.right;
          mBottom = borderInsets.bottom;
          mLeft = borderInsets.left;
      }

      /**
       * Returns the insets of the border.
       * @param c the component for which this border insets value applies
       */
      public Insets getBorderInsets(Component c)
      {
         return getBorderInsets();
      }

      /** 
       * Reinitialize the insets parameter with this Border's current Insets.
       *  
       * @param c the component for which this border insets value applies.
       * @param insets the object to be reinitialized.
       */
      public Insets getBorderInsets(Component c, Insets insets)
      {
         insets.left = mLeft;
         insets.top = mTop;
         insets.right = mRight;
         insets.bottom = mBottom;
         return insets;
      }

      /**
       * Returns the insets of the border.
       * 
       * @return the insets of the border.
       */
      public Insets getBorderInsets()
      {
         return new Insets(mTop, mLeft, mBottom, mRight);
      }

      /**
       * Returns whether or not the border is opaque.
       * 
       * @return true, a tab area border is opaque.
       */
      public boolean isBorderOpaque()
      {
         return true;
      }
   }
   
   /**
    * A tab area parent border.
    * 
    * @author Dave Longley
    */
   public class TabAreaParentBorder extends AbstractInsetBorder
   {
      /**
       * Creates a tab area parent border with the specified insets.
       * 
       * @param top the top inset of the border.
       * @param left the left inset of the border.
       * @param bottom the bottom inset of the border.
       * @param right the right inset of the border.
       */
      public TabAreaParentBorder(int top, int left, int bottom, int right)
      {
         super(top, left, bottom, right);
      }

      /**
       * Creates a tab area parent border with the specified insets.
       * 
       * @param borderInsets the insets of the border.
       */
      public TabAreaParentBorder(Insets borderInsets)
      {
         super(borderInsets);
      }

      /**
       * Draws the tab area border.
       */
      public void paintBorder(Component c, Graphics g,
                              int x, int y, int width, int height)
      {
         // ensure valid
         ensureValid();
         
         // get the selected tab area
         Component selected = getSelectedTabArea();
         
         // determine if this parent has the selected tab area
         boolean isSelected = (selected != null && selected.getParent() == c);      
         
         // determine the color based on selection
         Color color = (isSelected) ?
               getLightHighlightColor() : getHighlightColor();
               
         int top = (isSelected) ? y : y + 2;
         int left = x;
         int bottom = y + height;
         int right = x + width;
         
         // left highlight
         g.setColor(color);
         g.drawLine(left, top + 1, left, bottom);
         
         g.setColor(getHighlightColor());
         g.drawLine(left + 1, top + 1, left + 1, bottom);
         
         // top highlight
         g.setColor(getLightHighlightColor());
         g.drawLine(left + 1, top, right - 2, top);
         
         if(!isSelected)
         {
            g.setColor(getShadowColor());
            g.drawLine(left, top - 2, right, top - 2);
            g.drawLine(left, top - 1, right, top - 1);
            g.drawLine(left, top, right, top);
           
            g.setColor(getHighlightColor());
         }
         
         g.drawLine(left + 1, top + 1, right - 2, top + 1);
         
         // right shadow
         g.setColor(getShadowColor());
         g.drawLine(right - 2, top + 2, right - 2, bottom);

         // right dark shadow
         g.setColor(getDarkShadowColor());
         g.drawLine(right, top + 2, right, bottom);
         g.drawLine(right - 1, top + 1, right - 1, bottom);
      }
   }
   
   /**
    * A tab content parent border.
    * 
    * @author Dave Longley
    */
   public class TabContentParentBorder extends AbstractInsetBorder
   {
      /**
       * Creates a tab content parent border with the specified insets.
       * 
       * @param top the top inset of the border.
       * @param left the left inset of the border.
       * @param bottom the bottom inset of the border.
       * @param right the right inset of the border.
       */
      public TabContentParentBorder(int top, int left, int bottom, int right)
      {
         super(top, left, bottom, right);
      }

      /**
       * Creates a tab content parent border with the specified insets.
       * 
       * @param borderInsets the insets of the border.
       */
      public TabContentParentBorder(Insets borderInsets)
      {
         super(borderInsets);
      }

      /**
       * Draws the tab content border.
       */
      public void paintBorder(Component c, Graphics g,
                              int x, int y, int width, int height)
      {
         // ensure valid
         ensureValid();
         
         int top = x;
         int left = y;
         int bottom = y + height;
         int right = x + width;
         
         // left highlight
         g.setColor(getLightHighlightColor());
         g.drawLine(left, top, left, bottom);
         g.drawLine(left + 1, top, left + 1, bottom);
         
         // left selected
         g.setColor(getSelectedColor());
         g.drawLine(left + 2, top, left + 2, bottom);
         
         // upper top
         g.setColor(getLightHighlightColor());
         
         // get the selected tabArea
         Component tabArea = getSelectedTabArea();
         if(tabArea != null && tabArea.getParent() != null)
         {
            Component parent = tabArea.getParent();
            
            // get the view position
            Point vp = getTabAreaViewport().getViewPosition();
            
            // get the x position of the tab panel scroll pane
            int scrollPaneX = mTabAreaScrollPane.getX();
            
            // get the parent rectangle
            Rectangle pRect = new Rectangle(
                  parent.getX() - vp.x + scrollPaneX,
                  parent.getY(), parent.getWidth(), parent.getHeight());
            
            g.drawLine(left + 1, top, pRect.x + 1, top);
            g.drawLine(left + 1, top + 1, pRect.x + 1, top + 1);
            
            g.setColor(getSelectedColor());
            g.drawLine(pRect.x + 2, top, pRect.x + pRect.width - 1, top);
            g.drawLine(pRect.x + 2, top + 1,
                       pRect.x + pRect.width - 1, top + 1);
            
            g.setColor(getLightHighlightColor());
            g.drawLine(pRect.x + pRect.width - 1, top, right, top);
            g.drawLine(pRect.x + pRect.width - 1, top + 1, right, top + 1);
         }
         else
         {
            g.drawLine(left, top, right, top);
            g.drawLine(left, top + 1, right, top + 1);
         }
         
         // lower top
         g.setColor(getSelectedColor());
         g.drawLine(left + 2, top + 2, right - 4, top + 2);
         g.drawLine(left + 2, top + 3, right - 4, top + 3);
         
         //g.setColor(getSelectedColor());
         g.drawLine(left + 2, top + 4, right - 4, top + 4);
         
         // right selected
         g.setColor(getSelectedColor());
         g.drawLine(right - 2, top + 1, right - 2, bottom - 1);

         // right shadow
         g.setColor(getShadowColor());  
         g.drawLine(right - 1, top + 1, right - 1, bottom - 1);

         // right dark shadow
         g.setColor(getDarkShadowColor());
         g.drawLine(right, top + 1, right, bottom);
         
         // bottom selected
         g.setColor(getSelectedColor());
         g.drawLine(left + 2, bottom - 2, right - 2, bottom - 2);
         
         // bottom shadow
         g.setColor(getShadowColor());
         g.drawLine(left + 1, bottom - 1, right - 1, bottom - 1);
         
         // bottom dark shadow
         g.setColor(getDarkShadowColor());
         g.drawLine(left, bottom, right - 1, bottom);
      }
   }
   
   /**
    * Handles events received by the scroll controls.
    * 
    * @author Dave Longley
    */
   public class ScrollControlsHandler implements ActionListener,
                                                 MouseListener
   {
      /**
       * Called when an action is performed (i.e. scroll buttons are pressed).
       * 
       * @param e the action event.
       */
      public void actionPerformed(ActionEvent e)
      {
         if(e.getActionCommand().equals("scroll_left"))
         {
            scrollTabsLeft();
         }
         else if(e.getActionCommand().equals("scroll_right"))
         {
            scrollTabsRight();
         }
      }
   
      /**
       * Called when a mouse button is pressed.
       * 
       * @param e the mouse event.
       */
      public void mousePressed(MouseEvent e)
      {
         // if right button pressed
         if(e.getButton() == MouseEvent.BUTTON3)
         {
            if(e.getSource() == mLeftScrollButton)
            {
               // scroll all the way left
               scrollToTabArea(getTabArea(0));
            }
            else if(e.getSource() == mRightScrollButton)
            {
               // scroll all the way right
               int count = getTabCount();
               scrollToTabArea(getTabArea(count - 1));
            }
         }
      }
      
      /**
       * Called when a mouse button is clicked. Makes sure to select
       * tab content components when receiving any clicks.
       * 
       * @param e the mouse event.
       */
      public void mouseClicked(MouseEvent e)
      {
      }
   
      /**
       * Called when a mouse button is released.
       * 
       * @param e the mouse event.
       */
      public void mouseReleased(MouseEvent e)
      {
      }
      
      /**
       * Called when a mouse enters.
       * 
       * @param e the mouse event.
       */
      public void mouseEntered(MouseEvent e)
      {
      }
      
      /**
       * Called when a mouse exited.
       * 
       * @param e the mouse event.
       */
      public void mouseExited(MouseEvent e)
      {
      }
   }
   
   /**
    * Handles events receives from the inner components.
    * 
    * @author Dave Longley
    */
   public class TabPanelHandler implements MouseListener,
                                           MouseMotionListener,
                                           MouseWheelListener
   {
      /**
       * Returns true if the left mouse button is down.
       * 
       * @param e the mouse event.
       * @return true if the left mouse button is down, false if not.
       */
      protected boolean isLeftMouseButtonDown(MouseEvent e)
      {
         boolean rval = false;
         
         if((e.getModifiersEx() & MouseEvent.BUTTON1_DOWN_MASK) ==
            MouseEvent.BUTTON1_DOWN_MASK)
         {
            rval = true;
         }
         
         return rval;
      }
      
      /**
       * Returns true if no buttons are up on the mouse.
       * 
       * @param e the mouse event.
       * @return true if no buttons are up on the mouse.
       */
      protected boolean mouseButtonsUp(MouseEvent e)
      {
         boolean rval = false;
         
         if((e.getModifiersEx() & MouseEvent.BUTTON1_DOWN_MASK) !=
            MouseEvent.BUTTON1_DOWN_MASK &&
            (e.getModifiersEx() & MouseEvent.BUTTON2_DOWN_MASK) !=
            MouseEvent.BUTTON2_DOWN_MASK &&
            (e.getModifiersEx() & MouseEvent.BUTTON3_DOWN_MASK) !=
            MouseEvent.BUTTON3_DOWN_MASK)
         {
            rval = true;
         }
         
         return rval;
      }
      
      
      /**
       * Called when a mouse button is pressed.
       * 
       * @param e the mouse event.
       */
      public void mousePressed(MouseEvent e)
      {
         // try to get the tab area from the parent
         Component tabArea = getTabAreaFromParent(e.getComponent());
         if(tabArea == null)
         {
            // parent failed, so check for child
            tabArea = getTabAreaFromChild(e.getComponent());
         }
         
         if(tabArea != null && tabArea != getSelectedTabArea())
         {
            tabArea.getParent().setBackground(getUnselectedColor());
            repaint();
         }         
      }

      /**
       * Called when a mouse button is clicked. Makes sure to select
       * tab content components when receiving any clicks.
       * 
       * @param e the mouse event.
       */
      public void mouseClicked(MouseEvent e)
      {
         // try to get the tab area from the parent
         Component tabArea = getTabAreaFromParent(e.getComponent());
         if(tabArea == null)
         {
            // parent failed, so check for child
            tabArea = getTabAreaFromChild(e.getComponent());
         }
         
         Component content = getTabContent(tabArea);
         if(content != null)
         {
            // select the content
            setSelected(content);
         }
      }
   
      /**
       * Called when a mouse button is released.
       * 
       * @param e the mouse event.
       */
      public void mouseReleased(MouseEvent e)
      {
         // try to get the tab area from the parent
         Component tabArea = getTabAreaFromParent(e.getComponent());
         if(tabArea == null)
         {
            // parent failed, so check for child
            tabArea = getTabAreaFromChild(e.getComponent());
         }
         
         if(tabArea != null && tabArea != getSelectedTabArea())
         {
            tabArea.getParent().setBackground(getUnselectedColor());
            repaint();
         }         
      }
      
      /**
       * Called when a mouse enters.
       * 
       * @param e the mouse event.
       */
      public void mouseEntered(MouseEvent e)
      {
         // try to get the tab area from the parent
         Component tabArea = getTabAreaFromParent(e.getComponent());
         if(tabArea == null)
         {
            // parent failed, so check for child
            tabArea = getTabAreaFromChild(e.getComponent());
         }

         // set mouse entered tab area
         mMouseEnteredTabArea = tabArea;
         
         if(mouseButtonsUp(e))
         {
            if(tabArea != null && tabArea != getSelectedTabArea())
            {
               tabArea.getParent().setBackground(getUnselectedHighlightColor());
               repaint();
            }
         }
      }
      
      /**
       * Called when a mouse exited.
       * 
       * @param e the mouse event.
       */
      public void mouseExited(MouseEvent e)
      {
         // try to get the tab area from the parent
         Component tabArea = getTabAreaFromParent(e.getComponent());
         if(tabArea == null)
         {
            // parent failed, so check for child
            tabArea = getTabAreaFromChild(e.getComponent());
         }
         
         // unset mouse entered tab area
         mMouseEnteredTabArea = null;
         
         if(mouseButtonsUp(e))
         {
            if(tabArea != null && tabArea != getSelectedTabArea())
            {
               tabArea.getParent().setBackground(getUnselectedColor());
               repaint();
            }
         }
      }
      
      /**
       * Invoked when a mouse button is pressed on a component and then 
       * dragged. <code>MOUSE_DRAGGED</code> events will continue to be 
       * delivered to the component where the drag originated until the 
       * mouse button is released (regardless of whether the mouse position 
       * is within the bounds of the component).
       * <p> 
       * Due to platform-dependent Drag&Drop implementations, 
       * <code>MOUSE_DRAGGED</code> events may not be delivered during a native 
       * Drag&Drop operation.
       */
      public void mouseDragged(MouseEvent e)
      {
      }

      /**
       * Invoked when the mouse cursor has been moved onto a component
       * but no buttons have been pushed.
       */
      public void mouseMoved(MouseEvent e)      
      {
      }
      
      /**
       * Called when a mouse wheel moves.
       * 
       * @param e the mouse wheel event.
       */
      public void mouseWheelMoved(MouseWheelEvent e)
      {
         // get the number of wheel "clicks"
         int wheelClicks = e.getWheelRotation();
         if(wheelClicks != 0)
         {
            // get the selected tab
            int selected = getSelectedIndex();
            if(selected == -1)
            {
               selected = 0;
            }
            
            // get the highest index
            int highIndex = getTabCount() - 1;
         
            if(wheelClicks > 0)
            {
               while((selected + wheelClicks) > highIndex)
               {
                  wheelClicks -= (selected == highIndex) ?
                        1 : (highIndex - selected);
                  
                  selected = 0;
               }
            }
            else
            {
               while((selected + wheelClicks) < 0)
               {
                  wheelClicks += ((selected == 0) ? 1 : selected);
                  selected = highIndex;
               }
            }
            
            selected += wheelClicks;
            
            // set selected index
            setSelectedIndex(selected);
         }
      }   
   }
   
   /**
    * A thread used to flash a tab area's parent.
    * 
    * @author Dave Longley
    */
   public class TabAreaFlashThread extends Thread implements TabPanelListener
   {
      /**
       * The tab panel the tab is in.
       */
      protected TabPanel mTabPanel;
      
      /**
       * The tab area to flash.
       */
      protected Component mTabArea;
      
      /**
       * The tab content for the tab area.
       */
      protected Component mTabContent;
      
      /**
       * Set to true to flash the tab, false not to.
       */
      protected boolean mFlash;
      
      /**
       * The flash color.
       */
      protected Color mFlashColor;
      
      /**
       * How often to flash the tab area (in milliseconds).
       */
      protected long mFlashInterval;
      
      /**
       * Set to true if flashing should stop when the tab area is
       * selected, false if not.
       */
      protected boolean mStopWhenSelected;
      
      /**
       * Creates a new tab area flash thread.
       * 
       * @param tabPanel the tab panel the tab is in.
       * @param tabArea the tab area to flash.
       * @param tabContent the tab content for the tab area.
       */
      public TabAreaFlashThread(TabPanel tabPanel, 
                                Component tabArea, Component tabContent)
      {
         mTabPanel = tabPanel;
         mTabArea = tabArea;
         mTabContent = tabContent;
         mFlash = false;
      }
      
      /**
       * Starts flashing the tab area.
       * 
       * @param color the color to flash the tab area.
       * @param interval how often to flash the tab area (in milliseconds).
       * @param stopWhenSelected true to stop flashing upon selection, false to
       *                         keep flashing until instructed to stop.
       */
      public synchronized void startFlashing(Color color, long interval,
                                             boolean stopWhenSelected)
      {
         mFlash = true;
         mFlashColor = color;
         mFlashInterval = interval;
         mStopWhenSelected = stopWhenSelected;
         
         start();
      }
      
      /**
       * Stops flashing the tab area.
       */
      public synchronized void stopFlashing()
      {
         mFlash = false;
      }
      
      /**
       * Returns true if flashing, false if not.
       * 
       * @return true if flashing, false if not.
       */
      public synchronized boolean isFlashing()
      {
         return mFlash;
      }
      
      /**
       * Gets the flash color.
       * 
       * @return the flash color.
       */
      public Color getFlashColor()
      {
         return mFlashColor;
      }
      
      /**
       * Gets the flash interval.
       * 
       * @return the flash interval.
       */
      public long getFlashInterval()
      {
         return mFlashInterval;
      }
      
      /**
       * Return true if flashing should stop when the tab area is
       * selected, false if not.
       * 
       * @return true if the flashing should stop when the tab area
       *         is selected, false if not.
       */
      public boolean stopWhenSelected()
      {
         return mStopWhenSelected;
      }
      
      /**
       * Called when a new tab is added to a tab panel.
       * 
       * @param content the content component that was added to the
       *        tab panel.
       */
      public void tabAdded(Component content)
      {
         // do nothing
      }
      
      /**
       * Called when a new tab is remove from a tab panel.
       * 
       * @param content the content component that was removed from the
       *        tab panel.
       */
      public void tabRemoved(Component content)
      {
         // if tab was the one that was flashing, stop flashing
         if(content == mTabContent)
         {
            stopFlashing();
         }
      }
      
      /**
       * Called when the tab selection changes in a tab panel.
       * 
       * @param oldSelection the old tab selection (can be null).
       * @param newSelection the new tab selection.
       */
      public void tabSelectionChanged(Component oldSelection,
                                      Component newSelection)      
      {
         // if stopping when selected and was selected, then stop flashing
         if(stopWhenSelected() && newSelection == mTabContent)
         {
            stopFlashing();
         }
      }
      
      /**
       * Flashes the tab area.
       */
      public void run()
      {
         // add tab panel listener
         mTabPanel.addTabPanelListener(this);

         try
         {
            Color background = null;
            
            boolean isSelected = false;
            while(isFlashing())
            {
               // set to flash color and sleep
               mTabArea.getParent().setBackground(mFlashColor);
               Thread.sleep(getFlashInterval());
               
               // see if still flashing
               if(isFlashing())
               {
                  isSelected = (mTabArea == getSelectedTabArea());
                  
                  // get the background color
                  if(isSelected)
                  {
                     background = getSelectedColor();
                     
                     // stop flashing if appropriate
                     if(stopWhenSelected())
                     {
                        stopFlashing();
                     }
                  }
                  else
                  {
                     background = (getMouseEnteredTabArea() == mTabArea) ?
                        getUnselectedHighlightColor() : getUnselectedColor();
                  }     
                  
                  // set to background color and sleep
                  mTabArea.getParent().setBackground(background);
                  mTabArea.repaint();
                  Thread.sleep(getFlashInterval());
               }
            }
            
            // get the background color
            isSelected = (mTabArea == getSelectedTabArea());
            if(isSelected)
            {
               background = getSelectedColor();
            }
            else
            {
               background = (getMouseEnteredTabArea() == mTabArea) ?
                  getUnselectedHighlightColor() : getUnselectedColor();
            }
            
            mTabArea.getParent().setBackground(background);
            mTabArea.getParent().repaint();
         }
         catch(Throwable t)
         {
         }
         
         // remove tab panel listener
         mTabPanel.addTabPanelListener(this);
      }
   }
}
