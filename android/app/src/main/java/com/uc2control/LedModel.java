package com.uc2control;

import androidx.databinding.BaseObservable;
import androidx.databinding.Bindable;

import com.api.RestController;

public class LedModel extends BaseObservable {
    private RestController restController;
    private int ledcount = 0;
    private boolean leds_turned_on = false;

    public LedModel(RestController restController)
    {
        this.restController = restController;
    }
    @Bindable
    public int getLedcount() {
        return ledcount;
    }

    public void setLedcount(int ledcount) {
        this.ledcount = ledcount;
        notifyPropertyChanged(BR.ledcount);
    }

    public void setLeds_turned_on(boolean leds_turned_on) {
        this.leds_turned_on = leds_turned_on;
        notifyPropertyChanged(BR.leds_turned_on);
    }

    @Bindable
    public boolean getLeds_turned_on() {
        return leds_turned_on;
    }
}
