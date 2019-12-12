#include <bitswap.h>
#include <chipsets.h>
#include <color.h>
#include <colorpalettes.h>
#include <colorutils.h>
#include <controller.h>
#include <cpp_compat.h>
#include <dmx.h>
#include <FastLED.h>
#include <fastled_config.h>
#include <fastled_delay.h>
#include <fastled_progmem.h>
#include <fastpin.h>
#include <fastspi.h>
#include <fastspi_bitbang.h>
#include <fastspi_dma.h>
#include <fastspi_nop.h>
#include <fastspi_ref.h>
#include <fastspi_types.h>
#include <hsv2rgb.h>
#include <led_sysdefs.h>
#include <lib8tion.h>
#include <noise.h>
#include <pixelset.h>
#include <pixeltypes.h>
#include <platforms.h>
#include <power_mgt.h>

// 초 : 쾌적, 노 : 쏘쏘, 빨 : 경고
#include <FastLED.h>
#include "pitches.h"
#include <SoftwareSerial.h>
SoftwareSerial nodeSerial(5,6);//RX : 5 ,TX : 6


int r=0;
int g=0;
int b=0;

int r1=252;
int g1=252;

char savedch;

#define SoundSensorPin A1  //this pin read the analog voltage from the sound level meter
#define VREF  5.0  //voltage on AREF pin,default:operating voltage

#define LED_PIN     7
#define NUM_LEDS    288

CRGB Rleds[NUM_LEDS];

// notes in the melody:
int melody[] = {
  NOTE_C6, NOTE_E6, NOTE_G6, NOTE_E6, NOTE_C6, NOTE_E6, NOTE_G6,NOTE_E6, NOTE_C7, NOTE_B6, NOTE_A6, NOTE_G6, NOTE_F6, NOTE_E6, NOTE_D6, NOTE_C6, NOTE_C6, 0, NOTE_B5,0, NOTE_C6
};

int warning[] = {
  NOTE_E5,NOTE_E5, 0 ,NOTE_E5,NOTE_E5, 0
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,8,8,8,8,8,8
};

int WnoteDurations[] {
  8,8,8,8,8,8,8,8,8,8,8,8
};

