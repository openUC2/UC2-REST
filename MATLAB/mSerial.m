classdef mSerial < handle
    properties
        T_SERIAL_WARMUP = 3;
        serialport;
        baudrate;
        timeout = 1;
        identity = 'UC2_Feather';
        DEBUG = false;
        isSafetyBreak = false;
        versionFirmware = 'V2.0';
        NumberRetryReconnect = 0;
        MaxNumberRetryReconnect = 0;
        is_connected;
        serialdevice;
    end
    
    methods
        function obj = Serial(port, baudrate, timeout, identity, parent, DEBUG)
            if nargin < 6
                DEBUG = false;
            end
            if nargin < 5
                parent = [];
            end
            if nargin < 4
                identity = 'UC2_Feather';
            end
            if nargin < 3
                timeout = 1;
            end
            obj.serialport = port;
            obj.baudrate = baudrate;
            obj.timeout = timeout;
            obj.identity = identity;
            obj.parent = parent;
            obj.DEBUG = DEBUG;
            obj.open();
            
            if strcmp(obj.versionFirmware, 'V1.2')
                obj.parent.APIVersion = 1;
            elseif strcmp(obj.versionFirmware, 'V2.0')
                obj.parent.APIVersion = 2;
            end
        end
        
        function open(obj)
            obj.is_connected = false;
            obj.serialdevice = [];
            try
                obj.serialdevice = serial(obj.serialport, 'BaudRate', obj.baudrate, 'Timeout', 1);
                obj.is_connected = true;
                pause(obj.T_SERIAL_WARMUP);
                correctFirmware = obj.checkFirmware();
                if ~correctFirmware
                    error('Wrong firmware');
                end
                disp(['We are connected: ', num2str(obj.is_connected), ' on port: ', obj.serialdevice.Port]);
            catch e
                disp(e);
                available_ports = serialportlist("available");
                portslist = ["COM", "/dev/tt", "/dev/a", "/dev/cu.SLA", "/dev/cu.wchusb", "/dev/cu.usbserial"];
                descriptionlist = ["CH340", "CP2102"];
                for i = 1:numel(available_ports)
                    iport = available_ports(i);
                    if startsWith(iport, portslist) || startsWith(iport.Description, descriptionlist)
                        try
                            obj.serialdevice = serial(iport, 'BaudRate', obj.baudrate, 'Timeout', 1);
                            obj.serialdevice.WriteTimeout = 1;
                            obj.is_connected = true;
                            pause(obj.T_SERIAL_WARMUP);
                            correctFirmware = obj.checkFirmware();
                            if correctFirmware
                                obj.serialport = iport;
                                disp(['We are connected: ', num2str(obj.is_connected), ' on port: ', obj.serialdevice.Port]);
                                obj.NumberRetryReconnect = 0;
                                return;
                            end
                        catch e
                            disp(['Trying out port ', iport, ' failed']);
                            disp(e);
                            obj.is_connected = false;
                        end
                    end
                end
                obj.serialport = 'NotConnected';
                obj.serialdevice = SerialDeviceDummy();
                disp('No USB device connected! Using DUMMY!');
            end
        end
        
function result = checkFirmware(obj, timeout)
    % Check if the firmware is correct
    if nargin < 2
        timeout = 1;
    end
    
    path = '/state_get';
    message = struct('task', path);
    state = obj.post_json(path, message, timeout);
    
    try
        obj.versionFirmware = state.identifier_id;
    catch e
        obj.parent.logger.error(e);
        obj.versionFirmware = 'V2.0';
    end
    
    if strcmp(state.identifier_name, obj.identity)
        result = true;
    else
        result = false;
    end
end

function closeSerial(obj)
    obj.serialdevice.close();
end

function reconnect(obj)
    % Reconnect to serial device
    if obj.is_serial
        obj.initSerial(obj.serialport, obj.baudrate);
    end
end

function response = get_json(obj, path)
    % Perform an HTTP GET request and return the JSON response
    message = struct('task', path);
    message = jsonencode(message);
    obj.serialdevice.flushInput();
    obj.serialdevice.flushOutput();
    obj.parent.logger.debug(message);
    returnmessage = obj.serialdevice.write(message.encode('UTF-8'));
    response = returnmessage;
end

function response = post_json(obj, path, payload, getReturn, timeout)
    % Make an HTTP POST request and return the JSON response
    if nargin < 5
        timeout = 1;
    end
    
    if ~isfield(payload, 'task')
        payload.task = path;
    end
    
    if isfield(payload, 'isblock')
        is_blocking = payload.isblock;
    else
        is_blocking = true;
    end
    
    % write message to the serial
    obj.writeSerial(payload);
    
    if getReturn
        % read the return message
        returnmessage = obj.readSerial(is_blocking, timeout);
        if strcmp(returnmessage, 'deserializeJson() failed: NoMemory')
            % TODO: We will lose values here - need to set xyz coordinates!!
            obj.serialdevice.close();
            obj.parent.state.espRestart();
            obj.parent.state.restart();
            obj.open();
        end
    else
        returnmessage = false;
    end
    
    response = returnmessage;
end

function writeSerial(obj, payload)
    % Write JSON document to serial device
    try
        if strcmp(obj.serialport, 'NotConnected') && obj.NumberRetryReconnect < obj.MaxNumberRetryReconnect
            % try to reconnect
            obj.parent.logger.debug(['Trying to reconnect to serial device: ' num2str(obj.NumberRetryReconnect)]);
            obj.NumberRetryReconnect = obj.NumberRetryReconnect + 1;
            obj.open();
        end
        obj.serialdevice.flushInput();
        obj.serialdevice.flushOutput();
    catch e
        obj.parent.logger.error(e);
        try
            obj.serialdevice.delete();
        catch
        end
        obj.is_connected = false;
        % attempt to reconnect?
        try
            obj.open();
        catch
            response = -1;
            return;
        end
    end
    
    if isstruct(payload)
        payload = jsonencode(payload);
    end
    
    try
        if obj.DEBUG
            obj.parent.logger.debug(payload);
        end
        obj.serialdevice.write(payload.encode('UTF-8'));
    catch e
        obj.parent.logger.error(e);
    end
end

function breakCurrentCommunication(obj)
    % this breaks the wait-for-return command in readserial
    obj.isSafetyBreak = true;
    obj.serialdevice.flush

