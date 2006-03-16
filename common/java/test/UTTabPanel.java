/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
import com.db.common.gui.TabPanel;
import com.db.common.gui.EditableTabPanel;
import com.db.common.logging.*;

//import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

//import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.UIManager;

/**
 * This class is used to test tab panel functionality.
 * 
 * @author Dave Longley
 */
public class UTTabPanel implements ActionListener
{
   public static EditableTabPanel mTabPanel;
   public static JLabel mContent0;
   public static JButton mContent2;
   public static JButton mTab0;
   
   public static void main(String[] args)
   {
      // set up logger
      LoggerManager.setFile("dbcommon", "uttabpanel.log", false);
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
      
      JFrame frame = new JFrame("TEST FRAME");
      frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
      
      //mTabPanel = new TabPanel();
      mTabPanel = new EditableTabPanel();
      //mTabPanel.setTabSelectionPolicy(TabPanel.SELECT_NEXT_TAB_POLICY);
      
      //JLabel tab0 = new JLabel("index0");
      mTab0 = new JButton("tab0");
      mTab0.setActionCommand("tab_button");
      mTab0.addActionListener(new UTTabPanel());
      //JButton content0 = new JButton("content0");
      mContent0 = new JLabel("content0");
      mContent0.setOpaque(true);
      mContent0.setBackground(java.awt.Color.gray);
      
      /*JLabel tab1 = new JLabel("index1");
      JLabel tab2 = new JLabel("index2");
      JLabel tab3 = new JLabel("index3");
      JLabel tab4 = new JLabel("index4");
      JLabel tab5 = new JLabel("index5");
      JLabel tab6 = new JLabel("index6");
      JLabel tab7 = new JLabel("index7");
      JLabel tab8 = new JLabel("index8");
      JLabel tab9 = new JLabel("index9");
      JLabel tab10 = new JLabel("index10");
      JLabel tab11 = new JLabel("index11");*/
      JLabel tab12 = new JLabel("index12");
      
      JButton content1 = new JButton("content1");
      mContent2 = new JButton("content2");
      JButton content3 = new JButton("content3");
      JButton content4 = new JButton("content4");
      JButton content5 = new JButton("content5");
      JButton content6 = new JButton("content6");
      JButton content7 = new JButton("content7");
      JButton content8 = new JButton("content8");
      JButton content9 = new JButton("content9");
      //JButton content10 = new JButton("content10");
      //JButton content11 = new JButton("content11");
      JButton content12 = new JButton("content12");
      
      mTabPanel.addTab(mTab0, mContent0);
      mTabPanel.addTab("index 1", content1);
      mTabPanel.addTab("index 2", mContent2);
      mTabPanel.addTab("index 3", content3);
      /*mTabPanel.addTab("index 4", content4);
      mTabPanel.addTab("index 5", content5);
      mTabPanel.addTab("index 6", content6);
      mTabPanel.addTab("index 7", content7);
      mTabPanel.addTab("index 8", content8);
      mTabPanel.addTab("index 9", content9);*/
      
      /*mTabPanel.addTab(mTab0, mContent0);
      mTabPanel.addTab(tab1, content1);
      mTabPanel.addTab(tab2, mContent2);
      
      mTabPanel.addTab(tab3, content3);
      mTabPanel.addTab(tab4, content4);
      mTabPanel.addTab(tab5, content5);
      mTabPanel.addTab(tab6, content6);
      mTabPanel.addTab(tab7, content7);
      mTabPanel.addTab(tab8, content8);
      mTabPanel.addTab(tab9, content9);*/
      //mTabPanel.addTab(tab10, content10);
      //mTabPanel.addTab(tab11, content11);
      //tp.addTab(tab12, content12);
      
      /*mTabPanel.setTabArea(content3, tab3);
      mTabPanel.setTabContent(content3, content11);
      mTabPanel.setTabContent(content4, content10);
      
      mTabPanel.setSelected(content11);*/
      
      //((EditableTabPanel)mTabPanel).setTabArea(content11, "stuff");
      //((EditableTabPanel)mTabPanel).setTabArea(content11, "stuff2");
      /*((EditableTabPanel)mTabPanel).setTabArea(content11, "stuff3");
      ((EditableTabPanel)mTabPanel).setTabArea(content11, "stuff4");
      ((EditableTabPanel)mTabPanel).setTabArea(content11, "stuff5");
      ((EditableTabPanel)mTabPanel).setTabArea(content11, "stuff6");*/
      
      //tp.removeTab(content0);
      
      TabPanel tp2 = new TabPanel();
      JLabel tab13 = new JLabel("index13");
      JLabel tab14 = new JLabel("index14");
      JLabel tab15 = new JLabel("index15");
      JLabel tab16 = new JLabel("index16");
      JButton content13 = new JButton("content13");
      JButton content14 = new JButton("content14");
      JButton content15 = new JButton("content15");
      JButton content16 = new JButton("content16");
      tp2.addTab(tab12, content12);
      tp2.addTab(tab13, content13);
      tp2.addTab(tab14, content14);
      tp2.addTab(tab15, content15);
      tp2.addTab(tab16, content16);
      
      //tp.addTab(tp2, new JLabel("stuff"));
      
      
      //javax.swing.Icon i = 
      //com.sun.java.swing.plaf.windows.WindowsIconFactory.createFrameCloseIcon();
      //ImageIcon i = new ImageIcon();
      
      //tp.addTab("index1", i, null);
      /*tp.addTab(null, i, null);
      
      TabbedPane tp2 = new TabbedPane();
      tp2.addTab("inner", new JButton("thing"));
      
      tp.addTab(new JButton("stuff"), tp2);*/
      
      /*tp.add("index2", null);
      tp.add("index3", null);
      tp.add("index4", null);
      tp.add("index5", null);
      tp.add("index6", null);
      tp.add("index7", null);
      tp.add(contentButton2, tabButton2);
      tp.add("index8", null);
      tp.add("index9", null);
      tp.add("index10", null);
      tp.add("index12", null);
      tp.add("index13", null);
      tp.add("index14", null);
      tp.add("index15", null);
      tp.add("index16", null);
      tp.add("index17", null);*/
      
      frame.getContentPane().add(mTabPanel);
      frame.pack();
      frame.setSize(600, 600);
      frame.setVisible(true);

      /*try
      {
         for(int i = 0; i < 1500; i += 100)
         {
            Thread.sleep(1000);
            ((EditableTabPanel)mTabPanel).setTabArea(content11, "stuff" + i);
         }
      }
      catch(Throwable t)
      {
      }*/
      
      LoggerManager.debug("bmcommon", "Frame created!");      
   }
   
