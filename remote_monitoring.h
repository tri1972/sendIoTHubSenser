#ifndef REMOTE_MONITORING
#define REMOTE_MONITORING

#include "iothub_client.h"
#include <time.h>

#include <wiringPi.h>
#include <wiringPiSPI.h>

//#include "../lib/lib_mcp3002.h"
//#include "../lib/lib_mpl115a2.h"
//#include "../lib/lib_ST7032i.h"
//#include "../lib/lib_capture.h"

#define RELAYPIN 27
#define PATH_CONFIGFILE "/etc/sendIoTHubSenser.conf"

void remote_monitoring_run(void);
void remote_monitoring_init(IOTHUB_CLIENT_HANDLE *iotHubClientHandle);
//void callback_remote_monitoring_run(IOTHUB_CLIENT_HANDLE *iotHubClientHandle,double temperature);
void callback_remote_monitoring_run(IOTHUB_CLIENT_HANDLE *iotHubClientHandle,double temperature,double humidity,double externalTemperature,double pressure );
void getConnectString(char *deviceIdtmp,char *connectStringtmp);
#endif
