/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.common.gui;

import java.awt.Color;
import java.awt.Component;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Image;
import java.awt.BasicStroke;
import java.awt.Insets;
import java.awt.Point;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.geom.GeneralPath;
import java.awt.image.BufferedImage;
import java.util.HashMap;
import java.util.Iterator;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.Icon;
import javax.swing.ImageIcon;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JButton;
import javax.swing.border.Border;
import javax.swing.plaf.basic.BasicButtonUI;

/**
 * A tab panel that has "editor tabs." This means that they can be
 * closed using close buttons, etc.
 * 
 * @author Dave Longley
 */
public class EditableTabPanel extends TabPanel
{
   /**
    * A mapping of close buttons to tab content components.
    */
   protected HashMap mCloseButtonToContent;
   
   /**
    * A mapping of tab content components to close buttons.
    */
   protected HashMap mContentToCloseButton;
   
   /**
    * The close button unpressed icon.
    */
   protected Icon mCloseButtonUnpressedIcon;
   
   /**
    * The close button pressed icon.
    */
   protected Icon mCloseButtonPressedIcon;
   
   /**
    * The close button mouseover icon.
    */
   protected Icon mCloseButtonMouseOverIcon;
   
   /**
    * The close button size.
    */
   protected final static int CLOSE_BUTTON_SIZE = 16;
   
   /**
    * The close button image size.
    */
   protected final static int CLOSE_BUTTON_IMAGE_SIZE = CLOSE_BUTTON_SIZE;
   
   /**
    * The close button handler.
    */
   protected CloseButtonHandler mCloseButtonHandler;
   
   /**
    * The currently dragged tab area. Often null.
    */
   protected Component mDraggedTabArea;
   
   /**
    * The current close button policy.
    */
   protected int mCloseButtonPolicy;
   
   /**
    * A close button policy that enables all close buttons.
    */
   public final static int ALL_CLOSE_BUTTONS_VISIBLE_AND_ENABLED_POLICY = 0;

   /**
    * A close button policy that only enables the close button for 
    * the selected tab.
    */
   public final static int ONLY_SELECTED_TAB_CLOSE_BUTTON_ENABLED_POLICY = 1;
   
   /**
    * A close button policy that only shows the close button for 
    * the selected tab.
    */
   public final static int ONLY_SELECTED_TAB_CLOSE_BUTTON_VISIBLE_POLICY = 2;
   
   /**
    * Creates a new editable tab panel.
    */
   public EditableTabPanel()
   {
      // create hash tables
      mCloseButtonToContent = new HashMap();
      mContentToCloseButton = new HashMap();
      
      // create the close button icons
      createCloseButtonIcons();
      
      // set default close button policy
      mCloseButtonPolicy = ALL_CLOSE_BUTTONS_VISIBLE_AND_ENABLED_POLICY;
   }
   
   /**
    * Overridden to use new EditableTabPanelHandler with additional
    * features.
    * 
    * Gets the tab panel handler.
    * 
    * @return the tab panel handler.
    */
   protected TabPanelHandler getTabPanelHandler()
   {
      if(mTabPanelHandler == null)
      {
         mTabPanelHandler = new EditableTabPanelHandler();
      }
      
      return mTabPanelHandler;
   }   
   
   /**
    * Gets the close button handler.
    * 
    * @return the close button handler.
    */
   protected CloseButtonHandler getCloseButtonHandler()
   {
      if(mCloseButtonHandler == null)
      {
         mCloseButtonHandler = new CloseButtonHandler();
      }
      
      return mCloseButtonHandler;
   }
   
   /**
    * Gets the general path for drawing a curvy X.
    * 
    * @param top the top of the x.
    * @param left the left of the x.
    * @param bottom the bottom of the x.
    * @param right the right of the x.
    * @param middle the middle of the x.
    * @param width the width of the x.
    * @return the path used to draw the x.
    */
   protected GeneralPath getCurvyXPath(int top, int left, int bottom, int right,
                                       int middle, int width)
   {
      // set path for x
      GeneralPath path = new GeneralPath();
      
      // top path of x
      path.moveTo(right, top);
      path.lineTo(right - width, top);
      path.lineTo(middle + 1, middle - width - 1);
      path.lineTo(middle, middle - width - 1);
      path.lineTo(left + width + 1, top);
      
      // left path of x
      path.lineTo(left + 1, top);
      path.lineTo(left + 1, top + width);
      path.lineTo(middle - width, middle - 1);
      path.lineTo(middle - width, middle);
      path.lineTo(left + 1, bottom - width - 1);
      
      // bottom path of x
      path.lineTo(left + 1, bottom - 1);
      path.lineTo(left + width + 1, bottom - 1);
      path.lineTo(middle, middle + width);
      path.lineTo(middle + 1, middle + width);
      path.lineTo(right - width, bottom - 1);
      
      // right path of x
      path.lineTo(right, bottom - 1);
      path.lineTo(right, bottom - width - 1);
      path.lineTo(middle + width + 1, middle);
      path.lineTo(middle + width + 1, middle - 1);
      path.lineTo(right, top + width);
      
      // connect to top
      path.lineTo(right, top);
      
      return path;      
   }
   
