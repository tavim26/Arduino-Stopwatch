#include <WiFi.h>
#include <WebServer.h>

// Prototipuri pentru funcții
void handleStartChronometer();
void handleStopChronometer();
void handleResetChronometer();

void handleStartTimer();
void handleStopTimer();
void handleResetTimer();

void handleStartLapCounter();
void handleResetLapCounter();
void handleTime();

// Configurare rețea Wi-Fi
const char* ssid = "StopWatch";     // Numele rețelei Wi-Fi
const char* password = "12345678+"; // Parola rețelei Wi-Fi

WebServer server(80);

// Variabile pentru cronometru
unsigned long startChronometerTime = 0;
unsigned long elapsedChronometerTime = 0;
bool runningChronometer = false;

// Variabile pentru timer
unsigned long countdownTime = 0;
unsigned long startTimerTime = 0;
bool runningTimer = false;

// Variabile pentru lap counter
unsigned int lapCount = 0;
unsigned long lapStartTime = 0;
unsigned long lapInterval = 0; // Intervalul setat de utilizator (în secunde)
bool runningLapCounter = false;

// Generarea interfeței HTML
String generateHTML() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>ESP32 Stopwatch</title>";
  html += "<style>body { font-family: Arial; } button { margin: 5px; }</style>";
  html += "<script>";
  html += "function updateTime() {";
  html += "  fetch('/time').then(response => response.json()).then(data => {";
  html += "    document.getElementById('chronometer').innerText = data.chronometer;";
  html += "    document.getElementById('timer').innerText = data.timer;";
  html += "    document.getElementById('laps').innerText = data.laps;";
  html += "    setTimeout(updateTime, 1000);";
  html += "  });";
  html += "}";
  html += "function sendRequest(path) { fetch(path); }";
  html += "window.onload = updateTime;";
  html += "</script></head><body>";

  // Secțiunea pentru cronometru
  html += "<h1>ESP32 Stopwatch</h1>";
  html += "<h2>Chronometer</h2>";
  html += "<p id='chronometer'>00:00:00</p>";
  html += "<button onclick=\"sendRequest('/startChronometer')\">Start Chronometer</button>";
  html += "<button onclick=\"sendRequest('/stopChronometer')\">Stop Chronometer</button>";
  html += "<button onclick=\"sendRequest('/resetChronometer')\">Reset Chronometer</button>";

  // Secțiunea pentru timer
  html += "<h2>Timer</h2>";
  html += "<p id='timer'>00:00:00</p>";
  html += "<select id='timer_select'>";
  html += "  <option value='300'>5 Minutes</option>";
  html += "  <option value='600'>10 Minutes</option>";
  html += "  <option value='900'>15 Minutes</option>";
  html += "  <option value='1200'>20 Minutes</option>";
  html += "</select>";
  html += "<button onclick=\"sendRequest('/startTimer?time=' + document.getElementById('timer_select').value)\">Start Timer</button>";
  html += "<button onclick=\"sendRequest('/stopTimer')\">Stop Timer</button>";
  html += "<button onclick=\"sendRequest('/resetTimer')\">Reset Timer</button>";

  // Secțiunea pentru lap counter
  html += "<h2>Lap Counter</h2>";
  html += "<p id='laps'>0</p>";
  html += "<label for='lap_interval'>Set Lap Interval (seconds):</label>";
  html += "<input type='number' id='lap_interval' min='1' step='1'>";
  html += "<button onclick=\"sendRequest('/startLapCounter?interval=' + document.getElementById('lap_interval').value)\">Start Lap Counter</button>";
  html += "<button onclick=\"sendRequest('/resetLapCounter')\">Reset Lap Counter</button>";

  html += "</body></html>";
  return html;
}

// Funcție pentru formatarea timpului în format HH:MM:SS
String formatTime(unsigned long seconds) {
  unsigned long hrs = seconds / 3600;
  unsigned long mins = (seconds / 60) % 60;
  unsigned long secs = seconds % 60;
  char timeString[9];
  sprintf(timeString, "%02lu:%02lu:%02lu", hrs, mins, secs);
  return String(timeString);
}

