#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "StopWatch";     // Numele rețelei Wi-Fi
const char* password = "12345678+"; // Parola rețelei Wi-Fi

WebServer server(80);

unsigned long startTime = 0;      // Momentul în care a început cronometrul
unsigned long elapsedTime = 0;    // Timpul trecut salvat (în secunde)
unsigned long countdownTime = 0;  // Timpul pentru countdown (în secunde)
bool running = false;             // Flag pentru dacă cronometrul rulează
bool isCountdown = false;         // Flag pentru modul countdown

// Funcție pentru generarea paginii HTML
String generateHTML() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Cronometru ESP32</title>";
  html += "<script>";
  html += "function updateTime() {";
  html += "  fetch('/time').then(response => response.text()).then(data => {";
  html += "    document.getElementById('time').innerText = data;";
  html += "    setTimeout(updateTime, 1000);";
  html += "  });";
  html += "}";
  html += "function startTimer() { fetch('/start'); }";
  html += "function stopTimer() { fetch('/stop'); }";
  html += "function resetTimer() { fetch('/reset'); }";
  html += "function setTime() {";
  html += "  let time = document.getElementById('time_input').value;";
  html += "  if(time && time > 0) fetch('/set_time?time=' + time);";
  html += "}";
  html += "window.onload = updateTime;";
  html += "</script></head><body>";
  html += "<h1>Cronometru</h1>";
  html += "<p id='time'>00:00:00</p>";
  html += "<label for='time_input'>Setează timpul (în secunde):</label>";
  html += "<input type='number' id='time_input' min='1'>";
  html += "<button onclick='setTime()'>Setează Countdown</button>";
  html += "<button onclick='startTimer()'>Start</button>";
  html += "<button onclick='stopTimer()'>Stop</button>";
  html += "<button onclick='resetTimer()'>Reset</button>";
  html += "</body></html>";
  return html;
}

// Handlere pentru server
void handleRoot() {
  server.send(200, "text/html", generateHTML());
}

void handleTime() {
  unsigned long currentTime = millis();
  unsigned long displayTime = elapsedTime;

  if (running) {
    // Calculăm timpul curent, fie countdown, fie cronometrul normal
    if (isCountdown) {
      displayTime = countdownTime - (currentTime - startTime) / 1000;
      if (displayTime <= 0) {
        displayTime = 0;
        running = false; // Oprire automată la final
      }
    } else {
      displayTime = elapsedTime + (currentTime - startTime) / 1000;
    }
  }

  // Convertim timpul în ore, minute, secunde
  unsigned long seconds = displayTime % 60;
  unsigned long minutes = (displayTime / 60) % 60;
  unsigned long hours = (displayTime / 3600);

  char timeString[9];
  sprintf(timeString, "%02lu:%02lu:%02lu", hours, minutes, seconds);
  server.send(200, "text/plain", timeString);
}

void handleStart() {
  if (!running) {
    running = true;
    startTime = millis();
  }
  server.send(200, "text/plain", "Started");
}

void handleStop() {
  if (running) {
    elapsedTime += (millis() - startTime) / 1000; // Salvează timpul total
    running = false;
  }
  server.send(200, "text/plain", "Stopped");
}

void handleReset() {
  running = false;
  elapsedTime = 0;
  countdownTime = 0;
  isCountdown = false;
  server.send(200, "text/plain", "Reset");
}

void handleSetTime() {
  if (server.hasArg("time")) {
    countdownTime = server.arg("time").toInt();
    elapsedTime = 0;
    isCountdown = true;
    running = false; // Nu pornește automat
  }
  server.send(200, "text/plain", "Countdown set");
}

void setup() {
  Serial.begin(115200);

  if (WiFi.softAP(ssid, password)) {
    Serial.println("Wi-Fi AP started!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("Error: Could not start Wi-Fi!");
    while (1);
  }

  server.on("/", handleRoot);
  server.on("/time", handleTime);
  server.on("/start", handleStart);
  server.on("/stop", handleStop);
  server.on("/reset", handleReset);
  server.on("/set_time", handleSetTime);

  server.begin();
  Serial.println("HTTP server started.");
}

void loop() {
  server.handleClient();
}