   /**
    * Gets the general path for drawing an X.
    * 
    * @param top the top of the x.
    * @param left the left of the x.
    * @param bottom the bottom of the x.
    * @param right the right of the x.
    * @param middle the middle of the x.
    * @param width the width of the x.
    * @return the path used to draw the x.
    */
   protected GeneralPath getXPath(int top, int left, int bottom, int right,
                                  int middle, int width)
   {
      // set path for x
      GeneralPath path = new GeneralPath();
      
      // top path of x
      path.moveTo(right, top);
      path.lineTo(right - width, top);
      path.lineTo(middle, middle - width);
      path.lineTo(left + width, top);
      
      // left path of x
      path.lineTo(left, top);
      path.lineTo(left, top + width);
      path.lineTo(middle - width, middle);
      path.lineTo(left, bottom - width);
      
      // bottom path of x
      path.lineTo(left, bottom);
      path.lineTo(left + width, bottom);
      path.lineTo(middle, middle + width);
      path.lineTo(right - width, bottom);
      
      // right path of x
      path.lineTo(right, bottom);
      path.lineTo(right, bottom - width);
      path.lineTo(middle + width, middle);
      path.lineTo(right, top + width);
      
      // connect to top
      path.lineTo(right, top);
      
      return path;
   }
   
   /**
    * Creates a close button unpressed image.
    * 
    * @return a close button unpressed image.
    */
   protected Image createCloseButtonUnpressedImage()
   {
      // setup the close button image
      BufferedImage image = new BufferedImage(CLOSE_BUTTON_IMAGE_SIZE,
                                              CLOSE_BUTTON_IMAGE_SIZE,
                                              BufferedImage.TYPE_4BYTE_ABGR);
      
      // set the insets for the x
      int insets = 3;
      
      // set the bounds of the x
      int top = insets;
      int left = insets;
      int bottom = CLOSE_BUTTON_IMAGE_SIZE - insets;
      int right = CLOSE_BUTTON_IMAGE_SIZE - insets;
      int middle = (right + left) / 2;
      
      // set the width of the x
      int width = 2;
      
      // set path for x outline
      GeneralPath path = getCurvyXPath(top, left, bottom, right, middle, width);
      
      BasicStroke bs = new BasicStroke(1);
      
      // draw the x
      Graphics2D g2 = image.createGraphics();
      g2.setColor(Color.black);
      g2.setStroke(bs);
      g2.draw(path);
      
      return image;
   }
   
   /**
    * Creates a close button pressed image.
    * 
    * @return a close button pressed image.
    */
   protected Image createCloseButtonPressedImage()
   {
      // setup the close button image
      BufferedImage image = new BufferedImage(CLOSE_BUTTON_IMAGE_SIZE,
                                              CLOSE_BUTTON_IMAGE_SIZE,
                                              BufferedImage.TYPE_4BYTE_ABGR);
      
      // set the insets for the x
      int insets = 3;
      
      // set the bounds of the x
      int top = insets;
      int left = insets;
      int bottom = CLOSE_BUTTON_IMAGE_SIZE - insets;
      int right = CLOSE_BUTTON_IMAGE_SIZE - insets;
      int middle = (right + left) / 2;
      
      // set the width of the x
      int width = 2;
      
      // set path for x outline
      GeneralPath path = getCurvyXPath(top, left, bottom, right, middle, width);
      
      BasicStroke bs = new BasicStroke(1);
      
      // draw the x outline
      Graphics2D g2 = image.createGraphics();
      g2.setColor(Color.black);
      g2.setStroke(bs);
      g2.draw(path);

      // set path for x filler
      path = getCurvyXPath(top + 1, left + 1, bottom - 1, right - 1, middle, 1);

      // draw filler
      g2.setColor(Color.red);
      g2.draw(path);
      
      // set path for x filler
      path = getCurvyXPath(top + 1, left + 1, bottom - 1, right - 1, middle, 0);
      
      // draw filler
      g2.draw(path);
      
      return image;      
   }
   
