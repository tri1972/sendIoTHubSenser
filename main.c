#include  "sendIoTHubSenser.h"
#include  "rgbLed.h"
#include "lib/libRotaryEncoder.h"
#include "lib/BME280.h"
#include "lib/lib_mcp3425.h"
#include <lcd.h>
#include <math.h>


extern int positionRotary;//割り込みを使ったgetposition()での位置情報出力用変数
extern void getPositionISR(void);//割り込みを使ったロータリースイッチ位置検出

int main(void)
{
  char* deviceId;
  char* connectionString;
  char tmp[256];
  char tmp2[256];
  deviceId=tmp;
  connectionString=tmp2;
  /*
  getConnectString(deviceId,connectionString);
  printf("%sがmain関数で設定されています\n",connectionString);
  */
  printf("Start Sample Program!!\n");
  //時間構造体を初期化
  time_t current_time,before_time;//時間計測用構造体

  time(&before_time);
  double sec_time;//時間（秒）

  //WiringPI初期化
  if (wiringPiSetup() == -1)
    return 1;
  printf("Wiringpi setupOK\n");
  
  //BME280初期化
 if(init_dev()==1)  return 1;

  
  //LED初期化
  pinMode(GREENPIN, OUTPUT);
  pinMode(REDPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);
  pinMode(RELAYPIN, OUTPUT);
  digitalWrite(BLUEPIN, 0);
  digitalWrite(GREENPIN, 0);
  digitalWrite(REDPIN, 0);
  digitalWrite(RELAYPIN, 0);

  //LCD初期化
  int fd;
  char lcdStr[255];
  fd = lcdInit(2,16,4,0,3,4,5,6,7,0,0,0,0);
  lcdClear(fd);
  lcdPuts(fd,"Logging Start!");
  lcdPosition(fd,0,1);
  lcdPuts(fd,"OK");

  //ロータリーエンコーダ初期化
  pinMode(ROTARY_PORTA,INPUT);
  pinMode(ROTARY_PORTB,INPUT);
  //Wirinfpiロータリスイッチ割り込み
  wiringPiISR(ROTARY_PORTA, INT_EDGE_BOTH,getPositionISR);
  wiringPiISR(ROTARY_PORTB, INT_EDGE_BOTH,getPositionISR);

  int beforeRotaryPosition;
  int lcdSTATUS;

  //MPC3425初期化
  int fd3425;
  
  fd3425 = wiringPiI2CSetup(MPL3425_ID);

  
  /*
  //IOTHub初期化
  IOTHUB_CLIENT_HANDLE iotHubClientHandle= IoTHubClient_CreateFromConnectionString(connectionString, MQTT_Protocol);
  remote_monitoring_init(&iotHubClientHandle);
  printf ("IoThub Init OK!\n");

  if  (iotHubClientHandle== NULL)
  {
      printf("Failure in iotHubClientHandle_NULL\n");
  }
  else
  {
    printf("iotHubClientHandle OK!\n");
  }
  */

  struct dataRgbLedLoop loopData=initDataRgbLoop();

  bool gpioRelay=true;
  
  while(1){

    time(&current_time);
    sec_time=difftime(current_time,before_time);
    if(sec_time>60.0)//60秒に一回IOTへ送信
      {
	double nowTemp=getTemperature(fd3425);
	before_time=current_time;//60秒に一回時間カウンタをクリア
	/*
	callback_remote_monitoring_run(&iotHubClientHandle,nowTemp);
	*/
	//ファン用リレー オン/オフ
	if(TemperatureLimit<nowTemp){
	  gpioRelay=true;
	  printf ("Relay On\n");

	}else{
	  gpioRelay=false;
	  printf ("Relay Off\n");
	}
	digitalWrite(RELAYPIN, gpioRelay);
      }

    int mod=(int)sec_time % 1;
    if(mod == 0)
      {//1秒に一回LCD出力

	//ロータリースイッチデータ取得(割り込み)
	lcdSTATUS=abs(positionRotary) % 4;
	//printf("Now Position %d\n",positionRotary);

	readData();

	if(lcdSTATUS==0){
	  sprintf(lcdStr,"TEMP :%4f C",(double)calibration_T(temp_raw)/100.0);
	}else if(lcdSTATUS==1){
	  sprintf(lcdStr,"PRES :%4f hPa",(double)calibration_P(pres_raw)/100.0);
	}else if(lcdSTATUS==2){
	  sprintf(lcdStr,"HUMI :%4f %",(double)calibration_H(hum_raw)/1024.0);
	}else{
	  sprintf(lcdStr,"TEMPIN :%4f C",getTemperature(fd3425));
	}
	lcdPosition(fd,0,1);
	lcdPuts(fd,lcdStr);
      }
    
    //ロータリースイッチデータ取得(ポーリング)
    //int nowPosition;
    //nowPosition=getPosition();
    //lcdSTATUS=abs(nowPosition) % 3;
    //printf ("lcdStatus : %d Nowposition : %d \n",lcdSTATUS,abs(nowPosition));
      
    rgbLedLoop(&loopData);
  }
  return 0;
}
