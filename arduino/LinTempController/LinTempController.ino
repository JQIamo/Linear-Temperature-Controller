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
//#include <vector>



byte current_ch = 0; //0-3 to correspond to channel 1-4
boolean inSettingsMenu = false;

SerialCommand sCmd;

LCD lcd(RST_LCD, RS_LCD, CS_LCD);
//Encoder enc_test1(ENC_A1,ENC_B1,ENC_SW1);

//Initialize Encoder Objects; change these to arrays of encoders (one for each channel)
Encoder enc_ch_select(ENC_A2, ENC_B2, ENC_SW2);
Encoder enc_enter_settings(ENC_A1, ENC_B1, ENC_SW1);
Encoder enc_exit_settings(ENC_A1, ENC_B1, ENC_SW1);

//Encoder enc_set_t(ENC_A1, ENC_B1, ENC_SW1);
//Encoder enc_set_pgain(ENC_A1, ENC_B1, ENC_SW1);
//Encoder enc_set_Itc(ENC_A1, ENC_B1, ENC_SW1);
//Encoder enc_set_minT(ENC_A1, ENC_B1, ENC_SW1);
//Encoder enc_set_maxT(ENC_A1, ENC_B1, ENC_SW1);

Encoder enc_set_t[4] = {Encoder(ENC_A1, ENC_B1, ENC_SW1), Encoder(ENC_A1, ENC_B1, ENC_SW1), Encoder(ENC_A1, ENC_B1, ENC_SW1), Encoder(ENC_A1, ENC_B1, ENC_SW1)};
Encoder enc_set_pgain[4] = {Encoder(ENC_A1, ENC_B1, ENC_SW1), Encoder(ENC_A1, ENC_B1, ENC_SW1), Encoder(ENC_A1, ENC_B1, ENC_SW1), Encoder(ENC_A1, ENC_B1, ENC_SW1)};
Encoder enc_set_Itc[4] = {Encoder(ENC_A1, ENC_B1, ENC_SW1), Encoder(ENC_A1, ENC_B1, ENC_SW1), Encoder(ENC_A1, ENC_B1, ENC_SW1), Encoder(ENC_A1, ENC_B1, ENC_SW1)};
Encoder enc_set_minT[4] = {Encoder(ENC_A1, ENC_B1, ENC_SW1), Encoder(ENC_A1, ENC_B1, ENC_SW1), Encoder(ENC_A1, ENC_B1, ENC_SW1), Encoder(ENC_A1, ENC_B1, ENC_SW1)};
Encoder enc_set_maxT[4] = {Encoder(ENC_A1, ENC_B1, ENC_SW1), Encoder(ENC_A1, ENC_B1, ENC_SW1), Encoder(ENC_A1, ENC_B1, ENC_SW1), Encoder(ENC_A1, ENC_B1, ENC_SW1)};


//Initialize Menu Objects:
Menu main_menu(5);
Menu settings_menu(3);

//Initialize DAC and Temp Controller:
AD56X4R dac(CS_DAC, SCK_B, MOSI_B, Settings::dac_bits, Settings::dac_vref);
//WTC3243 tempCont(CS_POT1, SCK_B, MOSI_B, CS_DAC, Ch1::dac_ch, VMON1, ACT_T1, Ch1::pot_min, Ch1::pot_max);

////Initialize array of temp controllers:
WTC3243 tempControllers[4] = {WTC3243(CS_POT1, SCK_B, MOSI_B, CS_DAC, Settings::dac_ch[0], VMON1, ACT_T1, Settings::pot_min[0], Settings::pot_max[0]), 
                              WTC3243(CS_POT2, SCK_B, MOSI_B, CS_DAC, Settings::dac_ch[1], VMON2, ACT_T2, Settings::pot_min[1], Settings::pot_max[1]), 
                              WTC3243(CS_POT3, SCK_B, MOSI_B, CS_DAC, Settings::dac_ch[2], VMON3, ACT_T3, Settings::pot_min[2], Settings::pot_max[2]),
                              WTC3243(CS_POT4, SCK_B, MOSI_B, CS_DAC, Settings::dac_ch[3], VMON4, ACT_T4, Settings::pot_min[3], Settings::pot_max[3])};
                
//Initialize Metro object (deals with timing):
Metro timer(Settings::save_interval);

