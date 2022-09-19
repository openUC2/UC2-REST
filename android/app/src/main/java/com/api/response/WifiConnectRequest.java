package com.api.response;

import com.fasterxml.jackson.annotation.JsonAutoDetect;
import com.fasterxml.jackson.annotation.JsonProperty;

@JsonAutoDetect(fieldVisibility = JsonAutoDetect.Visibility.ANY)
public class WifiConnectRequest {

    @JsonProperty
    public String ssid ="";
    @JsonProperty
    public String PW ="";
    @JsonProperty
    public boolean AP = false;
}
