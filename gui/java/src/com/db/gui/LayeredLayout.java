/*
 * Copyright (c) 2004-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import java.awt.Component;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Vector;

/**
 * A layered layout. Plot your layout first, then add components
 * accordingly. The plot consists of a flexible grid where rows
 * and columns have weights as assigned at plot time. The later in
 * the order a coordinate is, the higher the "layer" it will be placed on.
 * This means that the last components in the plot will "overlap" the
 * previous ones.
 * 
 * @author Dave Longley
 */
public class LayeredLayout extends GridBagLayout
{
   /**
    * A mapping of grid coordinates to gridbag constraints. 
    */
   protected HashMap mGbcMap;
   
   /**
    * A vector that stores the coordinates in the order they
    * were passed to the layout manager so that components can
    * be added in the same order.
    */
   protected Vector mCoords;
   
   /**
    * An iterator that keeps track of which coordinate in the plot
    * should be filled with a component next.
    */
   protected int mCoordIterator;
   
   /**
    * The highest column in the plot.
    */
   protected int mHighCol;
   
   /**
    * The highest row in the plot.
    */
   protected int mHighRow;
   
   /**
    * Upperleft anchor.
    */
   public static final int UPPERLEFT = GridBagConstraints.NORTHWEST;
   
   /**
    * Left anchor.
    */
   public static final int LEFT = GridBagConstraints.WEST;
   
   /**
    * Right anchor.
    */
   public static final int RIGHT = GridBagConstraints.EAST;

   /**
    * Top anchor.
    */
   public static final int TOP = GridBagConstraints.NORTH;

   /**
    * Bottom anchor.
    */
   public static final int BOTTOM = GridBagConstraints.SOUTH;

   /**
    * Center anchor.
    */
   public static final int CENTER = GridBagConstraints.CENTER;

   /**
    * Default anchor.
    */
   public static final int DEFAULT_ANCHOR = GridBagConstraints.NORTHWEST;

   /**
    * Default weight.
    */
   public static final double DEFAULT_WEIGHT = 1.0;

   /**
    * Default fill.
    */
   public static final boolean DEFAULT_FILL = false;
   
   /**
    * Creates a new default LayeredLayout.
    */
   public LayeredLayout()
   {
      mCoordIterator = 0;
      initGbcMap(null);
   }   
   
   /**
    * Creates a new LayeredLayout with the given plot.
    * 
    * @param plot the plot of rows/columns/coordinates to use.
    */
   public LayeredLayout(int[][] plot)
   {
      mCoordIterator = 0;
      initGbcMap(plot);
   }
   
   /**
    * Gets the constraints, given coordinates.
    * 
    * @param coords the coordinates (col, row);
    * @return the constraints for the coordinate or null, if none found.
    */
   protected GridBagConstraints getConstraints(int[] coords)
   {
      String str = coords[0] + "," + coords[1];
      return (GridBagConstraints)mGbcMap.get(str);
   }
   
   /**
    * Puts the constraints for coordinates into the gbc map.
    * 
    * @param coords the coordinates (col, row);
    * @param gbc the constraints.
    */
   protected void putConstraints(int[] coords, GridBagConstraints gbc)
   {
      String str = coords[0] + "," + coords[1];
      mGbcMap.put(str, gbc);
   }
   
   /**
    * Converts a string of coords into an integer array of coords.
    * 
    * @param str the string of coords to convert.
    * @return the integer array of coords.
    */
   public int[] convertCoordsStr(String str)
   {
      int[] coords = new int[2];
      
      String[] split = str.split(",");
      try
      {
         coords[0] = Integer.parseInt(split[0]);
         coords[1] = Integer.parseInt(split[1]);
      }
      catch(Exception e)
      {
      }
      
      return coords;
   }
   
