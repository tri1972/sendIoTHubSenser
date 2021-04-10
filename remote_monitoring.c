#include "iothubtransportmqtt.h"
#include "schemalib.h"
#include "iothub_client.h"
#include "serializer_devicetwin.h"
#include "schemaserializer.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/platform.h"
#include "parson.h"
#include <syslog.h>
#include "remote_monitoring.h"

char* deviceId;
char* connectionString;
char errStr[256];

//static const char* deviceId = "[Device Id]";
//static const char* connectionString = "HostName=[IoTHub Name].azure-devices.net;DeviceId=[Device Id];SharedAccessKey=[Device Key]";

// Define the Model
BEGIN_NAMESPACE(Contoso);

/* Reported properties */
DECLARE_STRUCT(SystemProperties,
	       ascii_char_ptr, Manufacturer,
	       ascii_char_ptr, FirmwareVersion,
	       ascii_char_ptr, InstalledRAM,
	       ascii_char_ptr, ModelNumber,
	       ascii_char_ptr, Platform,
	       ascii_char_ptr, Processor,
	       ascii_char_ptr, SerialNumber
	       );

DECLARE_STRUCT(LocationProperties,
	       double, Latitude,
	       double, Longitude
	       );

DECLARE_STRUCT(ReportedDeviceProperties,
	       ascii_char_ptr, DeviceState,
	       LocationProperties, Location
	       );

DECLARE_MODEL(ConfigProperties,
	      WITH_REPORTED_PROPERTY(double, TemperatureMeanValue),
	      WITH_REPORTED_PROPERTY(uint8_t, TelemetryInterval)
	      );

/* Part of DeviceInfo */
DECLARE_STRUCT(DeviceProperties,
	       ascii_char_ptr, DeviceID,
	       _Bool, HubEnabledState
	       );

DECLARE_DEVICETWIN_MODEL(Thermostat,
			 /* Telemetry (temperature, external temperature and humidity) */
			 WITH_DATA(double, Temperature),
			 WITH_DATA(double, ExternalTemperature),
			 WITH_DATA(double, Humidity),
			 WITH_DATA(double, Pressure),
			 WITH_DATA(ascii_char_ptr, DeviceId),

			 /* DeviceInfo */
			 WITH_DATA(ascii_char_ptr, ObjectType),
			 WITH_DATA(_Bool, IsSimulatedDevice),
			 WITH_DATA(ascii_char_ptr, Version),
			 WITH_DATA(DeviceProperties, DeviceProperties),

			 /* Device twin properties */
			 WITH_REPORTED_PROPERTY(ReportedDeviceProperties, Device),
			 WITH_REPORTED_PROPERTY(ConfigProperties, Config),
			 WITH_REPORTED_PROPERTY(SystemProperties, System),

			 WITH_DESIRED_PROPERTY(double, TemperatureMeanValue, onDesiredTemperatureMeanValue),
			 WITH_DESIRED_PROPERTY(uint8_t, TelemetryInterval, onDesiredTelemetryInterval),

			 /* Direct methods implemented by the device */
			 WITH_METHOD(Reboot),
			 WITH_METHOD(InitiateFirmwareUpdate, ascii_char_ptr, FwPackageURI),

			 

			 /* Register direct methods with solution portal */
			 WITH_REPORTED_PROPERTY(ascii_char_ptr_no_quotes, SupportedMethods)
			 );

END_NAMESPACE(Contoso);
void onDesiredTemperatureMeanValue(void* argument)
{
  /* By convention 'argument' is of the type of the MODEL */
  Thermostat* thermostat = argument;
  printf("Received a new desired_TemperatureMeanValue = %f\r\n", thermostat->TemperatureMeanValue);

}

void onDesiredTelemetryInterval(void* argument)
{
  /* By convention 'argument' is of the type of the MODEL */
  Thermostat* thermostat = argument;
  printf("Received a new desired_TelemetryInterval = %d\r\n", thermostat->TelemetryInterval);
}