   /**
    * Creates a close button mouse over image.
    * 
    * @return a close button mouse over image.
    */
   protected Image createCloseButtonMouseOverImage()
   {
      // setup the close button image
      BufferedImage image = new BufferedImage(CLOSE_BUTTON_IMAGE_SIZE,
                                              CLOSE_BUTTON_IMAGE_SIZE,
                                              BufferedImage.TYPE_4BYTE_ABGR);
      
      // set the insets for the x
      int insets = 3;
      
      // set the bounds of the x
      int top = insets;
      int left = insets;
      int bottom = CLOSE_BUTTON_IMAGE_SIZE - insets;
      int right = CLOSE_BUTTON_IMAGE_SIZE - insets;
      int middle = (right + left) / 2;
      
      // set the width of the x
      int width = 2;
      
      // set path for x outline
      GeneralPath path = getCurvyXPath(top, left, bottom, right, middle, width);
      
      BasicStroke bs = new BasicStroke(1);
      
      // draw the x outline
      Graphics2D g2 = image.createGraphics();
      g2.setColor(Color.black);
      g2.setStroke(bs);
      g2.draw(path);

      // set path for x filler
      path = getCurvyXPath(top + 1, left + 1, bottom - 1, right - 1, middle, 1);

      // draw filler
      g2.setColor(Color.pink);
      g2.draw(path);
      
      // set path for x filler
      path = getCurvyXPath(top + 1, left + 1, bottom - 1, right - 1, middle, 0);
      
      // draw filler
      g2.draw(path);
      
      return image;      
   }   
   
   /**
    * Creates the icons for the close buttons.
    */
   protected void createCloseButtonIcons()
   {
      Image image = createCloseButtonUnpressedImage();
      mCloseButtonUnpressedIcon = new ImageIcon(image); 
      
      image = createCloseButtonPressedImage();
      mCloseButtonPressedIcon = new ImageIcon(image);
      
      image = createCloseButtonMouseOverImage();
      mCloseButtonMouseOverIcon = new ImageIcon(image);
   }
   
   /**
    * Creates a close button.
    * 
    * @param content the content the close button is for.
    * @return a close button.
    */
   protected JButton createCloseButton(Component content)
   {
      // create the close button
      JButton closeButton = new JButton();
      closeButton.setUI(new BasicButtonUI());
      closeButton.setOpaque(false);
      closeButton.setBorderPainted(false);
      Color transparent = new Color(0, 0, 0, 0);
      closeButton.setBackground(transparent);

      // set the close button size
      Dimension size = new Dimension(CLOSE_BUTTON_SIZE, CLOSE_BUTTON_SIZE);
      closeButton.setMinimumSize(size);
      closeButton.setPreferredSize(size);
      closeButton.setMaximumSize(size);
      
      // set icon for the close button
      closeButton.setIcon(mCloseButtonUnpressedIcon);
      closeButton.setPressedIcon(mCloseButtonPressedIcon);
      
      // add an entries to the hash tables
      mCloseButtonToContent.put(closeButton, content);
      mContentToCloseButton.put(content, closeButton);
      
      // add listeners
      closeButton.addActionListener(getCloseButtonHandler());
      closeButton.addMouseListener(getCloseButtonHandler());
      
      // enable/disable close button based on close button policy
      if(getCloseButtonPolicy() ==
         ONLY_SELECTED_TAB_CLOSE_BUTTON_ENABLED_POLICY)
      {
         closeButton.setEnabled(isSelected(content));
      }
      else
      {
         closeButton.setEnabled(true);
      }
      
      return closeButton;
   }
   
   /**
    * Gets the tab content component associated with the passed close button.
    * 
    * @param closeButton the close button to get the content for.
    * @return content the tab content component or null if there is no match.
    */
   protected Component getTabContentForCloseButton(Component closeButton)
   {
      return (Component)mCloseButtonToContent.get(closeButton);
   }
   
   /**
    * Gets the close button associated with the passed tab content component.
    * 
    * @param content the tab content component.
    * @return the close button for the content or null if there is no match.
    */
   protected JButton getCloseButton(Component content)
   {
      return (JButton)mContentToCloseButton.get(content);
   }
   
   /**
    * Creates a new editable tab area border. Overload this class to
    * change the borders on tab areas.
    * 
    * @return the new editable tab area border.
    */
   protected Border createTabAreaParentBorder()
   {
      // set a border to be painted later
      Insets insets = getTabAreaInsets();
      Border b = new EditableTabAreaParentBorder(insets);

      return b;
   }   
   
   /**
    * Creates a close button panel that encapsulates a tab area.
    * 
    * @param tabArea the tab area to encapsulate.
    * @param content the tab content for the tab area.
    * @return the close button panel to be inserted as the new tab area.
    */
   protected JPanel createCloseButtonPanel(Component tabArea, Component content)
   {
      CloseButtonJPanel cbp = new CloseButtonJPanel();

      // create a close button for the content
      JButton closeButton = createCloseButton(content);
      
      // set up the close button panel
      BoxLayout bl = new BoxLayout(cbp, BoxLayout.X_AXIS);
      cbp.setLayout(bl);
      cbp.setOpaque(false);
      
      // set horizontal strut size
      int strut = 5;
      
      // add components
      cbp.add(tabArea);
      cbp.add(Box.createHorizontalStrut(strut));
      cbp.add(closeButton);
      
      return cbp;
   }
   
