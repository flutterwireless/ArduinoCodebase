#include "Arduino.h"
#include "Flutter.h"


#include "Reset.h"
#include <SPI.h>



Flutter flutter;


uint8_t counter = 0;

volatile bool processInt = false;
volatile int processedInterrupts = 0;
volatile int interrupts = 0;

#define BOARD_ADDRESS 1
#define CHANNEL_SCHEDULE 0
#define BOARD_NAME "Flutter Master"


byte counterVal = 0;

byte displayMode = 0;
byte x=0;
byte y=0;

const byte PER_total = 18;
bool PER_array[PER_total];
byte packetError = 0;

bool transmitter = true;


byte counterOffset = 0;

byte dest_address = 2;

void button2();
void button1();
void radioInterrupt();
void softInt();



//    Setup     //
//==============//

void setup() {
attachInterrupt(255, softInt,0); //software Interrupt
Serial.begin(115200);
flutter.begin(BOARD_ADDRESS);

attachInterrupt(BUTTON2, button2, FALLING);
attachInterrupt(BUTTON1, button1, RISING);
  #ifdef FLUTTER_R2
  attachInterrupt(15, radioInterrupt, CHANGE);
  #else
  attachInterrupt(17, radioInterrupt, CHANGE);
  #endif




  for(int i = 0; i<PER_total; i++)
  {
    PER_array[i]=false;
  }
pinMode(6, OUTPUT);
pinMode(7, OUTPUT);
pinMode(8, OUTPUT);
pinMode(9, OUTPUT);


 


}



     
void loop() {
  // put your main code
  
  if(transmitter)
  {  


    if(BOARD_ADDRESS==1)
    {
      int index;
      int time_in_mills = millis();
      byte data[] = {0xA, 0x5, 1};
      index = flutter.transmitDataPacket(data, 3, dest_address);
      dest_address++;
      if(dest_address>2)
      {
        dest_address=2;
      }
      while(flutter.txSuccess(index)==false)
      {
        if(millis()>time_in_mills+35)
        {
          break;
        }
      }
      }
      /*
      for(int j = 0;j<4;j++)
      {
        for(int i = 0;i<255;i++)
        {
       
        switch(j)
       {
        case 0:
        flutter.setLED(0,0,i);
        delay(2);
        break;
        case 1:
        flutter.setLED(0,i,0);
        delay(2);
        break;
        case 2:
        flutter.setLED(i,0,0);
        delay(2);
        break;
        case 3:
        //flutter.setLED(i,i,i);
        delay(2);
        break;
        
        
       } 
         
       }
       for(int i = 255;i>=0;i--)
        {
       
        switch(j)
       {
        case 0:
        flutter.setLED(0,0,i);
        delay(2);
        break;
        case 1:
        flutter.setLED(0,i,0);
        delay(2);
        break;
        case 2:
        flutter.setLED(i,0,0);
        delay(2);
        break;
        case 3:
      //  flutter.setLED(i,i,i);
        delay(2);
        break;
        
        
       } 
         
       }
        
      }
      */
      
  /*
  display.clearDisplay();
        // text display tests
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Ping Unit:");
  display.setTextSize(1);
  display.println();
  display.setTextSize(5);
  
  display.print(" ");
  display.println(dest_address);
 
  //display.print("0x"); display.println(0xDEADBEEF, HEX);
  display.display();
    
*/


    


   // counterVal= (byte)(counter % 255);
    //flutterpacket(0xA, 0x05, counterVal, 0x20, 0x73);

   // byte data[] = {0xA, 0x5, counterVal};
 // flutter.transmitDataPacket(data, 3, 0xFF);

  //  counter++;
    delay(100);
   //flutter.transmitSyncPacket(0xFF);
    //flutter.process();

    }else
    {
/*
      if(flutter.process()==true)
      {
   
        

          int dev_type = flutter.dataBuffer[4];
          int cmd = flutter.dataBuffer[5];
          int val1 = flutter.dataBuffer[6];

          if(dev_type== 0xA && cmd == 0x05)
          {

          if(packetError<PER_total*0.15)
        {
          flutter.setLED(150,150,150);
        }
        else if(packetError<PER_total*0.30){
          flutter.setLED(0,255,0);
        }
        else if(packetError<PER_total*0.60){
          flutter.setLED(0,0,255);
        }
        else if(packetError>=PER_total*0.60){
          flutter.setLED(255,0,0);
        }


            int difference = val1-counterVal;

            if(difference<0)
            {
              difference+=255;
            }


          if(difference==1)
            {
              updatePacketArray(true);
              }else{
                for(int i=1; i<difference; i++)
                {
                  updatePacketArray(false);
                }
                updatePacketArray(true);
              }

               

            Serial.print("Val1: ");
            Serial.print(val1);
            Serial.print(" Counterval: ");
            Serial.print(counterVal);
            Serial.print(" Errors: ");
            Serial.println(packetError);
            for(int i = 0;i<PER_total;i++)
            {
              Serial.print(PER_array[i]==true?"0":"_");
            }
            Serial.println();
            Serial.println();
            
            updateDisplay();

            counterVal=val1;

*/

/*
            if(val1>counterVal+counterOffset+1)
            {
            flutter.setLED(50,50,50);
            matrix.clear();
            matrix.setRotation(2);
            matrix.drawBitmap(0, 0, arrow_bmp, 8, 8, LED_ON);
            matrix.writeDisplay();
            counterOffset=val1;
            delay(200);
            }else{
              counterVal=val1-counterOffset;
              fillmatrix(val1 % 64);

            }*/

     // }

     // }
      //delay(1);
      flutter.setLED(0,0,0);
    }

      
    
  }



