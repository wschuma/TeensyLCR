#ifndef USBSTORAGE_H_
#define USBSTORAGE_H_

#include <Arduino.h>
#include <USBHost_t36.h>

extern USBHost myusb;
extern USBDrive usbDrive;
extern USBFilesystem usbPartition1;

int usbCheckPartition();

#endif