#include "usbstorage.h"

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

