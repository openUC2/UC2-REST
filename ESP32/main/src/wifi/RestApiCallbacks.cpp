#include "RestApiCallbacks.h"

namespace RestApi
{
    char output[1000];
    const char *TAG = "RestApi";

    void resetNvFLash()
    {
        int erase = nvs_flash_erase(); // erase the NVS partition and...
        int init = nvs_flash_init();   // initialize the NVS partition.
        ESP_LOGI(TAG, "erased:%s init:%s", erase, init);
        delay(500);
    }

    void getIdentity()
    {
        // if(DEBUG) Serial.println("Get Identity");
        WifiController::getServer()->send(200, "application/json", state.identifier_name);
    }

    void handleNotFound()
    {
        String message = "File Not Found\n\n";
        message += "URI: ";
        message += (*WifiController::getServer()).uri();
        message += "\nMethod: ";
        message += ((*WifiController::getServer()).method() == HTTP_GET) ? "GET" : "POST";
        message += "\nArguments: ";
        message += (*WifiController::getServer()).args();
        message += "\n";
        for (uint8_t i = 0; i < (*WifiController::getServer()).args(); i++)
        {
            message += " " + (*WifiController::getServer()).argName(i) + ": " + (*WifiController::getServer()).arg(i) + "\n";
        }
        (*WifiController::getServer()).send(404, "text/plain", message);
    }

    void deserialize()
    {
        //int argcount = WifiController::getServer()->args();
        /*for (int i = 0; i < argcount; i++)
        {
            ESP_LOGI(TAG, "%s", WifiController::getServer()->arg(i));
        }*/
        String body = WifiController::getServer()->arg("plain");
        if (body != "")
        {
            deserializeJson(*WifiController::getJDoc(), body);
            serializeJsonPretty((*WifiController::getJDoc()), Serial);
        }
    }

    void serialize()
    {
        serializeJsonPretty((*WifiController::getJDoc()), Serial);
        serializeJson((*WifiController::getJDoc()), output);
        WifiController::getServer()->send(200, "application/json", output);
    }

    void ota()
    {
        WifiController::getServer()->sendHeader("Connection", "close");
        WifiController::getServer()->send(200, "text/html", otaindex);
    }

    void update()
    {
        WifiController::getServer()->sendHeader("Connection", "close");
        WifiController::getServer()->send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
        ESP.restart();
    }

    void scanWifi()
    {
        ESP_LOGI(TAG, "scanWifi");
        int networkcount = WiFi.scanNetworks();
        if (networkcount == -1)
        {
            while (true)
                ;
        }
        WifiController::getJDoc()->clear();
        for (int i = 0; i < networkcount; i++)
        {
            (*WifiController::getJDoc()).add(WiFi.SSID(i));
        }
        serializeJson((*WifiController::getJDoc()), output);
        WifiController::getServer()->send(200, "application/json", output);
    }

    void connectToWifi()
    {
        deserialize();
        ESP_LOGI(TAG, "connectToWifi");
        bool ap = (*WifiController::getJDoc())[keyWifiAP];
        String ssid = (*WifiController::getJDoc())[keyWifiSSID];
        String pw = (*WifiController::getJDoc())[keyWifiPW];
        ESP_LOGI(TAG, "ssid json: %s wifi:%s", ssid, WifiController::getSsid());
        ESP_LOGI(TAG, "pw json: %s wifi:%s", pw, WifiController::getPw());
        ESP_LOGI(TAG, "ap json: %s wifi:%s", boolToChar(ap), boolToChar(WifiController::getAp()));
        WifiController::setWifiConfig(ssid, pw, ap);
        ESP_LOGI(TAG, "ssid json: %s wifi:%s", ssid, WifiController::getSsid());
        ESP_LOGI(TAG, "pw json: %s wifi:%s", pw, WifiController::getPw());
        ESP_LOGI(TAG, "ap json: %s wifi:%s", boolToChar(ap), boolToChar(WifiController::getAp()));
        Config::setWifiConfig(ssid, pw, ap, false);
        WifiController::getJDoc()->clear();
        serialize();
        WifiController::setup();
        // ESP.restart();
    }

    void upload()
    {
        HTTPUpload &upload = WifiController::getServer()->upload();
        if (upload.status == UPLOAD_FILE_START)
        {
            ESP_LOGI(TAG, "Update: %s\n", upload.filename.c_str());
            if (!Update.begin(UPDATE_SIZE_UNKNOWN))
            { // start with max available size
                Update.printError(Serial);
            }
        }
        else if (upload.status == UPLOAD_FILE_WRITE)
        {
            /* flashing firmware to ESP*/
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
            {
                Update.printError(Serial);
            }
        }
        else if (upload.status == UPLOAD_FILE_END)
        {
            if (Update.end(true))
            { // true to set the size to the current progress
                ESP_LOGI(TAG, "Update Success: %u\nRebooting...\n", upload.totalSize);
            }
            else
            {
                Update.printError(Serial);
            }
        }
    }

#ifdef IS_MOTOR
    void FocusMotor_act()
    {
        deserialize();
        motor.act();
        serialize();
    }

    void FocusMotor_get()
    {
        deserialize();
        motor.get();
        serialize();
    }

    void FocusMotor_set()
    {
        deserialize();
        motor.set();
        serialize();
    }
#endif

#ifdef IS_LASER
    void Laser_act()
    {
        deserialize();
        laser.act();
        serialize();
    }

