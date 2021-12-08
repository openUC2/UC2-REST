#%%
from espclient import ESP32Client
import time
#%%
host = '192.168.137.96'
esp32 = ESP32Client(host, port=80)

#%%


esp32.move_z(steps=500,  speed=100)
#esp32.move_z(steps=-500,  speed=100)

#%%
import time 
esp32.set_led((0,0,0))
esp32.is_filter_init = True
esp32.set_laser('B', 10000, True)
time.sleep(2)
#%%
#esp32.is_filter_init = False
for i in range(2):
    esp32.set_laser('B', 0, False)
    esp32.set_laser('R', 1000, True)
    time.sleep(2)
    esp32.set_laser('R', 000, False)
    esp32.set_laser('B', 10000, True)
    time.sleep(2)
#%%
t_laser_on = 1
t_filter_move = 5
while 1:
    esp32.move_filter(-700, 20)
    time.sleep(t_filter_move)
    esp32.set_laser('B', 10000)
    time.sleep(t_laser_on)
    esp32.set_laser('B', 0)

    esp32.move_filter(700, 20)
    time.sleep(t_filter_move)
    esp32.set_laser('R', 10000)
    time.sleep(t_laser_on)
    esp32.set_laser('R', 0)

#%%

import time
esp32.set_laser('R', 10000)
time.sleep(1)
esp32.set_laser('R', 10000)
esp32.move_filter(-700, speed=20)
time.sleep(3)

esp32.set_laser('B', 10000)
time.sleep(1)
esp32.set_laser('B', 10000)
esp32.move_filter(700, speed=20)
time.sleep(3)
#%%
import time 

esp32.is_filter_init=False
while(True):
    esp32.set_laser('B', 10000, True)
    time.sleep(2)
    esp32.set_laser('R', 10000, True)
    time.sleep(2)
#%%
#esp32.move_filter(-2000,20)
esp32.move_filter(400,20)
#%%

esp32.set_laser('R', 0000, False)
esp32.move_filter(800, speed=20)
esp32.move_filter(-50, speed=20)
esp32.set_laser('B', 10000, False)

#%%
esp32.set_laser('B', 0000, False)
esp32.move_filter(-800, speed=20)
esp32.move_filter(50, speed=20)
esp32.set_laser('R', 10000, False)

#%%
esp32.set_led((255,255,255))
#%%

    esp32.set_laser('B', 10000, True)