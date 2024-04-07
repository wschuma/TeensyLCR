#ifndef BTN_BAR_MENU
#define BTN_BAR_MENU

#include <Arduino.h>
#include <ILI9341_t3n.h>
#include <XPT2046_Touchscreen.h>

/** Idle. Nothing to report. */
static const uint8_t BTN_BAR_MENU_EVENT_IDLE = 0xFF;


class BtnBarMenu {
public:
	BtnBarMenu(ILI9341_t3n* display);
  void init(void (*keyFeedback)()) {
    init(keyFeedback, "");
  }
  void init(void (*keyFeedback)(), const char* title) {
    _count = 0;
    current_page = 0;
    key_feedback = keyFeedback;
    _title = title;
  }
  int itemCount() {
    return _count;
  };
  int add(const char* label);
  int add(const char* label, void* callback);
  int add(const char* label, const char** subText);
  int add(const char* label, const char* sub_text, uint init_val, uint len);
  int processKey(int key_idx);
  int processTSPoint(TS_Point p);
  uint incVal(int id);
  uint pageCount();
  void draw();
  void drawTitle();

private:
  void drawButtonBarText(int id, int btnNr);

  ILI9341_t3n* d;
  int _count;
  const char* _title;
  struct btnBarButton {
    const char* label;
    const char** subText;
    const char* sub_text;
    uint len;
    uint value;
    bool hasSubText;
    bool has_sub_text;
  };
  struct btnBarButton buttons[15]; // max elements in a menu, increase as needed
  int current_page;
  void (*key_feedback)();
};


#endif