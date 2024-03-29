function returnMessage = sendMotorAction(esp)
% Create the JSON message for the motor
message = struct('task', '/motor_act', 'motor', struct('steppers', ...
    [struct('stepperid', 0, 'position', 10000, 'speed', 5000, 'isabs', 0, 'isaccel', 0), ...
    struct('stepperid', 1, 'position', 10000, 'speed', 5000, 'isabs', 0, 'isaccel', 0), ...
    struct('stepperid', 2, 'position', 10000, 'speed', 5000, 'isabs', 0, 'isaccel', 0), ...
    struct('stepperid', 3, 'position', 10000, 'speed', 5000, 'isabs', 0, 'isaccel', 0)]));

% Convert message to string
returnMessage = sendJSONMessage(esp, message);
end