void setup() {
  delay( 3000 ); // power-up safety delay
  Serial.begin(9600);
  FastLED.setBrightness(30);
  nodeSerial.begin(9600);

}
void alarmSystemOFF()
{
    /* Do not Change.*/
 float voltageValue,dbValue;
    voltageValue = analogRead(SoundSensorPin) / 1024.0 * VREF;
    dbValue = voltageValue * 50.0;  //convert voltage to decibel value
    Serial.print(dbValue,1);
    Serial.println(" dBA");
    delay(100);
  
  FastLED.addLeds<WS2812, LED_PIN, GRB>(Rleds, NUM_LEDS); // LED 색 초기화
  for(int i= int(2*dbValue); i< NUM_LEDS; i++) {
    Rleds[i]=CRGB(r,g,b);
  }

  FastLED.addLeds<WS2812, LED_PIN, GRB>(Rleds, NUM_LEDS);
  FastLED.setBrightness(20);
  
    for (int i = 0 ; i < 2* dbValue; i++)
    {
     Rleds[i] = CRGB::Blue;
    }
  
  FastLED.show();
  
}
void alarmSystemON()
{
  /* Do not Change.*/
 float voltageValue,dbValue;
    voltageValue = analogRead(SoundSensorPin) / 1024.0 * VREF;
    dbValue = voltageValue * 50.0;  //convert voltage to decibel value
    Serial.print(dbValue,1);
    Serial.println(" dBA");
    delay(100);
  
  FastLED.addLeds<WS2812, LED_PIN, GRB>(Rleds, NUM_LEDS); // LED 색 초기화
  for(int i= int(2*dbValue); i< NUM_LEDS; i++) {
    Rleds[i]=CRGB(r,g,b);
  }
  
  if(dbValue < 65) { //65이하 일때, 일반적으로 아무 말도 안할때 60 dba가 나옴
    FastLED.addLeds<WS2812, LED_PIN, GRB>(Rleds, NUM_LEDS);
      for(int i=0; i<2*dbValue; i++) {
      Rleds[i]=CRGB::Green;
      //delay(10);

    }
    FastLED.show();
  }
  
  else if(dbValue>=65 && dbValue<120) {
      FastLED.addLeds<WS2812, LED_PIN, GRB>(Rleds, NUM_LEDS);
      FastLED.setBrightness(30);
      
      for (int i = 0; i < 2*dbValue; i++)
      {
       Rleds[i] = CRGB::Yellow;
       //delay(10);
      }
      FastLED.show();
   
//      for (int i = 0; i < 100; i++){
//        FastLED.setBrightness(i);
//        FastLED.show();
//      }
//
//      for (int i = 60; i >= 0; i--){
//        FastLED.setBrightness(i);
//        FastLED.show();
//      }
      
  }

  /*dbValue  100 이상 */
  else {
    FastLED.addLeds<WS2812, LED_PIN, GRB>(Rleds, NUM_LEDS);
     for (int i = 0; i < 2*dbValue; i++) 
     {
       Rleds[i] = CRGB::Red;
     }
     //FastLED.show();

      for (int i = 0; i < 45; i++){
        FastLED.setBrightness(i);
        FastLED.show();
      }

//      for (int i = 60; i >= 0; i--){
//        FastLED.setBrightness(i);
//        FastLED.show();
//      }
     
     /* 경고음 코드 */
      for (int thisNote = 0; thisNote < 6; thisNote++) {
        // to calculate the note duration, take one second divided by the note type.
        //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
        
        int noteDuration = 1000 / WnoteDurations[thisNote];
        tone(8, warning[thisNote], noteDuration);
        

        // to distinguish the notes, set a minimum time between them.
        // the note's duration + 30% seems to work well:
        int pauseBetweenNotes = noteDuration * 2.40;
        delay(pauseBetweenNotes);
        // stop the tone playing:
        noTone(8);
        }
      
    delay(1000); 
  }
}

void loop() {
//  delay(100);
/******************************************************************/
  if(nodeSerial.available()){
    char ch = nodeSerial.read();
    //Serial.println(ch);
    if(ch == 'A')
    {
      Serial.println("Alarm System Activated");
      savedch = ch;
    }
    else if(ch == 'D')
    {
      Serial.println("Alarm System Deactivated");
      savedch = ch;
    }
    else if(ch == 'E')
    {
      Serial.println("Class Ends!");
      savedch = ch;
    }
    else if(ch == 'S') 
    {
      Serial.println("Class Starts!");
      savedch = ch;
      
  //  delay(1000); 
    }
    else
    {
      Serial.println("No Signal");
    }
  }
  else Serial.println("Buffer empty");
/******************************************************************/
  Serial.println(savedch);
  
  if(savedch == 'A')
  {
    alarmSystemON();
  }

  if(savedch == 'D')
  {
    alarmSystemOFF();
  }
  
  if(savedch == 'S')
  {
      for (int thisNote = 0; thisNote < 21; thisNote++) {
        // to calculate the note duration, take one second divided by the note type.
        //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
        
        int noteDuration = 1000 / noteDurations[thisNote];
        tone(8, melody[thisNote], noteDuration);
        
        // to distinguish the notes, set a minimum time between them.
        // the note's duration + 30% seems to work well:
        int pauseBetweenNotes = noteDuration * 1.20;
        delay(pauseBetweenNotes);
        // stop the tone playing:
        noTone(8);
        }
  }
  if(savedch=='E') {
    for (int thisNote = 0; thisNote < 21; thisNote++) {
        // to calculate the note duration, take one second divided by the note type.
        //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
        
        int noteDuration = 1000 / noteDurations[thisNote];
        tone(8, melody[thisNote], noteDuration);
        
        // to distinguish the notes, set a minimum time between them.
        // the note's duration + 30% seems to work well:
        int pauseBetweenNotes = noteDuration * 1.20;
        delay(pauseBetweenNotes);
        // stop the tone playing:
        noTone(8);
        }
  }
  
  nodeSerial.flush();
}
