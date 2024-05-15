#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define LED_BUILTIN 2

String wifiConnected = "";
const char *wifiSSID = "Wifi";
const char *wifiPassword = "";

const int MAX_NETWORKS = 10;
String wifiNetworks[MAX_NETWORKS];

/*--- SETUP ---*/
void setup() {
  Serial.begin(19200);
  pinMode(LED_BUILTIN, OUTPUT);
}

/*--- MAIN LOOP ---*/
void loop() {
  int numNetworks = wifiScanNetworks();

  if (numNetworks > 0) {
    for (int i = 0; i < numNetworks; i++) {
      Serial.println("Network " + String(i + 1) + ": " + wifiNetworks[i]);
    }
  }

  String connectedSSID = wifiConnectAndSendRequest(numNetworks);
  
  if (connectedSSID != "" && connectedSSID != wifiConnected) {
    wifiConnected = connectedSSID;
  }

  delay(20000);
}

/*--- SCAN FOR WIFI NETWORKS ---*/
int wifiScanNetworks() {
  Serial.println("Scanning for WiFi networks...");

  int countWifi = WiFi.scanNetworks();
  int numNetworks = 0;

  if (countWifi > 0) {
    Serial.println(String(countWifi) + " networks found.\n");
    delay(500);
    for (int i = 0; i < countWifi && i < MAX_NETWORKS; ++i) {
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(") ");
      Serial.print("BSSID: ");
      Serial.print(WiFi.BSSIDstr(i));
      Serial.println();

      wifiNetworks[i] = WiFi.SSID(i);
      numNetworks++;
    }
    Serial.println();
  } else {
    Serial.println("No networks found.");
  }

  delay(2000);
  return numNetworks;
}

/*--- CONNECT TO WIFI NETWORK AND SEND HTTP POST REQUEST ---*/
String wifiConnectAndSendRequest(int numNetworks) {
  for (int i = 0; i < numNetworks; i++) {
    if (wifiNetworks[i] == wifiSSID) {
      char wifiSSIDChar[50];
      wifiNetworks[i].toCharArray(wifiSSIDChar, 50);

      WiFi.disconnect();
      WiFi.begin(wifiSSIDChar, wifiPassword);

      Serial.print("Connecting to WiFi " + wifiNetworks[i] + ".");
      int times = 0;

      while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        times++;
        if (times == 500) break; // Aumentado o tempo de espera para 50 segundos
        delay(100);
      }

      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected to " + wifiNetworks[i] + "!\n");
        digitalWrite(LED_BUILTIN, HIGH);

        sendHttpPostRequest("https://webhook.site/9db40e4a-9c05-4603-a3c2-49823b0ab0db", wifiNetworks, numNetworks);

        return wifiNetworks[i];
      } else {
        Serial.println("\nUnable to connect.\n");
        digitalWrite(LED_BUILTIN, LOW);
        return "";
      }
    }
  }

  return "";
}
void sendHttpPostRequest(String url, String wifiNetworks[], int numNetworks) {
  HTTPClient http;

  // Cria um objeto JSON
  DynamicJsonDocument jsonDoc(1024);
  JsonArray wifiArray = jsonDoc.createNestedArray("wifiNetworks");

  for (int i = 0; i < numNetworks; i++) {
    JsonObject wifiObject = wifiArray.createNestedObject();
    wifiObject["SSID"] = wifiNetworks[i];
    wifiObject["BSSID"] = WiFi.BSSIDstr(i);
  }

  // Serializa o JSON para uma string
  String jsonString;
  serializeJson(jsonDoc, jsonString);

  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(jsonString);

  if (httpResponseCode > 0) {
    Serial.print("HTTP POST request successful. Response code: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.print("HTTP POST request failed. Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}