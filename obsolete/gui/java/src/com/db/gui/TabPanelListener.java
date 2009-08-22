/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import java.awt.Component;

/**
 * Listens to tab panels for tab panel events.
 * 
 * @author Dave Longley
 */
public interface TabPanelListener
{
   /**
    * Called when a new tab is added to a tab panel.
    * 
    * @param content the content component that was added to the tab panel.
    */
   public void tabAdded(Component content);
   
   /**
    * Called when a tab is removed from a tab panel.
    * 
    * @param content the content component that was removed from the tab panel.
    */
   public void tabRemoved(Component content);
   
   /**
    * Called when the tab selection changes in a tab panel.
    * 
    * @param oldSelection the old tab selection (can be null).
    * @param newSelection the new tab selection.
    */
   public void tabSelectionChanged(Component oldSelection,
                                   Component newSelection);
}
