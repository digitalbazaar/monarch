/*
 * Copyright (c) 2005 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.common.net;

/**
 * An HttpWebRequestServicer services http web requests. The implementing class
 * should service any http web request passed to its serviceHttpWebRequest()
 * method and reply with the passed http web response.
 * 
 * @author Dave Longley
 */
public interface HttpWebRequestServicer
{
   /**
    * Services an http web request (via whatever means is appropriate) and
    * responds using the passed HttpWebResponse object.
    * 
    * @param request the http web request.
    * @param response the http web response.
    */
   public void serviceHttpWebRequest(HttpWebRequest request,
                                     HttpWebResponse response);
   
   /**
    * Allows an http web request path to be set for this servicer. This is the
    * base path http clients must request for this servicer to be given the
    * http request to service. A servicer may need to know this information.
    * 
    * @param path the servicer's path.
    */
   public void setHttpWebRequestServicerPath(String path);
}
