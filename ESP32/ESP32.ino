/**
   BasicHTTPSClient.ino

    Created on: 14.10.2018

*/

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
// #include <LiquidCrystal_I2C.h>

// Set the pins on the I2C chip used for LCD connections:
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol

//LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // 設定 LCD I2C 位址 julie
// LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // 設定 LCD I2C 位址  jonson


#include "BLINKER_PMSX003ST.h"
#if defined(ESP32)
HardwareSerial pmsSerial(2);// UART1/Serial1 pins 16,17
#else
#include <SoftwareSerial.h>
SoftwareSerial pmsSerial(4,5);
#endif
BLINKER_PMSX003ST pms;

static int send_data_to_server = 20;
static int pms_error = 0;



//CA 到期日  2023年8月4日 星期五 清晨7:59:59
const char* rootCACertificate = \
"-----BEGIN CERTIFICATE-----\n" \ 
"MIIEvjCCA6agAwIBAgIQBtjZBNVYQ0b2ii+nVCJ+xDANBgkqhkiG9w0BAQsFADBh\n" \ 
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \ 
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \ 
"QTAeFw0yMTA0MTQwMDAwMDBaFw0zMTA0MTMyMzU5NTlaME8xCzAJBgNVBAYTAlVT\n" \ 
"MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxKTAnBgNVBAMTIERpZ2lDZXJ0IFRMUyBS\n" \ 
"U0EgU0hBMjU2IDIwMjAgQ0ExMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKC\n" \ 
"AQEAwUuzZUdwvN1PWNvsnO3DZuUfMRNUrUpmRh8sCuxkB+Uu3Ny5CiDt3+PE0J6a\n" \ 
"qXodgojlEVbbHp9YwlHnLDQNLtKS4VbL8Xlfs7uHyiUDe5pSQWYQYE9XE0nw6Ddn\n" \ 
"g9/n00tnTCJRpt8OmRDtV1F0JuJ9x8piLhMbfyOIJVNvwTRYAIuE//i+p1hJInuW\n" \ 
"raKImxW8oHzf6VGo1bDtN+I2tIJLYrVJmuzHZ9bjPvXj1hJeRPG/cUJ9WIQDgLGB\n" \ 
"Afr5yjK7tI4nhyfFK3TUqNaX3sNk+crOU6JWvHgXjkkDKa77SU+kFbnO8lwZV21r\n" \ 
"eacroicgE7XQPUDTITAHk+qZ9QIDAQABo4IBgjCCAX4wEgYDVR0TAQH/BAgwBgEB\n" \ 
"/wIBADAdBgNVHQ4EFgQUt2ui6qiqhIx56rTaD5iyxZV2ufQwHwYDVR0jBBgwFoAU\n" \ 
"A95QNVbRTLtm8KPiGxvDl7I90VUwDgYDVR0PAQH/BAQDAgGGMB0GA1UdJQQWMBQG\n" \ 
"CCsGAQUFBwMBBggrBgEFBQcDAjB2BggrBgEFBQcBAQRqMGgwJAYIKwYBBQUHMAGG\n" \ 
"GGh0dHA6Ly9vY3NwLmRpZ2ljZXJ0LmNvbTBABggrBgEFBQcwAoY0aHR0cDovL2Nh\n" \ 
"Y2VydHMuZGlnaWNlcnQuY29tL0RpZ2lDZXJ0R2xvYmFsUm9vdENBLmNydDBCBgNV\n" \ 
"HR8EOzA5MDegNaAzhjFodHRwOi8vY3JsMy5kaWdpY2VydC5jb20vRGlnaUNlcnRH\n" \ 
"bG9iYWxSb290Q0EuY3JsMD0GA1UdIAQ2MDQwCwYJYIZIAYb9bAIBMAcGBWeBDAEB\n" \ 
"MAgGBmeBDAECATAIBgZngQwBAgIwCAYGZ4EMAQIDMA0GCSqGSIb3DQEBCwUAA4IB\n" \ 
"AQCAMs5eC91uWg0Kr+HWhMvAjvqFcO3aXbMM9yt1QP6FCvrzMXi3cEsaiVi6gL3z\n" \ 
"ax3pfs8LulicWdSQ0/1s/dCYbbdxglvPbQtaCdB73sRD2Cqk3p5BJl+7j5nL3a7h\n" \ 
"qG+fh/50tx8bIKuxT8b1Z11dmzzp/2n3YWzW2fP9NsarA4h20ksudYbj/NhVfSbC\n" \ 
"EXffPgK2fPOre3qGNm+499iTcc+G33Mw+nur7SpZyEKEOxEXGlLzyQ4UfaJbcme6\n" \ 
"ce1XR2bFuAJKZTRei9AqPCCcUZlM51Ke92sRKw2Sfh3oius2FkOH6ipjv3U/697E\n" \ 
"A7sKPPcw7+uvTPyLNhBzPvOk\n" \ 
"-----END CERTIFICATE-----\n";


