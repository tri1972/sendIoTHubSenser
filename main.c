#include  "sendIoTHubSenser.h"
#include  "rgbLed.h"


int main(void)
{
  char* connectionString;
  connectionString = getConnectString();
  
  printf("Start Sample Program!!\n");
  //時間構造体を初期化
  time_t current_time,before_time;//時間計測用構造体
  time(&before_time);
  double sec_time;//時間（秒）

  //WiringPI初期化
  if (wiringPiSetup() == -1)
    return 1;
  printf("Wiringpi setupOK\n");

  //LED初期化
  pinMode(GREENPIN, OUTPUT);
  pinMode(REDPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);
  pinMode(RELAYPIN, OUTPUT);
  digitalWrite(BLUEPIN, 0);
  digitalWrite(GREENPIN, 0);
  digitalWrite(REDPIN, 0);
  digitalWrite(RELAYPIN, 0);

  int ans;
  ans=wiringPiSPISetup (channel, 1000000);
  if (ans < 0)
    {
      fprintf (stderr, "SPI Setup failed: %s\n", strerror (errno));
    }else
    {
      printf("Spi setup OK!!!\n");
    }

  char str[100];

  printf("Temperature = %f(C) \n", getTemperature(0));
  //IOTHub初期化
  IOTHUB_CLIENT_HANDLE iotHubClientHandle= IoTHubClient_CreateFromConnectionString(*connectionString, MQTT_Protocol);
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

  struct dataRgbLedLoop loopData=initDataRgbLoop();

  while(1){

    time(&current_time);
    sec_time=difftime(current_time,before_time);
    if(sec_time>60.0)//60秒に一回IOTへ送信
      {
	before_time=current_time;
	callback_remote_monitoring_run(&iotHubClientHandle,getTemperature(0));
      }
    rgbLedLoop(&loopData);
  }
  return 0;
}
