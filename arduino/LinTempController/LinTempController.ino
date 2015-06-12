#include <SPI.h>
#include <SerialCommand.h>
#include "Teensy_TempController.h"
#include "Encoder.h"
#include "LCD.h"
#include "AD5262.h"


SerialCommand sCmd;

LCD lcd(RST_LCD, RS_LCD, CS_LCD);
Encoder enc_test1(ENC_A1,ENC_B1,ENC_SW1);

Encoder enc_ch_select(ENC_A2,ENC_B2,21);
Encoder enc_set_t(ENC_A1,ENC_B1,ENC_SW1);
Encoder enc_set_pgain(ENC_A1,ENC_B1,ENC_SW1);
Encoder enc_set_Itc(ENC_A1,ENC_B1,ENC_SW1);

//Initialize digipot object:
#define POT1_MAX 227500UL
#define POT1_MIN 1100UL
AD5262 dPOT(CS_POT1,SCK_B,MOSI_B,POT1_MAX,POT1_MIN);

#define VREF 2.5
#define ANALOG_READ_BITS 12

//Define constants used to convert temp to voltage
//Current values use the Steinhart-Hart model and the following thermistor:
//Epcos B57862S0103F040 (digikey PN 495-2166-ND), 10kOhm, NTC
#define STEINHART_A    1.1254e-3
#define STEINHART_B    2.3469e-4
#define STEINHART_C    0.8584e-7
#define BIAS_CURRENT   100e-6

#define MIN_TEMP    10.0
#define MAX_TEMP    35.0

void unrecognizedCmd(const char *command){
  Serial.println("UNRECOGNIZED COMMAND");
}

void test_press_event(Encoder *this_encoder) {
  this_encoder->increment_step_size();
}

void test_hold_event(Encoder *this_encoder) {
  lcd.write("hold...",0x00);
  delay(2000);
  lcd.clear();
}



//

//moved to library
//void writeDigiPOT(byte address, byte value){
//  //Make sure communication lines are initialized
//  digitalWrite(SCK_B, LOW);
//  digitalWrite(CS_POT1,LOW);
//  //digitalWrite(MOSI_B, LOW);
//  
//  uint16_t data = word(address,value); //Combine address and value bits
//  int send_bit;
//	for (int i = 8; i >= 0; i--){
//		send_bit = (data >> i) & 0x01;	// mask out i_th bit
//										// start MSB first
//		digitalWrite(MOSI_B, send_bit);
//		delayMicroseconds(1);
//	
//		digitalWrite(SCK_B, HIGH);
//		delayMicroseconds(1);
//		digitalWrite(SCK_B, LOW);
//		delayMicroseconds(1);
//	}
//  //Set chip select pin back to high
//  digitalWrite(CS_POT1,HIGH);
//}

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

//read voltage at specified analog pin
float getVoltage(byte analogPin){
  int val = analogRead(analogPin);
  int maxReading = (1 << ANALOG_READ_BITS) - 1;
  float voltage = VREF*(float(val)/maxReading);
  return voltage;
}

//read temperature (assumes 10kOhm NTC thermistor, typically Digikey PN: 495-2166-ND)
double getTemp(){

  //get voltage across the thermistor:
  float V = getVoltage(ACT_T1);
  
  //calculate resistance of thermistor, assuming 100uA of bias current
  double R = V/BIAS_CURRENT;
  
  //calculate temp based on Steinhart-Hart equation: 1/T = A + B ln(R) + C ln(R)^3
  double T  = 1/(STEINHART_A + STEINHART_B*log(R) + STEINHART_C*pow(log(R),3.0));
  
  //convert from K to C
  T -= 273.15;
  
  return T;
}

//calculate and write set voltage given a desired set temperature (in Celcius)
void setTemp(double T){
  
  //Make sure desired temp is within the desired range
  if (T > MAX_TEMP){
    T = MAX_TEMP;
  } else if (T < MIN_TEMP) {
    T = MIN_TEMP;
  }
  
  //convert from C to K:
  T += 273.15;
  
  //calculate resistance of thermistor at desired temp using the inverse of the Steinhart-Hart equation:
  double x = (STEINHART_A-(1/T))/STEINHART_C;
  double y = sqrt( pow(STEINHART_B/(3*STEINHART_C),3) + pow(x/2,2));
  double R = exp(pow(y-x/2,1.0/3) - pow(y+x/2,1.0/3));
  
  double setV = BIAS_CURRENT*R;
  
  //setDAC(0,setV);
  
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
  
  data <<= 4; //bit shift by 4 bits is needed to move command bits to right location, shift would be 2 bits for 14 bit or 0 for 16 bit DAC
  writeDAC(B00011000,ch,data);
}

void test(){
   //writeDigiPOT(address, value);
   //writeDAC(B00111000,0,1); //enable internal voltage reference (1.247V actual)
   //writeDAC(command,address,value);
   //setDAC(0,0);
   //dPOT.setI(20.1);
   //Serial.println(dPOT.getVal(1));
   //Serial.println(dPOT.getR(1));
   //Serial.println(dPOT.getI());
   int val;
   //Serial.println(getVoltage(VMON1),4);
   //Serial.println(getTemp(),10);
   lcd.write("CH 1     23.48 C", 0x00);
   lcd.write("Out Mon   19.99V", 0x40);
   
}
void interruptWrapper(){
  enc_test1.interrupt();
}

void setup() {
  // put your setup code here, to run once:
  
  //Eventually make these objects and move these initialization lines to a constructor
  pinMode(MOSI_B, OUTPUT);
  pinMode(SCK_B, OUTPUT);
  //pinMode(CS_POT1, OUTPUT);
  pinMode(CS_DAC,OUTPUT);
  //digitalWrite(CS_POT1,HIGH);
  digitalWrite(CS_DAC,HIGH);
  
  //Set Up Analog Read Pins
  pinMode(VMON1,INPUT);
  pinMode(ACT_T1,INPUT);
  analogReference(EXTERNAL);
  analogReadResolution(ANALOG_READ_BITS);
  analogReadAveraging(10);
  
  //Set up encoder pins:
  pinMode(ENC_A1,INPUT);
  pinMode(ENC_B1,INPUT);
  pinMode(ENC_SW1,INPUT);
  pinMode(ENC_A2,INPUT);
  pinMode(ENC_B2,INPUT);
  pinMode(21,INPUT);
  
  //Initialize the LCD
  SPI.begin();
  lcd.init();
  
  attachInterrupt(ENC_A1, interruptWrapper, CHANGE);
  enc_test1.attach_button_press_event(test_press_event);
  enc_test1.attach_button_hold_event(test_hold_event);
  
  enc_test1.init(1.0, 0.0, 100.0);
  double step_sizes[]={1,5,10};
  String step_labels[]={"LSB","5 sec","10 apples"};
  enc_test1.define_step_sizes(3,step_sizes,step_labels);
  
  
  Serial.begin(9600);

  sCmd.addCommand("T",test);
  sCmd.setDefaultHandler(unrecognizedCmd);
  Serial.println("ready");
    

}

void loop() {
  // put your main code here, to run repeatedly:
//  int val;
//  float voltage;
//  float Vref = 2.5;
//  val = analogRead(VMON1);
//  voltage = Vref*(val/4095.0);
//  Serial.println(voltage,4);
//  sCmd.readSerial();
//  delay(500);
  //Serial.println(enc_test1.position());
  double currPos = enc_test1.position();
  lcd.write("Curr Pos: " + String(currPos),0x00);
  enc_test1.button_events();
  //delay(500);

  sCmd.readSerial();

}
