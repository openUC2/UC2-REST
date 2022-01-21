#%%
#%load_ext autoreload
#%autoreload 2

from espclient import ESP32Client
import time
#%%
host = '192.168.137.145'
esp32 = ESP32Client(host, port=80)

#%%
esp32.galvo_amp_y(100)
#%%
esp32.galvo_pos_x(100)
#%%
esp32.set_laser('R', 1000, True)

#%%#%%
esp32.set_laser('R', 000, True)
esp32.set_laser('B', 1000, True)

esp32.set_laser('B', 000, True)
esp32.set_laser('R', 1000, True)

#%%
while(1):
    esp32.set_laser('B', 32000, False)
    esp32.set_laser('B', 22000, False)


#%%

#reset
esp32.move_filter(steps=-3000, speed=250)

# red
esp32.move_filter(steps=1000, speed=250)

time.sleep(.5)
# blue
esp32.move_filter(steps=1000, speed=250)