    void Laser_get()
    {
        deserialize();
        laser.get();
        serialize();
    }

    void Laser_set()
    {
        deserialize();
        laser.set();
        serialize();
    }
#endif

#ifdef IS_DAC
    void Dac_act()
    {
        deserialize();
        dac.act();
        serialize();
    }

    void Dac_get()
    {
        deserialize();
        dac.get();
        serialize();
    }

    void Dac_set()
    {
        deserialize();
        dac.set();
        serialize();
    }
#endif
#ifdef IS_LED
    void Led_act()
    {
        deserialize();
        LedController::act();
        serialize();
    }

    void Led_get()
    {
        deserialize();
        LedController::get();
        serialize();
    }

    void Led_set()
    {
        deserialize();
        LedController::set();
        serialize();
    }
#endif

    void State_act()
    {
        deserialize();
        state.act();
        serialize();
    }

    void State_get()
    {
        deserialize();
        state.get();
        serialize();
    }

    void State_set()
    {
        deserialize();
        state.set();
        serialize();
    }
#ifdef IS_ANALOG
    void Analog_act()
    {
        deserialize();
        analog.act();
        serialize();
    }

    void Analog_get()
    {
        deserialize();
        analog.get();
        serialize();
    }

    void Analog_set()
    {
        deserialize();
        analog.set();
        serialize();
    }
#endif
#ifdef IS_DIGITAL

    void Digital_act()
    {
        deserialize();
        digital.act(WifiController::getJDoc());
        serialize();
    }

    void Digital_get()
    {
        deserialize();
        digital.get(WifiController::getJDoc());
        serialize();
    }

    void Digital_set()
    {
        deserialize();
        digital.set(WifiController::getJDoc());
        serialize();
    }
#endif
#ifdef IS_PID
    void Pid_act()
    {
        deserialize();
        pid.act();
        serialize();
    }

    void Pid_get()
    {
        deserialize();
        pid.get();
        serialize();
    }

    void Pid_set()
    {
        deserialize();
        pid.set();
        serialize();
    }
#endif
    void Config_act()
    {
        deserialize();
        Config::act();
        serialize();
    }

    void Config_get()
    {
        deserialize();
        Config::get();
        serialize();
    }

    void Config_set()
    {
        deserialize();
        Config::set();
        serialize();
    }
#ifdef IS_SLM
    void Slm_act()
    {
        deserialize();
        slm.act();
        serialize();
    }

    void Slm_get()
    {
        deserialize();
        slm.get();
        serialize();
    }

    void Slm_set()
    {
        deserialize();
        slm.set();
        serialize();
    }
#endif

    
    void getEndpoints()
    {
        deserialize();
        WifiController::getJDoc()->clear();
        (*WifiController::getJDoc()).add(ota_endpoint);
        (*WifiController::getJDoc()).add(update_endpoint);
        (*WifiController::getJDoc()).add(identity_endpoint);

        (*WifiController::getJDoc()).add(config_act_endpoint);
        (*WifiController::getJDoc()).add(config_set_endpoint);
        (*WifiController::getJDoc()).add(config_get_endpoint);

        (*WifiController::getJDoc()).add(state_act_endpoint);
        (*WifiController::getJDoc()).add(state_set_endpoint);
        (*WifiController::getJDoc()).add(state_get_endpoint);

        (*WifiController::getJDoc()).add(scanwifi_endpoint);
        (*WifiController::getJDoc()).add(connectwifi_endpoint);

#ifdef IS_LASER
        (*WifiController::getJDoc()).add(laser_act_endpoint);
        (*WifiController::getJDoc()).add(laser_set_endpoint);
        (*WifiController::getJDoc()).add(laser_get_endpoint);
#endif
#ifdef IS_MOTOR
        (*WifiController::getJDoc()).add(motor_act_endpoint);
        (*WifiController::getJDoc()).add(motor_set_endpoint);
        (*WifiController::getJDoc()).add(motor_get_endpoint);
#endif
#ifdef IS_PID
        (*WifiController::getJDoc()).add(PID_act_endpoint);
        (*WifiController::getJDoc()).add(PID_set_endpoint);
        (*WifiController::getJDoc()).add(PID_get_endpoint);
#endif
#ifdef IS_ANALOG
        (*WifiController::getJDoc()).add(analog_act_endpoint);
        (*WifiController::getJDoc()).add(analog_set_endpoint);
        (*WifiController::getJDoc()).add(analog_get_endpoint);
#endif
#ifdef IS_DIGITAL
        (*WifiController::getJDoc()).add(digital_act_endpoint);
        (*WifiController::getJDoc()).add(digital_set_endpoint);
        (*WifiController::getJDoc()).add(digital_get_endpoint);
#endif
#ifdef IS_DAC
        (*WifiController::getJDoc()).add(dac_act_endpoint);
        (*WifiController::getJDoc()).add(dac_set_endpoint);
        (*WifiController::getJDoc()).add(dac_get_endpoint);
#endif
#ifdef IS_SLM
        (*WifiController::getJDoc()).add(slm_act_endpoint);
        (*WifiController::getJDoc()).add(slm_set_endpoint);
        (*WifiController::getJDoc()).add(slm_get_endpoint);
#endif
#ifdef IS_LED
        (*WifiController::getJDoc()).add(ledarr_act_endpoint);
        (*WifiController::getJDoc()).add(ledarr_set_endpoint);
        (*WifiController::getJDoc()).add(ledarr_get_endpoint);
#endif
        serialize();
    }
}