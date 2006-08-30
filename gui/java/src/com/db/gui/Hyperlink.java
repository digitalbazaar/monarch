/*
 * Copyright (c) 2003-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;

import java.net.URL;

import java.util.Iterator;
import java.util.Vector;

import javax.swing.JEditorPane;
import javax.swing.UIManager;
import javax.swing.event.HyperlinkEvent;
import javax.swing.event.HyperlinkListener;

/**
 * A visual hyperlink.
 * 
 * @author Dave Longley
 */
public class Hyperlink extends JEditorPane implements HyperlinkListener
{
   /**
    * The action command for this hyperlink.
    */
   protected String mActionCommand;
   
   /**
    * The text to display on the hyperlink.
    */
   protected String mText;
   
   /**
    * The color of the text to display.
    */
   protected Color mTextColor;
   
   /**
    * The url for the hyperlink.
    */
   protected String mUrl;
   
   /**
    * The action listeners for this hyperlink.
    */
   protected Vector mActionListeners;
   
   /**
    * Creates a new default hyperlink.
    */
   public Hyperlink()
   {
      this("", "");
   }
   
   /**
    * Creates a new hyperlink with the given text.
    * 
    * @param text the text for the hyperlink.
    */
   public Hyperlink(String text)
   {
      this(text, "");
   }
   
   /**
    * Creates a new hyperlink with the given text and url.
    * 
    * @param text the url text to display.
    * @param url the actual url.
    */
   public Hyperlink(String text, String url)
   {
      setBackground(UIManager.getColor("Label.background"));
      setContentType("text/html");
      setFont(UIManager.getFont("Label.font"));
      
      addHyperlinkListener(this);
      setEditable(false);
      mActionListeners = new Vector();

      mActionCommand = "";
      setPage(text, url);
   }
   
   /**
    * Fires an action event to all action listeners.
    * 
    * @param e the action event to fire. 
    */
   protected void fireActionEvent(ActionEvent e)
   {
      Iterator i = mActionListeners.iterator();
      while(i.hasNext())
      {
         ActionListener al = (ActionListener)i.next();
         al.actionPerformed(e);
      }
   }
   
   /**
    * Creates the html from the text and url.
    * 
    * @return the html with the link.
    */
   protected String createHtml()
   {
      String html = "<html>";
      html += "<font";
      html += " face=\"" + getFont().getFontName();
      html += " size=\"" + (getFont().getSize()) + "\"";
      html += ">";
      html += "<a href=\"" + getUrl() + "\">" + mText + "</a>";
      html += "</font>";
      html += "</html>";
      
      return html;
   }
   
   /**
    * Turn on anti-aliasing.
    * 
    * @param g the graphics to paint with.
    */
   public void paint(Graphics g)
   {
      if(g instanceof Graphics2D)
      {
         Graphics2D g2 = (Graphics2D)g;
         
         // turn on text anti-aliasing
         g2.setRenderingHint(
            RenderingHints.KEY_TEXT_ANTIALIASING,
            RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
      }
      
      // call super class paint
      super.paint(g);
   }
   
   /**
    * Adds an action listener.
    * 
    * @param al the action listener to add.
    */
   public void addActionListener(ActionListener al)
   {
      if(al != null)
         mActionListeners.add(al);
   }
   
   /**
    * Removes an action listener.
    * 
    * @param al the action listener to remove.
    */
   public void removeActionListener(ActionListener al)
   {
      if(al != null)
         mActionListeners.remove(al);
   }
   
   /**
    * Sets the action command for this hyperlink.
    * 
    * @param actionCommand the action command for this hyperlink.
    */
   public void setActionCommand(String actionCommand)
   {
      mActionCommand = actionCommand;
   }
   
   /**
    * Called when the hyperlink gets clicked. Creates an action event
    * and passes it to listeners.
    * 
    * @param e the hyperlink event.
    */
   public void hyperlinkUpdate(HyperlinkEvent e)
   {
      if(e.getSource() == this)
      {
         if(e.getEventType() == HyperlinkEvent.EventType.ACTIVATED)
         {
            ActionEvent ae = new ActionEvent(this, 0, mActionCommand);
            fireActionEvent(ae);
         }
      }
   }
   
   /**
    * Overrides set page.
    * 
    * @param text the text to display for the link.
    */
   public void setPage(String text)
   {
      mText = text;
      setText(text);
   }
   
   /**
    * Overrides set page.
    * 
    * @param text the text to display for the link.
    * @param url the actual url.
    */
   public void setPage(String text, String url)
   {
      mUrl = url;
      setToolTipText(url);
      setText(text);
   }
   
   /**
    * Overrides set page so it does nothing.
    * 
    * @param page the page to do nothing with.
    */
   public void setPage(URL page)
   {
   }
   
   /**
    * Sets the text for the hyperlink.
    * 
    * @param text the text to display for the hyperlink.
    */
   public void setText(String text)
   {
      mText = text;
      super.setText(createHtml());
   }   
   
   /**
    * Gets the text for the hyperlink.
    * 
    * @return the text for the hyperlink.
    */
   public String getText()
   {
      return mText;
   }

   /**
    * Sets the text color for the hyperlink.
    * 
    * @param color the text color to display for the hyperlink.
    */
   public void setTextColor(Color color)
   {
      mTextColor = color;
      super.setText(createHtml());
   }
   
   /**
    * Gets the text color for the hyperlink.
    * 
    * @return the text color for the hyperlink.
    */
   public Color getTextColor()
   {
      return mTextColor;
   }
   
   /**
    * Sets the url for the hyperlink.
    * 
    * @param url the url for the hyperlink.
    */
   public void setUrl(String url)
   {
      mUrl = url;
      setToolTipText(url);
      super.setText(createHtml());
   }
   
   /**
    * Gets the url for the hyperlink.
    * 
    * @return the url for the hyperlink.
    */
   public String getUrl()
   {
      return mUrl;
   }
}
