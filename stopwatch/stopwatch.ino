#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>

// Șiruri de caractere cu mesaje predefinite
const String SETUP_INIT = "SETUP: Initializing ESP32 dev board";
const String SETUP_ERROR = "!!ERROR!! SETUP: Unable to start SoftAP mode";
const String SETUP_SERVER_START = "SETUP: HTTP server started --> IP addr: ";
const String SETUP_SERVER_PORT = " on port: ";
const String INFO_NEW_CLIENT = "New client connected";
const String INFO_DISCONNECT_CLIENT = "Client disconnected";

// Un header HTTP începe întotdeauna cu un cod de răspuns (e.g. HTTP/1.1 200 OK)
// și tipul conținutului, pentru a informa clientul, urmat de o linie goală:
const String HTTP_HEADER = "HTTP/1.1 200 OK\r\nContent-type:text/html\r\n\r\n";
const String HTML_WELCOME = "<h1>Welcome to your ESP32 Web Server!</h1>";

// Constante pentru configurarea de bază a WIFI
// SSID (Service Set IDentifier), numele rețelei
const char *SSID = "<your_unique_SSID>";

// Parola pentru rețea
// Implicit, ESP32 foloseste modul WPA/WPA2
// astfel că parola trebuie să aibă între 8 și 63 caractere ASCII
const char *PASS = "<your_password_here>";

// Portul implicit pentru un server HTTP este 80, conform RFC1340
const int HTTP_PORT_NO = 80;

// Initializare server HTTP pe portul 80
WiFiServer HttpServer(HTTP_PORT_NO);

void setup() {
  Serial.begin(9600);
  if (!WiFi.softAP(SSID)) {
    // înlocuiți cu if (!WiFi.softAP(SSID, PASS)) pentru a utiliza parola
    Serial.println(SETUP_ERROR);
    // Dacă nu se poate activa punctul de acces, blochează programul aici
    while (1)
      ;
  }
  // Citire adresă IP a AP-ului pentru mesaj de informare
  const IPAddress accessPointIP = WiFi.softAPIP();
  const String webServerInfoMessage = SETUP_SERVER_START + accessPointIP.toString()
                                      + SETUP_SERVER_PORT + HTTP_PORT_NO;
  // Pornire server HTTP
  HttpServer.begin();
  Serial.println(webServerInfoMessage);
}

void loop() {
  WiFiClient client = HttpServer.available();  // Ascultă pentru clienți noi
  if (client) {                                // dacă avem un client conectat,
    Serial.println(INFO_NEW_CLIENT);           // trimite un mesaj pe portul serial
    String currentLine = "";                   // Șir pentru a citi datele de la client
    while (client.connected()) {               // cât timp clientul este conectat
      if (client.available()) {                // dacă avem caractere de citit de la client,
        const char c = client.read();          // citește un caracter, apoi
        Serial.write(c);                       // tiparește la serial monitor
        if (c == '\n') {                       // dacă caracterul este new line
          // dacă linia este goală, avem două caractere newline consecutive
          // asta înseamnă finalul cererii HTTP de la client, deci trimitem răspuns:
          if (currentLine.length() == 0) {
            // Trimite mesaj de bun venit
            printWelcomePage(client);
            break;
          } else currentLine = "";
        } else if (c != '\r') {  // dacă există alte caractere în afară de carriage return
          currentLine += c;      // se adaugă la linia curentă
        }
      }
    }

    // se închide conexiunea:
    client.stop();
    Serial.println(INFO_DISCONNECT_CLIENT);
    Serial.println();
  }
}

void printWelcomePage(WiFiClient client) {
  // Răspunsul către client trebuie să conțină headerele corecte
  client.println(HTTP_HEADER);

  // Trimitem mesajul HTML
  client.print(HTML_WELCOME);

  // Răspunsul HTTP se termină cu o linie goală
  client.println();
}