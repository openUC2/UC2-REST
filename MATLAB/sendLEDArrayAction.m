function returnMessage = sendLEDArrayAction(esp, mode, ledArray)
    % Send LED array action JSON message over the serial connection
    
    % Create the message structure
    message = struct('task', '/ledarr_act', 'led', struct('LEDArrMode', mode, 'led_array', ledArray));
    
    % Convert message to string
    returnMessage = sendJSONMessage(esp, message);
    
end