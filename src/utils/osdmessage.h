#ifndef OSDMESSAGE_H_
#define OSDMESSAGE_H_

#include <ILI9341_t3n.h>


class OSDMessage {
public:
  OSDMessage(ILI9341_t3n* Display);
  void setMessage(const char* text);
  void setMessage(const __FlashStringHelper *f) { setMessage((const char*) f); };
  bool show();
  bool clean();
  void clear();

private:
  void draw();
  enum class OSDState
  {
    None,
    Show,
  };
  ILI9341_t3n* _d;
  const char* _text;
  OSDState _state = OSDState::None;
  uint32_t _timeout;
};


#endif
