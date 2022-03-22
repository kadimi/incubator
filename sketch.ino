#include "DHT.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Micro-climate settings.
#include  "lib/chicken.h"
// #include  "./quail.h"
#define   ANIMAL "Chicken"

#define AUTHOR           "kadimi.com"
#define ADDITIONAL_TIME  0
#define BOOT_DELAY       2
#define SPEED            1

// Display.
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define DHTPIN 2
#define DHTTYPE DHT22

// Elements and their pins.
#define MOTOR_A 3
#define MOTOR_B 4
#define HEATER 5
#define ATOMIZER 6
#define FAN 7

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
DHT dht(DHTPIN, DHTTYPE);

void textAt(char text[], int x, int y, int size = 1) {
  oled.setTextSize(size);
  oled.setCursor(x, y);
  oled.print(text);
  return;
}

void showStatus(char label4[], int element, boolean status, boolean highlight_if) {
  textAt(label4, 2 + ( 4 + (element-4) * 30) , 53);
  if (highlight_if && status) {
    oled.drawRect(4 + (element-4)*30, 51, 27, 11, WHITE);
  }
}

void on(int pin) {
  digitalWrite(pin, HIGH);
}

void off(int pin) {
  digitalWrite(pin, LOW);
}

void on_off(int pin, boolean on_when, boolean off_when) {
  if(on_when) on(pin);
  if(off_when) off(pin);
}

void setup() {
  pinMode(MOTOR_A, OUTPUT);
  pinMode(MOTOR_B, OUTPUT);
  pinMode(HEATER, OUTPUT);
  pinMode(FAN, OUTPUT);
  pinMode(ATOMIZER, OUTPUT);

  dht.begin();
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  oled.setTextColor(WHITE);
  oled.invertDisplay(true);
}