//===========================

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// ============================



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


void LCD_display_data(double p25, double form, double temp, double humi)
{
  display.clearDisplay();
  display.setTextSize(2);             
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0,0);

  
  display.print("PM25:");
  display.println(p25);
  
  display.print("Form:");
  display.println(form);

  display.print("Temp:");  
  display.println(temp);
  
  display.print("Humi:");
  display.println(humi);

  display.display();
  //delay(2000);
}


void LCD_no_finger(){
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(SSD1306_WHITE);             
  display.setCursor(0,0);
  display.println("Finger");
  display.println("Please");
  display.display();
}

void LCD_setup()
{
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
    {
      ; // Don't proceed, loop forever
    }
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(200); // Pause for 2 seconds
  LCD_no_finger();
}




void setup() {
  
    Serial.begin(115200);
    LCD_setup();
    
    // Serial.setDebugOutput(true);
    Serial.println("\nStart");
    pmsSerial.begin(9600);
    pms.begin(pmsSerial);
    pms.wakeUp();
    pms.setMode(PASSIVE);

    Serial.println();
    Serial.println();
    Serial.println();

    WiFi.mode(WIFI_STA);
    WiFiMulti.addAP("puper", "*999888*");
    WiFiMulti.addAP("ggggg", "hhhh8888");
    WiFiMulti.addAP("bbbbb", "5555dddd");

  

    Serial.print("Waiting for WiFi to connect...");
    while ((WiFiMulti.run() != WL_CONNECTED)) {
        Serial.print(".");
    }
    Serial.println(" connected");
    setClock();

 
}


void https_request(String url) {
    WiFiClientSecure *client = new WiFiClientSecure;
    if (client) {
        client->setCACert(rootCACertificate);
        {
            // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is
            HTTPClient https;
            Serial.println(url);
            Serial.print("[HTTPS] begin...\n");
            if (https.begin(*client, url)) { // HTTPS
                Serial.print("[HTTPS] GET...\n");
                // start connection and send HTTP header
                int httpCode = https.GET();
                Serial.printf("[HTTPS] code: %d\r\n", httpCode);
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
                    ESP.restart();
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

void loop() {
    Serial.println("debug:loop start");
    
    
    send_data_to_server = send_data_to_server + 1;


    pms.request();

    if (!pms.read()) {
        Serial.println("debug:pm read fail+");
		pms_error = pms_error +1;
		if(pms_error > 30){
			ESP.restart();
		}
        delay(1000);
        return;
    }
	else{
		pms_error = 0;
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

    LCD_display_data(pms.getPmCf1(2.5),    pms.getForm() ,    pms.getTemp(),    pms.getHumi());  
    

    if (send_data_to_server > 20) {
        setClock();
        String url = String("") + "https://api.thingspeak.com/update?api_key=1W08OWZCHPUB9IAX&field1=" + pms.getPmCf1(2.5) + "&field2=" + pms.getPmCf1(1.0) + "&field3=" + pms.getForm() + "&field4=" + pms.getTemp() + "&field5=" + pms.getHumi();
        
        https_request(url);
        Serial.println();
        send_data_to_server =0;
    }

    
    
    delay(1000);
    Serial.println("debug:loop done");
}