/* Handlers for direct methods */
METHODRETURN_HANDLE Reboot(Thermostat* thermostat)
{
  (void)(thermostat);

  METHODRETURN_HANDLE result = MethodReturn_Create(201, "\"Rebooting\"");
  printf("Received reboot request\r\n");
  return result;
}

METHODRETURN_HANDLE InitiateFirmwareUpdate(Thermostat* thermostat, ascii_char_ptr FwPackageURI)
{
  (void)(thermostat);

  METHODRETURN_HANDLE result = MethodReturn_Create(201, "\"Initiating Firmware Update\"");
  printf("Recieved firmware update request. Use package at: %s\r\n", FwPackageURI);
  return result;
}
/* Send data to IoT Hub */
static void sendMessage(IOTHUB_CLIENT_HANDLE iotHubClientHandle, const unsigned char* buffer, size_t size)
{
  IOTHUB_MESSAGE_HANDLE messageHandle = IoTHubMessage_CreateFromByteArray(buffer, size);
  if (messageHandle == NULL)
    {
      syslog(LOG_ERR,"unable to create a new IoTHubMessage");
    }
  else
    {
      if (IoTHubClient_SendEventAsync(iotHubClientHandle, messageHandle, NULL, NULL) != IOTHUB_CLIENT_OK)
	{
	  syslog(LOG_ERR,"failed to hand over the message to IoTHubClient");
	}
      else
	{
	  sprintf(errStr, "IoTHubClient accepted the message for delivery :%s", buffer);      
	  syslog(LOG_NOTICE,errStr);
	}

      IoTHubMessage_Destroy(messageHandle);
    }
  free((void*)buffer);
}
/* Callback after sending reported properties */
void deviceTwinCallback(int status_code, void* userContextCallback)
{
  (void)(userContextCallback);
  printf("IoTHub: reported properties delivered with status_code = %u\n", status_code);
}

Thermostat* CreateIoTHubDeviceTwin(IOTHUB_CLIENT_HANDLE iotHubClientHandle )
{
    if (iotHubClientHandle == NULL)
    {
      syslog(LOG_ERR,"Failure in iotHubClientHandle is NULL");
    }else{
    }
  Thermostat* thermostat = IoTHubDeviceTwin_CreateThermostat(iotHubClientHandle);
  if (thermostat == NULL)
    {
      syslog(LOG_ERR,"Failure in IoTHubDeviceTwin_CreateThermostat");
    }
  else
    {
      /* Set values for reported properties */
      thermostat->Config.TemperatureMeanValue = 55.5;
      thermostat->Config.TelemetryInterval = 3;
      thermostat->Device.DeviceState = "normal";
      thermostat->Device.Location.Latitude = 47.642877;
      thermostat->Device.Location.Longitude = -122.125497;
      thermostat->System.Manufacturer = "Contoso Inc.";
      thermostat->System.FirmwareVersion = "2.22";
      thermostat->System.InstalledRAM = "8 MB";
      thermostat->System.ModelNumber = "DB-14";
      thermostat->System.Platform = "Plat 9.75";
      thermostat->System.Processor = "i3-7";
      thermostat->System.SerialNumber = "SER21";
      /* Specify the signatures of the supported direct methods */
      thermostat->SupportedMethods = "{\"Reboot\": \"Reboot the device\", \"InitiateFirmwareUpdate--FwPackageURI-string\": \"Updates device Firmware. Use parameter FwPackageURI to specifiy the URI of the firmware file\"}";

      /* Send reported properties to IoT Hub */
      if (IoTHubDeviceTwin_SendReportedStateThermostat(thermostat, deviceTwinCallback, NULL) != IOTHUB_CLIENT_OK)
	{
	  syslog(LOG_ERR,"Failed sending serialized reported state");
	}
      else
	{
	  //printf("Send DeviceInfo object to IoT Hub at startup\n");

	  thermostat->ObjectType = "DeviceInfo";
	  thermostat->IsSimulatedDevice = 0;
	  thermostat->Version = "1.0";
	  thermostat->DeviceProperties.HubEnabledState = 1;
	  thermostat->DeviceProperties.DeviceID = (char*)deviceId;

	}
    }
  return thermostat;
}


