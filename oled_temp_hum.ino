#include "DHT.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define ADDITIONAL_TIME 0
#define BOOT_DELAY 2
#define SPEED 1

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define DHTPIN 2
#define DHTTYPE DHT22

#define HEATER 4
#define MOTOR 5
#define FAN 6
#define ATOMIZER 7

#define TEMPERATURE_OPTIMAL 37.5
#define TEMPERATURE_MIN 37.2
#define TEMPERATURE_MAX 38.9
#define HUMIDITY_OPTIMAL 60.0
#define HUMIDITY_MIN 50.0
#define HUMIDITY_MAX 65.0
#define TURN_EVERY 10
#define TURN_FOR 1

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
DHT dht(DHTPIN, DHTTYPE);

void textAt(char text[], int x, int y, int size = 1) {
  oled.setTextSize(size);
  oled.setCursor(x, y);
  oled.print(text);
  return;
}

void turn_on(int pin) {
  digitalWrite(pin, HIGH);
}

void turn_off(int pin) {
  digitalWrite(pin, LOW);
}

void setup() {
  pinMode(HEATER, OUTPUT);
  pinMode(MOTOR, OUTPUT);
  pinMode(FAN, OUTPUT);
  pinMode(ATOMIZER, OUTPUT);

  dht.begin();
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  oled.setTextColor(WHITE);
  oled.invertDisplay(true);
}

void loop() {

  // Addd boot delay.
  if (millis() < BOOT_DELAY) {
    return;
  }

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  unsigned int elapsed_m = millis() * SPEED + ADDITIONAL_TIME * 1000;
  unsigned int elapsed_s = elapsed_m / 1000;
  unsigned short int seconds = elapsed_s % 60;
  unsigned short int minutes = elapsed_s % 3600 / 60;
  unsigned short int hours = elapsed_s % 86400 / 3600;
  unsigned short int days = elapsed_s / 86400;
  char helper[20];
  boolean sec_half_1 = elapsed_m % 1000 < 500;
  char time_format[15];
  sprintf(time_format, "%s", sec_half_1 ? "%02d:%02d:%02d" : "%02d %02d %02d");

  // Control.
  boolean too_cold = temperature <= (TEMPERATURE_MIN/2 + TEMPERATURE_OPTIMAL/2);
  boolean too_hot = temperature >= (TEMPERATURE_MAX/2 + TEMPERATURE_OPTIMAL/2);
  boolean too_dry = humidity <= (HUMIDITY_MIN/2 + HUMIDITY_OPTIMAL/2);
  boolean too_wet = humidity >= (HUMIDITY_MAX/2 + HUMIDITY_OPTIMAL/2);
  boolean must_turn = elapsed_s % TURN_EVERY < TURN_FOR;
  if (too_cold) turn_on(HEATER);
  if (too_hot) turn_off(HEATER);
  if (too_dry) turn_on(ATOMIZER);
  if (too_wet) turn_off(ATOMIZER);
  if (too_wet || too_hot) turn_on(FAN); else turn_off(FAN);
  if (must_turn) turn_on(MOTOR); else turn_off(MOTOR);

  // Cleanup Display.
  oled.clearDisplay();

  // Add display.
  textAt("____ kadimi.com _____", 2, 1);
  sprintf(helper, "%02d", days);
  textAt(helper, 4, 12, 3);
  sprintf(helper, time_format, hours, minutes, seconds);
  textAt(helper, 4, 35);
  dtostrf(temperature, 4 ,1, helper);
  textAt(helper, 65, 12, 2);
  textAt("C", 116, 19);
  textAt(".", 111, 14);
  dtostrf(humidity, 4 ,1, helper);
  textAt(helper, 65, 30, 2);
  textAt("%", 113, 37);
  textAt("Heat Turn Hum. Fan", 6, 52);
  if (sec_half_1) {
    if (too_cold) oled.drawRect(4, 50, 27, 11, WHITE);
    if (too_dry) oled.drawRect(64, 50, 27, 11, WHITE);
    if (too_wet || too_hot) oled.drawRect(94, 50, 27, 11, WHITE);
    if (must_turn) oled.drawRect(34, 50, 27, 11, WHITE);
  }

  // Display.
  oled.display();
}
