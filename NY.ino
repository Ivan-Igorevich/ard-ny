#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_NeoPixel.h>
#include "pic.h"

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define LED_RED       3
#define LED_GREEN     9
#define BUZZER        7

// круг из диодов
#define ROUND_PIN     1
#define NUM_PIXELS    24

// RFID метка
#define SS_PIN        10
#define RST_PIN       8

// 8x8 диоды
#define MAX7219_CLK   4
#define MAX7219_CS    5
#define MAX7219_DIN   6

MFRC522 mfrc522(SS_PIN, RST_PIN);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, ROUND_PIN, NEO_GRB + NEO_KHZ800);
unsigned long uidDec, uidDecTemp;  // для храниения номера метки в десятичном формате

int touchesCount = 0;
int tempo = 200;
char notes[] = "eeeeeeegcde fffffeeeeddedg";
int duration[] = {1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2};

void playTheShit(char note, int duration) {
  char notesName[] = { 'c', 'd', 'e', 'f', 'g' };
  int tones[] = { 261, 293, 329, 349, 392 };

  for (int i = 0; i < sizeof(tones); i++) {
    // Bind the note took from the char array to the array notesName
    if (note == notesName[i]) {
      // Bind the notesName to tones
      tone(BUZZER, tones[i], duration);
    }
  }
}

// 8x8 инициализация и запись
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

// Нарисовать статичную картинку
void drawStatic(unsigned char* p) {
  for (int i = 1; i <= 8; i++) {
    writeMax(i, p[i - 1]);
  }
}

void drawDynamic(unsigned char p[][8], int steps) {
  for (int j = 0; j < steps; j++) {
    for (int i = 0; i < 8; i++)
      writeMax(i + 1, p[j][i]);
    delay(100);
  }  
}

// Очистить поле
void drawEmpty() {
  for (int i = 0; i < 8; i++)
    writeMax(i + 1, 0x00);
  delay(300);
}

// Заполнить круг красным и зелёным
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

// Постепенно очистить круг
void clearPixels() {
    for (int i = 0; i < NUM_PIXELS; i++) {
      delay(200);
      pixels.setPixelColor(i, pixels.Color(0, 0, 0));
      pixels.show();
    }
}

// Общий сетап платы при старте
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

// Основной цикл
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
  Serial.println(uidDec); // Выводим UID метки в консоль на всякий случай

  // Если коснулись ключом от моего домофона)))
  if (uidDec == 2499689214) {
    drawStatic(snow);
    switch (touchesCount) {
      case 0:
        drawEmpty();
        drawStatic(ball);
        runPixels();
        analogWrite(LED_GREEN, 100);
        break;
      case 1:
        drawEmpty();
        drawStatic(yr19);
        clearPixels();
        analogWrite(LED_RED, 0);
        break;
      case 2:
        drawEmpty();
        drawStatic(pine);
        runPixels();
        analogWrite(LED_RED, 200);
        break;
      case 3:
        drawEmpty();
        drawDynamic(heart, 20);
        analogWrite(LED_GREEN, 0);
        clearPixels();
        break;
      case 4:
        drawEmpty();
        drawStatic(gb);
        analogWrite(LED_RED, 200);
        runPixels();
        break;
      case 5:
        drawEmpty();
        drawStatic(snow);
        clearPixels();
        analogWrite(LED_RED, 200);
        break;
      default:
      // JINGLE BELLS
        for (int i = 0; i < sizeof(notes)-1; i++) {
          if (notes[i] == ' ') {
            // If find a space it rests
            delay(duration[i] * tempo);
          } else {
            playTheShit(notes[i], duration[i] * tempo);
          }
          // Pauses between notes
          delay((tempo*2)*duration[i]);
        }
        analogWrite(LED_RED, 0);
        analogWrite(LED_GREEN, 0);
        drawEmpty();
        touchesCount = -1;
    }
    touchesCount++;
  }
}
