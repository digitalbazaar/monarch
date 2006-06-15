/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
import java.awt.Color;
import java.awt.Dimension;
import java.awt.FontMetrics;
import java.awt.GridLayout;
import java.awt.Point;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.HashMap;

import com.db.gui.*;
import com.db.logging.*;

import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.UIManager;
import javax.swing.border.BevelBorder;
import javax.swing.border.LineBorder;

/**
 * This class is used to test layout manager functionality.
 * 
 * @author Dave Longley
 */
public class UTLayoutManager
{
   /**
    * Starts the test(s).
    * 
    * @param args the arguments for the test.
    */
   public static void main(String[] args)
   {
      // set up logger
      LoggerManager.setFile("dbcommon", "utlayoutmanager.log", false);
      LoggerManager.setFile("dbgui", "utlayoutmanager.log");
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
      
      // create test frame
      JFrame frame = new JFrame("TEST FRAME");
      frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
      
      // create content panel
      JPanel panel = new JPanel();
      
      // setup panel
      //setupMainPanel(panel);
      //setupSearchPanel(panel);
      //setupCatalogPanel(panel);
      //setupJTextAreaPanel(panel);

      // create a content pane with an editable tab panel in it
      JPanel contentPane = new JPanel();
      contentPane.setLayout(new GridLayout(1, 1));
      EditableTabPanel etp = new EditableTabPanel();
      contentPane.add(etp);
      
      // add the main panel to the tab panel 
      etp.addTab("tab 1", panel);
      
      panel = new JPanel();
      setupSearchPanel(panel);
      //setupJTextAreaPanel(panel);
      etp.addTab("tab 2", panel);
      
      etp.addTab("tab 3", createCenteredComponentPanel());
      
      etp.setSelectedIndex(2);
      
      //frame.setContentPane(panel);
      //contentPane.setBorder(new LineBorder(java.awt.Color.red, 20));
      //contentPane.add(panel);
      frame.setContentPane(contentPane);
      
      // set frame and panel size
      frame.pack();
      frame.setSize(600, 600);
      
      // setup panel
      //setupMainPanel(panel);

      // pack frame, set location
      //frame.pack();
      //frame.setLocationRelativeTo(null);
      
      // show frame
      frame.setVisible(true);
      
      System.out.println("Frame created!");
   }
   
