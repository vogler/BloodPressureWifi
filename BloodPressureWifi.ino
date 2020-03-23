#include <assert.h>
#include <MyConfig.h> // credentials, servers, ports
#define MQTT_TOPIC "sensors/blood-pressure"
#include "wifi_mqtt.h"

#define CK D5 // SCK_PIN, SK: serial clock input
#define DI D7 // MOSI_PIN, serial data input
#define CS D8 // SS_PIN, chip select input

volatile bool cs; // chip select signal
volatile bool di; // data input signal
volatile byte i = 26;  // current bit
volatile byte n;  // current block
volatile unsigned int b; // block buffer, 27 bits: start bit (always 1), 2 bits opcode, 8 bits address, 16 bits data

// bitmasks
// #define mask_op   0x3000000
// #define mask_addr 0xFF0000
// #define mask_data 0xFFFF

byte hiBP; // systolic (high) blood pressure
byte loBP; // diastolic (low) blood pressure
byte HR;   // heart rate

// A memory slot is 4x 16 bit words of data (blocks 3-6):
// Block 1: DI: 100 11xxxxxx = Write enable
// Block 2: DI: 110... = Read address 0 -> DO: 0x101 = 5 (start of next free memory slot)
// Block 3: Write address 5, bin: 0000100000000100, hex: 08 04, ints: 8 (month) 4 (?, constant)
// Block 4: Write address 6, bin: 0000101000010101, hex: 0A 15, ints: 10 (hour) 21 (day)
// Block 5: Write address 7, bin: 0111010100100000, hex: 75 20, ints: 117 (high BP - 20) 32 (minutes)
// Block 6: Write address 8, bin: 0011110001010111, hex: 3C 57, ints: 60 (HR) 87 (low BP)
// Block 7: Write address 0, bin: 0000001000000010, hex: 02 02, ints: 2 2 (number of occupied memory slots)
// Block 8: DI: 100 00xxxxxx = Write disable

// ISRs
void ICACHE_RAM_ATTR CS_change() {
  cs = !cs;
  if (!cs && b) { // falling edge, and something written to buffer
    n++;
    // serial output makes the interrupt take too long and corrupts the data for following blocks!
    // Serial.printf("Block %d: ", n);
    // Serial.println(b, BIN);
    // Serial.println(b & mask_data, HEX);
    // Serial.println();
    byte byte1 = (b & 0xFF00) >> 8;
    byte byte2 = b & 0x00FF;
    if (n == 5) {
      hiBP = byte1 + 20;
    } else if (n == 6) {
      HR = byte1;
      loBP = byte2;
      // Serial.printf("hiBP: %d, loBP: %d, HR: %d\n", hiBP, loBP, HR);
    } else if (n == 8) {
      n = 0; // sleep/reset ESP anyway?
      Serial.println("Last Block.");
    }
    i = 26; b = 0; // clear buffer
  }
}

void ICACHE_RAM_ATTR CK_rise() {
  // assert(cs); // sometimes CK rises before CS, that's why we reset data only on falling CS
  // di = digitalRead(DI); // takes too long?
  assert(i >= 0);
  if (di) b |= 1UL << i;
  i--;
}

void ICACHE_RAM_ATTR DI_change() {
  // assert(cs);
  di = !di;
}

void setup() {
  Serial.begin(500000);
  // Serial.setDebugOutput(true);
  Serial.println("setup");

  setup_wifi();
  setup_mqtt();

  pinMode(CS, INPUT);
  pinMode(CK, INPUT);
  pinMode(DI, INPUT);

  // RISING, FALLING, CHANGE
  attachInterrupt(digitalPinToInterrupt(CS), CS_change, CHANGE);
  attachInterrupt(digitalPinToInterrupt(CK), CK_rise, RISING);
  attachInterrupt(digitalPinToInterrupt(DI), DI_change, CHANGE);
}

void loop(){
  mqtt.loop(); // let it work, otherwise it can't publish
  delay(500);
  // mqtt.publish directly in ISR does not work!
  if (HR) {
    Serial.printf("hiBP: %d, loBP: %d, HR: %d\n", hiBP, loBP, HR);
    mqtt.publish(MQTT_TOPIC, json("\"hiBP\": %d, \"loBP\": %d, \"HR\": %d", hiBP, loBP, HR));
    Serial.println("Published to MQTT.");
    HR = 0; n = 0;
    // ESP.reset(); // this may reset before message is published! https://github.com/knolleary/pubsubclient/issues/452#issuecomment-505059218
  }
}