void updatePacketArray(bool success)
{
  byte errors = 0;
  for(int i = PER_total-1; i>0; i--)
  {
    PER_array[i]=PER_array[i-1];
    if(PER_array[i]==false)
    {
      errors++;
    }
  }
  PER_array[0]=success;
  if(PER_array[0]==false)
  {
    errors++;
  }

  packetError = errors;
}




void flutterpacket(int dev_type, int cmd, int val1, int val2, int val3, int val4)
{
  byte data[] = {dev_type, cmd, val1, val2, val3, val4};
  flutter.transmitDataPacket(data, 6, 0xFF);
}

void flutterpacket(int dev_type, int cmd, int val1, int val2, int val3)
{
  flutterpacket(dev_type, cmd, val1, val2, val3, 0xAA);
}

void flutterpacket(int dev_type, int cmd, int val1, int val2)
{
  flutterpacket(dev_type, cmd, val1, val2, 0xAA, 0xAA);
}

void flutterpacket(int dev_type, int cmd, int val1)
{
  flutterpacket(dev_type, cmd, val1, 0xAA, 0xAA, 0xAA);
}

            
void button1()
{

while(digitalRead(BUTTON1)==HIGH) //top button
  {
   flutter.setLED(255,0,0);
  }


      flutter.setLED(0,0,255);
      delayMicroseconds(16000);
      delayMicroseconds(16000);
      flutter.setLED(0,0,0);
      delayMicroseconds(16000);
      delayMicroseconds(16000);
      flutter.setLED(0,255,0);
      delayMicroseconds(16000);
      delayMicroseconds(16000);
      flutter.setLED(0,0,0);
      delayMicroseconds(16000);
      delayMicroseconds(16000);
      flutter.setLED(255,0,0);
      delayMicroseconds(16000);
      delayMicroseconds(16000);
      flutter.setLED(0,0,0);

     initiateReset(1);
      tickReset();
    
}

void button2()
{
  if(processInt)
  {
   while(digitalRead(BUTTON2)==LOW) //top button
  {
   flutter.setLED(255,0,0);
  }
  byte data[] = {0xA, 0x0, 0x1, 0x2, 0xFF};
  flutter.transmitDataPacket(data, 5, 0x00);
  flutter.setLED(0,0,0);
  }
  processInt=true;
  counter++;
}

void radioInterrupt()
{
   interrupts = flutter.interrupt();
}
void softInt()
{
  flutter.process();  
}

extern int tickInterrupt()
{
  return flutter.tickInt();
}




