#include "btn_bar_menu.h"
#include <ili9341_t3n_font_Arial.h>


/** Maximum number of menu elements on display */
static const uint8_t BTN_BAR_MENU_ELEMENTS_PER_SCREEN = 4;

static const uint BUTTON_SIZE = 80;
static const uint BUTTON_BAR_HEIGHT = 56;
static const uint BUTTON_BAR_Y_POS = 240 - BUTTON_BAR_HEIGHT;
static const uint BUTTON_BAR_TXT_Y_POS = BUTTON_BAR_Y_POS + 15;
static const uint BUTTON_BAR_SUBTXT_Y_POS = BUTTON_BAR_TXT_Y_POS + 27;

BtnBarMenu::BtnBarMenu(ILI9341_t3n *display) {
  _count = 0;
  current_page = 0;
  d = display;
};

int BtnBarMenu::add(const char *label) {
  int id = _count++;
  buttons[id].label = label;
  buttons[id].hasSubText = false;
  buttons[id].has_sub_text = false;
  return id;
}

int BtnBarMenu::add(const char *label, const char **subText) {
  int id = _count++;
  buttons[id].label = label;
  buttons[id].subText = subText;
  buttons[id].hasSubText = true;
  buttons[id].has_sub_text = false;
  return id;
}

int BtnBarMenu::add(const char *label, const char *sub_text, uint init_val, uint len) {
  int id = _count++;
  buttons[id].label = label;
  buttons[id].sub_text = sub_text;
  buttons[id].value = init_val;
  buttons[id].len = len;
  buttons[id].hasSubText = false;
  buttons[id].has_sub_text = true;
  return id;
}

uint BtnBarMenu::incVal(int id)
{
  buttons[id].value = ++buttons[id].value % buttons[id].len;
  return buttons[id].value;
}

uint BtnBarMenu::pageCount()
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
  d->fillRect(0, BUTTON_BAR_Y_POS - 13, 320, 13, ILI9341_BLACK);
  d->setFont(Arial_10);
  d->setTextColor(ILI9341_LIGHTGREY);
  d->setCursor(2, BUTTON_BAR_Y_POS - 12);
  d->println(_title);

	// Draw the buttons
  d->fillRect(0, BUTTON_BAR_Y_POS, 320, BUTTON_BAR_HEIGHT, ILI9341_BLACK);
  //d->fillRectVGradient(0, BUTTON_BAR_Y_POS, 320, BUTTON_BAR_HEIGHT, CL(150,150,150), CL(80,80,80));
  //d->fillRect(0, BUTTON_BAR_Y_POS, BUTTON_SIZE, BUTTON_BAR_HEIGHT, ILI9341_BLACK);
  //d->fillRect(BUTTON_SIZE, BUTTON_BAR_Y_POS, BUTTON_SIZE, BUTTON_BAR_HEIGHT, ILI9341_LIGHTGREY);
  //d->fillRect(BUTTON_SIZE * 2, BUTTON_BAR_Y_POS, BUTTON_SIZE, BUTTON_BAR_HEIGHT, ILI9341_DARKGREY);
  //d->fillRect(BUTTON_SIZE * 3, BUTTON_BAR_Y_POS, BUTTON_SIZE, BUTTON_BAR_HEIGHT, ILI9341_NAVY);
  
  // draw button frames
  d->drawRect(0, BUTTON_BAR_Y_POS, BUTTON_SIZE, BUTTON_BAR_HEIGHT, CL(150,150,150));
  d->drawRect(BUTTON_SIZE, BUTTON_BAR_Y_POS, BUTTON_SIZE, BUTTON_BAR_HEIGHT, CL(150,150,150));
  d->drawRect(BUTTON_SIZE * 2, BUTTON_BAR_Y_POS, BUTTON_SIZE, BUTTON_BAR_HEIGHT, CL(150,150,150));
  d->drawRect(BUTTON_SIZE * 3, BUTTON_BAR_Y_POS, BUTTON_SIZE, BUTTON_BAR_HEIGHT, CL(150,150,150));
  
	// Print visible options
  d->setFont(Arial_14);
  for (i = current_page * (BTN_BAR_MENU_ELEMENTS_PER_SCREEN - 1);
      i < current_page *
      (BTN_BAR_MENU_ELEMENTS_PER_SCREEN - 1) +
      btns_per_page &&
      i < _count; i++) {
    drawButtonBarText(i, btn);
    btn++;
  }
  if (_count > BTN_BAR_MENU_ELEMENTS_PER_SCREEN) {
    int x = (BTN_BAR_MENU_ELEMENTS_PER_SCREEN - 1) * BUTTON_SIZE + BUTTON_SIZE / 2;
    int y = BUTTON_BAR_TXT_Y_POS;
    d->setTextColor(ILI9341_WHITE);
    d->setFont(Arial_14);
    d->setCursor(x, y, true);
    d->print("...");
    y = BUTTON_BAR_SUBTXT_Y_POS;
    d->setTextColor(ILI9341_YELLOW);
    d->setCursor(x, y, true);
    String s = String(current_page + 1) + "/" + String(pageCount());
    d->print(s);
  }
}

void BtnBarMenu::drawButtonBarText(int id, int btnNr)
{
  int x = btnNr * BUTTON_SIZE + BUTTON_SIZE / 2;
  int y = BUTTON_BAR_TXT_Y_POS;
  d->setTextColor(ILI9341_WHITE);
  d->setCursor(x, y, true);
  d->print(buttons[id].label);
  if (buttons[id].hasSubText) {
    x = btnNr * BUTTON_SIZE + BUTTON_SIZE / 2;
    y = BUTTON_BAR_SUBTXT_Y_POS;
    d->setTextColor(ILI9341_YELLOW);
    d->setCursor(x, y, true);
    d->print(*buttons[id].subText);
  }
  if (buttons[id].has_sub_text) {
    x = btnNr * BUTTON_SIZE + BUTTON_SIZE / 2;
    y = BUTTON_BAR_SUBTXT_Y_POS;
    d->setTextColor(ILI9341_YELLOW);
    d->setCursor(x, y, true);
    d->print(buttons[id].sub_text[buttons[id].value]);
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
    if (current_page *
        (BTN_BAR_MENU_ELEMENTS_PER_SCREEN - 1) +
        (BTN_BAR_MENU_ELEMENTS_PER_SCREEN - 1) >= _count) {
			current_page = 0;
		} else {
			current_page++;
		}

		(void)(*key_feedback)();
    // Update menu on display
		draw();
		// Nothing selected yet
		return BTN_BAR_MENU_EVENT_IDLE;
  }
  
  int btnId = current_page * (BTN_BAR_MENU_ELEMENTS_PER_SCREEN - 1) + key_idx;
  if (btnId < _count) {
    (void)(*key_feedback)();
    // Got what we want. Return button id.
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
