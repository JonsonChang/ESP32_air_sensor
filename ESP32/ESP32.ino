/**
   BasicHTTPSClient.ino

    Created on: 14.10.2018

*/

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <LiquidCrystal_I2C.h>

// Set the pins on the I2C chip used for LCD connections:
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // 設定 LCD I2C 位址


#include "BLINKER_PMSX003ST.h"
#if defined(ESP32)
HardwareSerial pmsSerial(2);// UART1/Serial1 pins 16,17
#else
#include <SoftwareSerial.h>
SoftwareSerial pmsSerial(4,5);
#endif
BLINKER_PMSX003ST pms;

static int send_data_to_server = 20;
static int turn_off_backlight=600;

// This is GandiStandardSSLCA2.pem, the root Certificate Authority that signed 
// the server certifcate for the demo server https://jigsaw.w3.org in this
// example. This certificate is valid until Sep 11 23:59:59 2024 GMT
const char* rootCACertificate = \
"-----BEGIN CERTIFICATE-----\n" \ 
"MIIDxTCCAq2gAwIBAgIQAqxcJmoLQJuPC3nyrkYldzANBgkqhkiG9w0BAQUFADBs\n" \ 
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \ 
"d3cuZGlnaWNlcnQuY29tMSswKQYDVQQDEyJEaWdpQ2VydCBIaWdoIEFzc3VyYW5j\n" \ 
"ZSBFViBSb290IENBMB4XDTA2MTExMDAwMDAwMFoXDTMxMTExMDAwMDAwMFowbDEL\n" \ 
"MAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcGA1UECxMQd3d3\n" \ 
"LmRpZ2ljZXJ0LmNvbTErMCkGA1UEAxMiRGlnaUNlcnQgSGlnaCBBc3N1cmFuY2Ug\n" \ 
"RVYgUm9vdCBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMbM5XPm\n" \ 
"+9S75S0tMqbf5YE/yc0lSbZxKsPVlDRnogocsF9ppkCxxLeyj9CYpKlBWTrT3JTW\n" \ 
"PNt0OKRKzE0lgvdKpVMSOO7zSW1xkX5jtqumX8OkhPhPYlG++MXs2ziS4wblCJEM\n" \ 
"xChBVfvLWokVfnHoNb9Ncgk9vjo4UFt3MRuNs8ckRZqnrG0AFFoEt7oT61EKmEFB\n" \ 
"Ik5lYYeBQVCmeVyJ3hlKV9Uu5l0cUyx+mM0aBhakaHPQNAQTXKFx01p8VdteZOE3\n" \ 
"hzBWBOURtCmAEvF5OYiiAhF8J2a3iLd48soKqDirCmTCv2ZdlYTBoSUeh10aUAsg\n" \ 
"EsxBu24LUTi4S8sCAwEAAaNjMGEwDgYDVR0PAQH/BAQDAgGGMA8GA1UdEwEB/wQF\n" \ 
"MAMBAf8wHQYDVR0OBBYEFLE+w2kD+L9HAdSYJhoIAu9jZCvDMB8GA1UdIwQYMBaA\n" \ 
"FLE+w2kD+L9HAdSYJhoIAu9jZCvDMA0GCSqGSIb3DQEBBQUAA4IBAQAcGgaX3Nec\n" \ 
"nzyIZgYIVyHbIUf4KmeqvxgydkAQV8GK83rZEWWONfqe/EW1ntlMMUu4kehDLI6z\n" \ 
"eM7b41N5cdblIZQB2lWHmiRk9opmzN6cN82oNLFpmyPInngiK3BD41VHMWEZ71jF\n" \ 
"hS9OMPagMRYjyOfiZRYzy78aG6A9+MpeizGLYAiJLQwGXFK3xPkKmNEVX58Svnw2\n" \ 
"Yzi9RKR/5CYrCsSXaQ3pjOLAEFe4yHYSkVXySGnYvCoCWw9E1CAx2/S6cCZdkGCe\n" \ 
"vEsXCS+0yx5DaMkHJ8HSXPfqIbloEpw8nL+e/IBcm2PN7EeqJSdnoDfzAIJ9VNep\n" \ 
"+OkuE6N36B9K\n" \ 
"-----END CERTIFICATE-----\n";

// Not sure if WiFiClientSecure checks the validity date of the certificate.
// Setting clock just to be sure...
void setClock() {
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");

    Serial.print(F("Waiting for NTP time sync: "));
    time_t nowSecs = time(nullptr);
    while (nowSecs < 8 * 3600 * 2) {
        delay(500);
        Serial.print(F("."));
        yield();
        nowSecs = time(nullptr);
    }

    Serial.println();
    struct tm timeinfo;
    gmtime_r(&nowSecs, &timeinfo);
    Serial.print(F("Current time: "));
    Serial.print(asctime(&timeinfo));
}

WiFiMulti WiFiMulti;

void setup() {

    Serial.begin(115200);
    lcd.begin(16, 2); // 初始化 LCD，一行 16 的字元，共 2 行，預設開啟背光
    for (int i = 0; i < 3; i++) {
        lcd.backlight(); // 開啟背光
        delay(250);
        lcd.noBacklight(); // 關閉背光
        delay(250);
    }
    lcd.backlight();

    // Serial.setDebugOutput(true);
    Serial.println("\nStart");
    pmsSerial.begin(9600);
    pms.begin(pmsSerial);
    //  pms.wakeUp();
    pms.setMode(PASSIVE);

    Serial.println();
    Serial.println();
    Serial.println();

    WiFi.mode(WIFI_STA);
    WiFiMulti.addAP("ggggg", "hhhh8888");

    // wait for WiFi connection
    lcd.clear();
    lcd.setCursor(0, 0); // 設定游標位置在第一行行首
    lcd.print("WiFi connecting.");

    Serial.print("Waiting for WiFi to connect...");
    while ((WiFiMulti.run() != WL_CONNECTED)) {
        Serial.print(".");
    }
    Serial.println(" connected");
    setClock();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi OK.");
}

