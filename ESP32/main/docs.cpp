/*

   Serial protocol looks like so:

   {"task": "/state_get"}
   returns:

  ++
  {"identifier_name":"UC2_Feather","identifier_id":"V0.1","identifier_date":"2022-02-04","identifier_author":"BD"}
  --

  {"task": "/state_set", "isdebug":0}
  {"task": "/state_get", "active":1}


  retrieve sensor value
  {"task": "/readsensor_act", "readsensorID":0, "N_sensor_avg":100}
  {"task": "/readsensor_get", "readsensorID":0}
  {"task": "/readsensor_set", "readsensorID":0, "readsensorPIN":34, "N_sensor_avg":10}

  setup PID controller
  {"task": "/PID_act", "PIDactive":1, "target": 500}
  {"task": "/PID_act", "PIDactive":1, "Kp":5, "Ki":.1, "Kd":.1, "target": 500, "PID_updaterate":200}
  {"task": "/readsensor_get", "readsensorID":0}
  {"task": "/readsensor_set", "readsensorID":0, "readsensorPIN":34, "N_sensor_avg":10}



  turn on the laser:
  {"task": "/laser_act", "LASERid":1, "LASERval":10000, "LASERdespeckle":100}

  move the motor
  {"task": "/motor_act", "speed":1000, "pos1":4000, "pos2":4000, "pos3":4000, "isabs":1, "isen":1}
  {"task": "/motor_act", "speed":1000, "pos1":4000, "pos2":4000, "pos3":4000, "isabs":1, "isen":1} // move in the background
  {"task": "/motor_act", "speed1":1000,"speed2":100,"speed3":5000, "pos1":4000, "pos2":4000, "pos3":4000, "isabs":1,  "isen":1}
  {"task": "/motor_act", "isstop":1}
  {'task': '/motor_set', 'axis': 1, 'currentposition': 1}
  {'task': '/motor_set', 'axis': 1, 'sign': 1} // 1 or -1
  {'task': '/motor_set', 'axis': 1, 'sign': 1} // 1 or -1
  {'task': '/motor_get', 'axis': 1}
  {"task": "/motor_act", "speed":30, "pos1":400, "pos2":0, "pos3":0, "isabs":0, "isblock":0, "isen":1}


  operate the analog out
  {"task": "/analog_act", "analogid": 1, "analogval":1000}

  operate the digital out
  {"task": "/digital_act", "digitalid": 1, "digitalval":1}

  operate the dac (e.g. lightsheet)
  {"task": "/dac_act", "dac_channel": 1, "frequency":1, "offset":0, "amplitude":0, "clk_div": 1000}
  amplitude: 0,1,2,3

  # SLM display
  {"task": "/slm_act","slmMode": "ring", "posX":100, "posY": 100, "radius":100, "color": 10000}
  {"task": "/slm_act","slmMode": "full", "color": 10000}
  {"task": "/slm_act","slmMode": "clear"}

  operate ledmatrix
  // "pattern", "individual", "full", "off", "left", "right", "top", "bottom",
  {"task": "/ledarr_act","LEDArrMode": "full", "red":100, "green": 100, "blue":100}
  {"task": "/ledarr_act","LEDArrMode": "full", "red":0, "green": 0, "blue":0}
  {'task': '/ledarr_act', 'red': 193, 'green': 193, 'blue': 193, 'NLeds':16, 'LEDArrMode': 'top'}
  {'task': '/ledarr_act', 'red': 193, 'green': 193, 'blue': 193, 'NLeds':16, 'LEDArrMode': 'bottom'}
  {'task': '/ledarr_act', 'red': 193, 'green': 193, 'blue': 193, 'NLeds':16, 'LEDArrMode': 'left'}
  {'task': '/ledarr_act', 'red': 193, 'green': 193, 'blue': 193, 'NLeds':16, 'LEDArrMode': 'right'}
  {'task': '/ledarr_act', 'red': 193, 'green': 193, 'blue': 193, 'LEDArrMode': 'full'}



  {'task': '/scanner_act', 'scannernFrames': 1, 'scannerMode': 'pattern', 'arraySize': 9, 'i': [0, 16, 32, 48, 64, 80, 96, 112, 128], 'scannerLaserVal': 32000, 'scannerExposure': 500, 'scannerDelay': 500}

  {   "red": [     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     244,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     244,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0   ],   "green": [     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     244,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     244,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0   ],   "blue": [     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     244,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     244,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0   ],   "arraySize": 64,   "LEDArrMode": "array",   "task": "/ledarr_act"   }

  // attempt to have fast triggering

  We want to send a table of tasks:

  move motor, wait, take picture cam 1/2, wait, move motor..
  {
  "task": "multitable",
  "task_n": 9,
  "repeats_n": 5,
  "0": {"task": "/motor_act", "speed":1000, "pos1":4000, "pos2":4000, "pos3":4000, "isabs":1, "isblock":1, "isen":1},
  "1": {"task": "/state_act", "delay": 100},
  "2": {"task": "/digital_act", "digitalid": 1, "digitalval":1},
  "3": {"task": "/digital_act", "digitalid": 2, "digitalval":1},
  "4": {"task": "/digital_act", "digitalid": 2, "digitalval":0},
  "5": {"task": "/digital_act", "digitalid": 1, "digitalval":0},
  "6": {"task": "/laser_act", "LASERid":1, "LASERval":10000, "LASERdespeckle":100},
  "7": {"task": "/state_act", "delay": 100},
  "8": {"task": "/laser_act", "LASERid":1, "LASERval":10000, "LASERdespeckle":100}
  }



  // trigger camera at a rate of 20hz

  {"task": "/motor_act", "speed0":0, "speed1":0,"speed2":40,"speed3":9000, "isforever":1, "isaccel":1}
  {"task": "/state_set", "isdebug":0}
  {"task": "/state_act", "delay": 100}
  {
  "task": "multitable",
  "task_n": 2,
  "repeats_n": 200,
  "0": {"task": "/digital_act", "digitalid": 1, "digitalval":-1},
  "1": {"task": "/state_act", "delay": 50}
  }
  {"task": "/motor_act", "isstop":1}
  {"task": "/motor_act", "isenable":0}
  {"task": "/s-tate_set", "isdebug":1}



// load config
  {
  "task":"/config_set", 
  "motXstp": 2,
  "motXdir": 31,
  "motYstp": 27,
  "motYdir": 16,
  "motZstp": 12,
  "motZdir": 14,
  "motAstp": 22,
  "motAdir": 21,
  "motEnable": 13,
  "ledArrPin": 17,
  "ledArrNum": 16,
  "digitalPin1":10,
  "digitalPin2":11,
  "analogPin1":0,
  "analogPin2":0,
  "analogPin3":0,
  "laserPin1":4,
  "laserPin2":15,
  "laserPin3":0,
  "dacFake1":0,
  "dacFake2":0,
  "identifier": "UC2Standalone",
  "ssid": "Blynk",
  "PW": "12345678"
  }
*/
