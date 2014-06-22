// softKeyboardInterface.ino


#include "ADC.h"


const int light1 = 11;
const int light2 = 12;

const int ledPin = 13;
const int readPin = A9;

ADC *adc = new ADC(); // adc object

RingBuffer *buffer = new RingBuffer;

void setup() {

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(light1, OUTPUT);
  pinMode(light2, OUTPUT);


  Serial.begin(9600);


  adc->setAveraging(32); // set number of averages
  adc->setResolution(12); // set bits of resolution


  adc->setAveraging(32, ADC_1); // set number of averages
  adc->setResolution(12, ADC_1); // set bits of resolution


  delay(500);
}

String inStruct;                                                                                                                                                                                  ;
double value = 0;
int count = 1;
int params;

void loop() {


  if (Serial.available()) {
    // Interpret commands from computer
      inStruct = Serial.readStringUntil(',');
      
    }
  
    // If statements are used to include more than one logical variable type 
    if(inStruct=="lightOn") { 
      digitalWrite(light2, HIGH);

    } 
    else if(inStruct=="lightOff") { // conversion active?
      digitalWrite(light2, LOW);
    } 
    else if(inStruct.startsWith("cycVolt",0)) { // conversion active?
      Serial.println(inStruct);
           
      int sampTime    = inStruct.substring(7,9).toInt();
      int startVolt   = inStruct.substring(9,11).toInt();
      int endVolt     = inStruct.substring(11,13).toInt();
      int sweepRate   = inStruct.substring(13).toInt();
      cycVolt(sampTime, startVolt, endVolt, sweepRate);
    }
  }

// sampTime in seconds (Sampling time)
/// startVolt in volts
// endVolt in volts
// sweepRate

void cycVolt(int sampTime, int startVolt, int endVolt, int sweepRate) {
      int length = sampTime*200; // With delay of 5 ms, 200 samples per second
      for (int i = 0; i < length; i++) {
        value = adc->analogRead(readPin);

        buffer->write(value);

        Serial.println(buffer->read()*3.3/adc->getMaxValue(ADC_0),5);
        delay(5);
      }
      Serial.print("Sample Time: ");
      Serial.println(sampTime);
      Serial.print("Start Volt: ");
      Serial.println(startVolt);
      Serial.print("End Volt: ");
      Serial.println(endVolt);
      Serial.print("Sweep Rate: ");      
      Serial.println(sweepRate);
      inStruct = "";
}


