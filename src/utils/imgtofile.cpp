#include "imgtofile.h"

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


/*
 * Save an image to a bitmap file.
 */
void writeImageToFile(File *f, uint16_t *imageData, uint16_t width, uint16_t height) {
  // set headers
  int fileSize = sizeof(bmpFileHeader) + sizeof(bmpDIBHeader) + width * height * 2; // two bytes per pixel
  bmpFileHeader.fileSize = fileSize;
  bmpDIBHeader.imgWidth = width;
  bmpDIBHeader.imgHeight = -height;

  // write headers to file
  f->write(&bmpFileHeader, sizeof(bmpFileHeader));
  f->write(&bmpDIBHeader, sizeof(bmpDIBHeader));
  
  // write pixel data to file
  uint16_t pixelData;
  for (uint32_t pidx = 0; pidx < width * height; pidx++) {
    pixelData = *(imageData + pidx);
    pixelData = color565to555(pixelData);
    f->write(&pixelData, 2);
  }
  
  f->close();
}
