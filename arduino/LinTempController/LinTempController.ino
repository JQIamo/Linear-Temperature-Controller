#include <EEPROMex.h>
#include <EEPROMVar.h>
#include <Metro.h>
#include <SPI.h>
#include <SerialCommand.h>
//#include "Teensy_TempController.h"
#include "Encoder.h"
#include "LCD.h"
#include "AD5262.h"
#include "AD56X4R.h"
#include "WTC3243.h"
#include "TempContSettings.h"
#include "Menu.h"


//Declare global variables:
byte current_ch = 0; //0-3 to correspond to channel 1-4
boolean inSettingsMenu = false;

//Declare variables used to store EEPROM addresses:
int Settings::addressTempSetPt[4];
int Settings::addressPgain[4];
int Settings::addressItc[4];
int Settings::addressMinTemp[4];
int Settings::addressMaxTemp[4];
int Settings::addressFirstSave;

SerialCommand sCmd;

LCD lcd(PinMappings::RST_LCD, PinMappings::RS_LCD, PinMappings::CS_LCD);

//Initialize Encoder Objects
Encoder enc_ch_select(PinMappings::ENC_A2, PinMappings::ENC_B2, PinMappings::ENC_SW2);
Encoder enc_enter_settings(PinMappings::ENC_A1, PinMappings::ENC_B1, PinMappings::ENC_SW1);
Encoder enc_exit_settings(PinMappings::ENC_A1, PinMappings::ENC_B1, PinMappings::ENC_SW1);
Encoder enc_voltage_mon(PinMappings::ENC_A1, PinMappings::ENC_B1, PinMappings::ENC_SW1);

Encoder enc_set_t[4] = {Encoder(PinMappings::ENC_A1, PinMappings::ENC_B1, PinMappings::ENC_SW1), Encoder(PinMappings::ENC_A1, PinMappings::ENC_B1, PinMappings::ENC_SW1), Encoder(PinMappings::ENC_A1, PinMappings::ENC_B1, PinMappings::ENC_SW1), Encoder(PinMappings::ENC_A1, PinMappings::ENC_B1, PinMappings::ENC_SW1)};
Encoder enc_set_pgain[4] = {Encoder(PinMappings::ENC_A1, PinMappings::ENC_B1, PinMappings::ENC_SW1), Encoder(PinMappings::ENC_A1, PinMappings::ENC_B1, PinMappings::ENC_SW1), Encoder(PinMappings::ENC_A1, PinMappings::ENC_B1, PinMappings::ENC_SW1), Encoder(PinMappings::ENC_A1, PinMappings::ENC_B1, PinMappings::ENC_SW1)};
Encoder enc_set_Itc[4] = {Encoder(PinMappings::ENC_A1, PinMappings::ENC_B1, PinMappings::ENC_SW1), Encoder(PinMappings::ENC_A1, PinMappings::ENC_B1, PinMappings::ENC_SW1), Encoder(PinMappings::ENC_A1, PinMappings::ENC_B1, PinMappings::ENC_SW1), Encoder(PinMappings::ENC_A1, PinMappings::ENC_B1, PinMappings::ENC_SW1)};
Encoder enc_set_minT[4] = {Encoder(PinMappings::ENC_A1, PinMappings::ENC_B1, PinMappings::ENC_SW1), Encoder(PinMappings::ENC_A1, PinMappings::ENC_B1, PinMappings::ENC_SW1), Encoder(PinMappings::ENC_A1, PinMappings::ENC_B1, PinMappings::ENC_SW1), Encoder(PinMappings::ENC_A1, PinMappings::ENC_B1, PinMappings::ENC_SW1)};
Encoder enc_set_maxT[4] = {Encoder(PinMappings::ENC_A1, PinMappings::ENC_B1, PinMappings::ENC_SW1), Encoder(PinMappings::ENC_A1, PinMappings::ENC_B1, PinMappings::ENC_SW1), Encoder(PinMappings::ENC_A1, PinMappings::ENC_B1, PinMappings::ENC_SW1), Encoder(PinMappings::ENC_A1, PinMappings::ENC_B1, PinMappings::ENC_SW1)};

//Initialize Menu Objects:
Menu main_menu(5);
Menu settings_menu(3);

//Initialize DAC
AD56X4R dac(PinMappings::CS_DAC, PinMappings::SCK_B, PinMappings::MOSI_B, Settings::dac_bits, Settings::dac_vref);

