#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// Firebase Addons
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// WiFi
#define WIFI_SSID "N/A"
#define WIFI_PASSWORD "11111111"

// Firebase
#define API_KEY "AIzaSyAmerwMllKGTowx3aMVfIUlr7aeYCiNBWs"
#define DATABASE_URL "https://final-project-esp32-e3bdf-default-rtdb.asia-southeast1.firebasedatabase.app/"

// Firebase instances
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Sensor config
#define DHTPIN 14
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
#define LDR_PIN    35
#define SOIL_PIN   32

// I/O pins
#define LED_PIN     19
#define PUMP_PIN    25
#define BUTTON_PIN  18

// States
bool ledState = false;
bool pumpState = false;
bool manualMode = false;

// Debounce
bool lastButton = HIGH;
unsigned long lastDebounce = 0;
const unsigned long debounceDelay = 50;

// Timing
unsigned long lastSensorMillis = 0;
unsigned long lastFirebaseMillis = 0;
const unsigned long sensorInterval = 1000;
const unsigned long firebaseInterval = 5000;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  dht.begin();
  Wire.begin(21, 22);
  lcd.init(); lcd.backlight();
  lcd.setCursor(0, 0); lcd.print("Connecting...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300); Serial.print(".");
  }
  lcd.clear(); lcd.print("WiFi Connected");

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  if (Firebase.signUp(&config, &auth, "", "")) {
    signupOK = true;
    lcd.setCursor(0, 1); lcd.print("Firebase OK");
  } else {
    lcd.setCursor(0, 1); lcd.print("FB Error");
    while (true); // Stop here
  }

  delay(1000); lcd.clear();
}

void loop() {
  unsigned long now = millis();

  // Always read button for LED manual control
  int lightVal = analogRead(LDR_PIN);
  handleButton(lightVal);

  if (now - lastSensorMillis >= sensorInterval) {
    lastSensorMillis = now;
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();
    int soil = analogRead(SOIL_PIN);

    controlLED(lightVal);
    controlPump(soil);

    updateLCD(temp, hum, soil, lightVal);
    debugSerial(temp, hum, soil, lightVal);
  }

  if (signupOK && Firebase.ready() && now - lastFirebaseMillis >= firebaseInterval) {
    lastFirebaseMillis = now;
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();
    int soil = analogRead(SOIL_PIN);
    int light = analogRead(LDR_PIN);

    Firebase.RTDB.setFloat(&fbdo, "/sensor/temperature", temp);
    Firebase.RTDB.setFloat(&fbdo, "/sensor/humidity", hum);
    Firebase.RTDB.setInt(&fbdo, "/sensor/light", light);
    Firebase.RTDB.setInt(&fbdo, "/sensor/soil", soil);
    Firebase.RTDB.setBool(&fbdo, "/device/led", ledState);
    Firebase.RTDB.setBool(&fbdo, "/device/pump", pumpState);
  }
}

// ----- Functions -----
void handleButton(int lightVal) {
  static bool buttonPressed = false;
  int reading = digitalRead(BUTTON_PIN);

  if (reading == LOW && !buttonPressed) {
    buttonPressed = true;  // đánh dấu đã nhấn

    if (lightVal >= 1000 && lightVal <= 2000) {
      ledState = !ledState;
      manualMode = true;
      Serial.println("Manual toggle LED");
    }
  }

  if (reading == HIGH) {
    buttonPressed = false;  // reset để nhấn lần sau
  }
}


void controlLED(int lightVal) {
  if (lightVal > 2000) {
    manualMode = false;
    ledState = true;
  } else if (lightVal < 1000) {
    manualMode = false;
    ledState = false;
  }
  digitalWrite(LED_PIN, ledState);
}

void controlPump(int soilVal) {
  pumpState = soilVal >3200;
  digitalWrite(PUMP_PIN, pumpState ? HIGH : LOW); // LOW = ON if relay is active-low
}

void updateLCD(float temp, float hum, int soil, int light) {
  lcd.setCursor(0, 0);
  lcd.print("T:"); lcd.print(temp, 1); lcd.print((char)223); lcd.print("C ");
  lcd.print("H:"); lcd.print((int)hum); lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.print("Soil:"); lcd.print(soil); lcd.print(" ");
  lcd.print(pumpState ? "P:ON " : "P:OFF");
}

void debugSerial(float t, float h, int s, int l) {
  Serial.printf("Temp: %.1f | Hum: %.1f | Soil: %d | Light: %d | LED: %s | Pump: %s\n",
                t, h, s, l, ledState ? "ON" : "OFF", pumpState ? "ON" : "OFF");
}
