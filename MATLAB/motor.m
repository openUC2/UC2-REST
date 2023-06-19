classdef motor < handle
    
    properties
        isRunning
        mserial
        isCoreXY
        nMotors
        motorAxisOrder
        stepSizeT
        stepSizeX
        stepSizeY
        stepSizeZ
        steps_last
    end
    
    methods
        function obj = motor(mserial)
            obj.isRunning = false;
            obj.mserial = mserial;
            obj.isCoreXY = false;
            obj.stepSizeT = 1
            obj.stepSizeX = 1
            obj.stepSizeY = 1
            obj.stepSizeZ = 1  
            obj.steps_last = [0,0,0,0]
            obj.motorAxisOrder = [1,2,3,0]
        end
        
        function r = stop(obj, axis)
            if nargin < 2
                axisNumberList = 0:obj.nMotors-1;
            else
                axisNumberList = obj.xyztTo1230(axis);
            end
            
            motorPropList = cell(1,length(axisNumberList));
            for i = 1:length(axisNumberList)
                motorProp = struct('stepperid', obj.motorAxisOrder(axisNumberList(i)), 'isstop', true);
                motorPropList{i} = motorProp;
            end
            
            path = "/motor_act";
            payload = struct('task', path, 'motor', struct('steppers', motorPropList));
            
            if obj.isRunning
                obj.mserial.serial.breakCurrentCommunication();
            end
            
            r = obj.mserial.post_json(path, payload, false, 0);
        end
        
        function r = move_stepper(obj, steps, speed, is_absolute, timeout, is_blocking, is_enabled, backlash, acceleration)
            
            % set default parameters
            if nargin < 2, steps = [0,0,0,0]; end
            if nargin < 3, speed = [1000,1000,1000,1000]; end
            if nargin < 4, is_absolute = [false,false,false,false]; end
            if nargin < 5, timeout = gTIMEOUT; end
            if nargin < 6, is_blocking = true; end
            if nargin < 7, is_enabled = true; end
            if nargin < 8, backlash = [0,0,0,0]; end
            if nargin < 9, acceleration = [NaN, NaN, NaN, NaN]; end
            
            % convert inputs to arrays if necessary
            if isscalar(speed), speed = repmat(speed,1,4); end
            if isscalar(acceleration), acceleration = repmat(acceleration,1,4); end
            if isscalar(is_absolute), is_absolute = repmat(is_absolute,1,4); end
            if isscalar(backlash), backlash = repmat(backlash,1,4); end
            
            % determine the axis to operate
            axisToMove = find(abs(speed)>0);
            
            % detect change in direction
            for iMotor = 1:4
                if sign(obj.steps_last(iMotor)) ~= sign(steps(iMotor))
                    % we want to overshoot a bit
                    steps(iMotor) = steps(iMotor) + sign(steps(iMotor))*backlash(iMotor);
                end
            end
            
            % convert to physical units
            steps(1) = steps(1) / obj.stepSizeT;
            steps(2) = steps(2) / obj.stepSizeX;
            steps(3) = steps(3) / obj.stepSizeY;
            steps(4) = steps(4) / obj.stepSizeZ;
            
            % only consider those actions that are necessary
            motorPropList = {};
            for iMotor = 1:4
                if is_absolute(iMotor) || abs(steps(iMotor))>0
                    motorProp = struct('stepperid', obj.motorAxisOrder(iMotor), 'position', steps(iMotor), ...
                        'speed', speed(iMotor), 'isabs', is_absolute(iMotor), 'isaccel', 0, 'isen', is_enabled);
                    if ~isnan(acceleration(iMotor))
                        motorProp.accel = acceleration(iMotor);
                    end
                    motorPropList{end+1} = motorProp;
                end
            end
            
            path = '/motor_act';
            payload = struct('task', path, 'motor', struct('steppers', {motorPropList}));
            
            % save steps to track direction for backlash compensation
            obj.steps_last = steps;
            
            % drive motor
            obj.isRunning = true;
            r = obj.mserial.post_json(path, payload, is_blocking, timeout);
            
            % TODO: implement waiting until job has been done as in Python code
            
            % reset busy flag
            obj.isRunning = false;
            
        end
        
    end
end