////Initialize array of temp controllers:
WTC3243 tempControllers[4] = {WTC3243(PinMappings::CS_POT1, PinMappings::SCK_B, PinMappings::MOSI_B, PinMappings::CS_DAC, Settings::dac_ch[0], PinMappings::VMON1, PinMappings::ACT_T1, Settings::pot_min[0], Settings::pot_max[0]), 
                              WTC3243(PinMappings::CS_POT2, PinMappings::SCK_B, PinMappings::MOSI_B, PinMappings::CS_DAC, Settings::dac_ch[1], PinMappings::VMON2, PinMappings::ACT_T2, Settings::pot_min[1], Settings::pot_max[1]), 
                              WTC3243(PinMappings::CS_POT3, PinMappings::SCK_B, PinMappings::MOSI_B, PinMappings::CS_DAC, Settings::dac_ch[2], PinMappings::VMON3, PinMappings::ACT_T3, Settings::pot_min[2], Settings::pot_max[2]),
                              WTC3243(PinMappings::CS_POT4, PinMappings::SCK_B, PinMappings::MOSI_B, PinMappings::CS_DAC, Settings::dac_ch[3], PinMappings::VMON4, PinMappings::ACT_T4, Settings::pot_min[3], Settings::pot_max[3])};
                
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
  enc_ch_select.init(50001,0,100000); //The initial position of this encoder is chosen to open the settings menu at the min temp submode

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
  //code to test here
  Serial.println("test...");
}

void printVersion() {
    Serial.print("Current loaded version: ");
  Serial.println(Settings::versionNum);
}

void calibrate_hold_event(Encoder *this_encoder) {
  
  boolean click = false;
  
  lcd.write("DPots=Max, Click",0x000);
  lcd.write("R Knob to Cont. ",0x040);
  
  //Set digipots to max R values to give user an opportunity to measure them:
  for (int i = 0 ; i < 4 ; i++) {
    tempControllers[i]._dPOT.writeDigiPOT(0,255);
    tempControllers[i]._dPOT.writeDigiPOT(1,255);
  }
  
  //while loop to stay here until click
  while (click == false) {
    if (enc_enter_settings.button_pressed() == true){  //This could be any of the encoders mapped to the right knob
      click = true; 
      lcd.clear();
      delay(1000);
    }
  }
  
  //Set digipots to min R values to give users an opportunity to measure them:
  lcd.write("DPots=Min, Click",0x000);
  lcd.write("R Knob to Cont. ",0x040);
  
  for (int i = 0 ; i < 4 ; i++) {
    tempControllers[i]._dPOT.writeDigiPOT(0,0);
    tempControllers[i]._dPOT.writeDigiPOT(1,0);
  }
  
  //while loop to stay here until click
  while (click == true) {
    if (enc_enter_settings.button_pressed() == true){  //This could be any of the encoders mapped to the right knob
      click = false; 
      lcd.clear();
      delay(1000);
    }
  }
  
  
}

void interruptWrapper() {
  if (main_menu.current_mode() == 0) {
    enc_set_t[current_ch].interrupt();
  } else if (main_menu.current_mode() == 1) {
    enc_set_pgain[current_ch].interrupt();
  } else if (main_menu.current_mode() == 2) {
    enc_set_Itc[current_ch].interrupt();
  } else if (main_menu.current_mode()  == 4 && settings_menu.current_mode() == 0) {
    enc_set_minT[current_ch].interrupt();
  } else if (main_menu.current_mode()  == 4 && settings_menu.current_mode() == 1) {
    enc_set_maxT[current_ch].interrupt();
  }
}

void chSelectInterruptWrapper() {
  enc_ch_select.interrupt();
}

void mode_temp_tune() {
  
  monitorTemp();
  
  enc_set_t[current_ch].button_events(); //look for button events
  enc_ch_select.button_events();
  
  //read temperature set by encoder and the current set point
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
  
  enc_set_pgain[current_ch].button_events(); //look for button events
  enc_ch_select.button_events();

  //read prop gain value set by encoder and the current prop gain
  byte newPgain = enc_set_pgain[current_ch].position();
  byte oldPgain = tempControllers[current_ch].getP();

  //compare old to new value, update only if necessary
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
  
  enc_set_Itc[current_ch].button_events(); //look for button events
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
  enc_voltage_mon.button_events();
  
  monitorTemp();

  float outV = tempControllers[current_ch].getOutputVoltage();

  char lineToDisplay[17];
  snprintf(lineToDisplay, 17, "Out Mon %6.3f V", outV);

  lcd.write(lineToDisplay, 0x040);
}

void mode_enter_settings() {
  lcd.write("Hold R knob to  ", 0x000);
  lcd.write("change settings ", 0x040);

  enc_enter_settings.button_events();
  
}

