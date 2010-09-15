/**
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */

/**
 * Test of a simple JavaScript wrapper of a C++ web server with JavaScript
 * request handlers.
 * 
 * @author David I. Lehn <dlehn@digitalbazaar.com>
 */

function main()
{
   // basic stats
   var stats = {
      // FIXME
   };
   
   // web server configuration
   var cfg = {
      host: "localhost",
      //port: 0, // auto
      port: 19210,
      security: "off"
   };
   
   // create the web server
   var ws = monarch.test.makeSimpleWebServer();
   
   // initialize with the configuration
   ws.initialize(cfg);
   
   // add handlers
   
   // return no content
   ws.addHandler('/', function(ch) {});
   
   // return a string
   ws.addHandler('/pong', function(ch) {
      return "Pong!";
   });
   
   // return an amount of data
   ws.addHandler('/data', function(ch) {
      return (new Array(128 + 1)).join('.');
   });
   
   // return the stats
   ws.addHandler('/stats', function(ch) {
      return stats;
   });
   
   // reset the stats
   ws.addHandler('/reset', function(ch) {
      // FIXME
   });
   
   // quit the server
   ws.addHandler('/quit', function(ch) {
      ws.quit();
   });
   
   // server until ws.quit() is called
   ws.serve();
}

main();