   /**
    * Initializes the gridbag constraints map based on a set of
    * grid coordinates.
    * 
    * @param plot the plot that constitutes a set of grid coordinates.
    */
   protected void initGbcMap(int[][] plot)
   {
      mGbcMap = new HashMap();
      mCoords = new Vector();
      mHighCol = 0;
      mHighRow = 0;
      
      if(plot != null)
      {
         int count = plot.length;
         for(int i = 0; i < count; i++) 
         {
            int col = plot[i][0];
            int row = plot[i][1];
            
            // get the highest column
            if(col > mHighCol)
               mHighCol = col;
            
            // get the highest row
            if(row > mHighRow)
               mHighRow = row;
            
            // add an entry to the map
            putConstraints(new int[]{col, row}, new GridBagConstraints());
            
            // add the entry to the vector
            mCoords.add(new int[]{col, row});
         }
         
         // update the constraints for all of the coordinates
         updateConstraints();
      }
   }
   
   /**
    * Determines a dimension for a given coordinate.
    * 
    * @param col the column for the coordinate.
    * @param row the row for the coordinate.
    * @param dim the dimension to determine (0 for width, 1 for height).
    * @return the dimension.
    */
   protected int determineDimension(int col, int row, int dim)
   {
      int rval = 1;

      // the value to compare with
      int value = (dim == 0) ? col : row;
      
      // get precedence and previous coords
      int prev = -1;
      int overlap = -1;
      boolean foundCoords = false;
      Iterator i = mCoords.iterator();
      while(i.hasNext())
      {
         int[] coords = (int[])i.next();
         
         // find the previous coord
         if(value > coords[dim] && coords[dim] > prev)
            prev = coords[dim];
         
         // try to find the overlap coord
         if(foundCoords)
         {
            // if another coord is plotted after, it overlaps
            if(coords[dim] > value)
            {
               if((dim == 0 && coords[1] <= row) ||
                  (dim == 1 && coords[0] <= col))
               {
                  if(overlap == -1 || coords[dim] < overlap)
                     overlap = coords[dim];
               }
            }
         }
         
         // found the current coordinates
         if(coords[0] == col && coords[1] == row)
            foundCoords = true;
      }
      
      // if another coord overlaps, set return value to difference
      if(overlap != -1)
         rval = overlap - value;
      else
         rval = (dim == 0) ? (mHighCol - prev) : (mHighRow - prev);
         
      return rval;
   }
   
   /**
    * Determines the row width for a given coordinate.
    * 
    * @param col the column.
    * @param row the row.
    * @return the row width.
    */
   protected int determineRowWidth(int col, int row)
   {
      return determineDimension(col, row, 0);
   }

   /**
    * Determines the column height for a given coordinate.
    * 
    * @param col the column.
    * @param row the row.
    * @return the column height.
    */
   protected int determineColHeight(int col, int row)   
   {
      return determineDimension(col, row, 1);
   }
   
   /**
    * Updates the constraints for a given coordinate.
    * 
    * @param coord the coordinate to update the constraints for.
    */
   protected void updateConstraints(int[] coord)
   {
      int col = coord[0];
      int row = coord[1];
      
      GridBagConstraints gbc = getConstraints(coord);
      if(gbc != null)
      {
         // set the grid column constraints
         gbc.gridx = col;
         gbc.gridwidth = determineRowWidth(col, row);
         
         // set the grid row constraints
         gbc.gridy = row;
         gbc.gridheight = determineColHeight(col, row);
      }
   }
   
   /**
    * Inserts a value into an integer vector that is sorted from low to high.
    * 
    * @param v the vector to insert the value into.
    * @param value the value to insert.
    */
   protected void insertValue(Vector v, int value)
   {
      if(v.isEmpty())
         v.add(new Integer(value));
      else
      {
         boolean inserted = false;
         int size = v.size();
         for(int i = 0; i < size; i++)
         {
            int num = ((Integer)v.get(i)).intValue();
            if(value < num)
            {
               v.insertElementAt(new Integer(value), i);
               inserted = true;
               break;
            }
         }
         
         if(!inserted)
            v.add(new Integer(value));
      }
   }
   