   public void actionPerformed(ActionEvent e)
   {
      if(e.getActionCommand().equals("tab_button"))
      {
         EditableTabPanel tabPanel = (EditableTabPanel)mTabPanel;
         
         LoggerManager.debug("bmcommon", "Tab button pressed!");
         if(mTabPanel.isTabFlashing(mContent0))
         {
            System.out.println("button pressed");
            //((EditableTabPanel)mTabPanel).addTab("title", new JLabel());
            //mTab0.setText(mTab0.getText() + "BIGGER");
            //mTabPanel.setTabArea(mContent0, mTab0);
            mTabPanel.startFlashingTab(mContent2, true);
         }
         else
         {
            mTab0.setText("BIGGER AND BIGGER!");
            //mTabPanel.setTabArea(mContent0, mTab0);
            tabPanel.addTab("title", new JLabel());
            tabPanel.startFlashingTab(mContent2, true);
         }
         
         //boolean closeable = tabPanel.isTabCloseable(0);
         //System.out.println("tab was closeable=" + closeable);
         //tabPanel.setTabCloseable(0, !closeable);
         
         if(tabPanel.getCloseButtonPolicy() ==
            EditableTabPanel.ALL_CLOSE_BUTTONS_VISIBLE_AND_ENABLED_POLICY)
         {
            System.out.println("SETTING CLOSE BUTTON POLICY TO " +
                               "VISIBLE ONLY ON SELECTED TAB");
            tabPanel.setCloseButtonPolicy(
               EditableTabPanel.ONLY_SELECTED_TAB_CLOSE_BUTTON_VISIBLE_POLICY);
         }
         else if(tabPanel.getCloseButtonPolicy() ==
                 EditableTabPanel.ONLY_SELECTED_TAB_CLOSE_BUTTON_VISIBLE_POLICY)
         {
            System.out.println("SETTING CLOSE BUTTON POLICY TO " +
                               "ENABLE ONLY ON SELECTED TAB");
            tabPanel.setCloseButtonPolicy(
               EditableTabPanel.ONLY_SELECTED_TAB_CLOSE_BUTTON_ENABLED_POLICY);
         }
         else
         {
            System.out.println("SETTING CLOSE BUTTON POLICY TO ENABLED ALL");
            tabPanel.setCloseButtonPolicy(
               EditableTabPanel.ALL_CLOSE_BUTTONS_VISIBLE_AND_ENABLED_POLICY);
         }
      }
      else if(e.getActionCommand().equals("tab2_button"))
      {
         LoggerManager.debug("bmcommon", "Tab2 button pressed!");
      }
   }
}