void callback_remote_monitoring_run(IOTHUB_CLIENT_HANDLE *iotHubClientHandle,double temperature,double humidity,double externalTemperature,double pressure )
{
  if  (iotHubClientHandle== NULL)
  {
      syslog(LOG_ERR,"Failure in iotHubClientHandle_NULL");
  }
  else
  {
    //printf("iotHubClientHandle OK!\n");
  }

  Thermostat* thermostat =CreateIoTHubDeviceTwin(*iotHubClientHandle);

  //Thermostat* thermostat = IoTHubDeviceTwin_CreateThermostat(iotHubClientHandle);
  //printf("IoTHubDeviceTwin_CreateThermostat OK!!\n");

  if (thermostat == NULL)
    {
      syslog(LOG_ERR,"Failure in IoTHubDeviceTwin_CreateThermostat");
    }
  else
    {
      /* Send telemetry */
      thermostat->Temperature = temperature;
      thermostat->ExternalTemperature = externalTemperature;
      thermostat->Humidity = humidity;
      thermostat->Pressure = pressure;
      thermostat->DeviceId = (char*)deviceId;
    }
  unsigned char*buffer;

  size_t bufferSize;

  (void)printf("Sending sensor value Temperature = %f, Humidity = %f\n", thermostat->Temperature, thermostat->Humidity);

  if (SERIALIZE(&buffer, &bufferSize, thermostat->DeviceId, thermostat->Temperature, thermostat->Humidity, thermostat->ExternalTemperature,thermostat->Pressure) != CODEFIRST_OK)
    {
      syslog(LOG_ERR,"Failed to make JSON msg sending sensor value:");
    }
  else
    {
      sendMessage(*iotHubClientHandle, buffer, bufferSize);
    }

  ThreadAPI_Sleep(1000);
    
}


void remote_monitoring_destracutor(IOTHUB_CLIENT_HANDLE iotHubClientHandle )
{
  //IoTHubDeviceTwin_DestroyThermostat(thermostat);
  IoTHubClient_Destroy(iotHubClientHandle);
  serializer_deinit();
  platform_deinit(); 
}

char *setvalue(char *p, char *field, int size)
{
  if (*p == '"')
    {
      /* ここでのインクリメントは
       * 最後のインクリメントではポインタが＋２になる(評価された場合のみ)
       * *p != '"'が0になった場合のみ *(++p) == '"'が実行される
       * これはダブルクォート連続の時の対策
       */
      while (*(++p) && *p != '\n' && (*p != '"' || *(++p) == '"'))
	{
	  if (--size > 0)
	    {
	      *(field++) = *p;
	    }
	}
    }
  // ここの部分は""で囲まれていない部分が通るようになっている
  for ( ; *p && *p != ',' && *p != '\n'; p++)
    {
      if (--size > 0)
	{
	  *(field++) = *p;
	}
    }
  *field = '\0';
  return *p ? (p + 1) : p;
}

void parseDoubleQuote(char *buf,char *output){
    int i=0, counter=0;
    bool flagInQuote=false;
    char field[256];
  while(buf[i]!='\0'){
	if(buf[i]=='\"' && flagInQuote==false){
	  flagInQuote=true;
	  i++;
	}
	if(buf[i]=='\"' && flagInQuote==true){
	  field[counter]='\0';
	  break;
	}
	if(flagInQuote==true){
	  field[counter]=buf[i];
	  counter++;
	}	
	i++;
      }
  strcpy( output,field);
  //printf("fieldが%sと解析されました\n",field);

}

