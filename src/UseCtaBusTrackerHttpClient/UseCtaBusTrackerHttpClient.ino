#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <stdio.h>

// related to monocromatic OLED
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#include <ArduinoJson.h>

// related to monocromatic OLED
/* Uncomment the initialize the I2C address , uncomment only one, If you get a totally blank screen try the other*/
#define i2c_Address 0x3c // initialize with the I2C addr 0x3C Typically eBay OLED's
// #define i2c_Address 0x3d //initialize with the I2C addr 0x3D Typically Adafruit OLED's

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1    //   QT-PY / XIAO
#define MAX_CHARS 19     // number of max characters to show on the 096 OLED

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// related to wifi
#ifndef PRODUCTION_BUILD
#include "wifi-config.h"
#include "cta-config.h"
#endif

// related to CTA api
#define NORTH_CAMPBLE_WEST "950"
#define NORTH_CAMPBLE_EAST "893"
#define WESTERN_LEMOYNE_SOUTH "17343"
#define WESTERN_LEMOYNE_NORTH "8394"

int PUSH_BUTTON = 0;

// definitions
typedef struct
{
  char stopName[50];
  char route[10];
  char direction[20];
  char predictedTime[20];
  int predictedMin;
} BusPrediction;

void setup()
{

  setupSerial();
  setupWiFi();
  setupDisplay();

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PUSH_BUTTON, INPUT);
}

void loop()
{
  if ((WiFi.status() == WL_CONNECTED))
  {
    int buttonState = digitalRead(PUSH_BUTTON);

    if (buttonState == LOW)
    {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(1000);

      getPredictions();

      digitalWrite(LED_BUILTIN, LOW);
      delay(10000);
    }
  }

  delay(100);
}

void setupDisplay()
{
  delay(250);                       // wait for the OLED to power up
  display.begin(i2c_Address, true); // Address 0x3C default
                                    // display.setContrast (0); // dim display

  display.display();
  delay(1000);
}

void setupSerial()
{
  Serial.begin(115200);

  Serial.println();
  Serial.println();
  Serial.println();
}

void setupWiFi()
{
  WiFi.begin(STASSID, STAPSK);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
}

void getPredictions()
{
  WiFiClient client;
  HTTPClient http;
  String stopIds[] = {NORTH_CAMPBLE_WEST, NORTH_CAMPBLE_EAST, WESTERN_LEMOYNE_SOUTH, WESTERN_LEMOYNE_NORTH};
  Serial.print("stopIds: ");
  int stopIdsSize = sizeof(stopIds) / sizeof(stopIds[0]);

  for (int i = 0; i < stopIdsSize; i++)
  {
    Serial.println(stopIds[i]);
  }

  String stpidParam = buildStpidParamsForApi(stopIds, stopIdsSize);

  String query = "?key=" CTA_BUS_TRACKER_API_KEY "&format=json&stpid=" + stpidParam;
  String url = BASE_API_URL V2_GET_PREDICTIONS + query;
  Serial.print("[HTTP] begin getting predictions: " + url + "\n");

  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.GET();
  Serial.print("httpCode: ");
  Serial.println(httpCode);
  if (httpCode == HTTP_CODE_OK)
  {
    Serial.println(">>");

    const String &payload = http.getString();

    Serial.println(">>");
    Serial.println(payload);
    Serial.println("<<");
    // Parse the JSON response
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);

    Serial.print("Error-c str: ");
    Serial.println(error.c_str());
    Serial.print("Error-f str: ");
    Serial.println(error.f_str());
    Serial.print("Error-code: ");
    Serial.println(error.code());
    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    // Access the "prd" array
    JsonArray prd = doc["bustime-response"]["prd"];
    Serial.print("bustime-reponse.prd: ");
    Serial.println(prd);

    // Iterate through the predictions
    BusPrediction busPredictions[prd.size()];

    int i = 0;
    for (JsonVariant prediction : prd)
    {
      i++;
      // Extract relevant information
      String stpnm = prediction["stpnm"];
      String prdtm = prediction["prdtm"];
      String rt = prediction["rt"];
      String rtdir = prediction["rtdir"];
      int prdctdn = prediction["prdctdn"];

      BusPrediction p;
      strcpy(p.stopName, stpnm.c_str());
      strcpy(p.route, rt.c_str());
      strcpy(p.direction, rtdir.c_str());
      strcpy(p.predictedTime, prdtm.c_str());
      p.predictedMin = prdctdn;

      busPredictions[i] = p;

      Serial.print(i);
      if (i < 10)
      {
        Serial.print(" ");
      }
      // Use the extracted information
      Serial.print(" Stop Name: ");
      Serial.println(stpnm);
      Serial.print("   Route: ");
      Serial.println(rt);
      Serial.print("   Direction: ");
      Serial.println(rtdir);
      Serial.print("   Predicted Arrival in: ");
      Serial.println(prdctdn);
      Serial.print("   Predicted Arrival Time: ");
      Serial.println(prdtm);
      Serial.println();
    }

    // todo: do something with busPredictions
    int array_size = sizeof(busPredictions) / sizeof(busPredictions[0]);
    doSomethingWithIt(busPredictions, array_size);
  }
  else
  {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    // todo-mari: turn the error light on, output Error message.
  }

  http.end();

  const char *message = "Hola mi amor!";
  writeOnScreen(message);
}

