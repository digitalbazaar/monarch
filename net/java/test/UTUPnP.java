/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
import com.db.logging.Logger;
import com.db.logging.LoggerManager;
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
         
         UPnPDeviceDiscoverer discoverer = new UPnPDeviceDiscoverer();
         discoverer.discover();
         
         // FIXME:
         
         System.out.println("UPnP functionality test complete.");
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
}
