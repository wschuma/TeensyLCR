#ifndef imgtofile_h_
#define imgtofile_h_

#include <Arduino.h>
#include <FS.h>

void writeImageToFile(File *f, uint16_t *imageData, uint16_t width, uint16_t height);

#endif
