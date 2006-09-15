/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.upnp.UPnPDeviceDiscoverer;
import com.db.upnp.UPnPDiscoverResult;

/**
 * This class is used to test UPnP functionality.
 * 
 * @author Dave Longley
 */
public class UTUPnP
{
   /**
    * Runs the unit test.
    *
    * @param args the arguments.
    */
   public static void main(String[] args)
   {
      try
      {
         // set up loggers
         LoggerManager.setFile("dbupnp", "ut-upnp.log", false);
         LoggerManager.setConsoleVerbosity(
            "dbupnp", Logger.ERROR_VERBOSITY);
         
         // set up loggers
         LoggerManager.setFile("dbnet", "ut-upnp.log");
         LoggerManager.setConsoleVerbosity(
            "dbnet", Logger.ERROR_VERBOSITY);
         
         System.out.println("Starting UPnP functionality test...\n");
         
         System.out.println("Running UPnP device discovery...");
         UPnPDeviceDiscoverer discoverer = new UPnPDeviceDiscoverer();
         UPnPDiscoverResult[] results = discoverer.discover();
         System.out.println("UPnP device discovery complete.");
         
         System.out.println("Discovered UPnP devices:\n");
         
         // display results
         for(int i = 0; i < results.length; i++)
         {
            System.out.println(results[i].toString());
         }
         
         System.out.println("\nUPnP functionality test complete.");
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
}
