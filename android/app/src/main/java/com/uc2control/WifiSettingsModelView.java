package com.uc2control;

import androidx.lifecycle.ViewModel;

import com.api.ApiServiceCallback;
import com.api.RestClient;
import com.api.RestController;

import java.util.Observable;

import javax.inject.Inject;

import dagger.hilt.android.lifecycle.HiltViewModel;

@HiltViewModel
public class WifiSettingsModelView extends ViewModel {

    private WifiSettingsModel wifiSettingsModel;

    @Inject
    public WifiSettingsModelView(RestController restController)
    {
        wifiSettingsModel = new WifiSettingsModel(restController);
    }

    public WifiSettingsModel getWifiSettingsModel() {
        return wifiSettingsModel;
    }
}
