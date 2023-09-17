/*
 * Thanks to Andrew Mascolo's keyboard project for the original sketch
 * https://github.com/AndrewMascolo/Adafruit_Stuff/blob/master/Sketches/Keyboard.ino
 */

#include "Keyboard.hpp"

#include <Adafruit_GFX.h>
#include <SPI.h>
#include <Wire.h>
#include <pgmspace.h>

#define IsWithin(x, a, b) ((x >= a) && (x <= b))
#define TS_MINX 142
#define TS_MINY 125
#define TS_MAXX 3871
#define TS_MAXY 3727

const char Mobile_KB[3][13] PROGMEM = {
  { 0, 13, 10, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P' },
  { 1, 12, 9, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L' },
  { 3, 10, 7, 'Z', 'X', 'C', 'V', 'B', 'N', 'M' },
};

const char Mobile_NumKeys[3][13] PROGMEM = {
  { 0, 13, 10, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' },
  { 0, 13, 10, '-', '/', ':', ';', '(', ')', '$', '&', '@', '"' },
  { 5, 8, 5, '.', '\,', '?', '!', '\'' }
};

const char Mobile_SymKeys[3][13] PROGMEM = {
  { 0, 13, 10, '[', ']', '{', '}', '#', '%', '^', '*', '+', '=' },
  { 4, 9, 6, '_', '\\', '|', '~', '<', '>' },  //4
  { 5, 8, 5, '.', '\,', '?', '!', '\'' }
};

Keyboard::Keyboard(Adafruit_ILI9341* _tft, XPT2046_Touchscreen* _ts)
  : tft(_tft), ts(_ts) {
}

void Keyboard::drawPlayerInput(int playerIndex) {
  tft->fillScreen(ILI9341_BLACK);
  tft->setTextColor(ILI9341_WHITE);
  tft->setTextSize(2);
  tft->setCursor(100, 15);
  tft->printf("Player %d:", playerIndex);

  bufIndex = 0;
  drawKeyboard(Mobile_KB);
}

void Keyboard::drawKeyboard(const char type[][13]) {
  tft->fillRect(0, 95, 320, 145, ILI9341_BLACK);
  tft->setTextSize(2);
  tft->setTextColor(ILI9341_WHITE, ILI9341_ORANGE);
  for (int y = 0; y < 3; y++) {
    int ShiftRight = 15 * pgm_read_byte(&(type[y][0]));
    for (int x = 3; x < 13; x++) {
      if (x >= pgm_read_byte(&(type[y][1]))) break;

      drawButton(15 + (30 * (x - 3)) + ShiftRight, 100 + (30 * y), 20, 25);  // this will draw the button on the screen by so many pixels
      tft->setCursor(20 + (30 * (x - 3)) + ShiftRight, 105 + (30 * y));
      tft->print(char(pgm_read_byte(&(type[y][x]))));
    }
  }
  //ShiftKey
  drawButton(15, 160, 35, 25);
  tft->setCursor(27, 168);
  tft->print('^');

  //Special Characters
  drawButton(15, 190, 35, 25);
  tft->setCursor(21, 195);
  tft->print(F("SP"));

  //BackSpace
  drawButton(270, 160, 35, 25);
  tft->setCursor(276, 165);
  tft->print(F("BS"));

  //Return
  drawButton(270, 190, 35, 25);
  tft->setCursor(276, 195);
  tft->print(F("RT"));

  //Spacebar
  drawButton(60, 190, 200, 25);
  tft->setCursor(105, 195);
  tft->print(F("SPACE BAR"));
}

void Keyboard::drawButton(int x, int y, int w, int h) {
  // grey
  tft->fillRoundRect(x - 3, y + 3, w, h, 3, 0xec00);  //Button Shading

  // white
  tft->fillRoundRect(x, y, w, h, 3, ILI9341_WHITE);  // outter button color

  //red
  tft->fillRoundRect(x + 1, y + 1, w - 1 * 2, h - 1 * 2, 3, ILI9341_ORANGE);  //inner button color
}

void Keyboard::redrawKeyboard(bool shift, bool special) {
  if (special) {
    if (shift) {
      drawKeyboard(Mobile_SymKeys);
    } else {
      drawKeyboard(Mobile_NumKeys);
    }
  } else {
    drawKeyboard(Mobile_KB);
    tft->setTextColor(ILI9341_WHITE, ILI9341_ORANGE);
  }

  if (special)
    tft->setTextColor(ILI9341_RED, ILI9341_ORANGE);
  else
    tft->setTextColor(ILI9341_WHITE, ILI9341_ORANGE);

  tft->setCursor(21, 195);
  tft->print(F("SP"));

  if (shift)
    tft->setTextColor(ILI9341_RED, ILI9341_ORANGE);
  else
    tft->setTextColor(ILI9341_WHITE, ILI9341_ORANGE);

  tft->setCursor(27, 168);
  tft->print('^');
}

void Keyboard::getKeyPress(char* const textBuffer, const char textLimit) {
  char key = 0;
  static bool shift = false, special = false, back = false, lastSp = false, lastSh = false;

  if (ts->touched()) {
    //ShiftKey
    if (touchButton(15, 160, 35, 25)) {
      shift = !shift;
    }

    //Special Characters
    if (touchButton(15, 190, 35, 25)) {
      special = !special;
    }

    if (special != lastSp || shift != lastSh) {
      redrawKeyboard(shift, special);
      lastSp = special;
      lastSh = shift;
    }

    for (int y = 0; y < 3; y++) {
      int ShiftRight;
      if (special) {
        if (shift)
          ShiftRight = 15 * pgm_read_byte(&(Mobile_SymKeys[y][0]));
        else
          ShiftRight = 15 * pgm_read_byte(&(Mobile_NumKeys[y][0]));
      } else
        ShiftRight = 15 * pgm_read_byte(&(Mobile_KB[y][0]));

      for (int x = 3; x < 13; x++) {
        if (x >= (special ? (shift ? pgm_read_byte(&(Mobile_SymKeys[y][1])) : pgm_read_byte(&(Mobile_NumKeys[y][1]))) : pgm_read_byte(&(Mobile_KB[y][1])))) break;

        if (touchButton(15 + (30 * (x - 3)) + ShiftRight, 100 + (30 * y), 20, 25))  // this will draw the button on the screen by so many pixels
        {
          if (bufIndex < (textLimit - 1)) {

            if (special) {
              if (shift)
                textBuffer[bufIndex] = pgm_read_byte(&(Mobile_SymKeys[y][x]));
              else
                textBuffer[bufIndex] = pgm_read_byte(&(Mobile_NumKeys[y][x]));
            } else
              textBuffer[bufIndex] = (pgm_read_byte(&(Mobile_KB[y][x])) + (shift ? 0 : ('a' - 'A')));

            bufIndex++;
          }
          break;
        }
      }
    }

    //Spacebar
    if (touchButton(60, 190, 200, 25)) {
      textBuffer[bufIndex++] = ' ';
    }

    //BackSpace
    if (touchButton(270, 160, 35, 25)) {
      if ((bufIndex) > 0)
        bufIndex--;
      textBuffer[bufIndex] = 0;
      tft->fillRect(0, 77, 320, 20, ILI9341_BLACK);
    }

    //Return
    if (touchButton(270, 190, 35, 25)) {
      while (bufIndex > 0) {
        bufIndex--;
        textBuffer[bufIndex] = 0;
      }
      tft->fillRect(0, 77, 320, 20, ILI9341_BLACK);
    }
  }
  tft->setTextColor(ILI9341_WHITE);
  tft->setCursor(15, 80);
  tft->print(textBuffer);
  delay(200);
}

byte Keyboard::touchButton(int x, int y, int w, int h) {
  int X, Y;
  // Retrieve a point
  TS_Point p = ts->getPoint();
  X = map(p.x, TS_MINY, TS_MAXY, tft->width(), 0);
  Y = map(p.y, TS_MINX, TS_MAXX, tft->height(), 0);
  return (IsWithin(X, x, x + w) & IsWithin(Y, y, y + h));
}