   /**
    * Updates the gridbag constraints for every coordinate in the
    * gridbag constraint map. 
    */
   public void updateConstraints()
   {
      // set column and row constraints
      Iterator i = mGbcMap.keySet().iterator();
      while(i.hasNext())
      {
         int[] coords = convertCoordsStr((String)i.next());
         updateConstraints(coords);
      }
   }
   
   /**
    * Places the next component in this layout.
    * 
    * @param c the component to place.
    */
   public void placeNext(Component c)
   {
      placeNext(c, DEFAULT_WEIGHT, DEFAULT_WEIGHT,
                DEFAULT_FILL, DEFAULT_FILL, DEFAULT_ANCHOR, null);
   }
   
   /**
    * Places the next component in this layout.
    * 
    * @param c the component to add.
    * @param anchor where to anchor the component.
    */
   public void placeNext(Component c, int anchor)   
   {
      placeNext(c, DEFAULT_WEIGHT, DEFAULT_WEIGHT,
                DEFAULT_FILL, DEFAULT_FILL, anchor, null);
   }
   
   /**
    * Places the next component in this layout.
    * 
    * @param c the component to place.
    * @param hf whether or not to fill the component's horizontal space.
    * @param vf whether or not to fill the component's vertical space.
    */
   public void placeNext(Component c, boolean hf, boolean vf)   
   {
      placeNext(c, DEFAULT_WEIGHT, DEFAULT_WEIGHT,
                hf, vf, DEFAULT_ANCHOR, null);
   }
   
   /**
    * Places the next component in this layout.
    * 
    * @param c the component to place.
    * @param insets the insets for the component.
    */
   public void placeNext(Component c, Insets insets)   
   {
      placeNext(c, DEFAULT_WEIGHT, DEFAULT_WEIGHT,
                DEFAULT_FILL, DEFAULT_FILL, DEFAULT_ANCHOR, insets);
   }   
   
   /**
    * Places the next component in this layout.
    * 
    * @param c the component to place.
    * @param hf whether or not to fill the component's horizontal space.
    * @param vf whether or not to fill the component's vertical space.
    * @param insets the insets for the component.
    */
   public void placeNext(Component c, boolean hf, boolean vf, Insets insets)   
   {
      placeNext(c, DEFAULT_WEIGHT, DEFAULT_WEIGHT,
                hf, vf, DEFAULT_ANCHOR, insets);
   }
   
   /**
    * Places the next component in this layout.
    * 
    * @param c the component to place.
    * @param hw the component's horizontal weight (0-1.0).
    * @param vw the component's vertical weight (0-1.0).
    */
   public void placeNext(Component c, double hw, double vw)   
   {
      placeNext(c, hw, vw, DEFAULT_FILL, DEFAULT_FILL, DEFAULT_ANCHOR, null); 
   }
   
   /**
    * Places the next component in this layout.
    * 
    * @param c the component to place.
    * @param hw the component's horizontal weight (0-1.0).
    * @param vw the component's vertical weight (0-1.0).
    * @param insets the insets for the component.
    */
   public void placeNext(Component c, double hw, double vw, Insets insets)   
   {
      placeNext(c, hw, vw, DEFAULT_FILL, DEFAULT_FILL, DEFAULT_ANCHOR, insets);
   }
   
   /**
    * Places the next component in this layout.
    * 
    * @param c the component to place.
    * @param hw the component's horizontal weight (0-1.0).
    * @param vw the component's vertical weight (0-1.0).
    * @param anchor where to anchor the component.
    * @param insets the insets for the component.
    */
   public void placeNext(Component c, double hw, double vw, int anchor,
                         Insets insets)   
   {
      placeNext(c, hw, vw, DEFAULT_FILL, DEFAULT_FILL, anchor, insets);
   }   
   
