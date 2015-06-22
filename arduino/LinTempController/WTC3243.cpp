/*
   WTC3243.cpp  - Library for controlling a WTC3243 linear temperature controller from Team Wavelength
   
   Created by Ben Reschovsky, 2015
   JQI - Joint Quantum Institute

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

/*!	\\file	WTC3242.cpp
 *	\\brief	Library for controlling WTC3243 temperature controller
 *	\\details  Depends on AD5262 to control proportional gain/integrator time constant
 *          See http://www.teamwavelength.com/downloads/datasheets/wtc3243.pdf#page=1 for datasheet. 
 *
 *	\\author	Ben Reschovsky <breschov@umd.edu>
 *  \\version 0.1
 *	\\date	2015-06-10
 */

#include "Arduino.h"
#include "WTC3243.h"
#include "TempContSettings.h"
#include "AD5262.h"

WTC3243::WTC3243(byte CS_POT, byte SCK_PIN, byte MOSI_PIN, byte CS_DAC_PIN, byte DAC_CH, byte VMON_PIN, byte ACT_T_PIN, unsigned long R_MIN, unsigned long R_MAX) : _dPOT(CS_POT, SCK_PIN, MOSI_PIN, R_MAX, R_MIN)
{ 
	_CS_POT = CS_POT;
	_SCK_PIN = SCK_PIN;
	_MOSI_PIN = MOSI_PIN;

	_CS_DAC = CS_DAC_PIN;
	_DAC_CH = DAC_CH;
	_VMON_PIN = VMON_PIN;
	_ACT_T_PIN = ACT_T_PIN;

        _R_MIN = R_MIN;
	_R_MAX = R_MAX;

	_P_MIN = max(100.0/((100000.0/_R_MIN)+1.0),Settings::prop_min);
	_P_MAX = min(100.0/((100000.0/_R_MAX)+1.0),Settings::prop_max);
    	_Itc_MIN = max(0.53*(100000.0/_R_MAX + 1.0),Settings::int_tc_min);
    	_Itc_MAX = min(0.53*(100000.0/_R_MIN + 1.0),Settings::int_tc_max);

	//pin modes for CS lines should be done in the device class constructor
	//pin modes for MOSI/SCK pins should be done in setup()
	
	//set up pin modes for analog inputs
	pinMode(_VMON_PIN,INPUT);
	pinMode(_ACT_T_PIN,INPUT);

  	//set up analog read settings in setup()
	//analogReference(EXTERNAL);
  	//analogReadResolution(ANALOG_READ_BITS);	
}

/* Public Class Functions */

//initialize object with calibration variables that could (but probably won't) change from channel to channel

void WTC3243::init(double BIAS_CURRENT, double STEINHART_A, double STEINHART_B, double STEINHART_C, float MIN_TEMP, float MAX_TEMP){

	_BIAS_CURRENT = BIAS_CURRENT;
	_STEINHART_A = STEINHART_A;
	_STEINHART_B = STEINHART_B;
	_STEINHART_C = STEINHART_C;
	_MIN_TEMP = MIN_TEMP;
	_MAX_TEMP = MAX_TEMP;

}

/* Public Class Functions */

//Set the proportional gain in (A/V)
//From WTC3243 datasheet Rp = (100,000 / (100/PGAIN)-1 ), or PGAIN = 100/( 100,000/RP + 1 ) (A/V)
void WTC3243::setP(byte P){
	unsigned long Rp;
	if (P > _P_MAX) {
		_Pgain = _P_MAX;
		P = _P_MAX;
	} else if (P < _P_MIN) {
		_Pgain = _P_MIN;
		P = _P_MIN;
	} else {
		_Pgain = P;
	}
	
	Rp = 100000.0/( (100.0/P) - 1.0 );
	_dPOT.setR(0,Rp);
}
	

//Set the integrator time constant (in sec)
//From WTC3243 datasheet Rp = (100,000 / (1.89*Itc-1 ), or Itc = 0.53*( 100,000/RI + 1 ) (seconds)
void WTC3243::setI(float Itc){
	unsigned long Ri;
  	if (Itc > _Itc_MAX) {
		_Itc = _Itc_MAX;
		Itc = _Itc_MAX;
	} else if (Itc < _Itc_MIN) {
		_Itc = _Itc_MIN;
		Itc = _Itc_MIN;
	} else {
 		_Itc = Itc;
   
	}

	Ri = 100000.0/( 1.89*Itc - 1.0 );
	_dPOT.setR(1,Ri);
}

//Read current prop gain
byte WTC3243::getP(){
	return _Pgain;
}

//read current integrator time constant 
float WTC3243::getI(){
	return _Itc;
}

//Set desired temperature voltage
void WTC3243::setTempV(double V){
	_setV = V;
	//finalize this after making DAC class
}

//Set desired temperature
//finalize after making DAC class
void WTC3243::setTemp(double T){

	  //Make sure desired temp is within the desired range
	if (T > _MAX_TEMP){
		T = _MAX_TEMP;
	} else if (T < _MIN_TEMP) {
 		T = _MIN_TEMP;
	}

	_setT = T;
  
  	//convert from C to K:
  	T += 273.15;
  
  	//calculate resistance of thermistor at desired temp using the inverse of the Steinhart-Hart equation:
  	double x = (_STEINHART_A-(1/T))/_STEINHART_C;
  	double y = sqrt( pow(_STEINHART_B/(3*_STEINHART_C),3) + pow(x/2,2));
  	double R = exp(pow(y-x/2,1.0/3) - pow(y+x/2,1.0/3));
  
  	double setV = _BIAS_CURRENT*R;
  
  	//setDAC(_DAC_CH,setV);

}

//read voltage at specified analog pin
float readVoltage(byte analogPin){
  int val = analogRead(analogPin);
  int maxReading = (1 << Settings::analog_read_bits) - 1;
  float voltage = Settings::analog_vref*(float(val)/maxReading);
  return voltage;
}

//Get the measured temperature
float WTC3243::getActTemp(){

  	//get voltage across the thermistor:
	float V = readVoltage(_ACT_T_PIN);
  
 	//calculate resistance of thermistor
  	double R = V/_BIAS_CURRENT;
  
	//calculate temp based on Steinhart-Hart equation: 1/T = A + B ln(R) + C ln(R)^3
  	double T  = 1/(_STEINHART_A + _STEINHART_B*log(R) + _STEINHART_C*pow(log(R),3.0));
  
  	//convert from K to C
  	T -= 273.15;

	_actT = T;
  
  	return T;

}

//Get the temperature set point
float WTC3243::getTempSetPt(){

	return _setT;
}

//Get the temperature set voltage();
double WTC3243::getTempSetV(){
	return _setV;
}

//Get the error signal (act - set temp)
float WTC3243::getTempDev(){

	getActTemp();
	_tempDev = _actT - _setT;

	return _tempDev;
}

