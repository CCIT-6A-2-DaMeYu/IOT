#include <WiFi.h>
#include <FirebaseESP32.h>
#include "esp_camera.h"
#include "base64.h"
#include "DHT.h"

// Firebase objects
FirebaseData firebaseData;
FirebaseConfig config;
FirebaseAuth auth;

// Kamera pin configuration
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

// LED built-in pin
#define LED_BUILTIN 4

// Firebase host and authentication
#define FIREBASE_HOST "dameyu-iot-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "hrfOorsyBxnzoXrMj6ET7RdLTQnefalojdh430ri"

// WiFi credentials
const char* WIFI_SSID = "a";
const char* WIFI_PASSWORD = "why*#5319";

void setup() {
  Serial.begin(115200);
  delay(10);

  // Initialize LED built-in pin
//  pinMode(LED_BUILTIN, OUTPUT);
//  digitalWrite(LED_BUILTIN, LOW);

  // Connect to WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Initialize Firebase
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);

  // Camera configuration
  camera_config_t camera_config;
  camera_config.ledc_channel = LEDC_CHANNEL_0;
  camera_config.ledc_timer = LEDC_TIMER_0;
  camera_config.pin_d0 = Y2_GPIO_NUM;
  camera_config.pin_d1 = Y3_GPIO_NUM;
  camera_config.pin_d2 = Y4_GPIO_NUM;
  camera_config.pin_d3 = Y5_GPIO_NUM;
  camera_config.pin_d4 = Y6_GPIO_NUM;
  camera_config.pin_d5 = Y7_GPIO_NUM;
  camera_config.pin_d6 = Y8_GPIO_NUM;
  camera_config.pin_d7 = Y9_GPIO_NUM;
  camera_config.pin_xclk = XCLK_GPIO_NUM;
  camera_config.pin_pclk = PCLK_GPIO_NUM;
  camera_config.pin_vsync = VSYNC_GPIO_NUM;
  camera_config.pin_href = HREF_GPIO_NUM;
  camera_config.pin_sscb_sda = SIOD_GPIO_NUM;
  camera_config.pin_sscb_scl = SIOC_GPIO_NUM;
  camera_config.pin_pwdn = PWDN_GPIO_NUM;
  camera_config.pin_reset = RESET_GPIO_NUM;
  camera_config.xclk_freq_hz = 20000000; // Tetap menggunakan 40MHz
  camera_config.pixel_format = PIXFORMAT_JPEG;
  camera_config.frame_size = FRAMESIZE_VGA;
  camera_config.jpeg_quality = 10;
  camera_config.fb_count = 2;

  // Initialize camera
  esp_err_t err = esp_camera_init(&camera_config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    // Cobalah beberapa kali untuk menginisialisasi ulang kamera jika terjadi kesalahan
    for (int i = 0; i < 5; i++) {
      Serial.println("Re-initializing camera...");
      delay(1000); // Tambahkan penundaan sebelum mencoba lagi
      err = esp_camera_init(&camera_config);
      if (err == ESP_OK) {
        Serial.println("Camera re-initialized successfully");
        break;
      } else {
        Serial.printf("Camera re-init failed with error 0x%x\n", err);
      }
    }
    if (err != ESP_OK) {
      Serial.println("Camera initialization failed after multiple attempts. Restarting...");
      ESP.restart();
    }
    return;
  } else {
    Serial.println("Camera initialized successfully");
  }

  // Configure the camera sensor for 180 degree rotation
  sensor_t *s = esp_camera_sensor_get();
  s->set_vflip(s, 1); // Vertical flip
  s->set_hmirror(s, 1); // Horizontal mirror
}

void loop() {
  // Turn on LED built-in
  digitalWrite(LED_BUILTIN, HIGH);

  // Capture photo
  camera_fb_t *fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Failed to capture photo");
    // digitalWrite(LED_BUILTIN, LOW);
    delay(5000); // Tambahkan penundaan sebelum mencoba lagi
    return;
  }

  // Encode photo to Base64
  String base64Image = base64::encode(fb->buf, fb->len);

  // Upload Base64 encoded photo to Firebase
  if (Firebase.ready()) {
    if (Firebase.set(firebaseData, "/photos", base64Image)) {
      Serial.println("Photo uploaded to Firebase successfully!");
    } else {
      Serial.println("Failed to upload photo to Firebase.");
      Serial.println("REASON: " + firebaseData.errorReason());
    }
  }

  // Free the memory used by the frame buffer
  esp_camera_fb_return(fb);

  // Turn off LED built-in
//  digitalWrite(LED_BUILTIN, LOW);

  // Delay before capturing next photo
  delay(10000);
}
