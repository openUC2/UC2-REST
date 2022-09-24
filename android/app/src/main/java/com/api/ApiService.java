package com.api;

import com.api.response.LedArrRequest;
import com.api.response.LedArrResponse;
import com.api.response.WifiConnectRequest;

import retrofit2.Call;
import retrofit2.http.Body;
import retrofit2.http.GET;
import retrofit2.http.HTTP;
import retrofit2.http.Headers;
import retrofit2.http.POST;
import retrofit2.http.Query;

public interface ApiService {
    @GET("/")
    Call<String[]> getFeatures();

    @GET("/ledarr_get")
    Call<LedArrResponse> ledGet();

    @POST("/ledarr_act")
    Call<String> ledAct(@Body LedArrRequest request);

    @GET("/wifi/scan")
    Call<String[]> getSsids();

    @Headers("Content-Type: application/json")
    @POST("/wifi/connect")
    Call<String> connectToWifi(@Body WifiConnectRequest wifiConnectRequest);

    @GET("/resetnv")
    Call<Void> resetNvFlash();

}