   /**
    * Sets up a catalog panel.
    * 
    * @param catalogPanel the catalog panel to set up.
    */
   public static void setupCatalogPanel(JPanel catalogPanel)
   {
      //catalogPanel.setOpaque(false);
      catalogPanel.setLocation(5, 0);
      catalogPanel.setSize(400, 400);
      catalogPanel.setLayout(new PositionLayout(catalogPanel));
      
      // create components
      JLabel tableLabel = new JLabel("Sales Catalog");
      
      JButton refreshCatalogButton = new JButton("Refresh Catalog List");
      JButton newWareButton = new JButton("Make New Ware");
      JButton removeWaresButton = new JButton("Remove Ware(s)");
     
      // add table to scroll pane
      //JScrollPane warePane = new JScrollPane(mCatalogTable);
      
      // lay out components
      int insets = 0;//2;
      
      // table label constraints
      PositionConstraints tableLabelPC = new PositionConstraints();
      tableLabelPC.size = new Dimension(tableLabel.getPreferredSize());
      tableLabelPC.location = new Point(0, insets);
      
      // remove button constraints
      PositionConstraints removeButtonPC = new PositionConstraints();
      removeButtonPC.size = 
         new Dimension(removeWaresButton.getPreferredSize());
      //removeButtonPC.location = 
         //new Point(getWidth() - insets - removeButtonPC.size.width, insets);
      removeButtonPC.location = 
         new Point(catalogPanel.getWidth() - removeButtonPC.size.width, 0);
      removeButtonPC.anchor = PositionConstraints.ANCHOR_TOP |
                              PositionConstraints.ANCHOR_RIGHT;
      removeWaresButton.setName("removeButton");
      
      // new button constraints
      PositionConstraints newButtonPC = new PositionConstraints();
      newButtonPC.size = 
         new Dimension(newWareButton.getPreferredSize());
      newButtonPC.location = 
         new Point(catalogPanel.getWidth() - insets - removeButtonPC.size.width -
               insets - newButtonPC.size.width, insets);
      newButtonPC.anchor = PositionConstraints.ANCHOR_RIGHT;
      
      // refresh button constraints 
      PositionConstraints refreshButtonPC = new PositionConstraints();
      refreshButtonPC.size = 
         new Dimension(refreshCatalogButton.getPreferredSize());
      refreshButtonPC.location = 
         new Point(catalogPanel.getWidth() - insets - removeButtonPC.size.width -
               insets -newButtonPC.size.width - insets -
               refreshButtonPC.size.width, insets);
      refreshButtonPC.anchor = PositionConstraints.ANCHOR_RIGHT;
      
      // ware pane constraints
      PositionConstraints warePanePC = new PositionConstraints();
      warePanePC.size = 
         new Dimension(PositionConstraints.HORIZONTAL_FILL, 
               PositionConstraints.VERTICAL_FILL);
      warePanePC.location = 
         new Point(0, insets + removeButtonPC.size.height + insets);
      warePanePC.anchor = PositionConstraints.ANCHOR_ALL;
      
      // add components
      
      //add(tableLabel, tableLabelPC);
      //add(mRefreshCatalogButton, refreshButtonPC);
      //add(mNewWareButton, newButtonPC);
      catalogPanel.add(removeWaresButton, removeButtonPC);
      //add(warePane, warePanePC);
   }
   
