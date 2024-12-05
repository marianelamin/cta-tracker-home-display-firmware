#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <stdio.h>

#define BASE_API_URL "http://www.ctabustracker.com/bustime/api"
#define V2_GET_PREDICTIONS "/v2/getpredictions"
#define CTA_BUS_TRACKER_API_KEY "some-api-key"

#ifndef STASSID
#define STASSID "some-ssid"
#define STAPSK "some-pwd"
#endif

int PUSH_BUTTON = 0;

void setup() {

  setupSerial();
  setupWiFi();

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PUSH_BUTTON, INPUT);
}

void loop() {
  if ((WiFi.status() == WL_CONNECTED)) {      
    int buttonState = digitalRead(PUSH_BUTTON);
    Serial.println(buttonState);

    if(buttonState == LOW) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(1000);
      
      getPredictions();

      digitalWrite(LED_BUILTIN, LOW);
      delay(10000);
    }
  
  }

  delay(100);
}

void setupSerial() {
  Serial.begin(115200);

  Serial.println();
  Serial.println();
  Serial.println();
}

void setupWiFi() {
  WiFi.begin(STASSID, STAPSK);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
}

void getPredictions() {
    WiFiClient client;
    HTTPClient http;
    char * northCampbellWest = "950";
    char * northCampbellEast = "893";
    char * westernLeMoyneSouth = "17343";
    char * westernLeMoyneNorth = "8394";

    char * query = "?key=" CTA_BUS_TRACKER_API_KEY "&format=json&stpid=" "17343"+","+"8394"+","+"893"+","+"950";
    char * url = BASE_API_URL V2_GET_PREDICTIONS + query ;
    Serial.print("[HTTP] begin getting predictions: " +url+ "\n");

    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.GET();
    if (httpCode > 0) {
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      if (httpCode == HTTP_CODE_OK) {
        const String& payload = http.getString();
        // todo-mari: need to extract the info I need from here.
        Serial.println("received payload:\n<<");
        Serial.println(payload);
        Serial.println(">>");
        //char *message = "Hello Chicago!";
        writeOnScreen(payload);
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      // todo-mari: turn the error light on, output Error message.
    }

    http.end();
}

void writeOnScreen(const char *text) {
      Serial.printf(message);
}

