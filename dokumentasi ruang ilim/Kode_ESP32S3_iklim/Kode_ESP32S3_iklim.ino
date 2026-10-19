#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

/* ====================== KONFIGURASI WIFI ====================== */

const char *WIFI_SSID     = "Hotspot@LabIPA-FP";
const char *WIFI_PASSWORD = "";

/* ====================== KONFIGURASI SUPABASE ====================== */

const char *SUPABASE_URL = "https://ohytpjqzezorcxypfuhq.supabase.co";
const char *SUPABASE_ANON_KEY =
"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6Im9oeXRwanF6ZXpvcmN4eXBmdWhxIiwicm9sZSI6ImFub24iLCJpYXQiOjE3NjM2Nzk1MjMsImV4cCI6MjA3OTI1NTUyM30.Wo2fliz8sSaX8Qi7uZR7Z5V-mKWOzr3jY0FX1do3oCY";

const char *REST_SENSOR_LOGS = "/rest/v1/sensor_logs";
const char *REST_SETPOINT    = "/rest/v1/setpoint";

/* ====================== UART STM32 <-> ESP32 ====================== */

const int UART_RX_PIN = 18;
const int UART_TX_PIN = 17;

HardwareSerial STM(2);

/* ====================== TIMER ====================== */

unsigned long lastSetpointFetch = 0;
const unsigned long SETPOINT_FETCH_INTERVAL_MS = 5000;

/* ====================== STATE SETPOINT ====================== */

struct Setpoint {
  float suhu;
  float kelembapan;
  float cahaya;
  float suara;
  bool  valid;
} currentSetpoint = {25, 60, 300, 50, false};

/* ====================== PROTOTYPE ====================== */

void connectWiFi();
void handleWiFi();
bool sendSensorToSupabase(const String &rawJson);
bool fetchSetpointFromSupabase();
void sendSetpointToSTM32();

/* ====================== SETUP ====================== */

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n[ESP32-S3] Ruang Iklim IoT start");

  pinMode(UART_RX_PIN, INPUT);
  pinMode(UART_TX_PIN, OUTPUT);
  STM.begin(115200, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
  Serial.println("[UART] STM32 ready");

  connectWiFi();

  // ===== TEST INTERNET =====
  Serial.print("[TEST] DNS google.com: ");
  IPAddress ip;
  Serial.println(WiFi.hostByName("google.com", ip) ? "OK" : "FAIL");

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient test;
  test.begin(client, "https://www.google.com");
  int code = test.GET();
  Serial.print("[TEST] HTTPS Google code: ");
  Serial.println(code);
  test.end();
}

/* ====================== LOOP ====================== */

void loop() {
  handleWiFi();

  // === UART dari STM32 ===
  if (STM.available()) {
    String line = STM.readStringUntil('\n');
    line.trim();

    if (line.length() > 0) {
      Serial.println("[UART] " + line);
      if (WiFi.isConnected()) sendSensorToSupabase(line);
    }
  }

  // === FETCH SETPOINT ===
  if (millis() - lastSetpointFetch > SETPOINT_FETCH_INTERVAL_MS) {
    lastSetpointFetch = millis();
    if (WiFi.isConnected() && fetchSetpointFromSupabase()) {
      sendSetpointToSTM32();
    }
  }
}

/* ====================== WIFI ====================== */

void connectWiFi() {
  Serial.print("[WiFi] Connecting...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int retry = 30;
  while (WiFi.status() != WL_CONNECTED && retry--) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.isConnected()) {
    Serial.print("[WiFi] IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("[WiFi] FAILED");
  }
}

void handleWiFi() {
  static unsigned long lastTry = 0;
  if (WiFi.status() != WL_CONNECTED && millis() - lastTry > 5000) {
    lastTry = millis();
    Serial.println("[WiFi] Reconnect...");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }
}

/* ====================== SUPABASE ====================== */

bool sendSensorToSupabase(const String &rawJson) {
  StaticJsonDocument<256> doc;
  if (deserializeJson(doc, rawJson)) return false;

  StaticJsonDocument<512> out;
  out["suhu"]       = doc["suhu"];
  out["kelembapan"] = doc["kelembapan"];
  out["cahaya"]     = doc["cahaya"];
  out["suara"]      = doc["suara"];
  out["raw_json"]   = rawJson;

  String body;
  serializeJson(out, body);

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.setTimeout(15000);
  http.begin(client, String(SUPABASE_URL) + REST_SENSOR_LOGS);
  http.addHeader("apikey", SUPABASE_ANON_KEY);
  http.addHeader("Authorization", "Bearer " + String(SUPABASE_ANON_KEY));
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Prefer", "return=minimal");

  int code = http.POST(body);
  Serial.print("[POST sensor] ");
  Serial.println(code);
  http.end();
  return code >= 200 && code < 300;
}

bool fetchSetpointFromSupabase() {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.setTimeout(15000);
  http.begin(client,
    String(SUPABASE_URL) + REST_SETPOINT +
    "?id=eq.1&select=suhu_target,kelembapan_target,cahaya_target,suara_target"
  );
  http.addHeader("apikey", SUPABASE_ANON_KEY);
  http.addHeader("Authorization", "Bearer " + String(SUPABASE_ANON_KEY));

  int code = http.GET();
  if (code != 200) {
    Serial.print("[GET setpoint] FAIL ");
    Serial.println(code);
    http.end();
    return false;
  }

  StaticJsonDocument<256> doc;
  deserializeJson(doc, http.getString());
  http.end();

  JsonObject r = doc[0];
  currentSetpoint.suhu       = r["suhu_target"];
  currentSetpoint.kelembapan = r["kelembapan_target"];
  currentSetpoint.cahaya     = r["cahaya_target"];
  currentSetpoint.suara      = r["suara_target"];
  currentSetpoint.valid      = true;

  Serial.println("[Setpoint OK]");
  return true;
}

void sendSetpointToSTM32() {
  if (!currentSetpoint.valid) return;

  StaticJsonDocument<128> doc;
  doc["suhu_target"]       = currentSetpoint.suhu;
  doc["kelembapan_target"] = currentSetpoint.kelembapan;
  doc["cahaya_target"]     = currentSetpoint.cahaya;
  doc["suara_target"]      = currentSetpoint.suara;

  String json;
  serializeJson(doc, json);
  STM.println(json);

  Serial.println("[UART -> STM32] " + json);
}