// Gestionarea timpului pentru toate funcționalitățile
void handleTime() {
  unsigned long currentTime = millis();

  // Actualizarea cronometrului
  unsigned long displayChronometerTime = elapsedChronometerTime;
  if (runningChronometer) {
    displayChronometerTime += (currentTime - startChronometerTime) / 1000;
  }

  // Actualizarea timerului
  unsigned long displayTimerTime = countdownTime;
  if (runningTimer) {
    displayTimerTime = countdownTime - (currentTime - startTimerTime) / 1000;
    if (displayTimerTime <= 0) {
      displayTimerTime = 0;
      runningTimer = false;  // Timer oprit la 0
    }
  }

  // Actualizarea lap counter
  unsigned long displayLapTime = 0;
  if (runningLapCounter) {
    displayLapTime = (currentTime - lapStartTime) / 1000;
    if (lapInterval > 0 && displayLapTime >= lapInterval) {
      lapCount++;
      lapStartTime = currentTime;  // Resetează timpul pentru următorul lap
    }
  }

  // Construirea răspunsului JSON
  String json = "{";
  json += "\"chronometer\":\"" + formatTime(displayChronometerTime) + "\",";
  json += "\"timer\":\"" + formatTime(displayTimerTime) + "\",";
  json += "\"laps\":\"" + String(lapCount) + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

// Handlere pentru Lap Counter
void handleStartLapCounter() {
  if (server.hasArg("interval")) {
    lapInterval = server.arg("interval").toInt();
    if (lapInterval > 0) {
      lapStartTime = millis();
      runningLapCounter = true;
      lapCount = 0;  // Resetează numărul de ture
      server.send(200, "text/plain", "Lap counter started");
    } else {
      server.send(400, "text/plain", "Invalid interval");
    }
  } else {
    server.send(400, "text/plain", "Interval not provided");
  }
}

void handleResetLapCounter() {
  runningLapCounter = false;
  lapCount = 0;
  lapInterval = 0;
  server.send(200, "text/plain", "Lap counter reset");
}

// Funcții pentru Cronometru
void handleStartChronometer() {
  if (!runningChronometer) {
    runningChronometer = true;
    startChronometerTime = millis();
  }
  server.send(200, "text/plain", "Chronometer started");
}

void handleStopChronometer() {
  if (runningChronometer) {
    elapsedChronometerTime += (millis() - startChronometerTime) / 1000;
    runningChronometer = false;
  }
  server.send(200, "text/plain", "Chronometer stopped");
}

void handleResetChronometer() {
  runningChronometer = false;
  elapsedChronometerTime = 0;
  server.send(200, "text/plain", "Chronometer reset");
}

// Funcții pentru Timer
void handleStartTimer() {
  if (server.hasArg("time")) {
    countdownTime = server.arg("time").toInt();
    startTimerTime = millis();
    runningTimer = true;
    server.send(200, "text/plain", "Timer started");
  } else {
    server.send(400, "text/plain", "Time not provided");
  }
}

void handleStopTimer() {
  if (runningTimer) {
    countdownTime -= (millis() - startTimerTime) / 1000;
    runningTimer = false;
  }
  server.send(200, "text/plain", "Timer stopped");
}

void handleResetTimer() {
  runningTimer = false;
  countdownTime = 0;
  server.send(200, "text/plain", "Timer reset");
}

// Configurare inițială
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

  // Definire rute
  server.on("/", []() { server.send(200, "text/html", generateHTML()); });
  server.on("/time", handleTime);

  server.on("/startChronometer", handleStartChronometer);
  server.on("/stopChronometer", handleStopChronometer);
  server.on("/resetChronometer", handleResetChronometer);

  server.on("/startTimer", handleStartTimer);
  server.on("/stopTimer", handleStopTimer);
  server.on("/resetTimer", handleResetTimer);

  server.on("/startLapCounter", handleStartLapCounter);
  server.on("/resetLapCounter", handleResetLapCounter);

  server.begin();
  Serial.println("HTTP server started.");
}

// Buclă principală
void loop() {
  server.handleClient();
}
