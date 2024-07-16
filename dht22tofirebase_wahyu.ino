#include <WiFi.h>
#include <FirebaseESP32.h>
#include "DHT.h"

// Gantilah dengan kredensial Wi-Fi Anda
const char* ssid = "a";
const char* password = "Aksel*#5319";

// Firebase database URL
#define FIREBASE_DATABASE_URL "https://dameyu-iot-default-rtdb.firebaseio.com"

// Firebase Realtime Database secret (opsional, bisa menggunakan metode autentikasi JWT)
#define FIREBASE_SECRET "hrfOorsyBxnzoXrMj6ET7RdLTQnefalojdh430ri"

#define DHTPIN 26
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

FirebaseData firebaseData;      
FirebaseAuth auth;
FirebaseConfig config;

void setup() {
  Serial.begin(9600);

  // Menghubungkan ke Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("Connected to Wi-Fi");

  // Menampilkan alamat IP
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); // Menampilkan alamat IP ESP8266

  // Konfigurasi Firebase
  config.database_url = FIREBASE_DATABASE_URL;
  config.signer.tokens.legacy_token = FIREBASE_SECRET;

  // Memulai Firebase
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  dht.begin();
}
  
void loop() {
  // Membaca data suhu dan kelembaban dari sensor DHT22
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true); // Membaca suhu dalam Fahrenheit

  // Memeriksa apakah pembacaan dari sensor gagal
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Menghitung indeks panas dalam Fahrenheit
  float hif = dht.computeHeatIndex(f, h);
  // Menghitung indeks panas dalam Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  // Membulatkan nilai kelembaban dan suhu
  int h_rounded = round(h);
  int t_rounded = round(t);

  // Menampilkan nilai sensor pada serial monitor
  Serial.print(F("  Humidity: "));
  Serial.print(h_rounded);
  Serial.print(F("%  Temperature: "));
  Serial.print(t_rounded);
  Serial.println(F("°C "));

  // Mengirim data ke Firebase
  if (Firebase.ready()) {
    if (Firebase.setInt(firebaseData, "/sensor/temperature", t_rounded)) {
      Serial.println("Temperature data sent to Firebase successfully");
    } else {
      Serial.println("Failed to send Temperature data to Firebase");
      Serial.println(firebaseData.errorReason());
    }

    if (Firebase.setInt(firebaseData, "/sensor/humidity", h_rounded)) {
      Serial.println("Humidity data sent to Firebase successfully");
    } else {
      Serial.println("Failed to send Humidity data to Firebase");
      Serial.println(firebaseData.errorReason());
    }
  }
  
  // Delay 5 detik
  delay(10000);
}
