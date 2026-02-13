#include <Arduino.h>
#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>

#if __has_include("secrets.h")
#include "secrets.h"
#endif

#ifndef WIFI_SSID
#define WIFI_SSID "WIFI-0C06-GEII"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "iutgeiiiutgeii"
#endif

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
//  PROFIL STREAM
// =====================
enum StreamProfile {
  PROFILE_FLUIDE,
  PROFILE_QUALITE
};

static constexpr StreamProfile ACTIVE_PROFILE = PROFILE_FLUIDE;
static constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 30000;

WebServer server(80);
bool streamActive = false;

void applySensorTuning() {
  sensor_t* s = esp_camera_sensor_get();
  if (!s) return;

  // Auto controls
  s->set_gain_ctrl(s, 1);
  s->set_exposure_ctrl(s, 1);
  s->set_awb_gain(s, 1);

  // Image rendering compromise for RC view
  s->set_brightness(s, 0);
  s->set_contrast(s, 1);
  s->set_saturation(s, 1);

  // Reduce pumping / artifacts depending on scene
  s->set_aec2(s, 0);
  s->set_ae_level(s, 0);
  s->set_denoise(s, 1);
}

bool initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  const bool hasPsram = psramFound();

  if (ACTIVE_PROFILE == PROFILE_QUALITE) {
    config.frame_size = hasPsram ? FRAMESIZE_VGA : FRAMESIZE_CIF;
    config.jpeg_quality = 12;
  } else {
    config.frame_size = FRAMESIZE_CIF;
    config.jpeg_quality = 15;
  }

  config.fb_count = hasPsram ? 2 : 1;

#ifdef CAMERA_GRAB_LATEST
  config.grab_mode = CAMERA_GRAB_LATEST;
#endif
#ifdef CAMERA_FB_IN_PSRAM
  config.fb_location = hasPsram ? CAMERA_FB_IN_PSRAM : CAMERA_FB_IN_DRAM;
#endif

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("[CAM] Erreur init camera 0x%x\n", err);
    return false;
  }

  applySensorTuning();
  Serial.printf("[CAM] Init OK | psram=%s | frame=%d | quality=%d | fb_count=%d\n",
                hasPsram ? "yes" : "no",
                static_cast<int>(config.frame_size),
                config.jpeg_quality,
                config.fb_count);
  return true;
}

void handleStream() {
  if (streamActive) {
    server.send(503, "text/plain", "Stream already in use");
    return;
  }

  WiFiClient client = server.client();
  client.setNoDelay(true);
  streamActive = true;

  client.print(
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n"
    "Cache-Control: no-cache\r\n"
    "Pragma: no-cache\r\n\r\n"
  );

  Serial.println("[STREAM] Client connecte");

  while (client.connected()) {
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("[STREAM] Erreur capture");
      break;
    }

    client.printf("--frame\r\n");
    client.printf("Content-Type: image/jpeg\r\n");
    client.printf("Content-Length: %u\r\n\r\n", fb->len);
    size_t sent = client.write(fb->buf, fb->len);
    client.print("\r\n");

    esp_camera_fb_return(fb);

    if (sent != fb->len) {
      Serial.printf("[STREAM] Ecriture incomplete (%u/%u)\n", static_cast<unsigned>(sent), static_cast<unsigned>(fb->len));
      break;
    }

    yield();
    delay(1);
  }

  client.stop();
  streamActive = false;
  Serial.println("[STREAM] Client deconnecte");
}

void setup() {
  Serial.begin(115200);
  Serial.println("\nBoot ESP32-CAM...");

  if (!initCamera()) {
    Serial.println("Camera KO, reboot...");
    delay(3000);
    ESP.restart();
  }

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.printf("Connexion WiFi %s", WIFI_SSID);
  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_CONNECT_TIMEOUT_MS) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n[WiFi] Echec connexion, reboot...");
    delay(2000);
    ESP.restart();
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
