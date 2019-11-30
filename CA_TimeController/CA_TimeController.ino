/*
 * this CurrentTimeInput is for nodeMCU.
 * 
 * -RTC for nodemcu source from 
 * https://github.com/jumejume1/NodeMCU_ESP8266/blob/master/READ_TIME_FROM_INTERNET/READ_TIME_FROM_INTERNET.ino
 * Thanks to youtuber "Maker Tutor"
*/
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <time.h>

SoftwareSerial mySerial; //(Rx, Tx)

//const char* ssid = "WooChanPhone";
//const char* password = "sgn01043";

const char* ssid = "BEAKPICK";
const char* password = "skjw3649#";

typedef struct
{
  int hour;
  int minute;
  //int second;
}time_type;

int timeBellPin = 4; //a pin for speaker connected with arduino to ring the bell in each timeSet
int masterPin = 5; //a pin for alarm system

int timeInterval = 2/*15*/; //(minute) time rest between class
int classTime = 2/*75*/; //(minute) time of class

time_type firstClassTimeStart = {3,0};

int activateTime = 1/*3*/; //(minute) duration before class start. activateTime <= timeInterval
int compareMinute = 0;  //(minute) to check 1 minute past
int compareTimeSet = -1; //(Index of timeSet) save current wait to compare the change

time_type timeSet[22];

int timezone = 9 * 3600;  //KRT : +9 hours interval from UMT
int dst = 0;

struct tm* p_tm;

/***********************************scheduleTimeLine Start****************************************/ 
/*even numbers of timeSet index are classStartTime, odd numbers of timeSet index are classEndTime*/
/*************************************proved by BEAKPICK******************************************/
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
  for(int i = 1; i < 22; i+=2)
  {
    timeSet[i].hour = timeSet[i-1].hour + (int)(classTime/60);
    timeSet[i].minute = timeSet[i-1].minute + (int)(classTime%60);
    
    timeSet[i+1].minute = (timeSet[i].minute + (int)(timeInterval%60)) % 60;
    timeSet[i+1].hour = timeSet[i].hour + (int)(timeSet[i].minute + (int)timeInterval%60)/60;
  }
  
  Serial.println();
  for(int i = 0; i < 22; i++)
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
    if(timeSet[i].hour > p_tm->tm_hour) return i; //24hour system and sorted timeSet
    else if(timeSet[i].hour == p_tm->tm_hour && timeSet[i].minute > p_tm->tm_min) return i; //when current time is same with timeSet return next timeSet
  }
  return 0;
}
/**************************************syncTime End*******************************************/

void setup() 
{
  pinMode(timeBellPin, OUTPUT);
  pinMode(masterPin, OUTPUT);
  digitalWrite(timeBellPin, LOW);
  digitalWrite(masterPin, LOW);

  pinMode(D1, INPUT);
  pinMode(D2, OUTPUT);
  mySerial.begin(115200, D1, D2); //(Rx, Tx)
  Serial.begin(115200);
  delay(100);
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

  compareMinute = p_tm->tm_min;
  
  delay(1000);

  digitalWrite(timeBellPin, LOW);
  
  /*********************************TIMEBELLPIN HANDLE PART*********************************/
  if(compareTimeSet != syncTimeSet()) //when current time goes to one of timeSet
  {
    if(compareTimeSet % 2 == 0) //if timeSet means class starts(even number of timeSet index)
    {
      //digitalWrite(timeBellPin, HIGH);
      Serial.println("Ring TimeBell : class start!");
    }
    else  //if timeSet means class ends(odd number of timeSet index)
    {
      //digitalWrite(timeBellPin, HIGH);
      Serial.println("Ring TimeBell : class ends!");
    }
    compareTimeSet = syncTimeSet();
  }
  else compareTimeSet = syncTimeSet();
  /****************************************************************************************/

  /*********************************MASTERPIN HANDLE PART**********************************/
  if(compareTimeSet % 2  == 0)  //when waiting for class starts
  {
    //calculate activate time from the (classStartTime - activateTime)
    int tempH, tempM = 0;
    if(timeSet[compareTimeSet].minute - activateTime < 0)
    {
      tempH = timeSet[compareTimeSet].hour-1;
      if(tempH<0) tempH +=24;
      tempM = timeSet[compareTimeSet].minute - activateTime + 60;
    }
    else
    {
      tempH = timeSet[compareTimeSet].hour;
      tempM = timeSet[compareTimeSet].minute - activateTime;
    }
    
    //handle masterPin
    if(p_tm->tm_hour > tempH)
    {
      //digitalWrite(masterPin, HIGH);
      Serial.println("Alarm activated..");
    }
    else if(p_tm->tm_hour == tempH && p_tm->tm_min >= tempM)
    {
      //digitalWrite(masterPin, HIGH);
      Serial.println("Alarm activated..");
    }
  }
  else
  {
    digitalWrite(masterPin, LOW);  //when waiting for class ends, keep masterPin LOW
    Serial.println("Alarm deactivated..");
  }
  /*********************************************************************************************/
  
  Serial.print("waiting for ");
  Serial.print(compareTimeSet);
  Serial.print(" => ");
  Serial.print(timeSet[compareTimeSet].hour);
  Serial.print(" : ");
  Serial.println(timeSet[compareTimeSet].minute);
}
