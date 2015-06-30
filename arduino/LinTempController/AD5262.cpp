/*
   AD5262.cpp  - Library for controlling Analog Devices AD5262 digital potentiometer
   
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

/*!	\file	AD5262.cpp
 *	\brief	Library for controlling AD5262 chips
 *	\details Library for controlling a AD5262 digital potentiomer. 
 *          See http://www.analog.com/media/en/technical-documentation/data-sheets/AD5260_5262.pdf
 *          for the datasheet. 
 *
 *	\author	Ben Reschovsky <breschov@umd.edu>
 *  \version 0.1
 *	\date	2015-06-10
 */
 
 #include "Arduino.h"
 #include "AD5262.h"
 
 /* CONSTRUCTOR */

// Constructor function; initializes communication pinouts
AD5262::AD5262(byte CS_PIN, byte SCK_PIN, byte MOSI_PIN, unsigned long R_MAX, unsigned long R_MIN)
{
    _CS_PIN = CS_PIN;
    _SCK_PIN = SCK_PIN;
    _MOSI_PIN = MOSI_PIN;
    _R_MAX = R_MAX;
    _R_MIN = R_MIN;
    //_P_MIN = 100.0/((100000.0/_R_MIN)+1.0);
    //_P_MAX = 100.0/((100000.0/_R_MAX)+1.0);
    //_Itc_MIN = 0.53*(100000.0/_R_MAX + 1.0);
    //_Itc_MAX = 0.53*(100000.0/_R_MIN + 1.0);

    // sets up the pinmodes for output
    pinMode(_CS_PIN, OUTPUT);
	//assume that MOSI and SCK pins are setup elsewhere since they are not unique to this class

    // defaults for pin logic levels
    digitalWrite(_CS_PIN, HIGH);
    
}

/* PUBLIC CLASS FUNCTIONS */

// writeDigiPot(address, value) writes the 8bit number value to the chip register. Address controls which register is updated.
// Allowed values are 0 or 1.
// This version uses a software emulated/bit-bang version of SPI. Future versions could use the built-in SPI library

void AD5262::writeDigiPOT(byte address, byte value){

  //Check to make sure address is valid, if not return without doing anything
  if (address > 1) {
	return;
  }
	
  //Make sure communication lines are initialized
  digitalWrite(_SCK_PIN, LOW);
  digitalWrite(_CS_PIN,LOW);
  //digitalWrite(_MOSI_PIN, LOW);
  
  uint16_t data = word(address,value);
  _val[address] = value;
    
  int send_bit;
	for (int i = 8; i >= 0; i--){
		send_bit = (data >> i) & 0x01;	// mask out i_th bit
										// start MSB first
		digitalWrite(_MOSI_PIN, send_bit);
		delayMicroseconds(1);
	
		digitalWrite(_SCK_PIN, HIGH);
		delayMicroseconds(1);
		digitalWrite(_SCK_PIN, LOW);
		delayMicroseconds(1);
	}
  //Set chip select pin back to high
  digitalWrite(_CS_PIN,HIGH);
}

//Write a specific resistance value (R is in ohms, it is calibrated to _R_MAX, which should be measured for every device)
void AD5262::setR(byte address, unsigned long R){
  byte val;
  if (R > _R_MAX){
    val = 0;
    _R[address] = _R_MAX;
  } else if (R < _R_MIN) {
    val = 255;
    _R[address] = _R_MIN;
  } else {
    val = round((-255.0/_R_MAX)*R+255.0);
    _R[address] = R;
  }
  
  writeDigiPOT(address,val);
}


//getValue - returns current set  value
byte AD5262::getVal(byte address){
  return _val[address];
}

//getR - returns current resistance
unsigned long AD5262::getR(byte address){
  return _R[address];
}


