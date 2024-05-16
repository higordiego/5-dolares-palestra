#include <WiFi.h>
#include <HTTPClient.h>

const char *ssid = "Wifi";
const char *password = "";

IPAddress ips[255];
int currentIpIndex = 1;
int ports[] = {80, 22, 443};

void setup() {
  Serial.begin(19200);
  delay(10);
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  scanner();
  delay(5000);
}

void scanner() {
  if (currentIpIndex >= 255) {
    currentIpIndex = 0;
  }
  byte firstThreeOctets[3];
  IPAddress localIP = WiFi.localIP();
  for (int i = 0; i < 255; i++) {
    firstThreeOctets[i] = localIP[i];
  }
  IPAddress ipToTest(firstThreeOctets[0], firstThreeOctets[1], firstThreeOctets[2], currentIpIndex);
  // Data structure to store the results
  String resultJSON = "{\"results\": [";
  for (int j = 0; j < sizeof(ports) / sizeof(ports[0]); j++) {
    WiFiClient client;
    if (client.connect(ipToTest, ports[j])) {
      Serial.printf("Connected to %s on port %d\n", ipToTest.toString().c_str(), ports[j]);
      resultJSON += "{\"ip\": \"" + ipToTest.toString() + "\", \"port\": " + String(ports[j]) + ", \"status\": \"success\"},";
      client.stop();
    } else {
      Serial.printf("Connection failed to %s on port %d\n", ipToTest.toString().c_str(), ports[j]);
      resultJSON += "{\"ip\": \"" + ipToTest.toString() + "\", \"port\": " + String(ports[j]) + ", \"status\": \"failed\"},";
    }
  }
  resultJSON.remove(resultJSON.length() - 1); // Remove a vírgula extra no final
  resultJSON += "]}";
  // Increment the index for the next IP in the next call
  currentIpIndex++;
  sendResultsToServer(resultJSON);
}

void sendResultsToServer(String json) {
  HTTPClient http;

  // A URL para a qual você deseja enviar os resultados
  const char *url = "https://webhook.site/9db40e4a-9c05-4603-a3c2-49823b0ab0db";

  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(json);

  if (httpResponseCode > 0) {
    Serial.print("HTTP POST request successful. Response code: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.print("HTTP POST request failed. Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}
