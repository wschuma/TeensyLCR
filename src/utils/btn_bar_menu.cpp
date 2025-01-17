#include "btn_bar_menu.h"
#include <ili9341_t3n_font_Arial.h>


/** Maximum number of menu elements on display */
static const uint8_t BTN_BAR_MENU_ELEMENTS_PER_SCREEN = 4;

static const uint BUTTON_SIZE = 80;
static const uint BUTTON_BAR_HEIGHT = 56;
static const int BUTTON_BAR_Y_POS = 240 - BUTTON_BAR_HEIGHT;
static const uint BUTTON_BAR_TXT_Y_POS = BUTTON_BAR_Y_POS + 15;
static const uint BUTTON_BAR_SUBTXT_Y_POS = BUTTON_BAR_TXT_Y_POS + 27;

BtnBarMenu::BtnBarMenu(ILI9341_t3n *display) {
  _count = 0;
  _current_page = 0;
  _display = display;
};

void noAction()
{}

int BtnBarMenu::add(const char* label) {
  int id = _count++;
  _items[id].label = label;
  _items[id].hasSubText = false;
  _items[id].action = &noAction;
  return id;
}

int BtnBarMenu::add(const char* label, void (*action)()) {
  int id = _count++;
  _items[id].label = label;
  _items[id].hasSubText = false;
  _items[id].action = action;
  return id;
}

int BtnBarMenu::add(const char* label, const char** subText, void (*action)()) {
  int id = _count++;
  _items[id].label = label;
  _items[id].subText = subText;
  _items[id].hasSubText = true;
  _items[id].action = action;
  return id;
}

uint8_t BtnBarMenu::pageCount()
{
  if (_count > BTN_BAR_MENU_ELEMENTS_PER_SCREEN)
    return 1 + (_count - 1) / (BTN_BAR_MENU_ELEMENTS_PER_SCREEN - 1);
  else
    return 1;
}

void BtnBarMenu::draw() {
  int i;
  int btn = 0;
  int btns_per_page = BTN_BAR_MENU_ELEMENTS_PER_SCREEN - 1;
  if (_count <= BTN_BAR_MENU_ELEMENTS_PER_SCREEN)
    btns_per_page = BTN_BAR_MENU_ELEMENTS_PER_SCREEN;

  // draw title
  _display->fillRect(0, BUTTON_BAR_Y_POS - 13, 320, 13, ILI9341_BLACK);
  _display->setFont(Arial_10);
  _display->setTextColor(ILI9341_LIGHTGREY);
  _display->setCursor(2, BUTTON_BAR_Y_POS - 12);
  _display->println(_title);

  // Draw the buttons
  _display->fillRect(0, BUTTON_BAR_Y_POS, 320, BUTTON_BAR_HEIGHT, ILI9341_BLACK);
  //_display->fillRectVGradient(0, BUTTON_BAR_Y_POS, 320, BUTTON_BAR_HEIGHT, CL(150,150,150), CL(80,80,80));
  //_display->fillRect(0, BUTTON_BAR_Y_POS, BUTTON_SIZE, BUTTON_BAR_HEIGHT, ILI9341_BLACK);
  //_display->fillRect(BUTTON_SIZE, BUTTON_BAR_Y_POS, BUTTON_SIZE, BUTTON_BAR_HEIGHT, ILI9341_LIGHTGREY);
  //_display->fillRect(BUTTON_SIZE * 2, BUTTON_BAR_Y_POS, BUTTON_SIZE, BUTTON_BAR_HEIGHT, ILI9341_DARKGREY);
  //_display->fillRect(BUTTON_SIZE * 3, BUTTON_BAR_Y_POS, BUTTON_SIZE, BUTTON_BAR_HEIGHT, ILI9341_NAVY);
  
  // draw button frames
  _display->drawRect(0, BUTTON_BAR_Y_POS, BUTTON_SIZE, BUTTON_BAR_HEIGHT, CL(150,150,150));
  _display->drawRect(BUTTON_SIZE, BUTTON_BAR_Y_POS, BUTTON_SIZE, BUTTON_BAR_HEIGHT, CL(150,150,150));
  _display->drawRect(BUTTON_SIZE * 2, BUTTON_BAR_Y_POS, BUTTON_SIZE, BUTTON_BAR_HEIGHT, CL(150,150,150));
  _display->drawRect(BUTTON_SIZE * 3, BUTTON_BAR_Y_POS, BUTTON_SIZE, BUTTON_BAR_HEIGHT, CL(150,150,150));
  
  // Print visible items
  _display->setFont(Arial_14);
  for (i = _current_page * (BTN_BAR_MENU_ELEMENTS_PER_SCREEN - 1);
      i < _current_page *
      (BTN_BAR_MENU_ELEMENTS_PER_SCREEN - 1) +
      btns_per_page &&
      i < _count; i++) {
    drawButtonBarText(i, btn);
    btn++;
  }
  if (_count > BTN_BAR_MENU_ELEMENTS_PER_SCREEN) {
    int x = (BTN_BAR_MENU_ELEMENTS_PER_SCREEN - 1) * BUTTON_SIZE + BUTTON_SIZE / 2;
    int y = BUTTON_BAR_TXT_Y_POS;
    _display->setTextColor(ILI9341_WHITE);
    _display->setFont(Arial_14);
    _display->setCursor(x, y, true);
    _display->print("...");
    y = BUTTON_BAR_SUBTXT_Y_POS;
    _display->setTextColor(ILI9341_YELLOW);
    _display->setCursor(x, y, true);
    char buf[9];
    sprintf(buf, "%i/%i", _current_page + 1, pageCount());
    _display->print(buf);
  }
}

