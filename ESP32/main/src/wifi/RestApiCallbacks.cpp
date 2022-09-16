#include "RestApiCallbacks.h"

namespace RestApi
{
    WebServer * server;
    DynamicJsonDocument * jsonDocument;
    char output[1000];

    void setup(WebServer *ser, DynamicJsonDocument * jDoc)
    {
        server = ser;
        jsonDocument = jDoc;

        Serial.print("setup server nullptr:");
        Serial.println(server == nullptr);

        Serial.print("setup jsondoc nullptr:");
        Serial.println(jsonDocument == nullptr);
    }

    void getIdentity()
    {
        // if(DEBUG) Serial.println("Get Identity");
        server->send(200, "application/json", state.identifier_name);
    }

    void handleNotFound() {
        Serial.print("handleNotFound server nullptr:");
        Serial.println(server == nullptr);
        String message = "File Not Found\n\n";
        message += "URI: ";
        message += (*server).uri();
        message += "\nMethod: ";
        message += ((*server).method() == HTTP_GET) ? "GET" : "POST";
        message += "\nArguments: ";
        message += (*server).args();
        message += "\n";
        for (uint8_t i = 0; i < (*server).args(); i++) {
            message += " " + (*server).argName(i) + ": " + (*server).arg(i) + "\n";
        }
        (*server).send(404, "text/plain", message);
    }

    void deserialize()
    {
        if (server == nullptr)
            Serial.println("deserialize server is null");
        if(jsonDocument == nullptr)
            Serial.println("deserialize jsonDocument is null");
        String body = server->arg("plain");
        deserializeJson((*jsonDocument), body);
    }

    void serialize()
    {
        if(jsonDocument == nullptr)
            Serial.println("serialize jsonDocument is null");
        Serial.print("serialize");
        serializeJson((*jsonDocument), output);
        server->send(200, "application/json", output);
    }

    void ota()
    {
        server->sendHeader("Connection", "close");
        server->send(200, "text/html", otaindex);
    }

    void update()
    {
        server->sendHeader("Connection", "close");
        server->send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
        ESP.restart();
    }

    void scanWifi()
    {
        int networkcount = WiFi.scanNetworks();
        if (networkcount == -1) {
            while (true);
        }
        jsonDocument->clear();
        JsonArray ar = jsonDocument->createNestedArray();
        for (int i = 0; i < networkcount; i++) {
            ar.add(WiFi.SSID(i));
        }
        serializeJson((*jsonDocument), output);
        server->send(200, "application/json", output);
    }

    void upload()
    {
        HTTPUpload &upload = server->upload();
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
        digital.act(jsonDocument);
        serialize();
    }

    void Digital_get()
    {
        deserialize();
        digital.get(jsonDocument);
        serialize();
    }

    void Digital_set()
    {
        deserialize();
        digital.set(jsonDocument);
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
    void getEndpoints()
    {
        if (jsonDocument == nullptr)
        {
            Serial.println("getEndpoints failed jsondoc null");
        }
        
        deserialize();
        jsonDocument->clear();
        JsonArray ar = jsonDocument->createNestedArray();
        ar.add(ota_endpoint);
        ar.add(update_endpoint);
        ar.add(identity_endpoint);

        ar.add(config_act_endpoint);
        ar.add(config_set_endpoint);
        ar.add(config_get_endpoint);

        ar.add(state_act_endpoint);
        ar.add(state_set_endpoint);
        ar.add(state_get_endpoint);

#ifdef IS_LASER
        ar.add(laser_act_endpoint);
        ar.add(laser_set_endpoint);
        ar.add(laser_get_endpoint);
#endif
#ifdef IS_MOTOR
        ar.add(motor_act_endpoint);
        ar.add(motor_set_endpoint);
        ar.add(motor_get_endpoint);
#endif
#ifdef IS_PID
        ar.add(PID_act_endpoint);
        ar.add(PID_set_endpoint);
        ar.add(PID_get_endpoint);
#endif
#ifdef IS_ANALOG
        ar.add(analog_act_endpoint);
        ar.add(analog_set_endpoint);
        ar.add(analog_get_endpoint);
#endif
#ifdef IS_DIGITAL
        ar.add(digital_act_endpoint);
        ar.add(digital_set_endpoint);
        ar.add(digital_get_endpoint);
#endif
#ifdef IS_DAC
        ar.add(dac_act_endpoint);
        ar.add(dac_set_endpoint);
        ar.add(dac_get_endpoint);
#endif
#ifdef IS_SLM
        ar.add(slm_act_endpoint);
        ar.add(slm_set_endpoint);
        ar.add(slm_get_endpoint);
#endif
#ifdef IS_LED
        ar.add(ledarr_act_endpoint);
        ar.add(ledarr_set_endpoint);
        ar.add(ledarr_get_endpoint);
#endif
        
        serialize();
    }
}