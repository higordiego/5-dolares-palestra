#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <vector>

const char *ssid = "Bino";
const char *password = "";

WebServer server(80);
DNSServer dnsServer;

const byte DNS_PORT = 53;

struct Message {
  String sender;
  String text;
};

std::vector<Message> messages;  // Armazena as mensagens do chat

struct ClientInfo {
  String ip;
  String name;
};

std::vector<ClientInfo> clients;  // Armazena os clientes conectados

String generateRandomName() {
  const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  String name = "";
  for (int i = 0; i < 8; i++) {
    name += alphanum[random(0, sizeof(alphanum) - 1)];
  }
  return name;
}

String getClientName(String ip) {
  for (const auto& client : clients) {
    if (client.ip == ip) {
      return client.name;
    }
  }
  return "";
}

void setup() {
  Serial.begin(19200);

  // Configura o modo AP e inicia a rede
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  delay(100);

  // Configura o servidor DNS
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  // Inicializa o servidor web
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", getChatPage());
  });

  server.on("/register", HTTP_GET, []() {
    String clientIP = server.client().remoteIP().toString();
    String clientName = getClientName(clientIP);
    if (clientName == "") {
      clientName = generateRandomName();
      clients.push_back({clientIP, clientName});
    }
    server.send(200, "text/plain", clientName);
  });

  server.on("/send", HTTP_POST, []() {
    if (server.hasArg("sender") && server.hasArg("message")) {
      String sender = server.arg("sender");
      String message = server.arg("message");
      messages.push_back({sender, message});
      server.send(200, "text/plain", "Mensagem enviada");
    } else {
      server.send(400, "text/plain", "Erro: Nenhuma mensagem recebida");
    }
  });

  server.on("/messages", HTTP_GET, []() {
    String response;
    for (const auto& msg : messages) {
      response += "<span style='color: green;'>" + msg.sender + "</span>: " + msg.text + "<br>";
    }
    server.send(200, "text/html", response);
  });

  server.begin();
  Serial.println("Servidor web iniciado");

  // Adiciona um log quando um dispositivo se conecta
  WiFi.onEvent(WiFiEvent);
}

void loop() {
  server.handleClient();
  dnsServer.processNextRequest();
}

String getChatPage() {
  return "<!DOCTYPE html>"
         "<html lang='en'>"
         "<head>"
         "  <meta charset='UTF-8'>"
         "  <meta name='viewport' content='width=device-width, initial-scale=1.0'>"
         "  <title>Chat</title>"
         "  <style>"
         "    body, html { margin: 0; padding: 0; height: 100%; font-family: Arial, sans-serif; display: flex; flex-direction: column; font-size: 16px; background-color: #121212; color: #FFFFFF; }"
         "    #chat-container { display: flex; flex-direction: column; height: 100%; }"
         "    #chat-messages { flex: 1; overflow-y: scroll; padding: 10px; border-bottom: 1px solid #333333; }"
         "    #message-form { display: flex; padding: 10px; border-top: 1px solid #333333; }"
         "    #message-input { flex: 1; padding: 10px; border: 1px solid #333333; border-radius: 5px; margin-right: 10px; font-size: 16px; background-color: #1E1E1E; color: #FFFFFF; }"
         "    #message-form button { padding: 10px; border: none; background-color: #007BFF; color: white; border-radius: 5px; cursor: pointer; font-size: 16px; }"
         "    @media (max-width: 600px) {"
         "      #message-form { flex-direction: column; }"
         "      #message-input { margin-right: 0; margin-bottom: 10px; }"
         "      #message-form button { width: 100%; }"
         "    }"
         "  </style>"
         "</head>"
         "<body>"
         "  <div id='chat-container'>"
         "    <div id='chat-messages'></div>"
         "    <form id='message-form'>"
         "      <input type='text' id='message-input' placeholder='Digite sua mensagem'>"
         "      <button type='submit'>Enviar</button>"
         "    </form>"
         "  </div>"
         "  <script>"
         "    let userName = '';"
         ""
         "    function registerUser() {"
         "      const xhttp = new XMLHttpRequest();"
         "      xhttp.onreadystatechange = function() {"
         "        if (this.readyState === 4 && this.status === 200) {"
         "          userName = this.responseText.trim();"
         "        }"
         "      };"
         "      xhttp.open('GET', '/register', true);"
         "      xhttp.send();"
         "    }"
         ""
         "    const form = document.getElementById('message-form');"
         "    const messageInput = document.getElementById('message-input');"
         "    const chatMessages = document.getElementById('chat-messages');"
         ""
         "    form.addEventListener('submit', function(event) {"
         "      event.preventDefault();"
         "      const message = messageInput.value.trim();"
         "      if (userName !== '' && message !== '') {"
         "        appendMessage(userName, message);"
         "        sendMessage(userName, message);"
         "        messageInput.value = '';"
         "      }"
         "    });"
         ""
         "    function appendMessage(sender, message) {"
         "      const messageElement = document.createElement('div');"
         "      messageElement.innerHTML = `<span style='color: green;'>${sender}</span>: ${message}`;"
         "      chatMessages.appendChild(messageElement);"
         "      chatMessages.scrollTop = chatMessages.scrollHeight;"
         "    }"
         ""
         "    function sendMessage(sender, message) {"
         "      const xhttp = new XMLHttpRequest();"
         "      xhttp.onreadystatechange = function() {"
         "        if (this.readyState === 4 && this.status === 200) {"
         "          console.log('Mensagem enviada');"
         "        }"
         "      };"
         "      xhttp.open('POST', '/send', true);"
         "      xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');"
         "      xhttp.send(`sender=${encodeURIComponent(sender)}&message=${encodeURIComponent(message)}`);"
         "    }"
         ""
         "    function pollMessages() {"
         "      const xhttp = new XMLHttpRequest();"
         "      xhttp.onreadystatechange = function() {"
         "        if (this.readyState === 4 && this.status === 200) {"
         "          const messages = this.responseText.trim().split('<br>');"
         "          chatMessages.innerHTML = '';"
         "          messages.forEach((message, index) => {"
         "            if (message) {"
         "              chatMessages.innerHTML += message + '<br>';"
         "            }"
         "          });"
         "        }"
         "      };"
         "      xhttp.open('GET', '/messages', true);"
         "      xhttp.send();"
         "    }"
         ""
         "    setInterval(pollMessages, 2000);"
         "    window.onload = registerUser;"
         "  </script>"
         "</body>"
         "</html>";
}

// Função de callback para eventos Wi-Fi
void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case SYSTEM_EVENT_AP_STACONNECTED:
      Serial.println("Dispositivo conectado.");
      break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
      Serial.println("Dispositivo desconectado.");
      break;
    default:
      break;
  }
}