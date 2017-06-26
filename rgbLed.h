#ifndef RGBLED
#define RGBLED
#include <stdio.h>
#include <stdbool.h>
#include <wiringPi.h>
#include <time.h>

#define GREENPIN 3
#define REDPIN 5
#define BLUEPIN 4
//#define RELAYPIN 0

//RGBループ用データ構造体
struct dataRgbLedLoop{
  bool flagR;
  bool flagG;
  bool flagB;
  int counter;
  int rlux;
  int glux;
  int blux;
  int currentFlag;
  int beforeFlag;
  int calcData;
};

struct dataRgbLedLoop initDataRgbLoop(void);
void rgbLedLoop(struct dataRgbLedLoop *loopdat);
void rgbPoling(int r,int g,int b);


#endif