   /**
    * Sets up a search panel.
    * 
    * @param searchPanel the search panel to set up.
    */
   public static void setupSearchPanel(JPanel searchPanel)
   {
      //searchPanel.setOpaque(false);
      searchPanel.setSize(600, 600);
      
      // set layout
      searchPanel.setLayout(new PositionLayout(searchPanel));
      
      // create search media type map
      HashMap searchMediaTypeMap = new HashMap();
      searchMediaTypeMap.put("Albums", "audio_album");
      searchMediaTypeMap.put("Songs", "audio_song");
      searchMediaTypeMap.put("Video Clips", "video_clip");
      searchMediaTypeMap.put("Video Episodes", "video_episode");
      searchMediaTypeMap.put("Movies", "video_movie");
      searchMediaTypeMap.put("Music Videos", "video_music");
      
      // create search field
      //JTextField searchField = new JTextField();
      JButton searchField = new JButton("TESTING");
      searchField.addActionListener(new ButtonHandler());
      
      // create in label
      JLabel inLabel = new JLabel("in");
      inLabel.setOpaque(false);
      inLabel.setVerticalTextPosition(JLabel.BOTTOM);
      
      // create search category selection combo box
      JComboBox searchCategorySelection = new JComboBox();
      searchCategorySelection.addItem("Albums");
      searchCategorySelection.addItem("Songs");
      searchCategorySelection.addItem("Video Clips");
      searchCategorySelection.addItem("Movies");
      searchCategorySelection.addItem("Music Videos");
      
      // create search button
      JButton searchButton = new JButton("Search");
      
      // create advanced search checkbox
      JCheckBox advancedSearch = new JCheckBox("Advanced");
      advancedSearch.setOpaque(false);
      
      // create show non-sellable works checkbox
      JCheckBox showNonSellableWorks = 
         new JCheckBox("Show digital works not available for sale");
      showNonSellableWorks.setOpaque(false);
      showNonSellableWorks.setVisible(false);
      
      // create tab panel
      EditableTabPanel etp = new EditableTabPanel();
      
      // setup components' constraints
      int insets = 5;
      
      // search button constraints
      PositionConstraints searchButtonConstraints = new PositionConstraints();
      searchButtonConstraints.size =
         new Dimension(searchButton.getPreferredSize());
      searchButtonConstraints.location =
         new Point(searchPanel.getWidth() -
                   searchButtonConstraints.size.width, 0);
      searchButtonConstraints.anchor = PositionConstraints.ANCHOR_RIGHT |
                                       PositionConstraints.ANCHOR_TOP;

      // advanced search constraints
      PositionConstraints advancedSearchConstraints = new PositionConstraints();
      advancedSearchConstraints.size =
         new Dimension(advancedSearch.getPreferredSize().width,
                       searchButton.getPreferredSize().height);
      advancedSearchConstraints.location =
         new Point(searchButtonConstraints.location.x -
                   advancedSearchConstraints.size.width - insets, 0);
      advancedSearchConstraints.anchor = PositionConstraints.ANCHOR_RIGHT |
                                         PositionConstraints.ANCHOR_TOP;

      // search catalog selection constraints
      PositionConstraints scsConstraints = new PositionConstraints();
      scsConstraints.size =
         new Dimension(searchCategorySelection.getPreferredSize().width,
                       searchButton.getPreferredSize().height);
      scsConstraints.location =
         new Point(advancedSearchConstraints.location.x -
                   scsConstraints.size.width - insets, 0);
      scsConstraints.anchor = PositionConstraints.ANCHOR_RIGHT |
                              PositionConstraints.ANCHOR_TOP;
      
      // "in" label constraints
      PositionConstraints inLabelConstraints = new PositionConstraints();
      inLabelConstraints.size =
         new Dimension(inLabel.getPreferredSize().width,
                       searchButtonConstraints.size.height);
      inLabelConstraints.location =
         new Point(scsConstraints.location.x -
                   inLabelConstraints.size.width - insets, 0);
      inLabelConstraints.anchor = PositionConstraints.ANCHOR_RIGHT |
                                  PositionConstraints.ANCHOR_TOP;
      
      // search field constraints
      PositionConstraints searchFieldConstraints = new PositionConstraints();
      searchFieldConstraints.location = new Point(0, 0);
      searchFieldConstraints.size =
         new Dimension(inLabelConstraints.location.x - insets,
                       searchButton.getPreferredSize().height);
      searchFieldConstraints.anchor =
         PositionConstraints.ANCHOR_LEFT;// | PositionConstraints.ANCHOR_TOP |
         //PositionConstraints.ANCHOR_RIGHT;
      searchField.setName("TESTING");

      // non-sellable works checkbox constraints
      PositionConstraints nonSellableWorksConstraints =
         new PositionConstraints();
      nonSellableWorksConstraints.location =
         new Point(0, searchFieldConstraints.getBottom() + insets);
      nonSellableWorksConstraints.size =
         new Dimension(showNonSellableWorks.getPreferredSize());
      
      // tab panel constraints
      PositionConstraints tabPanelConstraints =
         new PositionConstraints();
      tabPanelConstraints.location =
         new Point(0, nonSellableWorksConstraints.getBottom() + insets);
      tabPanelConstraints.size =
         new Dimension(PositionConstraints.HORIZONTAL_FILL,
                       PositionConstraints.VERTICAL_FILL);
      tabPanelConstraints.anchor = PositionConstraints.ANCHOR_ALL;
      
      // add components
      searchPanel.add(searchField, searchFieldConstraints);
      searchPanel.add(inLabel, inLabelConstraints);
      searchPanel.add(searchCategorySelection, scsConstraints);
      searchPanel.add(searchButton, searchButtonConstraints);
      searchPanel.add(advancedSearch, advancedSearchConstraints);
      searchPanel.add(showNonSellableWorks, nonSellableWorksConstraints);
      searchPanel.add(etp, tabPanelConstraints);
   }
   
