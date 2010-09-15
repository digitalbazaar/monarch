/**
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */

/**
 * Test of a simple JavaScript wrapper of a C++ web server with JavaScript
 * request handlers.
 * 
 * @author David I. Lehn <dlehn@digitalbazaar.com>
 */

var Stats = function() {};

Stats.prototype.reset = function()
{
   this.start = +new Date();
   this.last = this.start;
   this.serviced = 0;
   this.contentBytes = 0;
};

Stats.prototype.service = function(bytes)
{
   if(typeof bytes === 'undefined')
   {
      bytes = 0;
   }
   this.last = +new Date();
   this.serviced++;
   this.contentBytes += bytes;
};

Stats.prototype.stats = function()
{
   var tms = this.last - this.start;
   var rate = (tms === 0) ?
      0.0 :
      this.serviced * 1000.0 / tms;

   return {
      serviced: this.serviced,
      contentBytes: this.contentBytes,
      "elapsed ms": tms,
      "req/s": rate 
   };
}

function main()
{
   // create and reset stats
   var stats = new Stats();
   stats.reset();
   
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
   ws.addHandler('/', function(ch) {
      stats.service();
   });
   
   // return a string
   ws.addHandler('/pong', function(ch) {
      stats.service(5);
      return "Pong!";
   });
   
   // return an amount of data
   ws.addHandler('/data', function(ch) {
      var data = (new Array(128 + 1)).join('.'); 
      stats.service(data.length);
      return data;
   });
   
   // return the stats
   ws.addHandler('/stats', function(ch) {
      return stats.stats();
   });
   
   // reset the stats
   ws.addHandler('/reset', function(ch) {
      stats.reset();
   });
   
   // quit the server
   ws.addHandler('/quit', function(ch) {
      ws.quit();
   });
   
   // server until ws.quit() is called
   ws.serve();
}

main();
