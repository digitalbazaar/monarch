/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.net.upnp.UPnPDevice;
import com.db.net.upnp.UPnPDeviceDiscoverer;

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
         LoggerManager.setFile(
            "upnp", "ut-upnp.log", false);
         LoggerManager.setConsoleVerbosity(
            "upnp", Logger.ERROR_VERBOSITY);
         
         LoggerManager.setFile("dbnet", "ut-upnp.log");
         LoggerManager.setConsoleVerbosity(
            "dbnet", Logger.ERROR_VERBOSITY);
         
         System.out.println("Starting UPnP functionality test...\n");
         
         System.out.println("Running UPnP device discovery...");
         UPnPDeviceDiscoverer discoverer = new UPnPDeviceDiscoverer();
         UPnPDevice[] devices = discoverer.discover();
         System.out.println("UPnP device discovery complete.");
         
         System.out.println("Discovered UPnP devices:\n");
         
         // display devices
         for(int i = 0; i < devices.length; i++)
         {
            System.out.println(devices[i].toString());
         }
         
         System.out.println("\nUPnP functionality test complete.");
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
}