   /**
    * Places the next component in this layout.
    * 
    * @param c the component to place.
    * @param hw the component's horizontal weight (0-1.0).
    * @param vw the component's vertical weight (0-1.0).
    * @param hf whether or not to fill the component's horizontal space.
    * @param vf whether or not to fill the component's vertical space.
    */
   public void placeNext(Component c, double hw, double vw,
                         boolean hf, boolean vf)   
   {
      placeNext(c, hw, vw, hf, vf, DEFAULT_ANCHOR, null);
   }
   
   /**
    * Places the next component in this layout.
    * 
    * @param c the component to add.
    * @param hf whether or not to fill the component's horizontal space.
    * @param vf whether or not to fill the component's vertical space.
    * @param anchor where to anchor the component.
    */
   public void placeNext(Component c, boolean hf, boolean vf, int anchor)   
   {
      placeNext(c, DEFAULT_WEIGHT, DEFAULT_WEIGHT,
                hf, vf, anchor, null);
   }
   
   /**
    * Places the next component in this layout.
    * 
    * @param c the component to add.
    * @param hf whether or not to fill the component's horizontal space.
    * @param vf whether or not to fill the component's vertical space.
    * @param anchor where to anchor the component.
    * @param insets the insets for the component.
    */
   public void placeNext(Component c, boolean hf, boolean vf, int anchor,
                         Insets insets)   
   {
      placeNext(c, DEFAULT_WEIGHT, DEFAULT_WEIGHT,
                hf, vf, anchor, insets);
   }
   
   /**
    * Places the next component in this layout.
    * 
    * @param c the component to add.
    * @param hw the component's horizontal weight (0-1.0).
    * @param vw the component's vertical weight (0-1.0).
    * @param anchor where to anchor the component.
    */
   public void placeNext(Component c, double hw, double vw, int anchor)   
   {
      placeNext(c, hw, vw, DEFAULT_FILL, DEFAULT_FILL, anchor, null);
   }
   
   /**
    * Places the next component in this layout.
    * 
    * @param c the component to add.
    * @param hw the component's horizontal weight (0-1.0).
    * @param vw the component's vertical weight (0-1.0).
    * @param hf whether or not to fill the component's horizontal space.
    * @param vf whether or not to fill the component's vertical space.
    * @param anchor where to anchor the component.
    */
   public void placeNext(Component c,
                         double hw, double vw, boolean hf, boolean vf,
                         int anchor)     
   {
      placeNext(c, hw, vw, hf, vf, anchor, null);
   }
   
   /**
    * Places the next component in this layout.
    * 
    * @param c the component to add.
    * @param hw the component's horizontal weight (0-1.0).
    * @param vw the component's vertical weight (0-1.0).
    * @param hf whether or not to fill the component's horizontal space.
    * @param vf whether or not to fill the component's vertical space.
    * @param insets the insets for the component.
    */
   public void placeNext(Component c,
                         double hw, double vw,
                         boolean hf, boolean vf, Insets insets)   
   {
      placeNext(c, hw, vw, hf, vf, DEFAULT_ANCHOR, insets);
   }
   
   /**
    * Places the next component in this layout.
    * 
    * @param c the component to add.
    * @param anchor where to anchor the component.
    * @param insets the insets for the component.
    */
   public void placeNext(Component c, int anchor, Insets insets)   
   {
      placeNext(c, DEFAULT_WEIGHT, DEFAULT_WEIGHT,
                DEFAULT_FILL, DEFAULT_FILL, anchor, insets);
   }
   
   /**
    * Places the next component in this layout.
    * 
    * @param c the component to add.
    * @param hw the component's horizontal weight (0-1.0).
    * @param vw the component's vertical weight (0-1.0).
    * @param hf whether or not to fill the component's horizontal space.
    * @param vf whether or not to fill the component's vertical space.
    * @param anchor where to anchor the component.
    * @param insets the insets for the component.
    */
   public void placeNext(Component c,
                         double hw, double vw, boolean hf, boolean vf,
                         int anchor, Insets insets)   
   {
      // see if iterator should be started/restarted
      if(mCoordIterator > mCoords.size())
      {
         mCoordIterator = 0;
      }
      
      if(mCoordIterator < mCoords.size())
      {
         int[] coord = (int[])mCoords.get(mCoordIterator);
         place(c, coord, hw, vw, hf, vf, anchor, insets);
      }
   }
   