void loop() {

  // Addd boot delay.
  if (millis() < (BOOT_DELAY) * 1000) {
    return;
  }

  unsigned long int elapsed_m = millis() * (SPEED) + (ADDITIONAL_TIME) * 1000 - (BOOT_DELAY) * 1000;
  unsigned long int elapsed_s = elapsed_m / 1000;
  unsigned short int seconds  = elapsed_s % 60;
  unsigned short int minutes  = elapsed_s % 3600 / 60;
  unsigned short int hours    = elapsed_s % 86400 / 3600;
  unsigned short int days     = elapsed_s / 86400;
  boolean sec_half_1          = elapsed_m % 1000 < 500;
  unsigned short int cycle;
  char helper[25];
  char time_format[15];
  sprintf(time_format, "%s", sec_half_1 ? "%02d:%02d:%02d" : "%02d %02d %02d");
  if (elapsed_s > (CYCLE_1_DURATION + CYCLE_2_DURATION)) {
    cycle = 0;
  } else if (elapsed_s > (CYCLE_1_DURATION)) {
    cycle = 2;
  } else {
    cycle = 1;
  }

  float humidity    = dht.readHumidity();
  float temperature = dht.readTemperature();
  float temperature_min;
  float temperature_optimal;
  float temperature_max;
  float humidity_min;
  float humidity_optimal;
  float humidity_max;
  unsigned short int turn_every;
  unsigned short int turn_direction_steps;
  unsigned short int turn_for;
  if (cycle == 1) {
    temperature_min      = CYCLE_1_TEMPERATURE_MIN;
    temperature_optimal  = CYCLE_1_TEMPERATURE_OPTIMAL;
    temperature_max      = CYCLE_1_TEMPERATURE_MAX;
    humidity_min         = CYCLE_1_HUMIDITY_MIN;
    humidity_optimal     = CYCLE_1_HUMIDITY_OPTIMAL;
    humidity_max         = CYCLE_1_HUMIDITY_MAX;
    turn_every           = CYCLE_1_TURN_EVERY;
    turn_direction_steps = CYCLE_1_TURN_DIRECTION_STEPS;
    turn_for             = CYCLE_1_TURN_FOR;
  } else if (cycle == 2) {
    temperature_min      = CYCLE_2_TEMPERATURE_MIN;
    temperature_optimal  = CYCLE_2_TEMPERATURE_OPTIMAL;
    temperature_max      = CYCLE_2_TEMPERATURE_MAX;
    humidity_min         = CYCLE_2_HUMIDITY_MIN;
    humidity_optimal     = CYCLE_2_HUMIDITY_OPTIMAL;
    humidity_max         = CYCLE_2_HUMIDITY_MAX;
    turn_every           = CYCLE_2_TURN_EVERY;
    turn_direction_steps = CYCLE_2_TURN_DIRECTION_STEPS;
    turn_for             = CYCLE_2_TURN_FOR;
  } else {
    temperature_optimal = 0;
    humidity_optimal    = 0;
  }

  oled.clearDisplay();

  // Header
  if (millis() % 5000 < 4000) {
    sprintf(helper, " %s %d", ANIMAL, cycle);
    textAt(helper, 0, 1);
    dtostrf(temperature_optimal, 4 ,1, helper);
    textAt(helper, 70, 1);
    dtostrf(humidity_optimal, 4 ,1, helper);
    textAt(helper, 100, 1);
  } else {
    sprintf(helper, " %s", AUTHOR);
    textAt(helper, 0, 1);
  }
  // textAt("---------------------", 0, 13);

  // Control
  boolean must_reverse;
  boolean must_turn;
  boolean must_turn_a;
  boolean must_turn_b;
  boolean too_cold;
  boolean too_hot;
  boolean too_dry;
  boolean too_wet;
  if(cycle) {
    must_reverse = elapsed_s % ( 2 * turn_every * turn_direction_steps) >= turn_every * turn_direction_steps;
    must_turn = elapsed_s % turn_every < (turn_for * SPEED);
    must_turn_a = must_turn && !must_reverse;
    must_turn_b = must_turn && must_reverse;
    too_cold = temperature <= (temperature_min/2 + temperature_optimal/2);
    too_hot = temperature >= (temperature_max/2 + temperature_optimal/2);
    too_dry = humidity <= (humidity_min/2 + humidity_optimal/2);
    too_wet = humidity >= (humidity_max/2 + humidity_optimal/2);
    on_off(MOTOR_A, must_turn_a, !must_turn || must_turn_b);
    on_off(MOTOR_B, must_turn_b, !must_turn || must_turn_a);
    on_off(HEATER, too_cold, too_hot);
    on_off(ATOMIZER, too_dry, too_wet);
    on_off(FAN, too_wet || too_hot, !too_wet && !too_hot);
  } else {
    on_off(MOTOR_A,  false, true);
    on_off(MOTOR_B,  false, true);
    on_off(HEATER,   false, true);
    on_off(ATOMIZER, false, true);
    on_off(FAN,      false, true);
  }

  // Display.
  sprintf(helper, "%02d", days);
  textAt(helper, 10, 18, 3);
  sprintf(helper, time_format, hours, minutes, seconds);
  textAt(helper, 6, 41);
  dtostrf(temperature, 4 ,1, helper);
  textAt(helper, 65, 18, 2);
  textAt("C", 116, 25);
  textAt(".", 111, 20);
  dtostrf(humidity, 4 ,1, helper);
  textAt(helper, 65, 36, 2);
  textAt("%", 113, 43);
  if(cycle) {
    showStatus(must_reverse ? "Tur>" : "Tur<", MOTOR_B,  must_turn,          sec_half_1);
    showStatus("Heat",                         HEATER,   too_cold,           sec_half_1);
    showStatus("Humi",                         ATOMIZER, too_dry,            sec_half_1);
    showStatus("Fan",                          FAN,      too_wet || too_hot, sec_half_1);
  } else {
    sprintf(helper, sec_half_1 ? "(  ) " : "(//) ");
    showStatus(helper, MOTOR_B,  false, sec_half_1);
    showStatus(helper, HEATER,   false, sec_half_1);
    showStatus(helper, ATOMIZER, false, sec_half_1);
    showStatus(helper, FAN,      false, sec_half_1);
  }
  oled.display();
}
