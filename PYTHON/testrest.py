
if __name__ == "__main__":
            
        
    #%
    host = '192.168.43.226'
    host = '192.168.2.147'
    host = '192.168.2.151'
    esp32 = ESP32Client(host, port=80)
    
    #esp32.set_led((100,2,5))
    #esp32.move_x(steps=2000, speed=8)
    #esp32.move_y(steps=2000, speed=6)
    
    #%%
    esp32.lens_x(value=10000)
    esp32.lens_z(value=10000)
    #%%
    for ix in range(0,32000,100):
        esp32.lens_x(value=ix)
        for iy in range(0,32000,100):
            esp32.lens_z(value=iy)
    esp32.lens_z(value=0)
    esp32.lens_x(value=0)
    
    #%%
    esp32.lens_x(value=0)
    esp32.lens_z(value=0)
    #%%
    for iy in range(0,1000,1):
        esp32.set_laser(np.sin(iy/1000*np.pi*100)*10000)
    
        
    #%%
    esp32.move_z(steps=500,  speed=1000)
    
    #%%
    for i in range(100):
        print(i)
        esp32.move_z(steps=i*100, speed=i*100)
        
    
    #%%
    for i in range(100):
        print(i)
        esp32.post(value = i)
    
    #%%
    esp32.set_laser_red(10000)
    esp32.set_laser_blue(10000)
    esp32.set_laser_green(10000)
    
    time.sleep(1)
    
    esp32.set_laser_red(0)
    esp32.set_laser_blue(0)
    esp32.set_laser_green(0)
    #%%
    esp32.move_filter(steps=-800, speed=20)
    # %%
    esp32.set_laser_red(0)
    esp32.set_laser_blue(0000)
    
    #%%
    esp32.set_laser_red(0)
    
    #%%
    esp32.set_led(colour=(0,255,255))
    
    #%%
    esp32.switch_filter()
    
    #%%
    image = np.random.randn(320,240)*255
    esp32.send_jpeg(image)
    
    #%%
    N_leds = 4
    I_max = 100
    iiter = 0 
    while(True):
        iiter+=1
        
        image = np.ones((320,240))*(iiter%2)*255 # np.random.randn(320,240)*
        esp32.send_jpeg(np.uint8(image))
        led_pattern = np.array((np.reshape(np.random.randint(0,I_max ,N_leds**2),(N_leds,N_leds)),
                       np.reshape(np.random.randint(0,I_max ,N_leds**2),(N_leds,N_leds)),
                       np.reshape(np.random.randint(0,I_max ,N_leds**2),(N_leds,N_leds))))
        
        esp32.send_ledmatrix(led_pattern)

test