#include "osdmessage.h"
#include <ili9341_t3n_font_Arial.h>


OSDMessage::OSDMessage(ILI9341_t3n *display) {
  _d = display;
};

void OSDMessage::setMessage(const char* text) {
  static const uint MESSAGE_TIMEOUT = 3000;
  _text = text;
  _state = MSG_DRAW;
  _timeout = millis() + MESSAGE_TIMEOUT;
}

void OSDMessage::show(bool force) {
  if (_state == MSG_NONE) {
    return;
  }
  if (_state == MSG_DRAW || force) {
    draw();
    _state = MSG_SHOW;
  }
  if (millis() > _timeout) {
    _state = MSG_CLEAR;
  }
}

void OSDMessage::draw() {
  // draw background
  _d->fillRect(10, 80, 300, 30, ILI9341_MAROON);
  // draw frame
  _d->drawRect(10, 80, 300, 30, CL(150,150,150));
  // draw text
  _d->setTextColor(ILI9341_WHITE);
  _d->setCursor(160, 95, true);
  _d->setFont(Arial_14);
  _d->print(_text);
}

bool OSDMessage::clean() {
  if (_state == MSG_CLEAR) {
    _d->fillRect(10, 80, 300, 30, ILI9341_BLACK);
    _state = MSG_NONE;
    return true;
  }
  return false;
}
