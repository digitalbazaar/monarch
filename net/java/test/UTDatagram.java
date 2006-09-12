/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
import java.net.DatagramPacket;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.net.datagram.DatagramClient;
import com.db.net.datagram.DatagramStream;

/**
 * This class is used to test Datagram functionality.
 * 
 * @author Dave Longley
 */
public class UTDatagram
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
            "dbnet", "ut-datagram.log", false);
         LoggerManager.setConsoleVerbosity(
            "dbnet", Logger.ERROR_VERBOSITY);
         
         System.out.println("Starting Datagram functionality test...");
         
         // create a datagram client
         DatagramClient client = new DatagramClient();
         DatagramStream stream = client.getMulticastStream(
            1901, "239.255.255.250", 1900, false);
         
         //String searchTarget = "upnp:rootdevice";
         String searchTarget = "ssdp:all";
         String CRLF = "\r\n";
         String mx = "3";
         StringBuffer packet = new StringBuffer();
         packet.append("M-SEARCH * HTTP/1.1" + CRLF);
         packet.append("HOST: 239.255.255.250:1900" + CRLF);
         packet.append("MAN: \"ssdp:discover\"" + CRLF);
         packet.append("MX: ").append(mx).append(CRLF);
         packet.append("ST: ").append(searchTarget).append(CRLF + CRLF);
         String toSend = packet.toString();
         byte[] pk = toSend.getBytes();
         stream.sendDatagram(new DatagramPacket(pk, pk.length));
         
         stream.setReadTimeout(10000);
         
         byte[] buffer = new byte[1024];
         DatagramPacket datagram2 = new DatagramPacket(buffer, buffer.length);
         stream.receiveDatagram(datagram2);
         
         String data = new String(
            datagram2.getData(), datagram2.getOffset(), datagram2.getLength());
         System.out.println("address=" + datagram2.getAddress());
         System.out.println("port=" + datagram2.getPort());
         System.out.println("data=" + data);
         
         buffer = new byte[1024];
         datagram2 = new DatagramPacket(buffer, buffer.length);
         stream.receiveDatagram(datagram2);
         
         data = new String(
            datagram2.getData(), datagram2.getOffset(), datagram2.getLength());
         System.out.println("address=" + datagram2.getAddress());
         System.out.println("port=" + datagram2.getPort());
         System.out.println("data=" + data);
         
         System.out.println("Datagram functionality test complete.");
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
}
