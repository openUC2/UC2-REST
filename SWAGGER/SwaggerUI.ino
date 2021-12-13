/*********************IMPORTANT***********************************
You need to change the IP of the string by the IP of your
ESP8266 on the network.
So, just do a find and replace of all ocurrences of 192.168.1.85
by your ESP8266 IP
*****************************************************************/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
 
ESP8266WebServer server(80);
 
const char* ssid = "yourNetworkName";
const char* password =  "yourNetworkPassword";

const char * swaggerJSON = "{\"swagger\":\"2.0\",\"info\":{\"description\":\"This is a sample server Petstore server.\",\"version\":\"1.0.0\",\"title\":\"IoT application\"},\"host\":\"192.168.1.85\",\"tags\":[{\"name\":\"Temperature\",\"description\":\"Getting temperature measurements\"}],\"paths\":{\"/temperature\":{\"get\":{\"tags\":[\"Temperature\"],\"summary\":\"Endpoint for getting temperature measurements\",\"description\":\"\",\"operationId\":\"getTemperature\",\"responses\":{\"200\":{\"description\":\"A list of temperature measurements\",\"schema\":{\"$ref\":\"#/definitions/temperatureMeasurement\"}}}}}},\"definitions\":{\"temperatureMeasurement\":{\"type\":\"object\",\"properties\":{\"value\":{\"type\":\"string\"},\"timestamp\":{\"type\":\"string\"}}}}}";
const char * swaggerUI = "<!DOCTYPE html><html><head> <meta charset=\"UTF-8\"> <meta http-equiv=\"x-ua-compatible\" content=\"IE=edge\"> <title>Swagger UI</title> <link href='https://cdnjs.cloudflare.com/ajax/libs/meyer-reset/2.0/reset.min.css' media='screen' rel='stylesheet' type='text/css'/> <link href='https://cdnjs.cloudflare.com/ajax/libs/swagger-ui/2.2.10/css/screen.css' media='screen' rel='stylesheet' type='text/css'/> <script>if (typeof Object.assign !='function'){(function (){Object.assign=function (target){'use strict'; if (target===undefined || target===null){throw new TypeError('Cannot convert undefined or null to object');}var output=Object(target); for (var index=1; index < arguments.length; index++){var source=arguments[index]; if (source !==undefined && source !==null){for (var nextKey in source){if (Object.prototype.hasOwnProperty.call(source, nextKey)){output[nextKey]=source[nextKey];}}}}return output;};})();}</script> <script src='https://cdnjs.cloudflare.com/ajax/libs/jquery/1.8.0/jquery-1.8.0.min.js' type='text/javascript'></script> <script>(function(b){b.fn.slideto=function(a){a=b.extend({slide_duration:\"slow\",highlight_duration:3E3,highlight:true,highlight_color:\"#FFFF99\"},a);return this.each(function(){obj=b(this);b(\"body\").animate({scrollTop:obj.offset().top},a.slide_duration,function(){a.highlight&&b.ui.version&&obj.effect(\"highlight\",{color:a.highlight_color},a.highlight_duration)})})}})(jQuery); </script> <script>jQuery.fn.wiggle=function(o){var d={speed:50,wiggles:3,travel:5,callback:null};var o=jQuery.extend(d,o);return this.each(function(){var cache=this;var wrap=jQuery(this).wrap(' <div class=\"wiggle-wrap\"></div>').css(\"position\",\"relative\");var calls=0;for(i=1;i<=o.wiggles;i++){jQuery(this).animate({left:\"-=\"+o.travel},o.speed).animate({left:\"+=\"+o.travel*2},o.speed*2).animate({left:\"-=\"+o.travel},o.speed,function(){calls++;if(jQuery(cache).parent().hasClass('wiggle-wrap')){jQuery(cache).parent().replaceWith(cache);}if(calls==o.wiggles&&jQuery.isFunction(o.callback)){o.callback();}});}});}; </script> <script src='https://cdnjs.cloudflare.com/ajax/libs/jquery.ba-bbq/1.2.1/jquery.ba-bbq.min.js' type='text/javascript'></script> <script src='https://cdnjs.cloudflare.com/ajax/libs/handlebars.js/4.0.5/handlebars.min.js' type='text/javascript'></script> <script src='https://cdnjs.cloudflare.com/ajax/libs/lodash-compat/3.10.1/lodash.min.js' type='text/javascript'></script> <script src='https://cdnjs.cloudflare.com/ajax/libs/backbone.js/1.1.2/backbone-min.js' type='text/javascript'></script> <script>Backbone.View=(function(View){return View.extend({constructor: function(options){this.options=options ||{}; View.apply(this, arguments);}});})(Backbone.View); </script> <script src='https://cdnjs.cloudflare.com/ajax/libs/swagger-ui/2.2.10/swagger-ui.min.js' type='text/javascript'></script> <script src='https://cdnjs.cloudflare.com/ajax/libs/highlight.js/9.10.0/highlight.min.js' type='text/javascript'></script> <script src='https://cdnjs.cloudflare.com/ajax/libs/highlight.js/9.10.0/languages/json.min.js' type='text/javascript'></script> <script src='https://cdnjs.cloudflare.com/ajax/libs/json-editor/0.7.28/jsoneditor.min.js' type='text/javascript'></script> <script src='https://cdnjs.cloudflare.com/ajax/libs/marked/0.3.6/marked.min.js' type='text/javascript'></script> <script type=\"text/javascript\">$(function (){url=\"http://192.168.1.85/swagger.json\"; hljs.configure({highlightSizeThreshold: 5000}); window.swaggerUi=new SwaggerUi({url: url, dom_id: \"swagger-ui-container\", supportedSubmitMethods: ['get', 'post', 'put', 'delete', 'patch'], validatorUrl: null, onComplete: function(swaggerApi, swaggerUi){}, onFailure: function(data){log(\"Unable to Load SwaggerUI\");}, docExpansion: \"none\", jsonEditor: false, defaultModelRendering: 'schema', showRequestHeaders: false, showOperationIds: false}); window.swaggerUi.load(); function log(){if ('console' in window){console.log.apply(console, arguments);}}}); </script></head> <body class=\"swagger-section\"><div id='header'><div class=\"swagger-ui-wrap\"> <a id=\"logo\" href=\"http://swagger.io\"><img class=\"logo__img\" alt=\"swagger\" height=\"30\" width=\"30\" src=\"https://cdnjs.cloudflare.com/ajax/libs/swagger-ui/2.2.10/images/logo_small.png\"/><span class=\"logo__title\">swagger</span></a><form id='api_selector'></form></div></div><div id=\"message-bar\" class=\"swagger-ui-wrap\" data-sw-translate>&nbsp;</div><div id=\"swagger-ui-container\" class=\"swagger-ui-wrap\"></div></body></html>";
const char * answer = "[{\"value\":\"10.5\",\"timestamp\":\"22/10/2017 10:10\"},{\"value\":\"11.0\",\"timestamp\":\"22/10/2017 10:20\"}]";

void setup() {
 
    Serial.begin(115200);
    WiFi.begin(ssid, password);  //Connect to the WiFi network
 
    while (WiFi.status() != WL_CONNECTED) {  //Wait for connection
 
        delay(500);
        Serial.println("Waiting to connect...");
 
    }
 
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());  //Print the local IP
 
    server.on("/temperature", handleTemperature); 
    server.on("/swagger.json", handleSwaggerJson);
    server.on("/swaggerUI", handleSwaggerUI);

    
    server.begin(); //Start the server
    Serial.println("Server listening");
 
}
 
void loop() {
 
    server.handleClient(); //Handling of incoming requests
 
}
 
void handleTemperature() { //Handler for the body path
 
    server.send(200, "application/json", answer);

}

void handleSwaggerJson() { //Handler for the body path
 
     server.send(200, "application/json", swaggerJSON);

}

void handleSwaggerUI() { //Handler for the body path 

      server.send(200, "text/html", swaggerUI);

}