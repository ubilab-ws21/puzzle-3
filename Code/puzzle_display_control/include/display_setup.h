#ifndef DISPLAY_SETUP_H
#define DISPLAY_SETUP_H


#include <SPI.h>
#include <Wire.h>


#include "Adafruit_GFX.h"

#include "Adafruit_RA8875.h"
#include "Adafruit_I2CDevice.h"

#define BACKGROUND_COLOR 0xFFF0


void init_display(void);
int setCalibrationMatrix( tsPoint_t * displayPtr, tsPoint_t * screenPtr, tsMatrix_t * matrixPtr);
int calibrateTSPoint( tsPoint_t * displayPtr, tsPoint_t * screenPtr, tsMatrix_t * matrixPtr );

void waitForTouchEvent(tsPoint_t * point);
tsPoint_t renderCalibrationScreen(uint16_t x, uint16_t y, uint16_t radius);
void tsCalibrate(void);

tsMatrix_t gettsMatrix(void);
Adafruit_RA8875 gettft(void);



#endif