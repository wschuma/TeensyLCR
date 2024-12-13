#include "helper.h"
#include "board.h"
#include "src/utils/usbstorage.h"
#include "src/utils/imgtofile.h"
#include <TimeLib.h>


void saveScreenshot(ILI9341_t3n *display)
{
  static const char* msgSuccess = "Screenshot saved.";
  static const char* msgErrFile = "Failed to create file!";
  static const char* msgNoUsb = "USB drive not connected!";

  char filePath[50];
  sprintf(filePath, "/screenshot_%04i-%02i-%02i_%02i-%02i-%02i.bmp", year(), month(), day(), hour(), minute(), second());

  int state = usbCheckPartition();
  if (state) {
    osdMessage.setMessage(msgNoUsb);
    return;
  }
  File f = usbPartition1.open(filePath, FILE_WRITE_BEGIN);
  if (!f)
  {
    osdMessage.setMessage(msgErrFile);
    return;
  }
  uint16_t *imageData = display->getFrameBuffer();
  writeImageToFile(&f, imageData, display->width(), display->height());
  
  osdMessage.setMessage(msgSuccess);
}
