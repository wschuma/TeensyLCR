#include "helper.h"
#include "board.h"
#include "src/utils/imgtofile.h"
#include <TimeLib.h>

USBHost myusb;
USBDrive usbDrive(myusb);

// USBFilesystem is a class based on claimed partitions discovered during
// initialization. We are using the first discovered partition here. More
// discovered partitions can be used by adding more instances of the
// 'USBFilesystem' class.
USBFilesystem usbPartition1(myusb);

int usbCheckPartition()
{
  // Wait for MSC drive to be auto detected. Uses 5 second timeout
  // and no drive detected error. Waits for drive to be inserted.
  Serial.println("Check USB drive...");
  if(!usbDrive.msDriveInfo.connected) {
    return -1;
  }

  Serial.println("check partition...");
  elapsedMillis mscTimeOut = 0;
  while(!usbPartition1) {
    myusb.Task();
    if(mscTimeOut > MEDIA_READY_TIMEOUT) {
      Serial.println("initialization failed!");
      return -2;
    }
    delay(1);
  }
  Serial.println("initialization done.");

  return 0;
}

void saveScreenshot(ILI9341_t3n *display)
{
  int state = usbCheckPartition();
  if (state) {
    osdMessage.setMessage(F("USB drive not connected!"));
    return;
  }

  // find filename
  char filePath[21];
  uint filenr = 0;
  do {
    if (++filenr > 9999) {
      return;
    }
    sprintf(filePath, "/screenshot_%04i.bmp", filenr);
  } while (usbPartition1.exists(filePath));

  File f = usbPartition1.open(filePath, FILE_WRITE_BEGIN);
  if (!f)
  {
    osdMessage.setMessage(F("Failed to create file!"));
    return;
  }

  // set timestamp of file
  DateTimeFields dtf;
  breakTime(Teensy3Clock.get(), dtf);
  f.setCreateTime(dtf);
  f.setModifyTime(dtf);

  uint16_t *imageData = display->getFrameBuffer();
  writeImageToFile(&f, imageData, display->width(), display->height());
  
  osdMessage.setMessage(F("Screenshot saved."));
}
