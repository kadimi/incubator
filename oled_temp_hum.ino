#include "DHT.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width,  in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define DHTPIN 2
#define DHTTYPE DHT11

#define PIN_HEATER 4
#define PIN_MOTOR 5
#define PIN_FAN 6
#define PIN_ATOMIZER 7
#define PERFECT_TEMPERATURE 37.5
#define PERFECT_HUMIDITY 70

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  pinMode(PIN_HEATER, OUTPUT);
  pinMode(PIN_MOTOR, OUTPUT);
  pinMode(PIN_FAN, OUTPUT);
  pinMode(PIN_ATOMIZER, OUTPUT);
  digitalWrite(PIN_HEATER, HIGH);
  digitalWrite(PIN_MOTOR, HIGH);
  digitalWrite(PIN_FAN, HIGH);
  digitalWrite(PIN_ATOMIZER, HIGH);

  Serial.begin(9600);
  dht.begin();
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  delay(2000);
  oled.clearDisplay();
  oled.setTextSize(2);
  oled.setTextColor(WHITE);
}

void loop() {

  delay(2000);

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  oled.clearDisplay();
  oled.display();    
  delay(100);

  oled.clearDisplay();
  oled.setCursor(0, 10);
  oled.println(h);
  oled.setCursor(0, 32);
  oled.println(t);
  oled.display();    

  boolean too_cold = t < PERFECT_TEMPERATURE;
  digitalWrite(PIN_HEATER, too_cold ? LOW : HIGH);

  boolean too_dry = h < PERFECT_HUMIDITY;
  boolean too_wet = h > PERFECT_HUMIDITY;
  digitalWrite(PIN_ATOMIZER, too_dry ? LOW : HIGH);
  digitalWrite(PIN_FAN, too_wet ? LOW : HIGH);
}
