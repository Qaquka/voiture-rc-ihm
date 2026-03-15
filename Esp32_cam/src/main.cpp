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

WiFiClient streamClient;
bool streamActive = false;

// =====================
//  SERVEUR HTTP
// =====================
WebServer server(80);
bool flashState = false;

// =====================
//  PAGE HTML
// =====================
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title>ESP32-CAM</title>
  <style>
    body { background:#111; color:#fff; text-align:center; font-family:Arial; }
    img { width:90vw; max-width:640px; margin-top:20px; }
    button { padding:10px 20px; font-size:16px; margin-top:15px; }
  </style>
</head>
<body>
  <h2>ESP32-CAM</h2>
  <img src="/stream">
  <br>
</body>
</html>
)rawliteral";

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

  config.xclk_freq_hz = 16000000;         
  config.pixel_format = PIXFORMAT_JPEG;

  config.frame_size   = FRAMESIZE_CIF;     
  config.jpeg_quality = 30;                 // compression
  config.fb_count     = 2;                  // mémoire tampon

  if (esp_camera_init(&config) != ESP_OK) {
    return false;
  }
  return true;
}

// =====================
//  HANDLERS
// =====================
void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleStream() {

  if (streamActive) {
    server.send(503, "text/plain", "Stream already in use");
    return;
  }

  streamClient = server.client();
  streamActive = true;

  Serial.println("[STREAM] Client connecté");

  streamClient.print(
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n"
    "Cache-Control: no-cache\r\n"
    "Pragma: no-cache\r\n"
    "\r\n"
  );

  while (streamClient.connected()) {

    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("[STREAM] Capture échouée");
      break;
    }

    streamClient.print("--frame\r\n");
    streamClient.print("Content-Type: image/jpeg\r\n");
    streamClient.printf("Content-Length: %u\r\n\r\n", fb->len);
    streamClient.write(fb->buf, fb->len);
    streamClient.print("\r\n");

    esp_camera_fb_return(fb);

    yield();
    delay(1);
  }

  streamClient.stop();
  streamActive = false;

  Serial.println("[STREAM] Client déconnecté");
}


// =====================
//  SETUP / LOOP
// =====================
void setup() {
  Serial.begin(115200);

  if (!initCamera()) {
    ESP.restart();
  }

  sensor_t *s = esp_camera_sensor_get();
  if (s) {
    s->set_framesize(s, FRAMESIZE_CIF);
    s->set_gain_ctrl(s, 1);
    s->set_exposure_ctrl(s, 1);
    s->set_awb_gain(s, 1);
    s->set_denoise(s, 1);
    s->set_contrast(s, -1);
    s->set_brightness(s, 0);
    s->set_aec2(s, 1);
    s->set_ae_level(s, 0);
    s->set_saturation(s, 2);
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  server.on("/", handleRoot);
  server.on("/stream", handleStream);
  server.begin();
}

void loop() {
  server.handleClient();
}