String buildStpidParamsForApi(const String *stopIds, int numStopIds)
{
  String stpidParam = "";

  Serial.print("stopIds size: ");
  Serial.println(numStopIds);
  for (int i = 0; i < numStopIds; i++)
  {
    stpidParam += stopIds[i];
    if (i < numStopIds - 1)
    {
      stpidParam += ",";
    }
  }

  Serial.print("stpidParam: ");
  Serial.println(stpidParam);
  return stpidParam;
}

void writeOnScreen(const char *text)
{
  Serial.println(text);
}

String truncatetext(String stpnm, int maxChars)
{
  char stpnm_shorten[maxChars];                        // 9 characters + 1 for null terminator
  strncpy(stpnm_shorten, stpnm.c_str(), maxChars - 1); // convert stop name from string to char *
  stpnm_shorten[maxChars - 1] = '\0';                  // Ensure null-termination

  return stpnm_shorten;
}

void doSomethingWithIt(BusPrediction predictions[], int predictionsLength)
{

  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SH110X_WHITE); // Draw white text
  display.setCursor(0, 0);            // Start at top-left corner
  display.println(F("---CTA BUS---"));
  display.println("Trying to invoke method: Do something with it");
  display.display();
  Serial.println("Trying to invoke method: Do something with it");
  for (int i = 0; i < predictionsLength; i++)
  {
    BusPrediction p = predictions[i];
    int position = i + 1; // i++;

    Serial.println(p.stopName);

    display.clearDisplay();
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(SH110X_WHITE); // Draw white text
    display.setCursor(0, 0);            // Start at top-left corner
    display.println(F("---CTA BUS---"));
    String stpnm_shorten = truncatetext(p.stopName, MAX_CHARS);
    display.print(position);
    if (position < 10)
    {
      display.print(" ");
    }
    display.print(" ");
    display.println(stpnm_shorten); // stop name, ie: North & Campbell
    display.print("   ");
    display.print(p.route); // route, ie: 49
    display.print(" ");
    display.println(p.direction); // direction, ie: South
    display.print("   ");
    display.print(p.predictedMin);
    display.println("min");
    display.display();
    delay(3000); // wait 3 s before showing the next result
    display.clearDisplay();
  }

  display.setCursor(0, 0); // Start at top-left corner
  display.println("doSomethingWithIt executed!");
  display.display();
  delay(1000);
  display.clearDisplay();
}
