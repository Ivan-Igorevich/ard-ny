#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

// round leds
#define ROUND_PIN     1
#define NUM_PIXELS    24

// tag
#define SS_PIN        10
#define RST_PIN       9

// 8x8 pixelboard
#define MAX7219_CLK   4
#define MAX7219_CS    5
#define MAX7219_DIN   6

MFRC522 mfrc522(SS_PIN, RST_PIN);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, ROUND_PIN, NEO_GRB + NEO_KHZ800);
unsigned long uidDec, uidDecTemp;  // для храниения номера метки в десятичном формате
unsigned char heart[33][8] = { 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x40, 0x40, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x80, 0x40, 0x40, 0x00, 0x00, 0x00, 
  0x00, 0x80, 0x80, 0x40, 0x40, 0x00, 0x00, 0x00, 
  0x40, 0x80, 0x80, 0x40, 0x40, 0x00, 0x00, 0x00, 
  0x60, 0x80, 0x80, 0x40, 0x40, 0x00, 0x00, 0x00, 
  0x60, 0x90, 0x80, 0x40, 0x40, 0x00, 0x00, 0x00, 
  0x60, 0x90, 0x88, 0x40, 0x40, 0x00, 0x00, 0x00, 
  0x60, 0x90, 0x88, 0x44, 0x40, 0x00, 0x00, 0x00, 
  0x60, 0x90, 0x88, 0x44, 0x44, 0x00, 0x00, 0x00, 
  0x60, 0x90, 0x88, 0x44, 0x44, 0x08, 0x00, 0x00, 
  0x60, 0x90, 0x88, 0x44, 0x44, 0x08, 0x10, 0x00, 
  0x60, 0x90, 0x88, 0x44, 0x44, 0x08, 0x10, 0x20, 
  0x60, 0x90, 0x88, 0x44, 0x44, 0x08, 0x10, 0x60, 
  0x60, 0x90, 0x88, 0x44, 0x44, 0x08, 0x90, 0x60, 
  0x60, 0x90, 0x88, 0x44, 0x44, 0x88, 0x90, 0x60 };
unsigned char pine[8] = {0x40, 0x64, 0x56, 0xcd, 0xcd, 0x56, 0x64, 0x40};
unsigned char yr19[8] = {0x7e, 0x89, 0x89, 0x46, 0x80, 0xff, 0x82, 0x04};
unsigned char ball[8] = {0x38, 0x44, 0xb2, 0xa3, 0x83, 0x82, 0x44, 0x38};

int touchesCount = 0;

void writeMaxByte(unsigned char DATA) {
  unsigned char i;
  digitalWrite(MAX7219_CS, LOW);
  for (i = 8; i >= 1; i--) {
    digitalWrite(MAX7219_CLK, LOW);
    digitalWrite(MAX7219_DIN, DATA & 0x80);
    DATA = DATA << 1;
    digitalWrite(MAX7219_CLK, HIGH);
  }
}

void writeMax(unsigned char address, unsigned char dat) {
  digitalWrite(MAX7219_CS, LOW);
  writeMaxByte(address);
  writeMaxByte(dat);
  digitalWrite(MAX7219_CS, HIGH);
}

void setupMax() {
  pinMode(MAX7219_CLK, OUTPUT);
  pinMode(MAX7219_CS, OUTPUT);
  pinMode(MAX7219_DIN, OUTPUT);
  delay(50);
  writeMax(0x09, 0x00);
  writeMax(0x0a, 0x03);
  writeMax(0x0b, 0x07);
  writeMax(0x0c, 0x01);
  writeMax(0x0f, 0x00);
}

void setupPixels() {
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  pixels.begin();  
}

void drawStatic(unsigned char* p) {
  for (int i = 1; i <= 8; i++) {
    writeMax(i, p[i - 1]);
  }
}

void drawHeart() {
  for (int j = 0; j < 17; j++) {
    for (int i = 0; i < 8; i++)
      writeMax(i + 1, heart[j][i]);
    delay(100);
  }
}

void drawEmpty() {
  for (int i = 0; i < 8; i++)
    writeMax(i + 1, 0x00);
  delay(1000);
}

void runPixels() {
    for (int i = 0; i < NUM_PIXELS; i++) {
      pixels.setPixelColor(i, 
                           pixels.Color(
                             (i % 2 == 0) ? 255 : 0, 
                             (i % 2 != 0) ? 255 : 0, 
                             0)
                          );
      pixels.show();
      delay(200);
    }
}

void clearPixels() {
    for (int i = 0; i < NUM_PIXELS; i++) {
      delay(200);
      pixels.setPixelColor(i, pixels.Color(0, 0, 0));
      pixels.show();
    }
}

void setup() {
  Serial.begin(9600);
  Serial.println("Waiting for card...");
  SPI.begin();  //  инициализация SPI
  setupPixels(); // инициализация круга с диодами
  mfrc522.PCD_Init(); // инициализация MFRC522
  setupMax(); // инициализация доски 8х8
  drawEmpty(); // обнуление доски
  touchesCount = 0;
}

void loop() {
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;
  uidDec = 0;

  // Выдача серийного номера метки.
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uidDecTemp = mfrc522.uid.uidByte[i];
    uidDec = uidDec * 256 + uidDecTemp;
  }
  Serial.println("Card UID: ");
  Serial.println(uidDec); // Выводим UID метки в консоль.

  if (uidDec == 2499689214) {
    switch (touchesCount) {
      case 0:
        drawEmpty(); // обнуление доски
        drawStatic(ball);
        break;
      case 1:
        drawEmpty(); // обнуление доски
        drawStatic(yr19);
        break;
      case 2:
        drawEmpty(); // обнуление доски
        drawStatic(pine);
        break;
      default:
        touchesCount = -1;
    }
    touchesCount++;
//    runPixels();
//    drawHeart();
//    delay(3000);
//    clearPixels();
//    drawEmpty();
  }
}