   /**
    * Sets up a panel with a JTextArea.
    * 
    * @param panel the panel to setup.
    */
   public static void setupJTextAreaPanel(JPanel panel)
   {
      // set layout
      panel.setSize(500, 500);
      panel.setLayout(new PositionLayout(panel));
      
      // create text area
      JTextArea textArea = new JTextArea();
      textArea.setEditable(false);
      textArea.setLineWrap(true);
      textArea.setWrapStyleWord(true);
      textArea.append(
         "This is a really long bunch of text for a text area. It is for " +
         "testing really large text areas to make sure that they wrap " +
         "properly. You know like a\n good wrapper and stuff. Words are " +
         "coming off of my fingers, LOLZ!\n" +
         
         "This is a really long bunch of text for a text area. It is for " +
         "testing really large text areas to make sure that they wrap " +
         "properly.\n You know like a good wrapper and stuff. Words are " +
         "coming off of my fingers, LOLZ!\n" + 
         
         "This is a really long bunch of text for a text area. It is for " +
         "testing really large text areas to make sure that they wrap " +
         "properly. You know like a good wrapper and stuff. Words are " +
         "coming off of my fingers, LOLZ!\n" +
      
         "This is a really long bunch of text for a text area. It is for " +
         "testing really large text areas to make sure that they wrap " +
         "properly. You know like a good wrapper and stuff. Words are " +
         "coming off of my fingers, LOLZ!\n" +
         
         "This is a really\n long bunch of text for a text area. It is for " +
         "testing really large text areas to make sure that they wrap " +
         "properly. You know like a good wrapper and stuff. Words are " +
         "coming off of my fingers, LOLZ!\n" +
         
         "This is a really long bunch of text for a text area. It is for " +
         "testing really large text areas to make sure that they wrap " +
         "properly. You know like a good wrapper and stuff. Words are " +
         "coming off of my fingers, LOLZ!\n");
      
      FontMetrics metrics = textArea.getFontMetrics(textArea.getFont());
      int columnWidth = metrics.charWidth('m');
      textArea.setColumns(panel.getWidth() / columnWidth);
      
      int height = PositionConstraints.
         getTextAreaPreferredHeight(textArea, panel.getWidth());
      
      // create scroll pane
      JScrollPane scrollPane = new JScrollPane(textArea);
      scrollPane.setVerticalScrollBarPolicy(
         JScrollPane.VERTICAL_SCROLLBAR_NEVER);
      scrollPane.setHorizontalScrollBarPolicy(
         JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
      scrollPane.setMinimumSize(
         new Dimension(scrollPane.getMinimumSize().width, height));
      
      // scroll pane constraints
      PositionConstraints scrollPaneConstraints = new PositionConstraints();
      scrollPaneConstraints.location = new Point(0, 0);
      scrollPaneConstraints.size =
         new Dimension(panel.getWidth(), height);
      scrollPaneConstraints.anchor =
         PositionConstraints.ANCHOR_TOP_LEFT_RIGHT;
      
      // add scroll pane
      panel.add(scrollPane, scrollPaneConstraints);
      
      // text area constraints
      /*PositionConstraints textAreaConstraints = new PositionConstraints();
      textAreaConstraints.location = new Point(0, 0);
      textAreaConstraints.size =
         new Dimension(textArea.getPreferredSize());
      textAreaConstraints.anchor = PositionConstraints.ANCHOR_ALL;
      
      // add text area
      panel.add(textArea, textAreaConstraints);*/
   }
   
   /**
    * Creates a panel with a centered component using a
    * PositionLayout.
    *
    * @return the panel with a centered component.
    */
   public static JPanel createCenteredComponentPanel()
   {
      JPanel panel = new JPanel();
      
      panel.setSize(300, 300);
      
      // set layout
      panel.setLayout(new PositionLayout(panel));
      
      // create colored panel (racist!)
      JPanel coloredPanel = new JPanel();
      coloredPanel.setBackground(Color.red);
      
      // colored panel constraints
      PositionConstraints coloredPanelConstraints = new PositionConstraints();
      coloredPanelConstraints.location = new Point(100, 100);
      coloredPanelConstraints.size = new Dimension(100, 100);
      coloredPanelConstraints.anchor =
         PositionConstraints.ANCHOR_NONE;
      coloredPanelConstraints.invertAnchor =
         PositionConstraints.ANCHOR_ALL;
      
      // add colored panel
      panel.add(coloredPanel, coloredPanelConstraints);
      
      return panel;
   }
   
   /**
    * Sets up the main panel.
    * 
    * @param panel the main panel.
    */
   public static void setupMainPanel(JPanel panel)
   {
      // set up layout manager
      panel.setLayout(new PositionLayout());
      
      // create buttons
      JButton button1 = new JButton("button1");
      PositionConstraints pc1 = new PositionConstraints();
      pc1.location = new Point(10, 10);
      pc1.size = new Dimension(30, 20);
      pc1.anchor = //PositionConstraints.ANCHOR_LEFT;// |
                   //PositionConstraints.ANCHOR_RIGHT;
                   PositionConstraints.ANCHOR_TOP;
      
      JButton button2 = new JButton("button2");
      PositionConstraints pc2 = new PositionConstraints();
      pc2.location = new Point(50, 10);
      pc2.size = new Dimension(30, 20);
      pc2.anchor = //PositionConstraints.ANCHOR_LEFT |
                   //PositionConstraints.ANCHOR_RIGHT;
                   PositionConstraints.ANCHOR_BOTTOM;

      JButton button3 = new JButton("button3");
      PositionConstraints pc3 = new PositionConstraints();
      pc3.location = new Point(90, 10);
      pc3.size = new Dimension(30, 20);
      pc3.anchor = PositionConstraints.ANCHOR_RIGHT;
                   //PositionConstraints.ANCHOR_LEFT |
                   //PositionConstraints.ANCHOR_RIGHT;
                   //PositionConstraints.ANCHOR_BOTTOM;

      JButton button4 = new JButton("button4");
      PositionConstraints pc4 = new PositionConstraints();
      pc4.location = new Point(10, 40);
      pc4.size = new Dimension(30, 20);
      pc4.anchor = //PositionConstraints.ANCHOR_LEFT;// |
                   //PositionConstraints.ANCHOR_RIGHT;
                   PositionConstraints.ANCHOR_BOTTOM |
                   PositionConstraints.ANCHOR_TOP;

      JButton button5 = new JButton("button5");
      PositionConstraints pc5 = new PositionConstraints();
      pc5.location = new Point(50, 40);
      pc5.size = new Dimension(30, 20);
      pc5.anchor = //PositionConstraints.ANCHOR_LEFT;// |
                   //PositionConstraints.ANCHOR_RIGHT;
                   PositionConstraints.ANCHOR_BOTTOM;

      JButton button6 = new JButton("button6");
      PositionConstraints pc6 = new PositionConstraints();
      pc6.location = new Point(90, 40);
      //pc6.size = new Dimension(30, 20);
      pc6.size = new Dimension(PositionConstraints.HORIZONTAL_FILL, 20);
      pc6.anchor = PositionConstraints.ANCHOR_LEFT |
                   PositionConstraints.ANCHOR_RIGHT |
                   PositionConstraints.ANCHOR_BOTTOM;
      
      // add buttons to panel
      panel.add(button1, pc1);
      panel.add(button2, pc2);
      panel.add(button3, pc3);
      panel.add(button4, pc4);
      panel.add(button5, pc5);
      panel.add(button6, pc6);

      // set panel properties
      panel.setBackground(Color.red);
      panel.setBorder(LineBorder.createBlackLineBorder());
      //panel.setBorder(new BevelBorder(BevelBorder.RAISED));
   }
   
   /**
    * Button Handler to catch the button presses.
    * 
    * @author Will Johnson
    */
   public static class ButtonHandler implements ActionListener
   {
      /**
       * Invoked when an action occurs.
       * 
       * @param e the action event.
       */
      public void actionPerformed(ActionEvent e)
      {
         
      }
   }
}
