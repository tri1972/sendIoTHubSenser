#include  "rgbLed.h"


struct dataRgbLedLoop initDataRgbLoop(void){

  struct dataRgbLedLoop initdat={ true,true,true,
				  0,0,0,0,
				  0x01,0x00,0x00
  };

  return initdat;

}

//LED色を決定、rgbそれぞれにOn時間を指定します
void rgbPoling(int r,int g,int b){
  //スリープ時間を指定
  struct timespec ts;
  ts.tv_sec=0;//1sを指定
  ts.tv_nsec=1000000;//0nsを指定

  int i,j;
  for(j=0;j<4;j++){
    for(i=0;i<1024;i++){
      if(i >r){
	digitalWrite(REDPIN, false);
      }else{
	digitalWrite(REDPIN, true);
      }
      if(i >g){
	digitalWrite(GREENPIN, false);
      }else{
	digitalWrite(GREENPIN, true);
      }
      if(i >b){
	digitalWrite(BLUEPIN, false);
      }else{
	digitalWrite(BLUEPIN, true);
      }
    }
  }
  nanosleep(&ts,NULL);//1msスリープ

}


void rgbLedLoop(struct dataRgbLedLoop *loopdat){
  /*
  useconds_t tick = *( int * )ptr;
  bool flagR=true,flagG=true,flagB=true;
  int counter=0,rlux=0,glux=0,blux=0;
  int currentFlag=0x01,beforeFlag=0x00,calcData=0x00;
  //スリープ時間を指定
  struct timespec ts;
  ts.tv_sec=0;//1sを指定
  ts.tv_nsec=1000000;//0nsを指定
  */
    loopdat->calcData=(loopdat->currentFlag & 0x01)-(loopdat->beforeFlag & 0x01);
    if(loopdat->calcData<0){
      loopdat->rlux--;
    }else if(loopdat->calcData==0){
      ;
    }else{
      loopdat->rlux++;
    }
    loopdat->calcData=(loopdat->currentFlag & 0x02)-(loopdat->beforeFlag & 0x02);
    if(loopdat->calcData<0){
      loopdat->glux--;
    }else if(loopdat->calcData==0){
      ;
    }else{
      loopdat->glux++;
    }
    loopdat->calcData=(loopdat->currentFlag & 0x04)-(loopdat->beforeFlag & 0x04);
    if(loopdat->calcData<0){
      loopdat->blux--;
    }else if(loopdat->calcData==0){
      ;
    }else{
      loopdat->blux++;
    }
    //printf("calcData=%d,currentData=%d,beforeData=%d,rlux=%d \r",calcData,currentFlag & 0x01,beforeFlag & 0x01,glux);
    //printf("R=%d,G=%d,B=%d \r",rlux,glux,blux);
    if(loopdat->counter>1024){
      loopdat->counter=0;
      loopdat->beforeFlag=loopdat->currentFlag;
      loopdat->currentFlag++;
    }else{
      loopdat->counter++;
    }

    //printf("%d\n",lightLux);
    //rgbPoling(0,glux,0);
    rgbPoling(loopdat->rlux,loopdat->glux,loopdat->blux);
  
}
