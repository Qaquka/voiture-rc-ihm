#include <Arduino.h>
#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>

// =====================
//  PARAMÈTRES WIFI
// =====================
const char* ssid     = "WIFI-0C06-GEII";
const char* password = "iutgeiiiutgeii";

// =====================
//  PINS ESP32-CAM AI THINKER
// =====================
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// =====================
//  SERVEUR HTTP
// =====================
WebServer server(80);

// =====================
//  INIT CAMERA
// =====================
bool initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // -------- QVGA (320x240) QUALITÉ TRÈS BASSE POUR FLUIDITÉ --------
  config.frame_size   = FRAMESIZE_QVGA;   // 320x240
  config.jpeg_quality = 40;               // très compressé, plus fluide
  config.fb_count     = 1;                // un seul buffer
  // -----------------------------------------------------------------

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("[CAM] Erreur init camera 0x%x\n", err);
    return false;
  }

  Serial.println("[CAM] Init OK");
  return true;
}

// =====================
//  FLUX MJPEG
// =====================
void handleStream() {
  WiFiClient client = server.client();

  client.print(
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n"
    "Cache-Control: no-cache\r\n"
    "Pragma: no-cache\r\n\r\n"
  );

  Serial.println("[STREAM] Client connecte");

  while (client.connected()) {
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("[STREAM] Erreur capture");
      break;
    }

    client.printf("--frame\r\n");
    client.printf("Content-Type: image/jpeg\r\n");
    client.printf("Content-Length: %u\r\n\r\n", fb->len);
    client.write(fb->buf, fb->len);
    client.print("\r\n");

    esp_camera_fb_return(fb);

    // Ajuste si besoin : 20 = fluide, 40 = plus stable
    delay(40);
  }

  Serial.println("[STREAM] Client deconnecte");
}

// =====================
//  SETUP / LOOP
// =====================
void setup() {
  Serial.begin(115200);
  Serial.println("\nBoot ESP32-CAM...");

  if (!initCamera()) {
    Serial.println("Camera KO, reboot...");
    delay(3000);
    ESP.restart();
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connexion WiFi ");
  Serial.print(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connecte !");
  Serial.print("IP : ");
  Serial.println(WiFi.localIP());

  server.on("/stream", HTTP_GET, handleStream);
  server.begin();

  Serial.println("Serveur HTTP actif.");
  Serial.print("➡ Ouvre : http://");
  Serial.print(WiFi.localIP());
  Serial.println("/stream");
}

void loop() {
  server.handleClient();
}