void unrecognizedCmd(const char *command) {
  Serial.println("UNRECOGNIZED COMMAND");
}

void incrementStepSize_pressEvent(Encoder *this_encoder) {
  this_encoder->increment_step_size();
}

void incrementChannel_pressEvent(Encoder *this_encoder) {
  current_ch = (current_ch + 1) % 4;
}

void save_settings_hold_event(Encoder *this_encoder) {
  //Save settings to EEPROM
  writeSettingstoMemory();
}

void dummy_hold_event(Encoder *this_encoder) {
  //Serial.println("here");
}

void dummy_press_event(Encoder *this_encoder) {
  //Serial.println("PUSH");
}

void enter_settings_menu_hold_event(Encoder *this_encoder) {
  lcd.clear();
  delay(750);
  inSettingsMenu = true;
  enc_ch_select.init(50001,0,100000);

}

void exit_settings_menu_hold_event(Encoder *this_encoder) {
  
  float minTemp = tempControllers[current_ch].getMinTemp();
  float maxTemp = tempControllers[current_ch].getMaxTemp();
  float setTemp = tempControllers[current_ch].getTempSetPt();
  
  //Do some error checking to make sure you selected good values
  if (minTemp > maxTemp) {
    lcd.write("     WARNING    ",0x00);
    lcd.write("  MinT > MaxT   ",0x40);
    delay(2000);
    enc_ch_select.init(50001,0,100000); //return to set min temp mode
  } else if (setTemp > maxTemp) {
    lcd.write("     WARNING    ",0x00);
    lcd.write("  SetT > MaxT   ",0x40);
    delay(2000);
    enc_ch_select.init(50002,0,100000); //return to set max temp mode
  } else if (setTemp < minTemp) {
    lcd.write("     WARNING    ",0x00);
    lcd.write("  SetT < MinT   ",0x40);
    delay(2000);
    enc_ch_select.init(50001,0,100000); //return to set min temp mode
  } else { //no warnings, continue back to main menu
    lcd.clear();
    delay(750);
    inSettingsMenu = false;
    //Re-initialize encoder in case min/max temps have changed
    enc_set_t[current_ch].init(enc_set_t[current_ch].position(), tempControllers[current_ch].getMinTemp(), tempControllers[current_ch].getMaxTemp());
    enc_ch_select.init(50000,0,100000);
  }
}

void test() {
  
}

void interruptWrapper() {
  if (main_menu.current_mode() == 0) {
    //enc_set_t.interrupt();
    enc_set_t[current_ch].interrupt();
  } else if (main_menu.current_mode() == 1) {
    //enc_set_pgain.interrupt();
    enc_set_pgain[current_ch].interrupt();
  } else if (main_menu.current_mode() == 2) {
    //enc_set_Itc.interrupt();
    enc_set_Itc[current_ch].interrupt();
  } else if (main_menu.current_mode()  == 4 && settings_menu.current_mode() == 0) {
    //enc_set_minT.interrupt();
    enc_set_minT[current_ch].interrupt();
  } else if (main_menu.current_mode()  == 4 && settings_menu.current_mode() == 1) {
    //enc_set_maxT.interrupt();
    enc_set_maxT[current_ch].interrupt();
  }
}

void chSelectInterruptWrapper() {
  enc_ch_select.interrupt();
}

void mode_temp_tune() {
  
  monitorTemp();
  
  enc_set_t[current_ch].button_events(); //look for button events (in this case changes step size)
  enc_ch_select.button_events();
  
  float newSetTemp = enc_set_t[current_ch].position();
  float oldSetTemp = tempControllers[current_ch].getTempSetPt();

  //compare old to new value, update only if necessary
  if (int(newSetTemp * 1000) != int(oldSetTemp * 1000)) { //since we're comparing floating pt #s, cast to ints to avoid problems)
    tempControllers[current_ch].setTemp(newSetTemp, dac);
  }

  //update display
  char lineToDisplay[17];
  snprintf(lineToDisplay, 17, "SetT(%3s) %5.2fC", enc_set_t[current_ch].step_size_label().c_str(), newSetTemp);
  //lcd.write("SetT(" + enc_set_t.step_size_label() + ") " + String(newSetTemp, 2) + "C", 0x040);
  lcd.write(lineToDisplay, 0x040);
}