void print_lcd_data(unsigned int pm25, double HCHO, double tmp, double hum) {
    lcd.clear();
    lcd.setCursor(0, 0);
    // lcd.print("PM2:10 HCHO:0.15");
    lcd.print(String("") + "PM2:" + pm25 + " HCHO:" + HCHO);
    lcd.setCursor(0, 1);
    // lcd.print("Tm:12.2 Hum:55.3");
    lcd.print(String("") + "Tm:" + tmp + " Hm:" + hum);
}

void https_request(String url) {
    WiFiClientSecure *client = new WiFiClientSecure;
    if (client) {
        client->setCACert(rootCACertificate);

        {
            // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is
            HTTPClient https;

            //       "https://api.thingspeak.com/update?api_key=JSZTG4B87AVJAUSI&field1=1&field2=2&field3=0.001&field4=25.3&field5=60"
            String url = String("") + "https://api.thingspeak.com/update?api_key=JSZTG4B87AVJAUSI&field1=" + pms.getPmCf1(2.5) + "&field2=" + pms.getPmCf1(1.0) + "&field3=" + pms.getForm() + "&field4=" + pms.getTemp() + "&field5=" + pms.getHumi();

            Serial.print("[HTTPS] begin...\n");
            if (https.begin(*client, url)) { // HTTPS
                Serial.print("[HTTPS] GET...\n");
                // start connection and send HTTP header
                int httpCode = https.GET();

                // httpCode will be negative on error
                if (httpCode > 0) {
                    // HTTP header has been send and Server response header has been handled
                    Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

                    // file found at server
                    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
                        String payload = https.getString();
                        Serial.println(payload);
                    }
                } else {
                    Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
                }

                https.end();
            } else {
                Serial.printf("[HTTPS] Unable to connect\n");
            }

            // End extra scoping block
        }

        delete client;
    } else {
        Serial.println("Unable to create client");
    }
}

void backlight_display(unsigned int pm25, double HCHO, double tmp, double hum) {

    bool is_over_spec = false;
     Serial.println(        turn_off_backlight);

    if (pm25 > 40 || HCHO > 0.06 || hum>80){
        is_over_spec = true;
        turn_off_backlight = 600;
    }

    if (turn_off_backlight <= 0) {
        lcd.noBacklight(); // 關閉背光
    } else {
        lcd.backlight(); // 開啟背光
    }
}

void loop() {

    send_data_to_server = send_data_to_server + 1;
    if(turn_off_backlight >0){
        turn_off_backlight--;
    }

    pms.request();

    if (!pms.read()) {
        return;
    }
    Serial.print("PM1.0(CF1)\t");
    Serial.print(pms.getPmCf1(1.0));
    Serial.println("ug/m3");
    Serial.print("PM2.5(CF1)\t");
    Serial.print(pms.getPmCf1(2.5));
    Serial.println("ug/m3");
    Serial.print("PM10(CF1)\t");
    Serial.print(pms.getPmCf1(10));
    Serial.println("ug/m3");
    Serial.print("PM1.0(ATO)\t");
    Serial.print(pms.getPmAto(1.0));
    Serial.println("ug/m3");
    Serial.print("PM2.5(ATO)\t");
    Serial.print(pms.getPmAto(2.5));
    Serial.println("ug/m3");
    Serial.print("PM10(ATO)\t");
    Serial.print(pms.getPmAto(10));
    Serial.println("ug/m3");
    Serial.print("  PCS0.3\t");
    Serial.print(pms.getPcs(0.3));
    Serial.println("pcs/0.1L");
    Serial.print("  PCS0.5\t");
    Serial.print(pms.getPcs(0.5));
    Serial.println("pcs/0.1L");
    Serial.print("  PCS1.0\t");
    Serial.print(pms.getPcs(1));
    Serial.println("pcs/0.1L");
    Serial.print("  PCS2.5\t");
    Serial.print(pms.getPcs(2.5));
    Serial.println("pcs/0.1L");
    Serial.print("  PCS5.0\t");
    Serial.print(pms.getPcs(5));
    Serial.println("pcs/0.1L");
    Serial.print("   PCS10\t");
    Serial.print(pms.getPcs(10));
    Serial.println("pcs/0.1L");
    Serial.print("Formalde\t");
    Serial.print(pms.getForm());
    Serial.println("ug/m3");
    Serial.print("Temperat\t");
    Serial.print(pms.getTemp());
    Serial.println("'C");
    Serial.print("Humidity\t");
    Serial.print(pms.getHumi());
    Serial.println("%");
    Serial.println();

// ======================WIFI=====================================

    if (send_data_to_server > 20) {
        setClock();
        String url = String("") + "https://api.thingspeak.com/update?api_key=JSZTG4B87AVJAUSI&field1=" + pms.getPmCf1(2.5) + "&field2=" + pms.getPmCf1(1.0) + "&field3=" + pms.getForm() + "&field4=" + pms.getTemp() + "&field5=" + pms.getHumi();
        https_request(url);
        Serial.println();
        send_data_to_server =0;
    }

    print_lcd_data(pms.getPmCf1(2.5), pms.getForm(), pms.getTemp(), pms.getHumi());
    backlight_display(pms.getPmCf1(2.5), pms.getForm(), pms.getTemp(), pms.getHumi());
    delay(1000);
}
