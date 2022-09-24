package com.api;

import android.util.Log;

import com.api.response.LedArrRequest;
import com.api.response.WifiConnectRequest;

public class RestClient {

    ApiService apiService;
    public RestClient(String url)
    {
        apiService = ApiServiceGenerator.createService(ApiService.class,url);
    }

    public String[] getFeatures() {
        return ApiServiceGenerator.executeSync(apiService.getFeatures());
    }

    public void getFeaturesAsync(ApiServiceCallback<String[]> callback)
    {
        apiService.getFeatures().enqueue(new ApiServiceCallbackAdapter<>(callback));
    }

    public void getSsids(ApiServiceCallback<String[]> callback)
    {
        apiService.getSsids().enqueue(new ApiServiceCallbackAdapter<>(callback));
    }

    public void connectToWifi(WifiConnectRequest wifiConnectRequest, ApiServiceCallback<String> callback)
    {
        Log.d(RestClient.class.getSimpleName(), wifiConnectRequest.toString());
        apiService.connectToWifi(wifiConnectRequest).enqueue(new ApiServiceCallbackAdapter<String>(callback));
    }

    public void resetNvFLash(ApiServiceCallback<Void> callback)
    {
        apiService.resetNvFlash().enqueue(new ApiServiceCallbackAdapter<>(callback));
    }

    public void setLedArr(LedArrRequest request, ApiServiceCallback<String> callback)
    {
        apiService.ledAct(request).enqueue(new ApiServiceCallbackAdapter<>(callback));
    }
}
