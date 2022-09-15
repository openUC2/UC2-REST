package com.api;

import retrofit2.Call;
import retrofit2.http.GET;

public interface ApiService {
    @GET("/features_get")
    Call<String[]> getFeatures();
}