   /**
    * Creates a label for a tab area with the specified title, icon,
    * and tooltip.
    * 
    * @param title the title.
    * @param icon the icon.
    * @param tooltip the tooltip text.
    * @return the created label.
    */
   protected JLabel createTabAreaLabel(String title, Icon icon, String tooltip)
   {
      // create a label
      JLabel label = new JLabel(title, icon, JLabel.TRAILING);
      
      // make label font plain
      Font font = label.getFont();
      Font newFont = new Font(font.getName(), Font.PLAIN, font.getSize());
      label.setFont(newFont);
      
      label.setOpaque(false);
      
      // set the tooltip for the tab area
      label.setToolTipText(tooltip);
      
      return label;
   }
   
   /**
    * Overridden to update label text when a tab is selected.
    * 
    * Selects a tab content component.
    * 
    * @param content the tab content component to select.
    * @return true if the content is selected, false if not.
    */
   protected synchronized boolean selectTabContent(Component content)
   {
      boolean rval = false;
      
      // get the old selected tab area
      Component oldTabArea = getSelectedTabArea();
      
      if(super.selectTabContent(content))
      {
         rval = true;
         
         Component tabArea = getTabArea(content);
         if(oldTabArea != null)
         {
            if(oldTabArea instanceof CloseButtonJPanel)
            {
               try
               {
                  JPanel panel = (JPanel)oldTabArea;
                  JLabel label = (JLabel)panel.getComponents()[0];
                  
                  Font font = label.getFont();
                  Font newFont =
                     new Font(font.getName(), Font.PLAIN, font.getSize());
                  label.setFont(newFont);
               }
               catch(Throwable t)
               {
               }
            }
         }
         
         if(tabArea != null)
         {
            if(tabArea instanceof CloseButtonJPanel)
            {
               try
               {
                  JPanel panel = (JPanel)tabArea;
                  JLabel label = (JLabel)panel.getComponents()[0];
                  
                  Font font = label.getFont();
                  Font newFont =
                     new Font(font.getName(), Font.BOLD, font.getSize());
                  label.setFont(newFont);
               }
               catch(Throwable t)
               {
               }
            }
         }
      }
      
      return rval;
   }
   
   /**
    * Adds a tab to the tab panel at a specified index. An index
    * of -1 will append the tab.
    * 
    * @param tabArea the component to display in the tab area. 
    * @param content the component to display in the content area.
    * @param index the index to add the tab at. 
    */
   public synchronized void addTab(Component tabArea,
                                   Component content, int index)
   {
      // create a close button panel
      JPanel cbp = createCloseButtonPanel(tabArea, content);
      
      // add tab, close button panel as the tab area
      super.addTab(cbp, content, index);
   }
   
   /**
    * Adds a tab that uses the passed title.
    * 
    * @param title the title for the tab.
    * @param content the content for the tab.
    * @see #addTab
    */
   public void addTab(String title, Component content)
   {
      addTab(title, null, null, content, -1);
   }

   /**
    * Adds a tab that uses the passed title and content at the specified index.
    * 
    * @param title the title for the tab.
    * @param content the content for the tab.
    * @param index the index to insert the tab at.
    * @see #addTab
    */
   public void addTab(String title, Component content, int index)
   {
      addTab(title, null, null, content, index);
   }
   
   /**
    * Adds a tab that uses the passed title, icon, and content.
    * 
    * @param title the title for the tab.
    * @param icon the icon for the tab.
    * @param content the content for the tab.
    * @see #addTab
    */
   public void addTab(String title, Icon icon, Component content)
   {
      addTab(title, icon, null, content, -1);
   }

   /**
    * Adds a tab that uses the passed title, icon, and content at the specified
    * index.
    * 
    * @param title the title for the tab.
    * @param icon the icon for the tab.
    * @param content the content for the tab.
    * @param index the index to insert the tab at.
    * @see #addTab
    */
   public void addTab(String title, Icon icon, Component content, int index)
   {
      addTab(title, icon, null, content, index);
   }
   
   /**
    * Adds a tab that uses the passed title, icon, tooltip, and content.
    * 
    * @param title the title for the tab.
    * @param icon the icon for the tab.
    * @param content the content for the tab.
    * @param tooltip the tooltip for the tab.
    * @see #addTab
    */
   public void addTab(String title, Icon icon, String tooltip,
                      Component content)
   {
      addTab(title, icon, tooltip, content, -1);
   }   
   
