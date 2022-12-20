/* -------------------------------------------------------------------
 * AdminESP - ElectronicIOT 2022
 * Sitio WEB: https://electroniciot.com
 * Correo: admim@electroniciot.com
 * Cel_WSP: +591 71243395
 * Plataforma: ESP32
 * Framework:  Arduino
 * Proyecto Admin Panel Tool para el ESP32 con HTNL, JavaScript, CSS
 * ESP32 Admin Tool v2.0
 * -------------------------------------------------------------------
*/

// -------------------------------------------------------------------
// Librerias
// -------------------------------------------------------------------
#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <TimeLib.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#define DHTPIN 4     // El número que se le debe asignar a DHTPIN debe ser el número del pin GPIO de la tarjeta ESP32 que se utilice para conectar el sensor DHT22.
#define DHTTYPE DHT22  // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);
// -------------------------------------------------------------------
// Archivos *.hpp - Fragmentar el Código
// -------------------------------------------------------------------
#include <DHT.h>
#include "settings.hpp"
#include "functions.hpp"
#include "settingsReset.hpp"
#include "settingsRead.hpp"
#include "settingsSave.hpp"
#include "esp32_wifi.hpp"
#include "esp32_mqtt.hpp"
#include "esp32_websocket.hpp"
#include "esp32_server.hpp"
#include "esp32_pinRestore.hpp"
#include "Udp.h"
// -------------------------------------------------------------------
// Setup
// -------------------------------------------------------------------

String readSensorTemperature(){
  // Read temperature as Celsius (the default)
 
  float t = dht.readTemperature();
  
  // Convert temperature to Fahrenheit
  //t = 1.8 * t + 32;
  if (isnan(t)) {    
    Serial.println("Failed to read from sensor!");
    return "";
  }
  else {
    Serial.println(t);
    return String(t);
  }
  
}
float tmp2;
String readSensorHumidity() {
  float h = dht.readHumidity();


  if (isnan(h)) {
    Serial.println("Failed to read from sensor!");
    return "";
  }
  else {
    Serial.println(h);
    return String(h);
  }
}

void setup() {
    // Baudrate
    Serial.begin(115200);
    // CPU Freq
    setCpuFrequencyMhz(240);
    // Inicio del Log por serial
    log("\nInfo: Iniciando Setup");
    // Configurar los Pines
    settingPines();
    // Inicio del SPIFFS                 
    if (!SPIFFS.begin(true)){
        log(F("Error: Falló la inicialización del SPIFFS"));
        while (true);
    }
    // Lee los estados de los Relays
    if(!settingsReadRelays()){
        //salvar estados de los relays
        settingsSaveRelays();
    }
    // Paso estados a los pines de los Relays
    setOnOffSingle(RELAY1,Relay01_status);
    setOnOffSingle(RELAY2,Relay02_status);
    // Lee la Configuración WiFi
    if(!settingsReadWiFi()){        
        // Salvar las configuraciones del WIFI
        settingsSaveWiFi();
    }
    // Incrementar el contador de reinicios
   // bootCount++;
    // Salvar las configuraciones del WIFI + incremento del bootCount
    settingsSaveWiFi();
    // Configuracion WIFI
    WiFi.disconnect(true);
    delay(1000);
    // Setup del WiFI
    wifi_setup(); 
    // Lee la Configuración MQTT
    if(!settingsReadMQTT()){
        // Salvar la configuracion del MQTT
        settingsSaveMQTT();
    } 
    // leer www_username/password
    if(!settingsReadAdmin()){
        // Salvar el usuario y Contraseña
        settingsSaveAdmin();
    }   
    // Inicializar el WebSockets
    InitWebSockets();
    // Inicializar el Servidor
printf(ap_nameap);
    InitServer();
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readSensorTemperature().c_str());
  });
   server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", "sdsd");
  });

  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readSensorHumidity().c_str());
  });

  server.on("/highcharts.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/highcharts.js", "text/javascript");
  });

    // Nos devuelve la lista de carpetas y archivos del SPIFFS ONLYDEBUG
    // listDir(SPIFFS, "/", 0); 
    setupPinRestore();
    log("Info: Setup completado");   

}


// -------------------------------------------------------------------
// Loop Pincipal Nucleo 0
// -------------------------------------------------------------------
void loop() {

    yield();
    // -------------------------------------------------------------------
    // WIFI
    // -------------------------------------------------------------------
    if (wifi_mode == WIFI_STA){
        wifiLoop();
    }else if (wifi_mode == WIFI_AP){
        wifiAPLoop();
    } 
    // -------------------------------------------------------------------
    // MQTT
    // -------------------------------------------------------------------
    if ((WiFi.status() == WL_CONNECTED) && (wifi_mode == WIFI_STA)){
        if(mqtt_server != 0){
            mqttLoop();
            if (mqttclient.connected()){
                if (millis() - lastMsg > mqtt_time){
                    lastMsg = millis();
                    mqtt_publish();
                }
            }      
        }
    }
    // -------------------------------------------------------------------
    // Enviar JSON por WS 
    // -------------------------------------------------------------------
    if (millis() - lastWsSend > 1000){
        lastWsSend = millis();
        WsMessage(GetJson(), "", "");
    }
    // -------------------------------------------------------------------
    // Monitorear la Interrupción del Pin 33
    // -------------------------------------------------------------------
    restoreIntLoop();
   /*if(UDP::available(tmp2))
    {
      
    }*/
}