
% Find the serial port connected to the ESP32 device
portInfo = instrhwinfo('serial');
portName = portInfo.AvailableSerialPorts;

espPort = '';
for i = 1:numel(portName)
    if contains(portName{i}, 'wchusb') || contains(portName{i}, 'UART') % Modify this condition according to your ESP32's serial port identifier
        espPort = portName{i};
        break;
    end
end

if isempty(espPort)
    error('ESP32 device not found');
end


% Establish connection with ESP32
esp = serial(espPort, 'BaudRate', 115200); % Modify the baud rate if necessary

% Open the serial port
fopen(esp);

% turn motors
sendMotorAction(esp);

% Create the LED array structure
ledArray = struct('id', 0, 'red', 0, 'green', 5, 'blue', 0);

% Send the LED array action JSON message
sendLEDArrayAction(esp, 1, ledArray);

% Close the serial port
fclose(esp);
delete(esp);
clear esp;