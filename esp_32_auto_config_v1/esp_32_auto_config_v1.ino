#include <WiFi.h>
#include <WebServer.h>

const char* ap_ssid = "ESP32_Config";
const char* ap_password = "123456789";

// Web server runs on port 80
WebServer server(80);

String ssid = "";
String password = "";

// HTML page for Wi-Fi setup
const char* html_page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 Wi-Fi Config</title>
</head>
<body>
  <h2>Configure Wi-Fi</h2>
  <form action="/save" method="post">
    <label for="ssid">SSID:</label><br>
    <input type="text" id="ssid" name="ssid"><br><br>
    <label for="pass">Password:</label><br>
    <input type="password" id="pass" name="pass"><br><br>
    <input type="submit" value="Save">
  </form>
</body>
</html>
)rawliteral";

// Handle root request
void handleRoot() {
  server.send(200, "text/html", html_page);
}

// Handle form submission to save Wi-Fi credentials
void handleSave() {
  if (server.hasArg("ssid") && server.hasArg("pass")) {
    ssid = server.arg("ssid");
    password = server.arg("pass");

    // Save credentials to non-volatile storage (optional)
    // For simplicity, using global variables here

    server.send(200, "text/html", "<h3>Credentials Saved. Rebooting...</h3>");
    delay(2000);
    ESP.restart(); // Restart to connect to new Wi-Fi
  } else {
    server.send(400, "text/plain", "Missing parameters");
  }
}

void setup() {
  Serial.begin(115200);
  
  // Start as AP
  WiFi.softAP(ap_ssid, ap_password);
  Serial.println("Access Point started");
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  // Define routes
  server.on("/", handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.begin();
  Serial.println("Web server started");
}

void loop() {
  server.handleClient();
  // Optional: attempt Wi-Fi connection if credentials are set
  if (ssid != "" && WiFi.status() != WL_CONNECTED) {
    Serial.println("Trying to connect to Wi-Fi...");
    WiFi.begin(ssid.c_str(), password.c_str());
    unsigned long startAttemptTime = millis();
    while (millis() - startAttemptTime < 20000) { // 20 seconds timeout
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        break;
      }
      delay(1000);
    }
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Failed to connect");
    }
  }
}