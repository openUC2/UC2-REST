#ifndef parameters_wifi_h
#define parameters_wifi_h

#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <WiFiGeneric.h>
#include <WiFiMulti.h>
#include <WiFiScan.h>
#include <WiFiServer.h>
#include <WiFiSTA.h>
#include <WiFiType.h>
#include <WiFiUdp.h>

#include "WiFi.h"
#include "Update.h"
#include "WebServer.h"
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

// setting up wifi parameters
static boolean hostWifiAP = false; // set this variable if you want the ESP32 to be the host
static boolean isCaptivePortal = true; // want to autoconnect to wifi networks?


static const char PROGMEM otaindex[] = R"rawliteral(
<!DOCTYPE HTML>
<html>
<p><span style="font-family: tahoma, arial, helvetica, sans-serif;">
<script src="https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js"></script>
</span></p>
<form method="POST" action="#" enctype="multipart/form-data" id="upload_form">
<h2><span style="font-family: tahoma, arial, helvetica, sans-serif;"><strong>OTA Updater for the UC2-REST</strong></span></h2>
<p><span style="font-family: tahoma, arial, helvetica, sans-serif;">Please go to the<a href="https://github.com/openUC2/UC2-REST/" title="URL" target="_blank" rel="noopener"> Github repository </a>of the UC2-REST and download the latest ".bin" file.&nbsp;&nbsp;</span><span style="font-family: tahoma, arial, helvetica, sans-serif;"></span></p>
<p><span style="font-family: tahoma, arial, helvetica, sans-serif;"><input type="file" name="update" /> <input type="submit" value="Update" /></span></p>
</form>
<div id="prg"><span style="font-family: tahoma, arial, helvetica, sans-serif;"><em>Progress</em>: 0%</span></div>
<p><span style="font-family: tahoma, arial, helvetica, sans-serif;">
<script>
  $('form').submit(function(e){
  e.preventDefault();
  var form = $('#upload_form')[0];
  var data = new FormData(form);
   $.ajax({
  url: '/update',
  type: 'POST',
  data: data,
  contentType: false,
  processData:false,
  xhr: function() {
  var xhr = new window.XMLHttpRequest();
  xhr.upload.addEventListener('progress', function(evt) {
  if (evt.lengthComputable) {
  var per = evt.loaded / evt.total;
  $('#prg').html('progress: ' + Math.round(per*100) + '%');
  }
  }, false);
  return xhr;
  },
  success:function(d, s) {
  console.log('success!')
 },
 error: function (a, b, c) {
 }
 });
 });
 </script>
</span></p>
</html>
)rawliteral";


static const char PROGMEM swagger_index[] = R"rawliteral(
  <html>
  <head>
    <meta charset="UTF-8">
    <link rel="stylesheet" type="text/css" href="/swagger-ui.css" >
    <style>
      .topbar {
        display: none;
      }
    </style>
  </head>

  <body>
    <div id="swagger-ui"></div>
    <script src="/swagger-ui-bundle.js"> </script>
    <script src="/swagger_standalone.js"> </script>
    <script>
      window.onload = function() {
       
        const ui = SwaggerUIBundle({
          url: "/openapi.yaml",
          dom_id: '#swagger-ui',
          deepLinking: true,
          presets: [
            SwaggerUIBundle.presets.apis,
            SwaggerUIStandalonePreset
          ],
          plugins: [
            SwaggerUIBundle.plugins.DownloadUrl
          ],
          layout: "StandaloneLayout"
        })
     
        window.ui = ui
      }
  </script>
  </body>
</html>
)rawliteral";


static const char PROGMEM swagger_openapi[] = R"rawliteral(
openapi: 3.0.1
info:
  title: UC2 REST-API
  description: This is a summary of controllable UC2 modules.
  version: 1.0.0
servers:
  - url: /
tags:
  - name: Temperature
    description: Getting temperature measurements
