#include "RestApiCallbacks.h"

namespace RestApi
{

    void getIdentity()
    {
        // if(DEBUG) Serial.println("Get Identity");
        wifi.server->send(200, "application/json", state.identifier_name);
    }

    void deserialize()
    {
        String body = (*wifi.server).arg("plain");
        deserializeJson((*wifi.jsonDocument), body);
    }

    void serialize()
    {
        serializeJson((*wifi.jsonDocument), wifi.output);
        (*wifi.server).send(200, "application/json", wifi.output);
    }

    void ota()
    {
        wifi.server->sendHeader("Connection", "close");
        wifi.server->send(200, "text/html", otaindex);
    }

    void update()
    {
        wifi.server->sendHeader("Connection", "close");
        wifi.server->send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
        ESP.restart();
    }

    void upload()
    {
        HTTPUpload &upload = wifi.server->upload();
        if (upload.status == UPLOAD_FILE_START)
        {
            Serial.printf("Update: %s\n", upload.filename.c_str());
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
                Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
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
        led.act();
        serialize();
    }

    void Led_get()
    {
        deserialize();
        led.get();
        serialize();
    }

    void Led_set()
    {
        deserialize();
        led.set();
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
        digital.act(wifi.jsonDocument);
        serialize();
    }

    void Digital_get()
    {
        deserialize();
        digital.get(wifi.jsonDocument);
        serialize();
    }

    void Digital_set()
    {
        deserialize();
        digital.set(wifi.jsonDocument);
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
        config.act();
        serialize();
    }

    void Config_get()
    {
        deserialize();
        config.get();
        serialize();
    }

    void Config_set()
    {
        deserialize();
        config.set();
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

}