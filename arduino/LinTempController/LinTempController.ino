#include <EEPROMex.h>
#include <EEPROMVar.h>
#include <Metro.h>
#include <SPI.h>
#include <SerialCommand.h>
#include "Teensy_TempController.h"
#include "Encoder.h"
#include "LCD.h"
#include "AD5262.h"
#include "AD56X4R.h"
#include "WTC3243.h"
#include "TempContSettings.h"
#include "Menu.h"





SerialCommand sCmd;

LCD lcd(RST_LCD, RS_LCD, CS_LCD);
//Encoder enc_test1(ENC_A1,ENC_B1,ENC_SW1);

//Initialize Encoder Objects;
Encoder enc_ch_select(ENC_A2, ENC_B2, 21);
Encoder enc_set_t(ENC_A1, ENC_B1, ENC_SW1);
Encoder enc_set_pgain(ENC_A1, ENC_B1, ENC_SW1);
Encoder enc_set_Itc(ENC_A1, ENC_B1, ENC_SW1);

//Initialize Menu Objects:
Menu main_menu(4);

//Initialize DAC and Temp Controller:
AD56X4R dac(CS_DAC, SCK_B, MOSI_B, Settings::dac_bits, Settings::dac_vref);
WTC3243 tempCont(CS_POT1, SCK_B, MOSI_B, CS_DAC, Ch1::dac_ch, VMON1, ACT_T1, Ch1::pot_min, Ch1::pot_max);

//Initialize Metro object (deals with timing):
Metro timer(Settings::save_interval);

void unrecognizedCmd(const char *command) {
  Serial.println("UNRECOGNIZED COMMAND");
}

void incrementStepSize_pressEvent(Encoder *this_encoder) {
  this_encoder->increment_step_size();
}


void test_press_event(Encoder *this_encoder) {
  this_encoder->increment_step_size();
}

void test_hold_event(Encoder *this_encoder) {
  //Save settings to EEPROM
  writeSettingstoMemory();
}

void secondary_hold_event(Encoder *this_encoder) {
  Serial.println("HELD");
  lcd.init();
}

void secondary_press_event(Encoder *this_encoder) {
  Serial.println("PUSH");
}


void test() {
  //writeDigiPOT(address, value);
  //writeDAC(B00111000,0,1); //enable internal voltage reference (1.247V actual)
  //writeDAC(command,address,value);
  //setDAC(0,0);
  //dPOT.setI(20.1);
  //Serial.println(dPOT.getVal(1));
  //Serial.println(dPOT.getR(1));
  //Serial.println(dPOT.getI());
  //int val;
  //Serial.println(getVoltage(VMON1),4);
  //Serial.println(getTemp(),10);
  //lcd.write("CH 1     23.48 C", 0x00);
  //lcd.write("Out Mon   19.99V", 0x40);

  //tempCont.setP(30);
  //tempCont.setI(0.82);

  //Serial.println(tempCont._dPOT.getR(1));

  //tempCont.setTemp(25.0, dac);
  //tempCont.setTempV(1.03,dac);
  //dac.setVoltage(0,2.3);
  //setDAC(0,.75);
  //dac.setIntRefV(1);
  Serial.println("here");
  lcd.init();

}

void interruptWrapper() {
  if (main_menu.current_mode() == 0) {
    enc_set_t.interrupt();
  } else if (main_menu.current_mode() == 1) {
    enc_set_pgain.interrupt();
  } else if (main_menu.current_mode() == 2) {
    enc_set_Itc.interrupt();
  }
}

void chSelectInterruptWrapper() {
  enc_ch_select.interrupt();
}

void mode_temp_tune() {
  enc_set_t.button_events(); //look for button events (in this case changes step size)
  float newSetTemp = enc_set_t.position();
  float oldSetTemp = tempCont.getTempSetPt();

  //compare old to new value, update only if necessary
  if (int(newSetTemp * 1000) != int(oldSetTemp * 1000)) { //since we're comparing floating pt #s, cast to ints to avoid problems)
    tempCont.setTemp(newSetTemp, dac);
  }

  //update display
  char lineToDisplay[17];
  snprintf(lineToDisplay, 17, "SetT(%3s) %5.2fC", enc_set_t.step_size_label().c_str(), newSetTemp);
  //lcd.write("SetT(" + enc_set_t.step_size_label() + ") " + String(newSetTemp, 2) + "C", 0x040);
  lcd.write(lineToDisplay, 0x040);
}