paths:
  /motor_act:
    post:
      tags:
        - Motors
      summary: Endpoint for controlling the motor
      operationId: state_act_fct_http
      description: Move the microscope in XYZ
      requestBody:
        description: Specify the place to move to (as a point) and the thing to move (combining motion target, mount, and model if necessary)
        required: true
        content:
          application/json:
            schema:
              type: object
              required:
                - speed
                - pos1
                - pos2
                - pos3
                - isabs
                - isblock
                - isen
              properties:
                speed:
                  type: number
                pos1:
                  type: number
                pos2:
                  type: number
                pos3:
                  type: number
                isabs:
                  type: number
                isblock:
                  type: number
                isen:
                  type: number
            examples:
              moveXYZ:
                description: Move the microscope in xyz.
                summary: Move in XYZ
                value:
                  speed: 1000
                  pos1: 1000
                  pos2: 2000
                  pos3: 3000
                  isabs: 1
                  isblock: 0
                  isen: 0

      responses:
        '200':
          description: Move successfully executed
          content:
            application/json:
              example:
                message: 'Move copmlete. New position: (25, 25, 50)'
        '400':
          description: Invalid request
          content:
            application/json:
              example:
                message: 'Invalid target key: ''robot'' (target must be one of ''mount'' or ''pipette'')'
  /motor_set:
    post:
      tags:
        - Motors
      summary: Endpoint for chhanging Motor settings
      operationId: state_set_fct_http
      description: Chhange motor settings
      requestBody:
        description: Specify the place to move to (as a point) and the thing to move (combining motion target, mount, and model if necessary)
        required: true
        content:
          application/json:
            schema:
              type: object
              required:
                - axis
              properties:
                axis:
                  type: number
                maxspeed:
                  type: number
                currentposition:
                  type: number
                acceleration:
                  type: number
                pinstep:
                  type: number
                pindir:
                  type: number
                isen:
                  type: number
                accel:
                  type: number
                deccel:
                  type: number
            examples:
              setX:
                description: Set some parameters for the motor X
                summary: Change settings X
                value:
                  axis: 1
                  maxspeed: 10000
                  currentposition: 100
                  acceleration: 1000
                  pinstep: 2
                  pindir: 3
                  isen: 0
                  accel: 1000
                  deccel: 1000

      responses:
        '200':
          description: Settings successfully updated
          content:
            application/json:
              example:
                message: 'Move copmlete. New position: (25, 25, 50)'
        '400':
          description: Invalid request
          content:
            application/json:
              example:
                message: 'Invalid target key: ''robot'' (target must be one of ''mount'' or ''pipette'')'
  /motor_get:
    get:
      tags:
        - Motors
      summary: Endpoint for getting Motor settings
      operationId: state_get_fct_http
      description: Get motor settings
      requestBody:
        description: Specify the place to move to (as a point) and the thing to move (combining motion target, mount, and model if necessary)
        required: true
        content:
          application/json:
            schema:
              type: object
              required:
                - axis
              properties:
                axis:
                  type: number
            examples:
              getX:
                description: Get some parameters for the motor X
                summary: Get settings X
                value:
                  axis: 1
      responses:
        '200':
          description: Settings successfully updated
          content:
            application/json:
              example:
                message: 'Move copmlete. New position: (25, 25, 50)'
        '400':
          description: Invalid request
          content:
            application/json:
              example:
                message: 'Invalid target key: ''robot'' (target must be one of ''mount'' or ''pipette'')'
  /ledarray_act:
    post:
      tags:
        - ledarray
      summary: Endpoint for controlling the ledarray
      operationId: state_act_fct_http
      description: Manipulate the microscopes LED matrix
      requestBody:
        description: Coming soon
        required: true
        content:
          application/json:
            schema:
              type: object
              required:
                - LEDArrMode
                - arraySize
                - red
                - green
                - blue
                - indexled
                - Nleds
              properties:
                LEDArrMode:
                  type: string
                arraySize:
                  type: number
                red:
                  type: number
                green:
                  type: number
                blue:
                  type: number
                indexled:
                  type: number
                Nleds:
                  type: number
            examples:
              singleLED:
                description: Turn on single LED
                summary: "To be entered"
                value:
                  LEDArrMode: "single"
                  indexled: 45
                  red: 22
                  green: 55
                  blue: 1

      responses:
        '200':
          description: LED successfully manipulated
          content:
            application/json:
              example:
                message: 'Done'
        '400':
          description: Invalid request
          content:
            application/json:
              example:
                message: ''
        
components:
  schemas:
    act_motor:
      type: object
)rawliteral";



static const char PROGMEM swagger_standalonejs[] = R"rawliteral(
)rawliteral";

static const char PROGMEM swagger_ui_bundlejs[] = R"rawliteral(
)rawliteral";

static const char PROGMEM swagger_ui_css[] = R"rawliteral(
)rawliteral";

#endif