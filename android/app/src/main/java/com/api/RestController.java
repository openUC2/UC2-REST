package com.api;

public class RestController
{
    private RestClient restClient;
    private String url;

    public RestController()
    {}

    public void setUrl(String url)
    {
        this.url = url;
        restClient = new RestClient(url);
    }

    public RestClient getRestClient()
    {
        return restClient;
    }
}
