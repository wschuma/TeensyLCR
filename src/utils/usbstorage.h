#ifndef USBSTORAGE_H_
#define USBSTORAGE_H_

#include <Arduino.h>
#include <USBHost_t36.h>
#include <ILI9341_t3n.h>

extern USBHost myusb;
extern USBDrive usbDrive;

int usbSaveScreenshot(ILI9341_t3n* display);

#endif