classdef mserial
    properties
        port
        device
        parent
        NumberRetryReconnect
        MaxNumberRetryReconnect
        DEBUG
        is_connected
        isSafetyBreak
    end
    methods
        function obj = mserial(port)
            obj.port = port;
            obj.device = serial(port, 'BaudRate',115200);
            fopen(obj.device);
            pause(3)
            obj.is_connected = true;
            obj.NumberRetryReconnect = 0;
            obj.MaxNumberRetryReconnect = 10;
            obj.DEBUG = false;
            obj.isSafetyBreak = false;
        end
        
        function close(obj)
            fclose(obj.device);
        end
        
        function returnmessage = get_json(obj, path)
            message.task = path;
            message_json = jsonencode(message);
            returnmessage = message_json;
        end
        
        function returnmessage = post_json(obj, path, payload, getReturn, timeout)
            if ~isstruct(payload) && payload == "{}"
                payload = struct()
            end
            
            if ~isfield(payload, 'task')
                payload.task = path;
            end
            
            if isfield(payload, 'isblock')
                is_blocking = payload.isblock;
            else
                is_blocking = true;
            end
            
            if ~exist('getReturn','var')
                getReturn = true;
            end 
            if ~exist('timeout','var')
                timeout = 1;
            end 
                        
            
            obj.writeSerial(payload);
            
            if getReturn
                returnmessage = obj.readSerial(is_blocking, timeout);
            else
                returnmessage = false;
            end
        end
        
        function writeSerial(obj, payload)
            try % reconnection not implemented
                if 0 && obj.is_connected == false && obj.NumberRetryReconnect < obj.MaxNumberRetryReconnect
                    fprintf('Trying to reconnect to serial device: %d\n', obj.NumberRetryReconnect);
                    obj.NumberRetryReconnect = obj.NumberRetryReconnect + 1;
                    fopen(obj.device);
                end
                
                flushinput(obj.device);
                flushoutput(obj.device);
                
            catch ME
                fprintf('Error: %s\n', ME.message);
                obj.is_connected = false;
                fopen(obj.device);
            end
            
            if isstruct(payload)
                payload = jsonencode(payload);
            end
            
            try
                fprintf(obj.device, '%s', payload);
            catch ME
                fprintf('Error: %s\n', ME.message);
            end
        end
        
        function breakCurrentCommunication(obj)
            obj.isSafetyBreak = true;
            flushinput(obj.device);
            flushoutput(obj.device);
        end

        function returnmessage = readSerial(obj, is_blocking, timeout)
            returnmessage = '';
            rmessage = '';
            t0 = tic;

            if is_blocking
                while is_blocking && ~obj.isSafetyBreak
                    try
                        rmessage = fgetl(obj.device);
                        if obj.DEBUG
                            fprintf('%s\n', rmessage);
                        end
                        returnmessage = [returnmessage, rmessage];
                        if strncmp(rmessage, "--", 2)
                            break;
                        end
                    catch ME
                        fprintf('Error: %s\n', ME.message);
                    end
                    if toc(t0) > timeout
                        break;
                    end
                end

                % casting to struct
                try
                    returnmessage = split(returnmessage, "--");
                    returnmessage = split(returnmessage{1}, "++");
                    returnmessage = strrep(returnmessage{end}, '\r', '');
                    returnmessage = strrep(returnmessage, '\n', '');
                    returnmessage = strrep(returnmessage, '''', '"');
                    if obj.DEBUG
                        fprintf('%s\n', returnmessage);
                    end
                    returnmessage = jsondecode(returnmessage);
                catch ME
                    if obj.DEBUG
                        fprintf('Casting json string from serial to Matlab struct failed\n');
                    end
                end

                
            end
        end
    end
end
