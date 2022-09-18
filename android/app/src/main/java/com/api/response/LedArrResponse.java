package com.api.response;

import com.api.enums.LedModes;
import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonProperty;

@JsonIgnoreProperties(ignoreUnknown = true)
public class LedArrResponse
{
    @JsonProperty("Nled")
    public int Nleds;

    @JsonProperty("LEDArrMode")
    public LedModes[] ledModes;
}
