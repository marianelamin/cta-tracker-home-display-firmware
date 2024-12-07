#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <stdio.h>
// #include <Arduino_JSON.h>

#include <ArduinoJson.h>

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

  String stpidParam = buildStpidParam(stopIds, stopIdsSize);

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

      Serial.print(i);
      if(i<10) {Serial.print(" ");}
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
  }
  else
  {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    // todo-mari: turn the error light on, output Error message.
  }

  const char *message = "Hola mi amor!";
  writeOnScreen(message);

  http.end();
}

String buildStpidParam(const String *stopIds, int numStopIds)
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
