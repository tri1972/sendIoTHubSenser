#include "sendIoTHubSenser.h"
#include "rgbLed.h"
#include "network.h"
#include "lib/libRotaryEncoder.h"
#include "lib/BME280.h"
#include "lib/lib_mcp3425.h"
#include "lib/lib_buttonWithLamp.h"
#include "daemonize/daemonize.h"
#include <lcd.h>
#include <math.h>
#include <stdbool.h>
#include <syslog.h>
#include <unistd.h>
#include <stdio.h>

extern int positionRotary;//割り込みを使ったgetposition()での位置情報出力用変数
extern void getPositionISR(void);//割り込みを使ったロータリースイッチ位置検出


static volatile int sigterm = 0;
static void handle_sigterm(int sig) { sigterm = 1; }

int main(void)
{
  char* deviceId;
  char* connectionString;
  char tmp[256];
  char tmp2[256];
  /* syslog オープン*/
  openlog("sendIoTHub", LOG_PID, LOG_DAEMON);
  //daemon化初期化
  
  if(!daemonize("/var/run/sendIoTHub.pid",
		"sendIoTHub", LOG_PID, LOG_DAEMON))
    {
      syslog(LOG_ERR,"failed to daemonize.");
      //fprintf(stderr, "failed to daemonize.\n");
      return 2; // fail to start daemon
    }
  
  //IPアドレス取得
  char* strIPAddress=getIpAddress();
  char strsyslog[255];
  sprintf(strsyslog,"sendIoTHub started!!!!!!!!!!!! IPAddress : %s",strIPAddress);
  syslog(LOG_NOTICE,strsyslog);
  
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
  double sec_time,sec_time_before;//時間（秒）
  bool isTimeChange=true;
  struct tm *time_st;
  
  //WiringPI初期化
  if (wiringPiSetup() == -1){
    syslog(LOG_ERR, "wiringPiSetup Aborted!");
    return 1;
  }
  printf("Wiringpi setupOK\n");

  //MPC3425初期化
  int fd3425;
  if ((fd3425 = wiringPiI2CSetup(MPL3425_ID)) == -1)
    {
      syslog(LOG_NOTICE, "wiringPiI2CSetup with MCP3425 Aborted!");
      return 1 ;
    }
  syslog(LOG_NOTICE, "wiringPiI2CSetup with MCP3425 OK!");
  
  //BME280初期化
  if(init_dev()==1){
    syslog(LOG_ERR, "wiringPiI2CSetup with BME280 Aborted!");
    return 1;
  }
  syslog(LOG_NOTICE, "wiringPiI2CSetup with BME280 OK!");
  
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
  char lcdStr1L[30];
  char lcdStr2L[30];
  fd = lcdInit(2,16,4,0,3,4,5,6,7,0,0,0,0);
  lcdClear(fd);
  lcdPuts(fd,"LCD Start!");
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
  syslog(LOG_NOTICE, "RotaryEncoder Init OK!");

  //ボタン初期化
  initButton();
  //ボタンoff→on時のみ割り込む
  wiringPiISR(BOARD_SW5,INT_EDGE_FALLING,getOnOffSwitch5);
  
  
  //IOTHub初期化
  getConnectString(deviceId,connectionString);
  IOTHUB_CLIENT_HANDLE iotHubClientHandle= IoTHubClient_CreateFromConnectionString(connectionString, MQTT_Protocol);
  remote_monitoring_init(&iotHubClientHandle);
  syslog(LOG_NOTICE, "IoThub Init OK!");

  if  (iotHubClientHandle== NULL)
    {
      syslog(LOG_NOTICE, "Failure in iotHubClientHandle_NULL");
    }
  else
    {
      syslog(LOG_NOTICE, "iotHubClientHandle OK!");
    }
  
  
  struct dataRgbLedLoop loopData=initDataRgbLoop();

  bool gpioRelay=true;

  //daemonループの開始
  syslog(LOG_NOTICE, "sendIoTHubSenserLoop started.");
  signal(SIGTERM, handle_sigterm);  
  while(!sigterm){
    time(&current_time);
    sec_time=difftime(current_time,before_time);
    if(sec_time!=sec_time_before){
      isTimeChange=true;
      sec_time_before=sec_time;
    }else{
      isTimeChange=false;
    }
    
    if(((int)sec_time % 60) == 0 && isTimeChange)//60秒に一回IOTへ送信
      {
	double nowTemp=getTemperature(fd3425);
	callback_remote_monitoring_run(&iotHubClientHandle,nowTemp,(double)calibration_H(hum_raw)/1024.0,(double)calibration_T(temp_raw)/100.0,(double)calibration_P(pres_raw)/100.0);	
	if(TemperatureLimit<nowTemp){
	  gpioRelay=true;
	}else{
	  gpioRelay=false;
	}
	digitalWrite(RELAYPIN, gpioRelay);
      }

    int mod=(int)sec_time % 1;
    if(((int)sec_time % 1) == 0 && isTimeChange)
      {//1秒に一回LCD出力

	//ロータリースイッチデータ取得(割り込み)
	lcdSTATUS=abs(positionRotary) % 6;
	readData();

	if(lcdSTATUS==0){
	  sprintf(lcdStr1L,"Temperature     ");
	  sprintf(lcdStr2L,"%-3.6f C     ",(double)calibration_T(temp_raw)/100.0);
	}else if(lcdSTATUS==1){
	  sprintf(lcdStr1L,"Pressure        ");
	  sprintf(lcdStr2L,"%-4.6f hPa ",(double)calibration_P(pres_raw)/100.0);
	}else if(lcdSTATUS==2){
	  sprintf(lcdStr1L,"Humidity        ");
	  sprintf(lcdStr2L,"%-2.6f %%     ",(double)calibration_H(hum_raw)/1024.0);
	}else if(lcdSTATUS==3){
	  sprintf(lcdStr1L,"ThermistaTemp   ");
	  sprintf(lcdStr2L,"%-3.6f C     ",getTemperature(fd3425));
	}else if(lcdSTATUS==4){
	  sprintf(lcdStr1L,"IPAddress:wlan0 ");
	  sprintf(lcdStr2L,"%s ",strIPAddress);
	}else{
	  time_st = localtime(&current_time);
	  sprintf(lcdStr1L, "%d-%d-%d     ",
		  time_st->tm_year + 1900,
		  time_st->tm_mon + 1,
		  time_st->tm_mday);
	  sprintf(lcdStr2L, "%02d:%02d:%02d       ",
		  time_st->tm_hour,
		  time_st->tm_min,
		  time_st->tm_sec);
	}
      }
    lcdPosition(fd,0,0);
    lcdPuts(fd,lcdStr1L);
    lcdPosition(fd,0,1);
    lcdPuts(fd,lcdStr2L);
    
    //ロータリースイッチデータ取得(ポーリング)
    //int nowPosition;
    //nowPosition=getPosition();
    //lcdSTATUS=abs(nowPosition) % 3;
    //printf ("lcdStatus : %d Nowposition : %d \n",lcdSTATUS,abs(nowPosition));
    rgbLedLoop(&loopData,isOnSw5);
  }
  syslog(LOG_NOTICE, "sendIoTHubSenser stopped.\n");
  return 0;
}
