package com.uc2control;

import android.util.Log;

import androidx.databinding.BaseObservable;
import androidx.databinding.Bindable;

import com.api.ApiServiceCallback;
import com.api.RestClient;
import com.api.RestController;
import com.api.response.WifiConnectRequest;

import java.util.Arrays;


public class WifiSettingsModel extends BaseObservable {
    private String[] supportedEndtpoints;
    String url = "http://192.168.4.1";
    String message;
    private WifiConnectRequest wifiConnectRequest = new WifiConnectRequest();
    private String[] wifi_ssids = new String[0];
    private RestController restController;

    public WifiSettingsModel(RestController restController)
    {
        this.restController = restController;
        setUrl(url);
    }

    public void onConnectButtonClick()
    {
        restController.setUrl(url);
        restController.getRestClient().getFeaturesAsync(getFeatureCallback);
        setMessage("Loading Endpoints");
    }

    private ApiServiceCallback<String[]> getFeatureCallback = new ApiServiceCallback<String[]>() {
        @Override
        public void onResponse(String[] response) {
            supportedEndtpoints = response;
            setMessage(Arrays.toString(supportedEndtpoints));
        }
    };

    public void onScanWifiClick()
    {
        restController.getRestClient().getSsids(getWifiScanCallback);
        setMessage("Loading Ssids");
    }

    private ApiServiceCallback<String[]> getWifiScanCallback = new ApiServiceCallback<String[]>() {
        @Override
        public void onResponse(String[] response) {
            setWifi_ssids(response);
        }
    };

    public void onResetNvFlashClick()
    {
        restController.getRestClient().resetNvFLash(new ApiServiceCallback<Void>() {
            @Override
            public void onResponse(Void response) {
                Log.d(WifiSettingsModel.class.getSimpleName(),"nv flash response");
            }
        });
    }

    public void setUrl(String url) {
        if (url == this.url)
            return;
        this.url = url;
        notifyPropertyChanged(BR.url);
    }

    @Bindable
    public String getUrl() {
        return url;
    }

    @Bindable
    public String getMessage() {
        return message;
    }

    public void setMessage(String message) {
        this.message = message;
        notifyPropertyChanged(BR.message);
    }

    @Bindable
    public String[] getWifi_ssids() {
        return wifi_ssids;
    }

    public void setWifi_ssids(String[] wifi_ssids) {
        this.wifi_ssids = wifi_ssids;
        notifyPropertyChanged(BR.wifi_ssids);
    }

    public void onConnectToWifiClick()
    {
        restController.getRestClient().connectToWifi(wifiConnectRequest,wificonnectCallback);
    }

    private ApiServiceCallback<String> wificonnectCallback = new ApiServiceCallback<String>() {
        @Override
        public void onResponse(String response) {
            Log.d(WifiSettingsModel.class.getSimpleName(),response);
        }
    };

    public WifiConnectRequest getWifiConnectRequest() {
        return wifiConnectRequest;
    }

    @Bindable
    public boolean getAp() {
        return wifiConnectRequest.AP;
    }

    public void setAp(boolean ap) {
        if (ap == wifiConnectRequest.AP)
            return;
        wifiConnectRequest.AP = ap;
        notifyPropertyChanged(BR.ap);
    }

    @Bindable
    public String getSsid() {
        return wifiConnectRequest.ssid;
    }

    public void setSsid(String ssid) {
        if (ssid == wifiConnectRequest.ssid)
            return;
        wifiConnectRequest.ssid = ssid;
        notifyPropertyChanged(BR.ssid);
    }

    @Bindable
    public String getPw() {
        return wifiConnectRequest.PW;
    }

    public void setPw(String pw) {
        if (pw == wifiConnectRequest.PW)
            return;
        this.wifiConnectRequest.PW = pw;
        notifyPropertyChanged(BR.pw);
    }

}