void mode_pgain_tune() {
  enc_set_pgain.button_events(); //look for button events (in this case changes step size)

  //read prop gain value set by encoder and the current prop gain
  byte newPgain = enc_set_pgain.position();
  byte oldPgain = tempCont.getP();

  if (newPgain != oldPgain) {
    tempCont.setP(newPgain);
  }

  //format display line to be 16 characters wide
  char lineToDisplay[17];
  //snprintf(lineToDisplay, 17,"Pgain  (%1s)    %2u",enc_set_pgain.step_size_label().c_str(),newPgain);
  snprintf(lineToDisplay, 17, "Pgain (%1s) %2u A/V", enc_set_pgain.step_size_label().c_str(), newPgain);

  //update LCD
  lcd.write(lineToDisplay, 0x040);
}

void mode_Itc_tune() {
  enc_set_Itc.button_events(); //look for button events (in this case changes step size)

  //read integrator time constant value set by encoder and the current time constant
  float newItc = enc_set_Itc.position();
  float oldItc = tempCont.getI();

  //compare old to new value, update only if necessary
  if (int(newItc * 100) != int(oldItc * 100)) {
    tempCont.setI(newItc);
  }

  char lineToDisplay[17];
  snprintf(lineToDisplay, 17, "Itc (%3s) %4.1f s", enc_set_Itc.step_size_label().c_str(), newItc);

  lcd.write(lineToDisplay, 0x040);
}

void mode_out_mon() {

  float outV = tempCont.getOutputVoltage();

  char lineToDisplay[17];
  snprintf(lineToDisplay, 17, "Out Mon %6.3f V", outV);

  lcd.write(lineToDisplay, 0x040);
}

void monitorTemp() {
  float currTemp = tempCont.getActTemp();
  char lineToDisplay[17];
  snprintf(lineToDisplay, 17, "Ch 1      %5.2fC", currTemp);

  lcd.write(lineToDisplay, 0x00);

}

void writeSettingstoMemory() {
      //save settings to EEPROM
      
    lcd.write(" Settings Saved ",0x40);
    
    cli(); //clear interrupts, not sure if this is necessary 
    EEPROM.updateFloat(Settings::addressTempSetPt, tempCont.getTempSetPt());
    EEPROM.updateByte(Settings::addressPgain, tempCont.getP());
    EEPROM.updateFloat(Settings::addressItc, tempCont.getI());
    sei(); //set interrupts 
    delay(1500);
}

//Declare variables used to store EEPROM addresses:
int Settings::addressTempSetPt;
int Settings::addressPgain;
int Settings::addressItc;



