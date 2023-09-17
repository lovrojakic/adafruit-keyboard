#ifndef TABLEBASKETBALL_KEYBOARD_H
#define TABLEBASKETBALL_KEYBOARD_H

#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>

class Keyboard {
  private:
    Adafruit_ILI9341* tft;
    XPT2046_Touchscreen* ts;
    char bufIndex;
  public:
    Keyboard(Adafruit_ILI9341* _tft, XPT2046_Touchscreen* _ts);
    void show();
    void drawPlayerInput(int playerIndex);
    void drawKeyboard(const char type[][13]);
    void redrawKeyboard(bool shift, bool special);
    void drawButton(int x, int y, int w, int h);
    void getKeyPress(char* textBuffer, const char textLimit);
    byte touchButton(int x, int y, int w, int h);

};

#endif
