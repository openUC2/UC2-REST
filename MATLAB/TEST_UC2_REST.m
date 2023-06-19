% DIP-Image related settings
rmpath('/Applications/dip/common/dipimage')
savepath

% Create an instance of the class and specify a serial port, e.g., 'COM4'
mySerialPort = mserial('/dev/tty.SLAB_USBtoUART');


% get state
path = "/state_get"
payload = "{}"

returnmessage = mySerialPort.post_json(path, payload)

% Display the return message
disp(returnmessage);

% driving motors 
mMotor = motor(mySerialPort)

% X,Y,Z,A
steps=[100,100,100,0];
speed=[15000,15000,1500,0];
is_absolute = false;
timeout = 100;
is_blocking = true;
is_enabled = true;


mMotor.move_stepper(steps, speed, is_absolute, timeout, is_blocking, is_enabled)

% Close the serial port when done
mySerialPort.close();