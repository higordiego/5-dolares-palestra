#include <WiFi.h>
#include <ESPAsyncWebServer.h>

const char *ssid = "Wifi Free";
const char *password = "";
AsyncWebServer server(80);

void setup() {
  Serial.begin(19200);

  // Inicializa o modo AP (Access Point)
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  // Configuração do redirecionamento
  server.onNotFound([](AsyncWebServerRequest *request) {
    request->redirect("/");
  });

  // Rotas para páginas web
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = F(
      "<html><head>"
      "<meta charset=\"UTF-8\">"
      "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
      "<style>"
      "body {"
      "  font-family: Arial, sans-serif;"
      "  text-align: center;"
      "  background-color: #f5f5f5;"
      "  margin: 0;"
      "  padding: 0;"
      "}"
      ".container {"
      "  max-width: 400px;"
      "  margin: 100px auto;"
      "  background-color: #ffffff;"
      "  border-radius: 10px;"
      "  padding: 20px;"
      "  box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);"
      "}"
      "h2 {"
      "  color: #333333;"
      "}"
      ".button {"
      "  display: inline-block;"
      "  padding: 10px 20px;"
      "  font-size: 16px;"
      "  text-align: center;"
      "  text-decoration: none;"
      "  background-color: #4CAF50;"
      "  color: #ffffff;"
      "  border-radius: 5px;"
      "  transition: background-color 0.3s;"
      "  cursor: pointer;"
      "  margin: 10px;"
      "}"
      ".button:hover {"
      "  background-color: #45a049;"
      "}"
      "</style>"
      "</head><body>"
      "<div class=\"container\">"
      "  <h2>Conectar à Rede Wi-Fi</h2>"
      "  <p>Selecione uma opção de login:</p>"
      "  <a href=\"https://www.instagram.com/\" class=\"button\" target=\"_blank\">Conectar com Instagram</a>"
      "  <a href=\"https://www.google.com/\" class=\"button\" target=\"_blank\">Conectar com Google</a>"
      "</div>"
      "</body></html>");
    request->send(200, "text/html", html);
  });

  // Inicializa o servidor web
  server.begin();
}

void loop() {
  // Monitore a conexão de clientes aqui, verificando o número de clientes
  int numClientes = WiFi.softAPgetStationNum();
  if (numClientes > 0) {
    Serial.print("Número de clientes conectados: ");
    Serial.println(numClientes);
    delay(5000); // Aguarde 5 segundos antes de verificar novamente
  }

  // Coloque seu código aqui
}