void setup() {
  // put your setup code here, to run once:
  
  Serial.begin(9600);

  //get address for EEPROM saved variables:
  Settings::addressTempSetPt = EEPROM.getAddress(sizeof(float));
  Settings::addressItc = EEPROM.getAddress(sizeof(float));
  Settings::addressPgain = EEPROM.getAddress(sizeof(byte));

  //Initialize communication pins
  pinMode(MOSI_B, OUTPUT);
  pinMode(SCK_B, OUTPUT);

  //Initialize DAC CS PIN
  pinMode(CS_DAC, OUTPUT);
  digitalWrite(CS_DAC, HIGH);

  //Set Up Analog Read Pins
  analogReference(EXTERNAL);
  analogReadResolution(Settings::analog_read_bits);
  analogReadAveraging(Settings::analog_read_avg); 

  //Set up encoder pins:
  pinMode(ENC_A1, INPUT);
  pinMode(ENC_B1, INPUT);
  pinMode(ENC_SW1, INPUT);
  pinMode(ENC_A2, INPUT);
  pinMode(ENC_B2, INPUT);
  pinMode(21, INPUT); //push botton for 2nd encoder, not currently hooked up but could add jumper wire

  //Initialize the LCD
  SPI.begin();
  delay(100);
  lcd.init();

  //Initialize the temperature controller:
  tempCont.init(Ch1::bias_current, Ch1::steinhart_A, Ch1::steinhart_B, Ch1::steinhart_C, Ch1::min_temp, Ch1::max_temp);

  //Read temp set pt from memory (or if that value is out of bounds, initialize to 20.0 C
  float initialTempSetPt = EEPROM.readFloat(Settings::addressTempSetPt);
  if (isnan(initialTempSetPt) || initialTempSetPt < Ch1::min_temp || initialTempSetPt > Ch1::max_temp){
    initialTempSetPt = 20.0;
  }
  enc_set_t.init(initialTempSetPt, Ch1::min_temp, Ch1::max_temp);
  double temp_step_sizes[] = {0.01, 0.1, 1.0};
  String temp_step_labels[] = {".01", "0.1", "1.0"};
  enc_set_t.define_step_sizes(3, temp_step_sizes, temp_step_labels);
  enc_set_t.attach_button_press_event(incrementStepSize_pressEvent);
  enc_set_t.attach_button_hold_event(test_hold_event); //This is probably just for testing

  byte initialPgain = EEPROM.readByte(Settings::addressPgain);
  if (initialPgain < Settings::prop_min || initialPgain > Settings::prop_max){
    initialPgain = 50;
  }
  enc_set_pgain.init(initialPgain, Settings::prop_min, Settings::prop_max);
  double pgain_step_sizes[] = {1, 5};
  String pgain_step_labels[] = {"1", "5"};
  enc_set_pgain.define_step_sizes(2, pgain_step_sizes, pgain_step_labels);
  enc_set_pgain.attach_button_press_event(incrementStepSize_pressEvent);
  enc_set_pgain.attach_button_hold_event(test_hold_event); //This is probably just for testing


  float initialItc = EEPROM.readFloat(Settings::addressItc);
  if (isnan(initialItc) || initialItc < Settings::int_tc_min || initialItc > Settings::int_tc_max){
    initialItc = 1.0;
  }
  enc_set_Itc.init(initialItc, Settings::int_tc_min, Settings::int_tc_max);
  double itc_step_sizes[] = {0.1, 0.5, 1.0};
  String itc_step_labels[] = {"0.1", "0.5", "1.0"};
  enc_set_Itc.define_step_sizes(3, itc_step_sizes, itc_step_labels);
  enc_set_Itc.attach_button_press_event(incrementStepSize_pressEvent);
  enc_set_Itc.attach_button_hold_event(test_hold_event); //This is probably just for testing

  
  attachInterrupt(ENC_A2, chSelectInterruptWrapper, CHANGE);
  enc_ch_select.init(50000, 0, 100000);
  enc_ch_select.attach_button_press_event(secondary_press_event); 
  enc_ch_select.attach_button_hold_event(secondary_hold_event);//Just for testing right now

  attachInterrupt(ENC_A1, interruptWrapper, CHANGE);
  main_menu.attach_mode(0, "Temperature", mode_temp_tune);
  main_menu.attach_mode(1, "Proportional Gain", mode_pgain_tune);
  main_menu.attach_mode(2, "Int Time Constant", mode_Itc_tune);
  main_menu.attach_mode(3, "Voltage Monitor", mode_out_mon);


  //enc_test1.attach_button_press_event(test_press_event);
  ///enc_test1.attach_button_hold_event(test_hold_event);

  //enc_test1.init(1.0, 0.0, 100.0);
  //double step_sizes[]={1,5,10};
  //String step_labels[]={"LSB","5 sec","10 apples"};
  //enc_test1.define_step_sizes(3,step_sizes,step_labels);


  

  sCmd.addCommand("T", test);
  sCmd.setDefaultHandler(unrecognizedCmd);

  //Turn on DAC's internal voltage reference:
  dac.setIntRefV(1);
  
  //write initial settings
  tempCont.setTemp(initialTempSetPt, dac);
  tempCont.setP(initialPgain);
  tempCont.setI(initialItc);

}

void loop() {
  // put your main code here, to run repeatedly:

  enc_ch_select.button_events();

  monitorTemp(); //function that displays the current measured temperature (of selected channel)
  main_menu.switch_to_mode(int(enc_ch_select.position()) % 4);  //turning ch select knob changes the menu mode
  main_menu.run_mode();  //run currently selected mode

  sCmd.readSerial();

  if (timer.check() == 1) {
    writeSettingstoMemory();
  }

}
