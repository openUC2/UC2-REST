package com.uc2control;

import androidx.lifecycle.ViewModel;

import com.api.ApiServiceCallback;
import com.api.RestClient;

import java.util.Observable;

import javax.inject.Inject;

import dagger.hilt.android.lifecycle.HiltViewModel;


public class WifiSettingsModelView extends ViewModel {

    private WifiSettingsModel wifiSettingsModel;

    public WifiSettingsModelView()
    {
        wifiSettingsModel = new WifiSettingsModel();
    }

    public WifiSettingsModel getWifiSettingsModel() {
        return wifiSettingsModel;
    }
}