void mode_pgain_tune() {
  
  monitorTemp();
  
  enc_set_pgain[current_ch].button_events(); //look for button events (in this case changes step size)
  enc_ch_select.button_events();

  //read prop gain value set by encoder and the current prop gain
  byte newPgain = enc_set_pgain[current_ch].position();
  byte oldPgain = tempControllers[current_ch].getP();

  if (newPgain != oldPgain) {
    tempControllers[current_ch].setP(newPgain);
  }

  //format display line to be 16 characters wide
  char lineToDisplay[17];
  //snprintf(lineToDisplay, 17,"Pgain  (%1s)    %2u",enc_set_pgain.step_size_label().c_str(),newPgain);
  snprintf(lineToDisplay, 17, "Pgain (%1s) %2u A/V", enc_set_pgain[current_ch].step_size_label().c_str(), newPgain);

  //update LCD
  lcd.write(lineToDisplay, 0x040);
}

void mode_Itc_tune() {
  
  monitorTemp();
  
  enc_set_Itc[current_ch].button_events(); //look for button events (in this case changes step size)
  enc_ch_select.button_events();

  //read integrator time constant value set by encoder and the current time constant
  float newItc = enc_set_Itc[current_ch].position();
  float oldItc = tempControllers[current_ch].getI();

  //compare old to new value, update only if necessary
  if (int(newItc * 100) != int(oldItc * 100)) {
    tempControllers[current_ch].setI(newItc);
  }

  char lineToDisplay[17];
  snprintf(lineToDisplay, 17, "Itc (%3s) %4.1f s", enc_set_Itc[current_ch].step_size_label().c_str(), newItc);

  lcd.write(lineToDisplay, 0x040);
}

void mode_out_mon() {
  
  enc_ch_select.button_events();
  
  monitorTemp();

  float outV = tempControllers[current_ch].getOutputVoltage();

  char lineToDisplay[17];
  snprintf(lineToDisplay, 17, "Out Mon %6.3f V", outV);

  lcd.write(lineToDisplay, 0x040);
}

void mode_enter_settings() {
  lcd.write("Hold R knob to  ", 0x000);
  lcd.write("change settings ", 0x040);
//  while ((int(enc_ch_select.position()) % 5) == 4) {
//    enc_enter_settings.button_events();
//  }
  enc_enter_settings.button_events();
  
}

void mode_exit_settings() {
  lcd.write("Hold R knob to  ", 0x000);
  lcd.write(" exit settings. ", 0x040);
//  while ((int(enc_ch_select.position()) % 5) == 4) {
//    enc_exit_settings.button_events();
//  }
  enc_exit_settings.button_events();
  
}

void mode_min_temp() {
   
  enc_set_minT[current_ch].button_events(); //look for button events (in this case changes step size)
  
  float newMinTemp = enc_set_minT[current_ch].position();
  float oldMinTemp = tempControllers[current_ch].getMinTemp();

  //compare old to new value, update only if necessary
  if (int(newMinTemp * 1000) != int(oldMinTemp * 1000)) { //since we're comparing floating pt #s, cast to ints to avoid problems)
    tempControllers[current_ch].setMinTemp(newMinTemp);
  }

  //update display
  char lineToDisplay[17];
  snprintf(lineToDisplay, 17, "MinT(%3s) %5.1fC", enc_set_minT[current_ch].step_size_label().c_str(), newMinTemp);
  //lcd.write("SetT(" + enc_set_t.step_size_label() + ") " + String(newSetTemp, 2) + "C", 0x040);
  lcd.write(lineToDisplay, 0x040);
  
  snprintf(lineToDisplay, 17, "  Ch%u Min Temp  ",current_ch+1);
  lcd.write(lineToDisplay, 0x000);
  
}

void mode_max_temp() {

  enc_set_maxT[current_ch].button_events(); //look for button events (in this case changes step size)
    
  float newMaxTemp = enc_set_maxT[current_ch].position();
  float oldMaxTemp = tempControllers[current_ch].getMaxTemp();

  //compare old to new value, update only if necessary
  if (int(newMaxTemp * 1000) != int(oldMaxTemp * 1000)) { //since we're comparing floating pt #s, cast to ints to avoid problems)
    tempControllers[current_ch].setMaxTemp(newMaxTemp);
  }

  //update display
  char lineToDisplay[17];
  snprintf(lineToDisplay, 17, "MaxT(%3s) %5.1fC", enc_set_maxT[current_ch].step_size_label().c_str(), newMaxTemp);
  //lcd.write("SetT(" + enc_set_t.step_size_label() + ") " + String(newSetTemp, 2) + "C", 0x040);
  lcd.write(lineToDisplay, 0x040);
  

  snprintf(lineToDisplay, 17, "  Ch%u Max Temp  ",current_ch+1);
  lcd.write(lineToDisplay, 0x000);
}

