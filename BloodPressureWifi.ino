#define CK D5 // SCK_PIN, SK: serial clock input
#define DI D7 // MOSI_PIN, serial data input
#define CS D8 // SS_PIN, chip select input

volatile bool cs;
volatile bool di;
volatile byte i;
volatile unsigned int b; // 27 bits: start bit (always 1), 2 bits opcode, 8 bits address, 16 bits data
// bitmasks
#define mask_op   0x60000000
#define mask_addr 0x1FE00000
#define mask_data 0x1FFFFF

// ISRs
void ICACHE_RAM_ATTR CS_change() {
  cs = !cs;
  // Serial.printf("CS: %s\n", cs ? "high" : "low");
  if (cs) {
    i = 31; b = 0; // clear buffer
  } else {
    Serial.println(b, BIN);
    Serial.println((b & mask_data) >> 5, HEX);
    Serial.println();
  }
}

void ICACHE_RAM_ATTR CK_rise() {
  if (!cs) return;
  di = digitalRead(DI);
  // Serial.printf("DI: %d\n", di);
  if (di) b |= 1UL << i;
  i--;
}

void setup() {
  Serial.begin(115200);
  // Serial.setDebugOutput(true);
  Serial.println("setup");

  pinMode(CS, INPUT);
  pinMode(CK, INPUT);
  pinMode(DI, INPUT);

  // RISING, FALLING, CHANGE
  attachInterrupt(digitalPinToInterrupt(CS), CS_change, CHANGE);
  attachInterrupt(digitalPinToInterrupt(CK), CK_rise, RISING);
  // attachInterrupt(digitalPinToInterrupt(DI), DI_change, CHANGE);
}

void loop(){
  
}