   /**
    * Places a component in this layout.
    * 
    * @param c the component to place.
    * @param coords the component's coordinates (col, row).
    */
   public void place(Component c, int[] coords)   
   {
      place(c, coords, DEFAULT_WEIGHT, DEFAULT_WEIGHT,
            DEFAULT_FILL, DEFAULT_FILL, DEFAULT_ANCHOR, null);
   }   
   
   /**
    * Places a component in this layout.
    * 
    * @param c the component to place.
    * @param coords the component's coordinates (col, row).
    * @param anchor where to anchor the component.
    */
   public void place(Component c, int[] coords, int anchor)    
   {
      place(c, coords, DEFAULT_WEIGHT, DEFAULT_WEIGHT,
            DEFAULT_FILL, DEFAULT_FILL, anchor, null);
   }
   
   /**
    * Places a component in this layout.
    * 
    * @param c the component to place.
    * @param coords the component's coordinates (col, row).
    * @param insets the insets for the component.
    */
   public void place(Component c, int[] coords, Insets insets)     
   {
      place(c, coords, DEFAULT_WEIGHT, DEFAULT_WEIGHT,
            DEFAULT_FILL, DEFAULT_FILL, DEFAULT_ANCHOR, insets);
   }   
   
   /**
    * Places a component in this layout.
    * 
    * @param c the component to place.
    * @param coords the component's coordinates (col, row).
    * @param hf whether or not to fill the component's horizontal space.
    * @param vf whether or not to fill the component's vertical space.
    */
   public void place(Component c, int[] coords, boolean hf, boolean vf)   
   {
      place(c, coords, DEFAULT_WEIGHT, DEFAULT_WEIGHT,
            hf, vf, DEFAULT_ANCHOR, null);
   }
   
   /**
    * Places a component in this layout.
    * 
    * @param c the component to place.
    * @param coords the component's coordinates (col, row).
    * @param hf whether or not to fill the component's horizontal space.
    * @param vf whether or not to fill the component's vertical space.
    * @param insets the insets for the component.
    */
   public void place(Component c, int[] coords,
                     boolean hf, boolean vf, Insets insets)     
   {
      place(c, coords, DEFAULT_WEIGHT, DEFAULT_WEIGHT,
            hf, vf, DEFAULT_ANCHOR, insets);
   }
   
   /**
    * Places a component in this layout.
    * 
    * @param c the component to place.
    * @param coords the component's coordinates (col, row).
    * @param hw the component's horizontal weight (0-1.0).
    * @param vw the component's vertical weight (0-1.0).
    */
   public void place(Component c, int[] coords, double hw, double vw)   
   {
      place(c, coords, hw, vw, DEFAULT_FILL, DEFAULT_FILL,
            DEFAULT_ANCHOR, null);
   }
   
   /**
    * Places a component in this layout.
    * 
    * @param c the component to place.
    * @param coords the component's coordinates (col, row).
    * @param hw the component's horizontal weight (0-1.0).
    * @param vw the component's vertical weight (0-1.0).
    * @param insets the insets for the component.
    */
   public void place(Component c, int[] coords,
                     double hw, double vw, Insets insets)   
   {
      place(c, coords, hw, vw, DEFAULT_FILL, DEFAULT_FILL,
            DEFAULT_ANCHOR, insets);
   }
   
