package com.api.response;

import androidx.databinding.BaseObservable;
import androidx.databinding.Bindable;

import com.fasterxml.jackson.annotation.JsonAutoDetect;
import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.annotation.JsonValue;
import com.uc2control.BR;

@JsonAutoDetect(fieldVisibility = JsonAutoDetect.Visibility.ANY)
public class WifiConnectRequest {

    @JsonProperty
    public String ssid ="";
    @JsonProperty
    public String PW ="";
    @JsonProperty
    public boolean AP = false;
}
