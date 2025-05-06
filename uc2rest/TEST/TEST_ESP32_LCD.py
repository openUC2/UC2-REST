#%%
import uc2rest
import numpy as np
import time

port = "unknown"
port = "/dev/cu.usbmodem101"

ESP32 = uc2rest.UC2Client(serialport=port, baudrate=115200, DEBUG=True, skipFirmwareCheck=False)
#ESP32.serial.sendMessage('{"task":"/home_act", "home": {"steppers": [{"stepperid":1, "timeout": 20000, "speed": 15000, "direction":1, "endposrelease":3000}]}}')

ESP_PANEL_LCD_H_RES = 800
ESP_PANEL_LCD_V_RES = 480


''' TEST LCD '''

# Create LedMatrix object, pass a reference to your “parent” that has post_json()
my_lcd = ESP32.lcd
    
my_lcd.set_timeout(.5)
    

for i in range(ESP_PANEL_LCD_H_RES//2):
    # Turn off all LEDs
    my_lcd.set_grating(i, r=255, g=255, b=255, horizontal=True)
    #time.sleep(0.1)
    
for i in range(ESP_PANEL_LCD_V_RES//2):
    # Turn off all LEDs
    my_lcd.set_grating(i, r=255, g=255, b=255, horizontal=False)
    #time.sleep(0.1)


# blacnk screen
my_lcd.clear(r=0, g=0, b=0)
my_lcd.set_color(r=255, g=255, b=255)

# have line moving vertically 
dThickness = 10
for i in range(0, ESP_PANEL_LCD_H_RES, dThickness):
    # 
    # {"task":"/lcd_act","action":"hline","x":0,"y":10,"len":800,"width":1,"r":255,"g":255,"b":0}
    # {"task": "/lcd_act", "action": "vline", "x": 0, "y": 54, "len": 480, "width": 1, "r": 255, "g": 255, "b": 255, "qid": 156}
    my_lcd.hline(0, i, ESP_PANEL_LCD_H_RES, width=dThickness)
    #time.sleep(0.1)
my_lcd.clear(r=0, g=0, b=0)    
for i in range(0, ESP_PANEL_LCD_V_RES, dThickness):
    # Turn off all LEDs
    my_lcd.vline(i, 0, ESP_PANEL_LCD_V_RES, width=dThickness)
    #time.sleep(0.1)
    
