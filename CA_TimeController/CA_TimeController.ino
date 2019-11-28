/*
 * this CurrentTimeInput is for nodeMCU.
 * 
 * -RTC for nodemcu source from 
 * https://github.com/jumejume1/NodeMCU_ESP8266/blob/master/READ_TIME_FROM_INTERNET/READ_TIME_FROM_INTERNET.ino
 * Thanks to youtuber "Maker Tutor"
*/

#include <ESP8266WiFi.h>

#include <time.h>

//const char* ssid = "WooChanPhone";
//const char* password = "sgn01043";

const char* ssid = "BEAKPICK";
const char* password = "skjw3649#";

typedef struct
{
  int hour;
  int minute;  
}time_type;

int classTimeIdentifierPin = 13;
int masterDecisionPin = 14;

int timeInterval = 15; //minute
int classTime = 75; //minute

time_type firstClassTimeStart = {9,0};

int activateTime = 3; //minute duration before class start. activateTime <= timeInterval
int tempTimelast = 0; //to check how long the time past->going to be removed

time_type timeSet[21];  //to set the schedule by 

int timezone = 9 * 3600;  //KRT : +9 hours interval from UMT
int dst = 0;

struct tm* p_tm;

/*********************************scheduleTimeLine Start**************************************/ 
/*even number of timeSet index is classStartTime, odd number of timeSet index is classEndTime*/
/***********************************proved by BEAKPICK****************************************/
void scheduleTimeLine()
{
  if(classTime == 0)
  {
    Serial.println("Please check the classtime.");
    return;
  }
  if(activateTime > timeInterval)
  {
    Serial.println("activateTime exceeds timeInterval");
    return;
  }
  
  timeSet[0].hour = firstClassTimeStart.hour;
  timeSet[0].minute = firstClassTimeStart.minute;
  for(int i = 1; i < 21; i+=2)
  {
    timeSet[i].hour = timeSet[i-1].hour + (int)(classTime/60);
    timeSet[i].minute = timeSet[i-1].minute + (int)(classTime%60);
    
    timeSet[i+1].minute = (timeSet[i].minute + (int)(timeInterval%60)) % 60;
    timeSet[i+1].hour = timeSet[i].hour + (int)(timeSet[i].minute + (int)timeInterval%60)/60;
  }
  
  Serial.println();
  for(int i = 0; i < 20; i++)
  {
    Serial.print(timeSet[i].hour);
    Serial.print(" : ");
    Serial.println(timeSet[i].minute);
  }
}
/**********************************scheduleTimeLine End***************************************/

/***********************************syncTimeSet Start*****************************************/ 
/********************declare which timeSet nodeMCU has to wait for****************************/
/***********************************proved by BEAKPICK****************************************/
int syncTimeSet()
{
  for(int i = 0; i < 21; i++)
  {
    if(timeSet[i].hour > p_tm->tm_hour) return i;
    else if(timeSet[i].hour == p_tm->tm_hour && timeSet[i].minute > p_tm->tm_min) return i;
  }
  return 0;
}
/**************************************syncTime End*******************************************/

void setup() 
{
  
  pinMode(classTimeIdentifierPin,OUTPUT);

  digitalWrite(classTimeIdentifierPin,LOW);



  Serial.begin(115200);

  scheduleTimeLine();

  Serial.println();

  Serial.print("Wifi connecting to ");

  Serial.println( ssid );



  WiFi.begin(ssid,password);



  Serial.println();

  

  Serial.print("Connecting");



  while( WiFi.status() != WL_CONNECTED ){

      delay(500);

      Serial.print(".");        

  }


  //digitalWrite( classTimeIdentifierPin , HIGH); //gonna use later

  Serial.println();



  Serial.println("Wifi Connected Success!");

  Serial.print("NodeMCU IP Address : ");

  Serial.println(WiFi.localIP() );



  configTime(timezone, dst, "pool.ntp.org","time.nist.gov");

  Serial.println("\nWaiting for Internet time");



  while(!time(nullptr)){

     Serial.print("*");

     delay(1000);

  }

  Serial.println("\nTime response....OK");
  
}



void loop() {

  

  time_t now = time(nullptr);

  /*struct tm**/ p_tm = localtime(&now);

  Serial.print(p_tm->tm_mday);

  Serial.print("/");

  Serial.print(p_tm->tm_mon + 1);

  Serial.print("/");

  Serial.print(p_tm->tm_year + 1900);

  

  Serial.print(" ");

  

  Serial.print(p_tm->tm_hour);

  Serial.print(":");

  Serial.print(p_tm->tm_min);

  Serial.print(":");

  Serial.println(p_tm->tm_sec);

  delay(1000);

  Serial.print("TimeSet = ");
  Serial.println(syncTimeSet());

}
