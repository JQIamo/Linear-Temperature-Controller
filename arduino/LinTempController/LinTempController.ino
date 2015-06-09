#include <SPI.h>
#include <SerialCommand.h>
#include "Teensy_TempController.h"
SerialCommand sCmd;

void unrecognizedCmd(const char *command){
  Serial.println("UNRECOGNIZED COMMAND");
}

void helloWorld(){
  Serial.println("Hello World");
}

void writeDigiPOT(byte address, byte value){
  //Make sure communication lines are initialized
  digitalWrite(SCK_B, LOW);
  digitalWrite(CS_POT1,LOW);
  //digitalWrite(MOSI_B, LOW);
  
  uint16_t data = word(address,value); //Combine address and value bits
  int send_bit;
	for (int i = 8; i >= 0; i--){
		send_bit = (data >> i) & 0x01;	// mask out i_th bit
										// start MSB first
		digitalWrite(MOSI_B, send_bit);
		delayMicroseconds(1);
	
		digitalWrite(SCK_B, HIGH);
		delayMicroseconds(1);
		digitalWrite(SCK_B, LOW);
		delayMicroseconds(1);
	}
  //Set chip select pin back to high
  digitalWrite(CS_POT1,HIGH);
}

void writeDAC(byte command, byte address, word data)
{
  //Make sure communication lines are initialized
  digitalWrite(SCK_B, HIGH);
  digitalWrite(CS_DAC,LOW);
  //digitalWrite(MOSI_B, LOW);
  
  // The first byte is composed of two bits of nothing, then
  // the command bits, and then the address bits. Masks are
  // used for each set of bits and then the fields are OR'ed
  // together.
  byte first = (command & B00111000) | (address & B00000111);
  int send_bit;
	for (int i = 7; i >= 0; i--){
		send_bit = (first >> i) & 0x01;	// mask out i_th bit
										// start MSB first
		digitalWrite(MOSI_B, send_bit);
		delayMicroseconds(1);
	
		digitalWrite(SCK_B, LOW);
		delayMicroseconds(1);
		digitalWrite(SCK_B, HIGH);
		delayMicroseconds(1);
	}
  
  // Send the data word. Must be sent byte by byte, MSB first.
  
  uint16_t second = data;
  	for (int i = 15; i >= 0; i--){
		send_bit = (second >> i) & 0x01;	// mask out i_th bit
										// start MSB first
		digitalWrite(MOSI_B, send_bit);
		delayMicroseconds(1);
	
		digitalWrite(SCK_B, LOW);
		delayMicroseconds(1);
		digitalWrite(SCK_B, HIGH);
		delayMicroseconds(1);
	}
      
  // Set the Slave Select pin back to high since we are done
  // sending the command.
  digitalWrite(CS_DAC,HIGH);
}
void setDACrefV(boolean onoff){
  writeDAC(B00111000,0,onoff); //enable internal voltage reference (1.247V actual)
}

void setDAC(byte ch, double val){
  if (ch > 4) {
    return; //channel isn't valid, return without doing anything
  }
  
  double Vref = 1.247;
  word data;
  
  if (val > (Vref*2)) {
    data = 4095; //command is larger than max voltage allowed, default to max voltage
  }
  else if (val < 0) {
    data = 0; //command is less than zero, default to zero
  }
  else {
    data = (4095*val/(2*Vref)); //DAC transfer function for 12bit DAC, internal ref voltage
  }
  
  data = (data << 4); //bit shift by 4 bits is needed to move command bits to right location, shift would be 2 bits for 14 bit or 0 for 16 bit DAC
  writeDAC(B00011000,ch,data);
}

void test(){
  byte command = B00011000; 
  byte address = 0; 
  word value = (1641 << 4); 
   //writeDigiPOT(address, value);
   //writeDAC(B00111000,0,1); //enable internal voltage reference (1.247V actual)
   //writeDAC(command,address,value);
   setDAC(0,0);
}


void setup() {
  // put your setup code here, to run once:
  
  //Eventually make these objects and move these initialization lines to a constructor
  pinMode(MOSI_B, OUTPUT);
  pinMode(SCK_B, OUTPUT);
  pinMode(CS_POT1, OUTPUT);
  pinMode(CS_DAC,OUTPUT);
  digitalWrite(CS_POT1,HIGH);
  digitalWrite(CS_DAC,HIGH);
  
  Serial.begin(9600);
  
  sCmd.addCommand("H",helloWorld);
  sCmd.addCommand("T",test);
  sCmd.setDefaultHandler(unrecognizedCmd);
  Serial.println("ready");

}

void loop() {
  // put your main code here, to run repeatedly:
  
  sCmd.readSerial();

}