   /**
    * Places a component in this layout.
    * 
    * @param c the component to place.
    * @param coords the component's coordinates (col, row).
    * @param hw the component's horizontal weight (0-1.0).
    * @param vw the component's vertical weight (0-1.0).
    * @param hf whether or not to fill the component's horizontal space.
    * @param vf whether or not to fill the component's vertical space.
    */
   public void place(Component c, int[] coords,
                     double hw, double vw, boolean hf, boolean vf)
   {
      place(c, coords, hw, vw, hf, vf, DEFAULT_ANCHOR, null);
   }
   
   /**
    * Places a component in this layout.
    * 
    * @param c the component to place.
    * @param coords the component's coordinates (col, row).
    * @param hf whether or not to fill the component's horizontal space.
    * @param vf whether or not to fill the component's vertical space.
    * @param anchor where to anchor the component.
    */
   public void place(Component c, int[] coords,
                     boolean hf, boolean vf, int anchor)    
   {
      place(c, coords, DEFAULT_WEIGHT, DEFAULT_WEIGHT,
            hf, vf, anchor, null);
   }
   
   /**
    * Places a component in this layout.
    * 
    * @param c the component to place.
    * @param coords the component's coordinates (col, row).
    * @param anchor where to anchor the component.
    * @param insets the insets for the component.
    */
   public void place(Component c, int[] coords, int anchor, Insets insets)   
   {
      place(c, coords, DEFAULT_WEIGHT, DEFAULT_WEIGHT,
            DEFAULT_FILL, DEFAULT_FILL, anchor, insets);
   }
   
   /**
    * Places a component in this layout.
    * 
    * @param c the component to place.
    * @param coords the component's coordinates (col, row).
    * @param hf whether or not to fill the component's horizontal space.
    * @param vf whether or not to fill the component's vertical space.
    * @param anchor where to anchor the component.
    * @param insets the insets for the component.
    */
   public void place(Component c, int[] coords,
                     boolean hf, boolean vf, int anchor, Insets insets)   
   {
      place(c, coords, DEFAULT_WEIGHT, DEFAULT_WEIGHT,
            hf, vf, anchor, insets);
   }
   
   /**
    * Places a component in this layout.
    * 
    * @param c the component to place.
    * @param coords the component's coordinates (col, row).
    * @param hw the component's horizontal weight (0-1.0).
    * @param vw the component's vertical weight (0-1.0).
    * @param anchor where to anchor the component.
    */
   public void place(Component c, int[] coords,
                     double hw, double vw, int anchor)   
   {
      place(c, coords, hw, vw, DEFAULT_FILL, DEFAULT_FILL, anchor);
   }
   
   /**
    * Places a component in this layout.
    * 
    * @param c the component to place.
    * @param coords the component's coordinates (col, row).
    * @param hw the component's horizontal weight (0-1.0).
    * @param vw the component's vertical weight (0-1.0).
    * @param insets the insets for the component.
    * @param anchor where to anchor the component.
    */
   public void place(Component c, int[] coords,
                     double hw, double vw, int anchor, Insets insets)   
   {
      place(c, coords, hw, vw, DEFAULT_FILL, DEFAULT_FILL, anchor, insets);
   }   
   
   /**
    * Places a component in this layout.
    * 
    * @param c the component to place.
    * @param coords the component's coordinates (col, row).
    * @param hw the component's horizontal weight (0-1.0).
    * @param vw the component's vertical weight (0-1.0).
    * @param hf whether or not to fill the component's horizontal space.
    * @param vf whether or not to fill the component's vertical space.
    * @param anchor where to anchor the component.
    */
   public void place(Component c, int[] coords,
                     double hw, double vw, boolean hf, boolean vf, int anchor)   
   {
      place(c, coords, hw, vw, hf, vf, anchor, null);
   }
   
   /**
    * Places a component in this layout.
    * 
    * @param c the component to place.
    * @param coords the component's coordinates (col, row).
    * @param hw the component's horizontal weight (0-1.0).
    * @param vw the component's vertical weight (0-1.0).
    * @param hf whether or not to fill the component's horizontal space.
    * @param vf whether or not to fill the component's vertical space.
    * @param insets the insets for the component.
    */
   public void place(Component c, int[] coords,
                     double hw, double vw, boolean hf, boolean vf,
                     Insets insets)   
   {
      place(c, coords, hw, vw, hf, vf, DEFAULT_ANCHOR, insets);
   }
   
