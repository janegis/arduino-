

// ANALOGINIAI PINAI
#define POT 0
#define THERM 1
#define PHOTO 2
#define JOYX 6
#define JOYY 7

// SKAITMENINIAI PINAI
#define SRV_PIN 2
#define BTN 3
#define MOS 4
#define LED_R 5
#define LED_G 6
#define RELAY 10
#define LED_B 11

#include <Servo.h>
Servo servo;

#include <LiquidCrystal_I2C.h>

// 0x27 ARBA 0x3f
LiquidCrystal_I2C lcd(0x27, 16, 2);


#include "thermistorMinim.h";
thermistor therm(THERM, 10000, 3950);

#include "EncButton.h"
#include "Parser.h"
#include "AsyncStream.h"  // SERIAL SINCHRONIZUOTAS SKAITYMAS
AsyncStream<50> serial(&Serial, ';');   // NURODOME NUSKAITYMA IR STOP SINBOLI

EncButton<EB_TICK, BTN> btn;
bool flag = 0;

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  servo.attach(SRV_PIN);
  pinMode(13, 1);
  pinMode(LED_R, 1);
  pinMode(LED_G, 1);
  pinMode(LED_B, 1);
  pinMode(MOS, 1);
  pinMode(RELAY, 1);
}

// IŠ ARDUINO  I KOMPIUTERI, TERMINATORIUS \n
// 0,POTENCIOMETRAS,FOTOREZISTORIUS,TERMISTORIUS
// 1,MYGTUKAS
// 2,DZIOISTIKAS

// IŠ KOMPIUTERIO I ARDUINO TERMINATORIUS ;
// 0  LEDAS 13 Standartinis arduino pinas imontuotas į moduli
// 1  R,G,B
// 2  SERVO
// 3  FENAS
// 4  RELE
// 5  TEKSTAS

void loop() {
  parsing();

  btn.tick();
  static uint32_t tmr = 0;
  if (millis() - tmr > 100) {
    tmr = millis();
    Serial.print(0);
    Serial.print(',');
    Serial.print(analogRead(POT));
    Serial.print(',');
    Serial.print(analogRead(PHOTO));
    Serial.print(',');
    Serial.println(therm.getTempAverage(), 2);
  }

  static uint32_t tmr2 = 0;
  if (millis() - tmr2 > 50) {
    tmr2 = millis();
    Serial.print(2);
    Serial.print(',');
    Serial.print(analogRead(JOYX));
    Serial.print(',');
    Serial.println(analogRead(JOYY));
  }

  if (btn.isClick()) {
    flag = !flag;
    Serial.print(1);
    Serial.print(',');
    Serial.println(flag);
  }
}

// PARSINGO FUNKCIJA APKLAUSEME LOOP KOMANDOJE
void parsing() {
  if (serial.available()) {
    char buf[50];
    Serial.readBytesUntil(';' ,buf, 50);
    Parser data(serial.buf, ',');  // PARSERIUI ATIDUODAME
    int ints[10];           // MASYVAS DUOMENU SKAITYMUI IKI 10 REIKŠMIŲ
    data.parseInts(ints);   // PARSINAME I VIDU

    switch (ints[0]) {
      case 0: digitalWrite(13, ints[1]);
        break;
      case 1:
        analogWrite(LED_R, ints[1]);
        analogWrite(LED_G, ints[2]);
        analogWrite(LED_B, ints[3]);
        break;
      case 2:
        servo.write(ints[1]);
        break;
      case 3:
        digitalWrite(MOS, ints[1]);
        break;
      case 4:
        digitalWrite(RELAY, ints[1]);
        break;
      case 5:
        data.split();
        lcd.clear();
        lcd.home();
        lcd.print(data[1]);
        break;
    }
  }
}
