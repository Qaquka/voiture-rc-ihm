#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ctype.h>

#if __has_include("secrets.h")
#include "secrets.h"
#endif

#ifndef WIFI_SSID
#define WIFI_SSID "WIFI-0C06-GEII"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "iutgeiiiutgeii"
#endif

#define LED_PIN 2   // LED intégrée (ESP32 DevKit)

// ========= PARAMÈTRES Wi-Fi =========
// ========= SERVEUR HTTP =========
WebServer server(80);

bool isValidCommand(char cmd) {
  return cmd=='F' || cmd=='B' || cmd=='L' || cmd=='R' || cmd=='S';
}

// Traite la commande reçue (F/B/L/R/S)
void handleCommand(char cmd) {
  switch (cmd) {
    case 'F': Serial.println("CMD: FORWARD (F)"); break;
    case 'B': Serial.println("CMD: BACKWARD (B)"); break;
    case 'L': Serial.println("CMD: LEFT (L)"); break;
    case 'R': Serial.println("CMD: RIGHT (R)"); break;
    case 'S': Serial.println("CMD: STOP (S)"); break;
    default:  Serial.printf("CMD: inconnu (%c)\n", cmd); break;
  }

  // Exemple : LED ON avancer, OFF au STOP
  if (cmd == 'F') {
    digitalWrite(LED_PIN, HIGH);
  } else if (cmd == 'S') {
    digitalWrite(LED_PIN, LOW);
  }
}

// Handler /cmd?c=F|B|L|R|S
void handleCmd() {
  String c = server.hasArg("c") ? server.arg("c") : "";
  Serial.print("Commande recue: ");
  Serial.println(c);

  // --- CORS pour l’IHM ---
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");

  if (c.length() != 1) {
    server.send(400, "text/plain", "ERR: missing/invalid cmd");
    return;
  }

  char cmd = toupper(c[0]);
  if (!isValidCommand(cmd)) {
    server.send(400, "text/plain", "ERR: unknown cmd");
    return;
  }

  handleCommand(cmd);
  server.send(200, "text/plain", "OK");
}

// OPTIONS /cmd (pré-flight éventuel)
void handleCmdOptions() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.send(204);  // No Content
}

// Page d’info basique
void handleRoot() {
  String msg;
  msg  = "ESP32-RC connecte au Wi-Fi.\n";
  msg += "IP : " + WiFi.localIP().toString() + "\n";
  msg += "Utiliser /cmd?c=F|B|L|R|S depuis l'IHM.\n";

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", msg);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.println("\n[ESP32] Demarrage…");
  Serial.printf("Connexion au Wi-Fi \"%s\"…\n", WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // Attente de la connexion
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (++tries > 60) {   // ~30 s
      Serial.println("\nEchec connexion Wi-Fi !");
      break;
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WiFi] Connecte !");
    Serial.print("Adresse IP : ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("[WiFi] NON connecte, mais on lance quand meme le serveur.");
  }

  // Routes HTTP
  server.on("/", HTTP_GET, handleRoot);
  server.on("/cmd", HTTP_GET, handleCmd);
  server.on("/cmd", HTTP_OPTIONS, handleCmdOptions);

  server.begin();
  Serial.println("[HTTP] Serveur demarre sur le port 80");
}

void loop() {
  server.handleClient();
  delay(1);
}