void mode_exit_settings() {
  lcd.write("Hold R knob to  ", 0x000);
  lcd.write(" exit settings. ", 0x040);

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
    for (int i = 0; i < 4 ; i++) {
      EEPROM.updateFloat(Settings::addressTempSetPt[i], tempControllers[i].getTempSetPt());
      EEPROM.updateByte(Settings::addressPgain[i], tempControllers[i].getP());
      EEPROM.updateFloat(Settings::addressItc[i], tempControllers[i].getI());
      EEPROM.updateFloat(Settings::addressMinTemp[i], tempControllers[i].getMinTemp());
      EEPROM.updateFloat(Settings::addressMaxTemp[i], tempControllers[i].getMaxTemp());
    }
    EEPROM.updateBit(Settings::addressFirstSave,0,0); //Toggle this bit from 1 (default if nothing has ever been written at this location) to 0 to indicate that we have made at least one save
    sei(); //set interrupts 
    delay(1500);
}

void setup() {
  // put your setup code here, to run once:
  
  Serial.begin(9600);

  //get address for EEPROM saved variables:
  for (int i = 0; i < 4 ; i++) {
    Settings::addressTempSetPt[i] = EEPROM.getAddress(sizeof(float));
    Settings::addressItc[i] = EEPROM.getAddress(sizeof(float));
    Settings::addressPgain[i] = EEPROM.getAddress(sizeof(byte));
    Settings::addressMinTemp[i] = EEPROM.getAddress(sizeof(float));
    Settings::addressMaxTemp[i] = EEPROM.getAddress(sizeof(float));
  }
  Settings::addressFirstSave = EEPROM.getAddress(sizeof(boolean));
  
  //Initialize communication pins
  pinMode(PinMappings::MOSI_B, OUTPUT);
  pinMode(PinMappings::SCK_B, OUTPUT);

  //Initialize DAC CS PIN
  pinMode(PinMappings::CS_DAC, OUTPUT);
  digitalWrite(PinMappings::CS_DAC, HIGH);

  //Set Up Analog Read Pins
  analogReference(EXTERNAL);
  analogReadResolution(Settings::analog_read_bits);
  analogReadAveraging(Settings::analog_read_avg); 

  //Set up encoder pins:
  pinMode(PinMappings::ENC_A1, INPUT);
  pinMode(PinMappings::ENC_B1, INPUT);
  pinMode(PinMappings::ENC_SW1, INPUT);
  pinMode(PinMappings::ENC_A2, INPUT);
  pinMode(PinMappings::ENC_B2, INPUT);
  pinMode(PinMappings::ENC_SW2, INPUT); 

  //Initialize the LCD
  SPI.begin();
  delay(100);
  lcd.init();
  
  //Turn on DAC's internal voltage reference:
  dac.setIntRefV(1);
  
  
  //Loop through the 4 channels, performing various set-up commands:
  for (int i = 0; i < 4; i++) {
    
    //Read min/max temperatures from memory (or if that value isn't valid, initialize to default values
    float initialMinTemp = EEPROM.readFloat(Settings::addressMinTemp[i]);
    float initialMaxTemp = EEPROM.readFloat(Settings::addressMaxTemp[i]);
    if (isnan(initialMinTemp) || initialMinTemp < -99.9 || initialMinTemp > 999.9){
      initialMinTemp = Settings::default_min_temp;
    }
    if (isnan(initialMaxTemp) || initialMaxTemp < -99.9 || initialMaxTemp > 999.9){
      initialMaxTemp = Settings::default_max_temp;
    } 
    //Initialize temperature controllers with min/max temp values
    tempControllers[i].init(Settings::bias_current[i], Settings::steinhart_A[i], Settings::steinhart_B[i], Settings::steinhart_C[i], initialMinTemp, initialMaxTemp);
    
    //Initialize encoders
    double minTemp_step_sizes[] = {0.1, 1.0, 5.0};
    String minTemp_step_labels[] = {"0.1", "1.0", "5.0"};
  
    enc_set_minT[i].init(initialMinTemp, -99.9, 999.9);
    enc_set_minT[i].define_step_sizes(3, minTemp_step_sizes, minTemp_step_labels);
    enc_set_minT[i].attach_button_press_event(incrementStepSize_pressEvent);
    enc_set_minT[i].attach_button_hold_event(dummy_hold_event);
    
    double maxTemp_step_sizes[] = {0.1, 1.0, 5.0};
    String maxTemp_step_labels[] = {"0.1", "1.0", "5.0"};

    enc_set_maxT[i].init(initialMaxTemp, -99.9, 999.9);
    enc_set_maxT[i].define_step_sizes(3, maxTemp_step_sizes, maxTemp_step_labels);
    enc_set_maxT[i].attach_button_press_event(incrementStepSize_pressEvent);
    enc_set_maxT[i].attach_button_hold_event(dummy_hold_event);


    
    //Read temp set point from memory:
    float initialTempSetPt = EEPROM.readFloat(Settings::addressTempSetPt[i]);
    if (isnan(initialTempSetPt) || initialTempSetPt < tempControllers[i].getMinTemp() || initialTempSetPt > tempControllers[i].getMaxTemp()){
      initialTempSetPt = Settings::default_set_temp;
    }
    
    //Initialize encoders:
    double temp_step_sizes[] = {0.01, 0.1, 1.0};
    String temp_step_labels[] = {".01", "0.1", "1.0"};
    
    enc_set_t[i].init(initialTempSetPt, tempControllers[i].getMinTemp(), tempControllers[i].getMaxTemp());
    enc_set_t[i].define_step_sizes(3, temp_step_sizes, temp_step_labels);
    enc_set_t[i].attach_button_press_event(incrementStepSize_pressEvent);
    enc_set_t[i].attach_button_hold_event(save_settings_hold_event); 
    
    //Update set temperatures:
    tempControllers[i].setTemp(initialTempSetPt, dac);




    //Read saved proportional gain
    byte initialPgain = EEPROM.readByte(Settings::addressPgain[i]);
    if (initialPgain < Settings::prop_min || initialPgain > Settings::prop_max || EEPROM.readBit(Settings::addressFirstSave,0) == 1){ //Use default value if read value is out of bounds OR if flag indicates that no saves have been made to memory yet
      initialPgain = Settings::default_prop;
    }
  
    //Initialize encoders
    double pgain_step_sizes[] = {1, 5};
    String pgain_step_labels[] = {"1", "5"};
  
    enc_set_pgain[i].init(initialPgain, Settings::prop_min, Settings::prop_max);
    enc_set_pgain[i].define_step_sizes(2, pgain_step_sizes, pgain_step_labels);
    enc_set_pgain[i].attach_button_press_event(incrementStepSize_pressEvent);
    enc_set_pgain[i].attach_button_hold_event(save_settings_hold_event); 
    
    //Update proportional gain settings:
    tempControllers[i].setP(initialPgain);




    //Read saved integrator time constant
    float initialItc = EEPROM.readFloat(Settings::addressItc[i]);
    if (isnan(initialItc) || initialItc < Settings::int_tc_min || initialItc > Settings::int_tc_max){
      initialItc = Settings::default_Itc;
    }
    
    //Initialize encoders
    double itc_step_sizes[] = {0.1, 0.5, 1.0};
    String itc_step_labels[] = {"0.1", "0.5", "1.0"};

    enc_set_Itc[i].init(initialItc, Settings::int_tc_min, Settings::int_tc_max);
    enc_set_Itc[i].define_step_sizes(3, itc_step_sizes, itc_step_labels);
    enc_set_Itc[i].attach_button_press_event(incrementStepSize_pressEvent);
    enc_set_Itc[i].attach_button_hold_event(save_settings_hold_event); 
    
    //Update integrator time constants:
    tempControllers[i].setI(initialItc);
    
  } //end for loop
  
  attachInterrupt(PinMappings::ENC_A2, chSelectInterruptWrapper, CHANGE);
  enc_ch_select.init(50000, 0, 100000);
  enc_ch_select.attach_button_press_event(incrementChannel_pressEvent); 
  enc_ch_select.change_hold_time(3000); //make hold time extra long
  enc_ch_select.attach_button_hold_event(calibrate_hold_event);
  
  enc_enter_settings.attach_button_hold_event(enter_settings_menu_hold_event);
  enc_enter_settings.attach_button_press_event(dummy_press_event);
  
  enc_exit_settings.attach_button_hold_event(exit_settings_menu_hold_event);
  enc_exit_settings.attach_button_press_event(dummy_press_event);
  
  enc_voltage_mon.attach_button_press_event(dummy_press_event);
  enc_voltage_mon.attach_button_hold_event(save_settings_hold_event);
  
  
  attachInterrupt(PinMappings::ENC_A1, interruptWrapper, CHANGE);
  main_menu.attach_mode(0, "Temperature", mode_temp_tune);
  main_menu.attach_mode(1, "Proportional Gain", mode_pgain_tune);
  main_menu.attach_mode(2, "Int Time Constant", mode_Itc_tune);
  main_menu.attach_mode(3, "Voltage Monitor", mode_out_mon);
  main_menu.attach_mode(4, "Enter Settings",mode_enter_settings);
  
  settings_menu.attach_mode(0,"Min Temperature",mode_min_temp);
  settings_menu.attach_mode(1,"Max Temperature",mode_max_temp);
  settings_menu.attach_mode(2,"Exit Settings",mode_exit_settings);



  sCmd.addCommand("T", test);
  sCmd.addCommand("Ver",printVersion);
  sCmd.setDefaultHandler(unrecognizedCmd);

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