void getConnectString(char *deviceIdtmp,char *connectStringtmp){

  FILE *fp;
  char buf[256]; // 256にしているのは手抜き実装
  char *field,*field2;
  char *p;
  char *ret;
  /*  ここで、ファイルポインタを取得する */
  if ((fp = fopen(PATH_CONFIGFILE, "r")) == NULL) {
    syslog(LOG_NOTICE,"Inifile open error!!");
    exit(EXIT_FAILURE);/* エラーの場合は通常、異常終了する */
  }

  /* (4)ファイルの読み（書き）*/
  while (fgets(buf, 256, fp) != NULL) {
    char s1[] = "deviceId";
    /* ここではfgets()により１行単位で読み出し */
    if ((ret = strstr(buf, s1)) != NULL ) {
       parseDoubleQuote(buf,deviceIdtmp);
       deviceId  =deviceIdtmp;
       break;
    } else {
      sprintf(errStr, "%sはありませんでした", s1);      
      syslog(LOG_ERR,errStr);
    }
  }
  fseek(fp, 0L, SEEK_SET);
  while (fgets(buf, 256, fp) != NULL) {
    char s2[] = "connectionString";
    /* ここではfgets()により１行単位で読み出し */
    if ((ret = strstr(buf, s2)) != NULL ) {
      parseDoubleQuote(buf,connectStringtmp);
      connectionString=connectStringtmp;
      break;
    }
    else
    {
      sprintf(errStr,"%sはありませんでした．\n", s2);      
      syslog(LOG_ERR,errStr);
    }
  }
  sprintf(errStr,"connectionString %s が設定されました．\n",connectStringtmp);      
  syslog(LOG_NOTICE,errStr);
  fclose(fp);/* (5)ファイルのクローズ */
}


void remote_monitoring_init(IOTHUB_CLIENT_HANDLE *iotHubClientHandle   )
{
  
  if (platform_init() != 0)
    {
      syslog(LOG_ERR,"Failed to initialize the platform.");
    }
  else
    {
      if (SERIALIZER_REGISTER_NAMESPACE(Contoso) == NULL)
	{
	    syslog(LOG_ERR,"Unable to SERIALIZER_REGISTER_NAMESPACE");
	}
      else
	{
	  //IOTHUB_CLIENT_HANDLE iotHubClientHandleInstance= IoTHubClient_CreateFromConnectionString(connectionString, MQTT_Protocol);
	  //iotHubClientHandle = &iotHubClientHandleInstance;
	  	  
	  if (*iotHubClientHandle == NULL)
	    {
	      syslog(LOG_ERR,"Failure in IoTHubClient_CreateFromConnectionString");
	    }
	  else
	    {
#ifdef MBED_BUILD_TIMESTAMP
	      // For mbed add the certificate information
	      if (IoTHubClient_SetOption(*iotHubClientHandle, "TrustedCerts", certificates) != IOTHUB_CLIENT_OK)
		{
		  syslog(LOG_ERR,"Failed to set option \"TrustedCerts\"");
		}
#endif // MBED_BUILD_TIMESTAMP
	      Thermostat *thermostat;
	      thermostat=CreateIoTHubDeviceTwin(*iotHubClientHandle);
	      unsigned char* buffer;
	      size_t bufferSize;

	      if (SERIALIZE(&buffer, &bufferSize, thermostat->ObjectType, thermostat->Version, thermostat->IsSimulatedDevice, thermostat->DeviceProperties) != CODEFIRST_OK)
		{
  		  syslog(LOG_ERR,"Failed serializing DeviceInfo");
		}	
	      else
		{
		  sendMessage(*iotHubClientHandle, buffer, bufferSize);
		}
	    }
	}
    }
}










