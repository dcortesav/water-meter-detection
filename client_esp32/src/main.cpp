#include <Arduino.h>
#include <WiFi.h>
#include "esp_camera.h"
#include <HTTPClient.h>

const char* ssid = "XXXXXXX";
const char* password = "XXXXXX";
String serverName = "http://172.20.10.8:8000/upload"; 

// Pines AI-Thinker
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

void takeAndSendPhoto(); 

void setup() {
  Serial.begin(115200);
  Serial.println("\n--- INICIANDO SISTEMA OV3660 ---");

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
  
  // IMPORTANTE: Bajamos a 10MHz para máxima estabilidad con el OV3660
  config.xclk_freq_hz = 10000000; 
  config.pixel_format = PIXFORMAT_JPEG;

  if(psramFound()){
    // Usamos SXGA. El OV3660 soporta hasta QXGA (2048x1536) pero es muy pesado ahora.
    config.frame_size = FRAMESIZE_VGA; 
    config.jpeg_quality = 12; 
    config.fb_count = 1;
  } else {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Inicializar cámara
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Error FATAL de camara: 0x%x\n", err);
    // Si falla, intentamos reiniciar la placa en 3 segundos
    delay(3000);
    ESP.restart();
    return;
  }

  // AJUSTES ESPECÍFICOS PARA OV3660
  sensor_t * s = esp_camera_sensor_get();
  if (s->id.PID == OV3660_PID) {
    Serial.println("Sensor OV3660 Detectado Correctamente");
    s->set_brightness(s, 1);  // Un poco más de brillo
    s->set_saturation(s, 0); 
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);      // El OV3660 suele venir invertido verticalmente
  }

  // Conexión WiFi
  WiFi.begin(ssid, password);
  // Desactivamos el ahorro de energía para que no tire el voltaje
  WiFi.setSleep(false); 
  
  Serial.print("Conectando WiFi");
  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 20) {
    delay(500);
    Serial.print(".");
    intentos++;
  }

  if(WiFi.status() == WL_CONNECTED){
    Serial.println("\nWiFi Conectado!");
    // Flash rápido para indicar éxito visualmente
    pinMode(4, OUTPUT);
    digitalWrite(4, HIGH); delay(200); digitalWrite(4, LOW);
  } else {
    Serial.println("\nFallo WiFi. Revisa energía.");
  }
}

void loop() {
  // Modo Automático (Ya que vamos a quitar el USB)
  takeAndSendPhoto();
  
  // Esperamos 15 segundos para dejar enfriar el sensor
  Serial.println("Enfriando 15s...");
  delay(15000); 
}

void takeAndSendPhoto() {
  if(WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
    delay(1000); // Dar tiempo a reconexión
    return;
  }

  Serial.println("Tomando foto...");
  
  // Limpieza de buffer
  camera_fb_t * fb = esp_camera_fb_get();
  esp_camera_fb_return(fb); 
  delay(1000); 

  // Captura Real
  fb = esp_camera_fb_get();
  if(!fb) {
    Serial.println("Fallo captura");
    return;
  }

  Serial.printf("Enviando %u bytes...\n", fb->len);
  HTTPClient http;
  http.begin(serverName);
  http.addHeader("Content-Type", "image/jpeg");
  
  int httpResponseCode = http.POST(fb->buf, fb->len);
  
  if (httpResponseCode > 0) {
    Serial.printf("Enviado OK: %d\n", httpResponseCode);
  } else {
    Serial.printf("Error envio: %s\n", http.errorToString(httpResponseCode).c_str());
  }
  
  http.end();
  esp_camera_fb_return(fb); 
}