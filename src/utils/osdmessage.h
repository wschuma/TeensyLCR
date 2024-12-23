#ifndef OSDMESSAGE_H_
#define OSDMESSAGE_H_

#include <ILI9341_t3n.h>


class OSDMessage {
public:
	OSDMessage(ILI9341_t3n* Display);
  void setMessage(const char* text);
  bool show();
  bool clean();

private:
  void draw();
  enum osd_state
  {
    MSG_NONE,
    MSG_SHOW,
  };
  ILI9341_t3n* _d;
  const char* _text;
  osd_state _state = MSG_NONE;
  uint32_t _timeout;
};


#endif
