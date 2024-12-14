#ifndef HELPER_H_
#define HELPER_H_

#include <Arduino.h>
#include <ILI9341_t3n.h>
#include <USBHost_t36.h>

extern USBHost myusb;
extern USBDrive usbDrive;
extern USBFilesystem usbPartition1;

int usbCheckPartition();
void saveScreenshot(ILI9341_t3n *display);

#endif