   /**
    * Places a component in this layout.
    * 
    * @param c the component to place.
    * @param coords the component's coordinates (col, row).
    * @param hw the component's horizontal weight (0-1.0).
    * @param vw the component's vertical weight (0-1.0).
    * @param hf whether or not to fill the component's horizontal space.
    * @param vf whether or not to fill the component's vertical space.
    * @param anchor where to anchor the component.
    * @param insets the insets for the component.
    */
   public void place(Component c, int[] coords,
                     double hw, double vw, boolean hf, boolean vf,
                     int anchor, Insets insets)
   {
      GridBagConstraints gbc = getConstraints(coords);
      if(gbc == null)
      {
         if(coords[0] > mHighCol)
         {
            mHighCol = coords[0];
         }
         
         if(coords[1] > mHighRow)
         {
            mHighRow = coords[1];
         }
         
         gbc = new GridBagConstraints();
         putConstraints(coords, gbc);
         mCoords.add(coords);
         updateConstraints();
      }
      
      // find which coordinate in the list this is to set component iterator
      for(int i = 0; i < mCoords.size(); i++)
      {
         int[] value = (int[])mCoords.get(i);
         if(value[0] == coords[0] && value[1] == coords[1])
         {
            mCoordIterator = i + 1;
            break;
         }
      }
      
      gbc.weightx = hw * 100;
      gbc.weighty = vw * 100;
      
      if(hf && vf)
      {
         gbc.fill = GridBagConstraints.BOTH;
      }
      else if(hf)
      {
         gbc.fill = GridBagConstraints.HORIZONTAL;
      }
      else if(vf)
      {
         gbc.fill = GridBagConstraints.VERTICAL;
      }
      else
      {
         gbc.fill = GridBagConstraints.NONE;
      }
      
      gbc.anchor = anchor;
      
      if(insets == null)
      {
         insets = new Insets(0, 0, 0, 0);
      }
      
      gbc.insets = insets;
      
      setConstraints(c, gbc);
   }
   
   /**
    * Prints debugging information for this entire layout to the console.
    */
   public void debug()
   {
      Iterator i = mGbcMap.keySet().iterator();
      while(i.hasNext())
      {
         int[] coords = convertCoordsStr((String)i.next());
         
         System.out.println("coords: " + coords[0] + "," + coords[1]);
         GridBagConstraints gbc = getConstraints(coords);
         
         System.out.println("gridx: " + gbc.gridx);
         System.out.println("gridy: " + gbc.gridy);
         System.out.println("gridwidth: " + gbc.gridwidth);
         System.out.println("gridheight: " + gbc.gridheight);
         System.out.println("x weight: " + gbc.weightx);
         System.out.println("y weight: " + gbc.weighty);
         System.out.println("");
      }
   }
   
   /**
    * Print the next set of debugging information to the console.
    */
   public void debugNext()
   {
      int[] coords = new int[]{0, 0};
      
      if(mCoords.size() > 0 && mCoordIterator > mCoords.size())
      {
         coords = (int[])mCoords.get(0);
      }
      else if(mCoordIterator < mCoords.size())
      {
         coords = (int[])mCoords.get(mCoordIterator);
      }
      
      System.out.println("coords: " + coords[0] + "," + coords[1]);
      GridBagConstraints gbc = getConstraints(coords);

      System.out.println("gridx: " + gbc.gridx);
      System.out.println("gridy: " + gbc.gridy);
      System.out.println("gridwidth: " + gbc.gridwidth);
      System.out.println("gridheight: " + gbc.gridheight);
      System.out.println("x weight: " + gbc.weightx);
      System.out.println("y weight: " + gbc.weighty);
      System.out.println("");
   }
}
