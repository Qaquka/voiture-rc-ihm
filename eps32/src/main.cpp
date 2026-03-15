#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#include <ctype.h>

#define SERVER_PORT 4080

// ===============================
// PORTS MOTEUR
// ===============================
const int IN1 = 18;
const int IN2 = 23;
const int PWM = 19;

// ===============================
// SERVO
// ===============================
Servo servoDirection;
const int PIN_SERVO = 5;

const int ANGLE_GAUCHE = 60;
const int ANGLE_CENTRE = 94;
const int ANGLE_DROITE = 120;

// ===============================
// VARIABLES
// ===============================
int P = 25;
char currentSpeedLevel = '0';

char motorState = 'S';      // A / R / S
char directionState = 'C';  // G / D / C

// ===============================
// WIFI
// ===============================
const char* ssid = "WIFI-0C06-GEII";
const char* password = "iutgeiiiutgeii";

WiFiServer TCPserver(SERVER_PORT);
WiFiClient client;
WebServer HTTPserver(80);

// ===============================
// VITESSE
// ===============================
void set_speed(char level) {
  currentSpeedLevel = level;

  switch (level) {
    case '0': P = 25; break;
    case '1': P = 40; break;
    case '2': P = 55; break;
    case '3': P = 75; break;
    default:  P = 25; break;
  }

  Serial.print("Vitesse niveau ");
  Serial.print(level);
  Serial.print(" -> PWM = ");
  Serial.println(P);
}

// ===============================
// APPLICATION MOTEUR
// ===============================
void applyMotor() {
  if (motorState == 'A') {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(PWM, P);
    Serial.println("Moteur -> AVANT");
  }
  else if (motorState == 'R') {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    analogWrite(PWM, P);
    Serial.println("Moteur -> ARRIERE");
  }
  else {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    analogWrite(PWM, 0);
    Serial.println("Moteur -> STOP");
  }
}

// ===============================
// APPLICATION DIRECTION
// ===============================
void applyDirection() {
  if (directionState == 'G') {
    servoDirection.write(ANGLE_GAUCHE);
    Serial.println("Direction -> GAUCHE");
  }
  else if (directionState == 'D') {
    servoDirection.write(ANGLE_DROITE);
    Serial.println("Direction -> DROITE");
  }
  else {
    servoDirection.write(ANGLE_CENTRE);
    Serial.println("Direction -> CENTRE");
  }
}

// ===============================
// COMMANDE
// ===============================
void command_motor(char c) {
  c = toupper((unsigned char)c);

  Serial.print("Commande recue : ");
  Serial.println(c);

  // vitesses
  if (c >= '0' && c <= '3') {
    set_speed(c);
    applyMotor();
    return;
  }

  // moteur
  if (c == 'A') {
    motorState = 'A';
    applyMotor();
    return;
  }

  if (c == 'R') {
    motorState = 'R';
    applyMotor();
    return;
  }

  if (c == 'S') {
    motorState = 'S';
    applyMotor();
    return;
  }

  // direction
  if (c == 'G') {
    directionState = 'G';
    applyDirection();
    return;
  }

  if (c == 'D') {
    directionState = 'D';
    applyDirection();
    return;
  }

  if (c == 'C') {
    directionState = 'C';
    applyDirection();
    return;
  }

  Serial.println("Commande inconnue");
}

// ===============================
// TCP MANETTE
// ===============================
void Clientconnecte(WiFiServer &server, WiFiClient &client) {
  static unsigned long lastPrint = 0;

  if (!client || !client.connected()) {
    WiFiClient newClient = server.available();

    if (newClient) {
      client = newClient;
      Serial.print("Client connecté: ");
      Serial.println(client.remoteIP());
    } else {
      if (millis() - lastPrint >= 3000) {
        Serial.println("En attente d'un client TCP...");
        lastPrint = millis();
      }
    }
  }
}

// ===============================
// HTTP
// ===============================
void addCORS() {
  HTTPserver.sendHeader("Access-Control-Allow-Origin", "*");
  HTTPserver.sendHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
  HTTPserver.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

void handleOptions() {
  addCORS();
  HTTPserver.send(204, "text/plain", "");
}

void handleRoot() {
  addCORS();

  String msg = "";
  msg += "ESP32 voiture RC OK\n";
  msg += "IP : " + WiFi.localIP().toString() + "\n";
  msg += "Commandes : /cmd?c=A|R|G|D|S|C|0|1|2|3\n";
  msg += "A = avancer\n";
  msg += "R = reculer\n";
  msg += "G = gauche\n";
  msg += "D = droite\n";
  msg += "S = stop moteur\n";
  msg += "C = centre direction\n";

  HTTPserver.send(200, "text/plain", msg);
}

void handleCmd() {
  addCORS();

  if (!HTTPserver.hasArg("c")) {
    HTTPserver.send(400, "text/plain", "ERR");
    return;
  }

  String cmdStr = HTTPserver.arg("c");
  if (cmdStr.length() != 1) {
    HTTPserver.send(400, "text/plain", "ERR");
    return;
  }

  char c = cmdStr.charAt(0);
  command_motor(c);

  HTTPserver.send(200, "text/plain", "OK");
}

// ===============================
// SETUP HTTP
// ===============================
void setupHTTPServer() {
  HTTPserver.on("/", HTTP_GET, handleRoot);
  HTTPserver.on("/cmd", HTTP_GET, handleCmd);
  HTTPserver.on("/cmd", HTTP_OPTIONS, handleOptions);

  HTTPserver.begin();
  Serial.println("HTTP OK");
}

// ===============================
// SETUP
// ===============================
void setup() {
  Serial.begin(9600);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(PWM, OUTPUT);

  servoDirection.setPeriodHertz(50);
  servoDirection.attach(PIN_SERVO, 750, 2250);
  servoDirection.write(ANGLE_CENTRE);

  WiFi.begin(ssid, password);

  Serial.println("Connexion WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi OK");
  Serial.print("IP : ");
  Serial.println(WiFi.localIP());

  TCPserver.begin();
  setupHTTPServer();

  set_speed('1');
  motorState = 'S';
  directionState = 'C';

  applyMotor();
  applyDirection();
}

// ===============================
// LOOP
// ===============================
void loop() {
  HTTPserver.handleClient();

  Clientconnecte(TCPserver, client);

  if (client && client.available()) {
    char c = client.read();
    command_motor(c);
  }
}
