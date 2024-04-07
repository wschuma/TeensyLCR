#include "usbstorage.h"
#include <TimeLib.h>

/*
 * Converts RGB565 format to RGB555 format.
 */
#define color565to555(c) ((c & 0xffc0) >> 1) | (c & 0x1f)

// save previous packing, then set packing for 16-bits
# pragma pack (push)
# pragma pack (2)
struct bmpFileHeader_t {
  uint16_t  fileId = 0x4d42;   // 'bm'
  uint32_t  fileSize;
  uint16_t  reserved1 = 0;
  uint16_t  reserved2 = 0;
  uint32_t  offsetImgData = 54; // file header size + DIB header size
} bmpFileHeader;

struct bmpDIBHeader_t {
  uint32_t  size = 40;
  int32_t   imgWidth;
  int32_t   imgHeight;
  int16_t   planes = 1;
  uint16_t  bitsPerPixel = 16;
  uint32_t  compression = 0;
  uint32_t  imgSize = 0; // dummy 0 for BI_RGB
  int32_t   resX = 0;
  int32_t   resY = 0;
  uint32_t  colorsInPalette = 0;
  uint32_t  colorsImportant = 0; // 0 when every color is important
} bmpDIBHeader;
// restore previous packing
# pragma pack (pop)

USBHost myusb;
USBDrive usbDrive(myusb);

// USBFilesystem is a class based on claimed partitions discovered during
// initialization. We are using the first discovered partition here. More
// discovered partitions can be used by adding more instances of the
// 'USBFilesystem' class.
USBFilesystem partition1(myusb);
elapsedMillis mscTimeOut; 

// Save the headers and the image data into the .bmp file
int saveImage(USBFilesystem* fs, const char* filePath, uint16_t *imageData, uint16_t width, uint16_t height) {
  // set headers
  int fileSize = sizeof(bmpFileHeader) + sizeof(bmpDIBHeader) + width * height * 2; // two bytes per pixel
  bmpFileHeader.fileSize = fileSize;
  bmpDIBHeader.imgWidth = width;
  bmpDIBHeader.imgHeight = -height;

  // Write the bitmap file
  File f = fs->open(filePath, FILE_WRITE_BEGIN);
  if (!f)
  {
    Serial.println("Unable to create file!");
    return -3;
  }
  f.write(&bmpFileHeader, sizeof(bmpFileHeader));
  f.write(&bmpDIBHeader, sizeof(bmpDIBHeader));
  
  uint16_t pixelData;
  for (uint32_t pidx = 0; pidx < width * height; pidx++) {
    pixelData = *(imageData + pidx);
    pixelData = color565to555(pixelData);
    f.write(&pixelData, 2);
  }

  f.close();
  return 0;
}

int usbSaveScreenshot(ILI9341_t3n* display)
{
  // Wait for MSC drive to be auto detected. Uses 5 second timeout
  // and no drive detected error. Waits for drive to be inserted.
  Serial.println("Check USB drive...");
  if(!usbDrive.msDriveInfo.connected) {
    return -1;
  }

  Serial.println("check partition...");
  mscTimeOut = 0;
  while(!partition1) {
    myusb.Task();
    if(mscTimeOut > MEDIA_READY_TIMEOUT) {
      Serial.println("initialization failed!");
      return -2;
    }
    delay(1);
  }
  Serial.println("initialization done.");

  char filePath[50];
  sprintf(filePath, "/screenshot_%04i-%02i-%02i_%02i-%02i-%02i.bmp", year(), month(), day(), hour(), minute(), second());

  Serial.print("Saving image to ");
  Serial.print(filePath);
  Serial.println("...");

  uint16_t *imageData = display->getFrameBuffer();
  int state = saveImage(&partition1, filePath, imageData, display->width(), display->height());
  if (state != 0)
    return state;

  Serial.println("done");
  return 0;
}