void monitorTemp() {
  float currTemp = tempControllers[current_ch].getActTemp();
  byte ch_num = current_ch + 1;
  char lineToDisplay[17];
  snprintf(lineToDisplay, 17, "Ch %u      %5.2fC", ch_num, currTemp);

  lcd.write(lineToDisplay, 0x00);

}

void writeSettingstoMemory() {
      //save settings to EEPROM
      
    lcd.write(" Settings Saved ",0x40);
    
    cli(); //clear interrupts, not sure if this is necessary 
    EEPROM.updateFloat(Settings::addressTempSetPt, tempControllers[current_ch].getTempSetPt());
    EEPROM.updateByte(Settings::addressPgain, tempControllers[current_ch].getP());
    EEPROM.updateFloat(Settings::addressItc, tempControllers[current_ch].getI());
    sei(); //set interrupts 
    delay(1500);
}

//Declare variables used to store EEPROM addresses:
int Settings::addressTempSetPt;
int Settings::addressPgain;
int Settings::addressItc;

//Declare calibration/setting variables:
float Ch1::min_temp;
float Ch1::max_temp;

float Settings::min_temp[4];
float Settings::max_temp[4];


void setup() {
  // put your setup code here, to run once:
  
  Serial.begin(9600);

  //get address for EEPROM saved variables:
  Settings::addressTempSetPt = EEPROM.getAddress(sizeof(float));
  Settings::addressItc = EEPROM.getAddress(sizeof(float));
  Settings::addressPgain = EEPROM.getAddress(sizeof(byte));
  
  Ch1::min_temp = 10.0; //temporary
  Ch1::max_temp = 35.0;
  
  for (int i = 0; i < 4 ; i++) {
    Settings::min_temp[i] = 10.0;
    Settings::max_temp[i] = 35.0;
  }

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
  pinMode(ENC_SW2, INPUT); 

  //Initialize the LCD
  SPI.begin();
  delay(100);
  lcd.init();

  //Initialize the temperature controllers:
  for (int i = 0; i < 4; i++) {
    tempControllers[i].init(Settings::bias_current[i], Settings::steinhart_A[i], Settings::steinhart_B[i], Settings::steinhart_C[i], Settings::min_temp[i], Settings::max_temp[i]);
  }

  //Read temp set pt from memory (or if that value is out of bounds, initialize to 20.0 C
  float initialTempSetPt = EEPROM.readFloat(Settings::addressTempSetPt);
  if (isnan(initialTempSetPt) || initialTempSetPt < Ch1::min_temp || initialTempSetPt > Ch1::max_temp){
    initialTempSetPt = 20.0;
  }
  
  double temp_step_sizes[] = {0.01, 0.1, 1.0};
  String temp_step_labels[] = {".01", "0.1", "1.0"};
  
  for (int i = 0; i<4 ; i++) {
    enc_set_t[i].init(initialTempSetPt, Settings::min_temp[i], Settings::max_temp[i]);
    enc_set_t[i].define_step_sizes(3, temp_step_sizes, temp_step_labels);
    enc_set_t[i].attach_button_press_event(incrementStepSize_pressEvent);
    enc_set_t[i].attach_button_hold_event(save_settings_hold_event); 
  }

  byte initialPgain = EEPROM.readByte(Settings::addressPgain);
  if (initialPgain < Settings::prop_min || initialPgain > Settings::prop_max){
    initialPgain = 50;
  }
  
  double pgain_step_sizes[] = {1, 5};
  String pgain_step_labels[] = {"1", "5"};
  
  for (int i = 0; i<4 ; i++) {
    enc_set_pgain[i].init(initialPgain, Settings::prop_min, Settings::prop_max);
    enc_set_pgain[i].define_step_sizes(2, pgain_step_sizes, pgain_step_labels);
    enc_set_pgain[i].attach_button_press_event(incrementStepSize_pressEvent);
    enc_set_pgain[i].attach_button_hold_event(save_settings_hold_event); 
  }

  float initialItc = EEPROM.readFloat(Settings::addressItc);
  if (isnan(initialItc) || initialItc < Settings::int_tc_min || initialItc > Settings::int_tc_max){
    initialItc = 1.0;
  }
  
  double itc_step_sizes[] = {0.1, 0.5, 1.0};
  String itc_step_labels[] = {"0.1", "0.5", "1.0"};
  for (int i = 0; i<4 ; i++) {
    enc_set_Itc[i].init(initialItc, Settings::int_tc_min, Settings::int_tc_max);
    enc_set_Itc[i].define_step_sizes(3, itc_step_sizes, itc_step_labels);
    enc_set_Itc[i].attach_button_press_event(incrementStepSize_pressEvent);
    enc_set_Itc[i].attach_button_hold_event(save_settings_hold_event); 
  }
  
  attachInterrupt(ENC_A2, chSelectInterruptWrapper, CHANGE);
  enc_ch_select.init(50000, 0, 100000);
  enc_ch_select.attach_button_press_event(incrementChannel_pressEvent); 
  enc_ch_select.attach_button_hold_event(dummy_hold_event);
  
  enc_enter_settings.attach_button_hold_event(enter_settings_menu_hold_event);
  enc_enter_settings.attach_button_press_event(dummy_press_event);
  
  enc_exit_settings.attach_button_hold_event(exit_settings_menu_hold_event);
  enc_exit_settings.attach_button_press_event(dummy_press_event);
  
  double minTemp_step_sizes[] = {0.1, 1.0, 5.0};
  String minTemp_step_labels[] = {"0.1", "1.0", "5.0"};
  for (int i = 0; i<4 ; i++) {
    enc_set_minT[i].init(Settings::min_temp[i], -99.9, 999.9);
    enc_set_minT[i].define_step_sizes(3, minTemp_step_sizes, minTemp_step_labels);
    enc_set_minT[i].attach_button_press_event(incrementStepSize_pressEvent);
  }
  
  double maxTemp_step_sizes[] = {0.1, 1.0, 5.0};
  String maxTemp_step_labels[] = {"0.1", "1.0", "5.0"};
  for (int i = 0; i<4 ; i++) {
    enc_set_maxT[i].init(Settings::max_temp[i], -99.9, 999.9);
    enc_set_maxT[i].define_step_sizes(3, maxTemp_step_sizes, maxTemp_step_labels);
    enc_set_maxT[i].attach_button_press_event(incrementStepSize_pressEvent);
  }
  
  attachInterrupt(ENC_A1, interruptWrapper, CHANGE);
  main_menu.attach_mode(0, "Temperature", mode_temp_tune);
  main_menu.attach_mode(1, "Proportional Gain", mode_pgain_tune);
  main_menu.attach_mode(2, "Int Time Constant", mode_Itc_tune);
  main_menu.attach_mode(3, "Voltage Monitor", mode_out_mon);
  main_menu.attach_mode(4, "Enter Settings",mode_enter_settings);
  
  settings_menu.attach_mode(0,"Min Temperature",mode_min_temp);
  settings_menu.attach_mode(1,"Max Temperature",mode_max_temp);
  settings_menu.attach_mode(2,"Exit Settings",mode_exit_settings);



  sCmd.addCommand("T", test);
  sCmd.setDefaultHandler(unrecognizedCmd);

  //Turn on DAC's internal voltage reference:
  dac.setIntRefV(1);
  
  //write initial settings
  for (int i = 0 ; i < 4 ; i++) {
    tempControllers[i].setTemp(initialTempSetPt, dac);
    tempControllers[i].setP(initialPgain);
    tempControllers[i].setI(initialItc);
  }

}

void loop() {
  // put your main code here, to run repeatedly:

  if (inSettingsMenu == false) {
    main_menu.switch_to_mode(int(enc_ch_select.position()) % 5);  //turning ch select knob changes the menu mode
    main_menu.run_mode();  //run currently selected mode
  } else {
    settings_menu.switch_to_mode(int(enc_ch_select.position()) % 3);
    settings_menu.run_mode(); //run settings mode
  }

  sCmd.readSerial();

  if (timer.check() == 1) {
    writeSettingstoMemory();
  }

}