   /**
    * Adds a tab that uses the passed title, icon, tooltip, and content at the
    * specified index.
    * 
    * @param title the title for the tab.
    * @param icon the icon for the tab.
    * @param tooltip the tooltip text for the tab.
    * @param content the content for the tab.
    * @param index the index to insert the tab at.
    * @see #addtab
    */
   public synchronized void addTab(String title, Icon icon, String tooltip,
                                   Component content, int index)
   {
      // create a tab area label
      JLabel tabArea = createTabAreaLabel(title, icon, tooltip);
      
      // add the tab
      addTab(tabArea, content, index);
   }
   
   /**
    * Removes a tab from the tab panel.
    * 
    * @param content the content component that was displayed in the tab.
    */
   public synchronized void removeTab(Component content)   
   {
      // get the close button associated with the content
      JButton closeButton = (JButton)mContentToCloseButton.get(content);
      if(closeButton != null)
      {
         // remove listeners
         closeButton.removeActionListener(getCloseButtonHandler());
         closeButton.removeMouseListener(getCloseButtonHandler());
         
         // remove entries from the hash tables
         mCloseButtonToContent.remove(closeButton);
         mContentToCloseButton.remove(content);
      }
      
      // remove the tab
      super.removeTab(content);
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
         boolean isSelected = (getSelectedTabArea() == oldTabArea);
         
         // get the old tab area index
         int index = getTabAreaIndex(oldTabArea);
         
         // save background
         Color background = oldTabArea.getParent().getBackground();
         
         // remember whether or not the tab is closeable
         boolean closeable = isTabCloseable(content);
         
         // remove the old tab area
         removeTabArea(oldTabArea);
         
         // get the old close button
         JButton closeButton = (JButton)mContentToCloseButton.get(content);
         if(closeButton != null)
         {
            // remove the mapping
            mCloseButtonToContent.remove(closeButton);
         }
         
         // create the new tab area
         JPanel cbp = createCloseButtonPanel(tabArea, content);
         
         // add the new tab area in its place
         addTabArea(cbp, index);
         
         // map the tab area to the content and vice versa
         mAreaToContent.put(cbp, content);
         mContentToArea.put(content, cbp);

         // set background
         cbp.getParent().setBackground(background);
         
         // set closeable status of tab
         setTabCloseable(content, closeable);
         
         // reselect tab area if appropriate
         if(isSelected)
         {
            selectTabContent(content);
         }
         
         // revalidate, repaint
         revalidate();
         repaint();
      }
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
      super.setTabContent(oldContent, newContent);
      
      Component tabArea = getTabArea(newContent);
      if(tabArea != null)
      {
         // remap the close button in the tab area
         JButton closeButton = getCloseButton(oldContent);
         mContentToCloseButton.remove(oldContent);
         mCloseButtonToContent.put(closeButton, newContent);
         mContentToCloseButton.put(newContent, closeButton);
      }
      
