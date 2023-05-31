function response = sendJSONMessage(esp, message)
    % Send JSON message over the serial connection
    
    % Convert message to string
    jsonString = jsonencode(message);
    
    % Send the message
    fprintf(esp, jsonString);
    
    % wait for the response
    response = fscanf(esp);
end
