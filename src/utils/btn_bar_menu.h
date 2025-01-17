#ifndef BTN_BAR_MENU_H
#define BTN_BAR_MENU_H

#include <Arduino.h>
#include <ILI9341_t3n.h>
#include <XPT2046_Touchscreen.h>

/** Idle. Nothing to report. */
static const uint8_t BTN_BAR_MENU_EVENT_IDLE = 0xFF;


class BtnBarMenu {
 public:
  BtnBarMenu(ILI9341_t3n* display);
  /** Initialize the menu without title. */
  void init(void (*keyFeedback)()) {
    init(keyFeedback, "");
  }
  /** Initialize the menu. Add a menu title. */
  void init(void (*keyFeedback)(), const char* title) {
    _count = 0;
    _current_page = 0;
    key_feedback = keyFeedback;
    _title = title;
  }
  /** Returns the menu item count. */
  int itemCount() {
    return _count;
  };
  /** Add a menu item without any action. */
  int add(const char* label);
  /** Add a menu item with action. */
  int add(const char* label, void (*action)());
  /** Add a menu item with sub text and action. */
  int add(const char* label, const char** subText, void (*action)());
  /** Process button press */
  int processKey(int key_idx);
  /** Process touch press */
  int processTSPoint(TS_Point p);
  /** Returns the menu page count. */
  uint8_t pageCount();
  /** Draw the menu to display screen. */
  void draw();

 private:
  void drawButtonBarText(int id, int btnNr);
  ILI9341_t3n* _display;
  uint8_t _count;
  const char* _title;
  struct MenuItem {
    const char* label;
    const char** subText;
    bool hasSubText;
    void (*action)();
  };
  struct MenuItem _items[15]; // max elements in a menu, increase as needed
  uint8_t _current_page;
  void (*key_feedback)();
};


#endif // BTN_BAR_MENU_H