void BtnBarMenu::drawButtonBarText(int id, int btnNr)
{
  int x = btnNr * BUTTON_SIZE + BUTTON_SIZE / 2;
  int y = BUTTON_BAR_TXT_Y_POS;
  _display->setTextColor(ILI9341_WHITE);
  _display->setCursor(x, y, true);
  _display->print(_items[id].label);
  if (_items[id].hasSubText) {
    x = btnNr * BUTTON_SIZE + BUTTON_SIZE / 2;
    y = BUTTON_BAR_SUBTXT_Y_POS;
    _display->setTextColor(ILI9341_YELLOW);
    _display->setCursor(x, y, true);
    _display->print(*_items[id].subText);
  }
}

int BtnBarMenu::processKey(int key_idx)
{
  if (key_idx >= BTN_BAR_MENU_ELEMENTS_PER_SCREEN) {
    // Unknown key
    return BTN_BAR_MENU_EVENT_IDLE;
  }

  if (_count > BTN_BAR_MENU_ELEMENTS_PER_SCREEN &&
      key_idx == BTN_BAR_MENU_ELEMENTS_PER_SCREEN - 1) {
    // next page
    if (_current_page *
        (BTN_BAR_MENU_ELEMENTS_PER_SCREEN - 1) +
        (BTN_BAR_MENU_ELEMENTS_PER_SCREEN - 1) >= _count) {
      _current_page = 0;
    } else {
      _current_page++;
    }

    (void)(*key_feedback)();
    // Update menu on display
    draw();
    _display->updateScreen();
    // Nothing selected yet
    return BTN_BAR_MENU_EVENT_IDLE;
  }
  
  int btnId = _current_page * (BTN_BAR_MENU_ELEMENTS_PER_SCREEN - 1) + key_idx;
  if (btnId < _count) {
    (void)(*key_feedback)();
    (void)(*_items[btnId].action)();
    // Got what we want. Return item id.
    return btnId;
  }

  // Unknown menu element
  return BTN_BAR_MENU_EVENT_IDLE;
}

int BtnBarMenu::processTSPoint(TS_Point p)
{
  if (p.y < BUTTON_BAR_Y_POS)
    return BTN_BAR_MENU_EVENT_IDLE;

  int key_idx = p.x / BUTTON_SIZE;
  return processKey(key_idx);
}