void remote_monitoring_run(void)
{
  if (platform_init() != 0)
    {
      printf("Failed to initialize the platform.\n");
    }
  else
    {
      if (SERIALIZER_REGISTER_NAMESPACE(Contoso) == NULL)
	{
	  printf("Unable to SERIALIZER_REGISTER_NAMESPACE\n");
	}
      else
	{
	  IOTHUB_CLIENT_HANDLE iotHubClientHandle = IoTHubClient_CreateFromConnectionString(connectionString, MQTT_Protocol);
	  if (iotHubClientHandle == NULL)
	    {
	      printf("Failure in IoTHubClient_CreateFromConnectionString\n");
	    }
	  else
	    {
#ifdef MBED_BUILD_TIMESTAMP
	      // For mbed add the certificate information
	      if (IoTHubClient_SetOption(iotHubClientHandle, "TrustedCerts", certificates) != IOTHUB_CLIENT_OK)
		{
		  printf("Failed to set option \"TrustedCerts\"\n");
		}
#endif // MBED_BUILD_TIMESTAMP
	      Thermostat* thermostat = IoTHubDeviceTwin_CreateThermostat(iotHubClientHandle);
	      if (thermostat == NULL)
		{
		  printf("Failure in IoTHubDeviceTwin_CreateThermostat\n");
		}
	      else
		{
		  /* Set values for reported properties */
		  thermostat->Config.TemperatureMeanValue = 55.5;
		  thermostat->Config.TelemetryInterval = 3;
		  thermostat->Device.DeviceState = "normal";
		  thermostat->Device.Location.Latitude = 47.642877;
		  thermostat->Device.Location.Longitude = -122.125497;
		  thermostat->System.Manufacturer = "Contoso Inc.";
		  thermostat->System.FirmwareVersion = "2.22";
		  thermostat->System.InstalledRAM = "8 MB";
		  thermostat->System.ModelNumber = "DB-14";
		  thermostat->System.Platform = "Plat 9.75";
		  thermostat->System.Processor = "i3-7";
		  thermostat->System.SerialNumber = "SER21";
		  /* Specify the signatures of the supported direct methods */
		  thermostat->SupportedMethods = "{\"Reboot\": \"Reboot the device\", \"InitiateFirmwareUpdate--FwPackageURI-string\": \"Updates device Firmware. Use parameter FwPackageURI to specifiy the URI of the firmware file\"}";

		  /* Send reported properties to IoT Hub */
		  if (IoTHubDeviceTwin_SendReportedStateThermostat(thermostat, deviceTwinCallback, NULL) != IOTHUB_CLIENT_OK)
		    {
		      printf("Failed sending serialized reported state\n");
		    }
		  else
		    {
		      printf("Send DeviceInfo object to IoT Hub at startup\n");

		      thermostat->ObjectType = "DeviceInfo";
		      thermostat->IsSimulatedDevice = 0;
		      thermostat->Version = "1.0";
		      thermostat->DeviceProperties.HubEnabledState = 1;
		      thermostat->DeviceProperties.DeviceID = (char*)deviceId;

		      unsigned char* buffer;
		      size_t bufferSize;

		      if (SERIALIZE(&buffer, &bufferSize, thermostat->ObjectType, thermostat->Version, thermostat->IsSimulatedDevice, thermostat->DeviceProperties) != CODEFIRST_OK)
			{
			  (void)printf("Failed serializing DeviceInfo\n");
			}
		      else
			{
			  sendMessage(iotHubClientHandle, buffer, bufferSize);
			}
		      

		      /* Send telemetry */
		      thermostat->Temperature = 50;
		      thermostat->ExternalTemperature = 55;
		      thermostat->Humidity = 50;
		      thermostat->DeviceId = (char*)deviceId;

		      while (1)
			{
			  unsigned char*buffer;

			  size_t bufferSize;

			  (void)printf("Sending sensor value Temperature = %f, Humidity = %f\n", thermostat->Temperature, thermostat->Humidity);

			  if (SERIALIZE(&buffer, &bufferSize, thermostat->DeviceId, thermostat->Temperature, thermostat->Humidity, thermostat->ExternalTemperature) != CODEFIRST_OK)
			    {
			      (void)printf("Failed sending sensor value\r\n");
			    }
			  else
			    {
			      sendMessage(iotHubClientHandle, buffer, bufferSize);
			    }

			  ThreadAPI_Sleep(1000);
			}

		      IoTHubDeviceTwin_DestroyThermostat(thermostat);
		    }
		}
	      IoTHubClient_Destroy(iotHubClientHandle);
	    }
	  serializer_deinit();
	}
    }
  platform_deinit();
}
			  
