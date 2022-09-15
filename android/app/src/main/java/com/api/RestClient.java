package com.api;

public class RestClient {

    ApiService apiService;
    public RestClient(String url)
    {
        apiService = ApiServiceGenerator.createService(ApiService.class,url);
    }

    public String[] getFeatures() throws Exception {
        return ApiServiceGenerator.executeSync(apiService.getFeatures());
    }
}
