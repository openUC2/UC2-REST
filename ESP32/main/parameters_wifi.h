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
boolean hostWifiAP = false; // set this variable if you want the ESP32 to be the host
boolean isCaptivePortal = true; // want to autoconnect to wifi networks?
const char* mSSID = "BenMur";//"UC2 - F8Team"; //"IPHT - Konf"; // "Blynk";
const char* mPASSWORD = "MurBen3128"; //"_lachmannUC2"; //"WIa2!DcJ"; //"12345678";
const char* mSSIDAP = "UC2";
const char* hostname = "youseetoo";




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
