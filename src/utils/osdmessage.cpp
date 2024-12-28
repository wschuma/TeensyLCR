#include "osdmessage.h"
#include <ili9341_t3n_font_Arial.h>


OSDMessage::OSDMessage(ILI9341_t3n *display) {
  _d = display;
};

/*
 * Set the message to be displayed.
 */
void OSDMessage::setMessage(const char* text) {
  static const uint MESSAGE_TIMEOUT = 3000;
  _text = text;
  _state = OSDState::Show;
  _timeout = millis() + MESSAGE_TIMEOUT;
}

/*
 * If set by `setMessage`, draws the message to screen.
 * Needs to be called before `updateScreen()`.
 */
bool OSDMessage::show() {
  if (_state == OSDState::Show) {
    draw();
    return true;
  }
  return false;
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

/*
 * Cleans the message area after timeout. Returns true in this case, otherwise false.
 * Needs to be called after `updateScreen()` or before drawing something to screen.
 */
bool OSDMessage::clean() {
  if (_state == OSDState::Show && millis() > _timeout) {
    _d->fillRect(10, 80, 300, 30, ILI9341_BLACK);
    _state = OSDState::None;
    return true;
  }
  return false;
}

/*
 * Clear the OSD message. `show()` no longer draws messages.
 */
void OSDMessage::clear() {
  _state = OSDState::None;
}