      revalidate();
      repaint();
   }
   
   /**
    * Sets the title for the tab area of an existing tab.
    *
    * @param content the tab content of the tab to change.
    * @param title the title for the tab.
    */
   public synchronized void setTabArea(Component content, String title)
   {
      Component tabArea = getTabArea(content);
      if(tabArea != null)
      {
         if(tabArea instanceof CloseButtonJPanel)
         {
            try
            {
               JPanel panel = (JPanel)tabArea;
               Icon icon = null;
               String toolTip = null;
               if(panel.getComponents()[0] instanceof JLabel)
               {
                  JLabel label = (JLabel)panel.getComponents()[0];
                  icon = label.getIcon();
                  toolTip = label.getToolTipText();
               }
               
               // create a tab area label
               JLabel newLabel = createTabAreaLabel(title, icon, toolTip);
               setTabArea(content, newLabel);
            }
            catch(Throwable t)
            {
            }
         }
         else
         {
            // create a tab area label
            JLabel newLabel = createTabAreaLabel(title, null, null);
            setTabArea(content, newLabel);
         }
      }
   }
   
   /**
    * Sets the title and icon for the tab area of an existing tab.
    *
    * @param content the tab content of the tab to change.
    * @param title the title for the tab.
    * @param icon the icon for the tab.
    */
   public synchronized void setTabArea(Component content, 
                                       String title, Icon icon)
   {
      Component tabArea = getTabArea(content);
      if(tabArea != null)
      {
         if(tabArea instanceof CloseButtonJPanel)
         {
            try
            {
               JPanel panel = (JPanel)tabArea;
               String toolTip = null;
               if(panel.getComponents()[0] instanceof JLabel)
               {
                  JLabel label = (JLabel)panel.getComponents()[0];
                  toolTip = label.getToolTipText();
               }
               
               // create a tab area label
               JLabel newLabel = createTabAreaLabel(title, icon, toolTip);
               setTabArea(content, newLabel);
            }
            catch(Throwable t)
            {
            }
         }
         else
         {
            // create a tab area label
            JLabel newLabel = createTabAreaLabel(title, icon, null);
            setTabArea(content, newLabel);
         }
      }
   }
   
   /**
    * Sets the title, icon, and tooltip for the tab area of an existing tab.
    *
    * @param content the tab content of the tab to change.
    * @param title the title for the tab.
    * @param icon the icon for the tab.
    * @param tooltip the tooltip text for the tab. 
    */
   public synchronized void setTabArea(Component content, String title,
                                       Icon icon, String tooltip)
   {
      Component tabArea = getTabArea(content);
      if(tabArea != null)
      {
         // create a tab area label
         JLabel newLabel = createTabAreaLabel(title, icon, tooltip);
         
         // set the new tab area
         setTabArea(content, newLabel);
      }
   }
   
   /**
    * Sets the selected content component.
    * 
    * Overridden to enable/disable close buttons as appropriate.
    * 
    * @param content the content component to set as selected.
    */
   public void setSelected(Component content)
   {
      if(getCloseButtonPolicy() ==
         ONLY_SELECTED_TAB_CLOSE_BUTTON_ENABLED_POLICY)
      {
         // if the close button policy only allows the selected tab to have
         // an enabled close button, then take appropriate action

         // get the old selection
         Component oldSelection = getSelected();
         if(oldSelection != null)
         {
            // get close button
            JButton closeButton = getCloseButton(oldSelection);
            if(closeButton != null)
            {
               closeButton.setEnabled(false);
            }
         }
         
         // get the new selection close button
         JButton closeButton = getCloseButton(content);
         if(closeButton != null)
         {
            closeButton.setEnabled(true);
         }
      }
      else if(getCloseButtonPolicy() ==
              ONLY_SELECTED_TAB_CLOSE_BUTTON_VISIBLE_POLICY)
      {
         // if the close button policy only allows the selected tab to have
         // a visible close button, then take appropriate action
         
         // get the old selection
         Component oldSelection = getSelected();
         if(oldSelection != null)
         {
            // make old selection uncloseable
            setTabCloseable(oldSelection, false);
         }
         
         // make new selection closeable
         setTabCloseable(content, true);
      }         
      
      super.setSelected(content);
   }
   
   /**
    * Sets the close button policy for this tab panel.
    * 
    * @param policy the close button policy to use.
    */
   public synchronized void setCloseButtonPolicy(int policy)
   {
      if(policy == ALL_CLOSE_BUTTONS_VISIBLE_AND_ENABLED_POLICY)
      {
         mCloseButtonPolicy = policy;
         
         // enable all close buttons
         Iterator i = getTabContentIterator();
         while(i.hasNext())
         {
            Component content = (Component)i.next();
            
            // get close button
            JButton closeButton = getCloseButton(content);
            if(closeButton != null)
            {
               closeButton.setEnabled(true);
            }
            
            // make tab closeable
            setTabCloseable(content, true);
         }
      }
      else if(policy == ONLY_SELECTED_TAB_CLOSE_BUTTON_ENABLED_POLICY)
      {
         mCloseButtonPolicy = policy;
         
         // get selected tab content
         Component selected = getSelected();

         // disable all close buttons except selected tab's
         Iterator i = getTabContentIterator();
         while(i.hasNext())
         {
            Component content = (Component)i.next();
            
            // make tab closeable
            setTabCloseable(content, true);
            
            // get close button
            JButton closeButton = getCloseButton(content);
            if(closeButton != null)
            {
               // set enabled based on selected status
               closeButton.setEnabled(content == selected);
            }
         }
      }
      else if(policy == ONLY_SELECTED_TAB_CLOSE_BUTTON_VISIBLE_POLICY)
      {
         mCloseButtonPolicy = policy;
         
         // get selected tab content
         Component selected = getSelected();

         // mark all tabs uncloseable except selected tab
         Iterator i = getTabContentIterator();
         while(i.hasNext())
         {
            Component content = (Component)i.next();
            setTabCloseable(content, content == selected);
            
            // enable close button
            JButton closeButton = getCloseButton(content);
            if(closeButton != null)
            {
               closeButton.setEnabled(true);
            }
         }         
      }
   }
   
   /**
    * Gets the close button policy for this tab panel.
    * 
    * @return the close button policy for this tab panel.
    */
   public int getCloseButtonPolicy()
   {
      return mCloseButtonPolicy;
   }
   
   /**
    * Sets whether or not a tab is closeable.
    * 
    * @param content the content of the tab to set.
    * @param closeable whether or not the tab is closeable. 
    */
   public synchronized void setTabCloseable(Component content, 
                                            boolean closeable)
   {
      if(content != null && isTabCloseable(content) != closeable)
      {
         // get the tab area
         Component tabArea = getTabArea(content);
         if(tabArea != null && tabArea instanceof CloseButtonJPanel)
         {
            // get the close button for the content
            JButton closeButton = getCloseButton(content);
            if(closeButton != null)
            {
               CloseButtonJPanel panel = (CloseButtonJPanel)tabArea;

               if(closeable)
               {
                  // add the close button and the strut
                  int strut = 5;
                  panel.add(Box.createHorizontalStrut(strut));
                  panel.add(closeButton);
               }
               else
               {
                  // remove the close button and the strut
                  panel.remove(closeButton);
                  panel.remove(1);
               }
            }
         }
      }
   }
   
   /**
    * Sets whether or not a tab is closeable.
    * 
    * @param index the index of the tab to set.
    * @param closeable whether or not the tab is closeable. 
    */
   public void setTabCloseable(int index, boolean closeable)
   {
      setTabCloseable(getTabContent(index), closeable);
   }
   
   /**
    * Determines whether or not a tab is closeable.
    * 
    * @param content the content of the tab to get the closeable status of.
    * @return true if the tab is closeable, false if not or if the tab
    *         doesn't exist.
    */
   public boolean isTabCloseable(Component content)
   {
      boolean rval = false;
      
      if(content != null)
      {
         // get the tab area
         Component tabArea = getTabArea(content);
         if(tabArea != null && tabArea instanceof CloseButtonJPanel)
         {
            // get the close button for the content
            JButton closeButton = getCloseButton(content);
            if(closeButton != null)
            {
               CloseButtonJPanel panel = (CloseButtonJPanel)tabArea;
               Component[] components = panel.getComponents();
               for(int i = 0; i < components.length; i++)
               {
                  if(components[i] == closeButton)
                  {
                     rval = true;
                     break;
                  }
               }
            }
         }
      }
      
      return rval;
   }
   
   /**
    * Determines whether or not a tab is closeable.
    * 
    * @param index the index of the tab to get the closeable status of.
    * @return true if the tab is closeable, false if not or if the tab
    *         doesn't exist.
    */
   public boolean isTabCloseable(int index)
   {
      return isTabCloseable(getTabContent(index));
   }
   
   /**
    * A close button panel.
    * 
    * @author Dave Longley
    */
   public class CloseButtonJPanel extends JPanel
   {
   }

   /**
    * A tab area parent border with additional look and feel/features.
    * 
    * @author Dave Longley
    */
   public class EditableTabAreaParentBorder extends TabAreaParentBorder
   {
      /**
       * Creates an editable tab area parent border with the specified insets.
       * 
       * @param top the top inset of the border.
       * @param left the left inset of the border.
       * @param bottom the bottom inset of the border.
       * @param right the right inset of the border.
       */
      public EditableTabAreaParentBorder(int top, int left,
                                         int bottom, int right)
      {
         super(top, left, bottom, right);
      }

      /**
       * Creates a editable tab area parent border with the specified insets.
       * 
       * @param borderInsets the insets of the border.
       */
      public EditableTabAreaParentBorder(Insets borderInsets)
      {
         super(borderInsets);
      }

      /**
       * Draws the tab area border.
       */
      public void paintBorder(Component c, Graphics g,
                              int x, int y, int width, int height)      
      {
         // paint the main border
         super.paintBorder(c, g, x, y, width, height);
         
         // paint the drag border
         
         // see if a tab is being dragged
         if(getMouseEnteredTabArea() != null && mDraggedTabArea != null &&
            getMouseEnteredTabArea() != mDraggedTabArea)
         {
            // get the content component
            Component tabArea = getTabAreaFromParent(c);
            
            // see if the mouse is over this tab area parent 
            if(tabArea == getMouseEnteredTabArea())
            {
               // determine to paint left border or right border
               boolean paintLeft = true;
               
               // if mouse was on right half of tab area use paint right
               Point p = c.getMousePosition();
               int middle = c.getWidth() / 2;
               if(p != null && p.x > middle)
               {
                  paintLeft = false;
               }

               int top = 0;
               int left = 0;
               int bottom = c.getHeight();
               int right = c.getWidth();
               
               // draw the drag border
               g.setColor(Color.red);
               
               if(paintLeft)
               {
                  // left
                  g.drawLine(left, top, left, bottom);
                  g.drawLine(left + 1, top, left + 1, bottom);
               }
               else
               {
                  // right
                  g.drawLine(right, top, right, bottom);
                  g.drawLine(right - 1, top, right - 1, bottom);
                  g.drawLine(right - 2, top, right - 2, bottom);
               }
            }
         }
      }
   }
   
   /**
    * Handles events from the inner components.
    * 
    * Allows tabs to be swapped via the mouse.
    * 
    * @author Dave Longley
    */
   public class EditableTabPanelHandler extends TabPanelHandler
   {
      /**
       * Called when a mouse button is pressed.
       * 
       * @param e the mouse event.
       */
      public void mousePressed(MouseEvent e)
      {
         super.mousePressed(e);
         
         Component c = e.getComponent();
         Component tabArea = getTabAreaFromChild(c);
         Component content = getTabContent(tabArea);
         if(content != null)
         {
            // if not a close button, then make sure to select the content
            if(getTabContentForCloseButton(c) == null)
            {
               setSelected(content);
               
               // if left button is down, set dragged content
               if(isLeftMouseButtonDown(e))
               {
                  // set the currently dragged tab area
                  mDraggedTabArea = tabArea;
               }
            }
         }
      }
      
      /**
       * Called when a mouse button is released.
       * 
       * @param e the mouse event.
       */
      public void mouseReleased(MouseEvent e)
      {
         super.mouseReleased(e);
         
         // see if there was dragged content and over a tab
         if(mDraggedTabArea != null && getMouseEnteredTabArea() != null &&
            mDraggedTabArea != getMouseEnteredTabArea())
         {
            // get the tab index
            int index = getTabAreaIndex(getMouseEnteredTabArea());
            if(index != -1)
            {
               // if mouse was on right half of tab area use index plus 1
               Container parent = getMouseEnteredTabArea().getParent();
               Point p = parent.getMousePosition();
               int middle = parent.getWidth() / 2;
               if(p != null && p.x > middle)
               {
                  index++;
               }
               else
               {
                  // get dragged content index
                  int dIndex = getTabAreaIndex(mDraggedTabArea);
                     
                  // if dragged content directly to the left of
                  // mouse over content and mouse is on left side,
                  // no tabs should be moved so set the indices equal
                  if(dIndex == (index - 1))
                  {
                     index = dIndex;
                  }
               }
               
               // move dragged tab to new index
               Component content = getTabContent(mDraggedTabArea);
               if(moveTab(content, index))
               {
                  // select the dragged content
                  setSelected(content);
               }
            }
         }
         
         // repaint drag borders if dragging
         if(mDraggedTabArea != null)
         {
            repaint();
         }
         
         // release any dragged tab area
         mDraggedTabArea = null;
      }
      
      /**
       * Called when a mouse enters.
       * 
       * @param e the mouse event.
       */
      public void mouseEntered(MouseEvent e)
      {
         super.mouseEntered(e);

         // if left button is not down
         if(!isLeftMouseButtonDown(e))
         {
            // mouse not pressed, so let go of dragged tab area
            mDraggedTabArea = null;
         }
         
         // repaint drag borders if dragging
         if(mDraggedTabArea != null)
         {
            repaint();
         }
      }
      
      /**
       * Called when a mouse exited.
       * 
       * @param e the mouse event.
       */
      public void mouseExited(MouseEvent e)
      {
         super.mouseExited(e);

         // if left button is not down
         if(!isLeftMouseButtonDown(e))
         {
            // mouse not pressed, so let go of dragged tab area
            mDraggedTabArea = null;
         }
         
         // repaint drag borders if dragging
         if(mDraggedTabArea != null)
         {
            repaint();
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
         super.mouseDragged(e);

         // repaint the drag borders
         repaint();
      }
   }
   
   /**
    * Handles events from the close buttons.
    * 
    * @author Dave Longley
    */
   public class CloseButtonHandler implements ActionListener,
                                              MouseListener
   {
      /**
       * Called whenever an action is performed.
       * 
       * @param e the action event.
       */
      public void actionPerformed(ActionEvent e)
      {
         Object src = e.getSource();
         if(src instanceof Component)
         {
            Component content = getTabContentForCloseButton((Component)src);
            
            // remove the tab
            removeTab(content);
         }
      }
      
      /**
       * Called when a mouse button is pressed.
       * 
       * @param e the mouse event.
       */
      public void mousePressed(MouseEvent e)
      {
      }

      /**
       * Called when a mouse button is clicked.
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
         // only show highlight on close button if not dragging a tab
         if(mDraggedTabArea == null)
         {
            Iterator i = mCloseButtonToContent.keySet().iterator();
            while(i.hasNext())
            {
               JButton closeButton = (JButton)i.next();
               if(closeButton == e.getSource())
               {
                  closeButton.setIcon(mCloseButtonMouseOverIcon);
                  break;
               }
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
         Iterator i = mCloseButtonToContent.keySet().iterator();
         while(i.hasNext())
         {
            JButton closeButton = (JButton)i.next();
            if(closeButton == e.getSource())
            {
               closeButton.setIcon(mCloseButtonUnpressedIcon);
               break;
            }
         }
      }   
   }   
}
