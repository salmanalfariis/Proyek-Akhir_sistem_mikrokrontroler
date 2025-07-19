#define BLYNK_TEMPLATE_ID "TMPL69iwLWtA4"
#define BLYNK_TEMPLATE_NAME "Papan Informasi Otomatis"
#define BLYNK_AUTH_TOKEN "-jJ_Sa3ZakhuUoL-R0U0zuo0zpgfHguh"

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "BlynkTimer.h"
#include <time.h>

#define DHTPIN 15
#define DHTTYPE DHT22
#define BUTTON_PIN 14

#define RED_PIN 19
#define GREEN_PIN 18
#define BLUE_PIN 17

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

bool tombolTekan = false;
unsigned long alarmStartTime = 0;
const unsigned long alarmDuration = 5000;
bool ledState = false;
unsigned long lastBlinkTime = 0;
const unsigned long blinkInterval = 300;

BLYNK_WRITE(V0) {
  String teksDariBlynk = param.asString();
  tampilkanAnimasi(teksDariBlynk);
  Blynk.virtualWrite(V2, "Pesan diterima: " + teksDariBlynk);
}

void tampilkanAnimasi(String teks) {
  lcd.clear();
  lcd.setCursor(0, 0);
  for (int i = 0; i < teks.length(); i++) {
    lcd.print(teks[i]);
    delay(100);
  }
}

void tampilkanSuhu() {
  float suhu = dht.readTemperature();
  if (!isnan(suhu)) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Suhu:");
    lcd.setCursor(0, 1);
    lcd.print(suhu);
    lcd.print(" C");
    Blynk.virtualWrite(V1, suhu);
  }
}

void periksaTombol() {
  if (digitalRead(BUTTON_PIN) == LOW && !tombolTekan) {
    tombolTekan = true;
    alarmStartTime = millis();

    tampilkanAnimasi("Alarm Bel Bunyi");
    Blynk.virtualWrite(V3, 255);
  }
}

void kendaliAlarm() {
  if (tombolTekan) {
    if (millis() - alarmStartTime <= alarmDuration) {
      if (millis() - lastBlinkTime >= blinkInterval) {
        lastBlinkTime = millis();
        ledState = !ledState;
        digitalWrite(RED_PIN, !ledState);
        digitalWrite(GREEN_PIN, LOW);
        digitalWrite(BLUE_PIN, LOW);
      }
    } else {
      tombolTekan = false;
      digitalWrite(RED_PIN, LOW);
      digitalWrite(GREEN_PIN, LOW);
      digitalWrite(BLUE_PIN, LOW);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Stanby");

      Blynk.virtualWrite(V3, 0);
    }
  }
}

void tampilkanWaktu() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Gagal mendapatkan waktu");
    return;
  }
  char buffer[64];
  strftime(buffer, sizeof(buffer), "📅 %A, %d %B %Y\n⏰ %H:%M:%S WIB", &timeinfo);

  Blynk.virtualWrite(V4, buffer);
}

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  dht.begin();
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(BLUE_PIN, LOW);

  lcd.setCursor(0, 0);
  lcd.print("Papan LCD IoT");
  lcd.setCursor(0, 1);
  lcd.print("Ready...");

  Blynk.begin(BLYNK_AUTH_TOKEN, "Wokwi-GUEST", "");
  configTime(7 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  timer.setInterval(5000L, tampilkanSuhu);
  timer.setInterval(200L, periksaTombol);
  timer.setInterval(100, kendaliAlarm);
  timer.setInterval(1000L, tampilkanWaktu);
}

void loop() {
  Blynk.run();
  timer.run();
}
