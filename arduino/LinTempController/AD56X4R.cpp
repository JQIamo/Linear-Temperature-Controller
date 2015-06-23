/*
   AD56X4R.h  - Library for controlling Analog Devices AD56X4 quad channel DACs w/internal reference
   
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

/*!	\file	AD56X4R.h
 *	\brief	Library for controlling AD56X4R chips
 *	\details Library for controlling a AD5664R, AD5644R, and AD5624R (16, 14, 12 bits)
 *          See http://www.analog.com/media/en/technical-documentation/data-sheets/AD5624R_5644R_5664R.pdf
 *          for the datasheet. This library uses a software-emulated MOSI communication scheme but could
 *			easily be extended to use the built-in SPI interface. 
 *
 *	\author	Ben Reschovsky <breschov@umd.edu>
 *  \version 0.1
 *	\date	2015-06-22
 */

#include "Arduino.h"
#include "AD56X4R.h"

 /* CONSTRUCTOR */

// Constructor function; initializes communication pinouts
AD56X4R::AD56X4R(byte CS_PIN, byte SCK_PIN, byte MOSI_PIN, byte num_bits, double volt_ref){
        _CS_PIN = CS_PIN;
        _SCK_PIN = SCK_PIN;
        _MOSI_PIN = MOSI_PIN;
	
	if (num_bits == 12) {
		_dac_precision = 12;
	} else if (num_bits == 14) {
		_dac_precision = 14;
	} else if (num_bits == 16) {
		_dac_precision = 16;
	} else {
		Serial.println("WARNING:DAC object initialized with wrong number of bits of precision. Allowed values are 12, 14, or 16");
		_dac_precision = 12; //choose a reasonable default
	}
	_volt_ref = volt_ref;
	_volt_max = _volt_ref*2.0; //This factor of two is only valid for internal voltage reference
	
	// sets up the pinmodes for output
        pinMode(_CS_PIN, OUTPUT);
	//assume that MOSI and SCK pins are setup elsewhere since they are not unique to this class

        // defaults for pin logic levels
        digitalWrite(_CS_PIN, HIGH);	
}
		
/* PUBLIC CLASS FUNCTIONS */

//function that writes the voltage val to the DAC channel ch (executes immediately):
void AD56X4R::setVoltage(byte ch, double Vout){
	if (ch > 4) {
		Serial.println("WARNING:unrecognized DAC Channel, allowed values are 0,1,2,3,4");
		return; //channel isn't valid, return without doing anything
        }
		
		word maxVal = (1 << _dac_precision) - 1;
		word data;
		if (Vout > _volt_max) {
			data = maxVal; //command is larger than max voltage allowed, default to max voltage
			_val[ch] = maxVal;
			_voltage[ch] = _volt_max;
		} else if (Vout < 0) {
			data = 0; //command is less than zero, default to zero
			_val[ch] = 0;
			_voltage[ch] = 0.0;
		} else {
			data = round(maxVal*Vout/_volt_max); //DAC transfer function
			_val[ch] = data;
			_voltage[ch] = Vout;
		}
		
		byte bit_shift;
		if (_dac_precision == 12) {
			bit_shift = 4;
		} else if (_dac_precision == 14) {
			bit_shift = 2;
		} else {
			bit_shift = 0;
		}
		
		//bit shift (depends on DAC precision) is needed to move command bits to right location:
		data <<= bit_shift;
		writeDAC(B00011000,ch,data);		
}
		
//Send an actual command to the DAC. This version uses a software emulated SPI protocol
void AD56X4R::writeDAC(byte command, byte address, word data){

  //Make sure communication lines are initialized
  digitalWrite(_SCK_PIN, HIGH);
  digitalWrite(_CS_PIN,LOW);
  //digitalWrite(_MOSI_PIN, LOW);
  
  // The first byte is composed of two bits of nothing, then
  // the command bits, and then the address bits. Masks are
  // used for each set of bits and then the fields are OR'ed
  // together.
  byte first = (command & B00111000) | (address & B00000111);
  int send_bit;
	for (int i = 7; i >= 0; i--){
		send_bit = (first >> i) & 0x01;	// mask out i_th bit
										// start MSB first
		digitalWrite(_MOSI_PIN, send_bit);
		delayMicroseconds(1);
	
		digitalWrite(_SCK_PIN, LOW);
		delayMicroseconds(1);
		digitalWrite(_SCK_PIN, HIGH);
		delayMicroseconds(1);
	}
  
  // Send the data word. Must be sent byte by byte, MSB first.
  uint16_t second = data;
  	for (int i = 15; i >= 0; i--){
		send_bit = (second >> i) & 0x01;	// mask out i_th bit
										// start MSB first
		digitalWrite(_MOSI_PIN, send_bit);
		delayMicroseconds(1);
	
		digitalWrite(_SCK_PIN, LOW);
		delayMicroseconds(1);
		digitalWrite(_SCK_PIN, HIGH);
		delayMicroseconds(1);
	}
      
  // Set the Slave Select pin back to high since we are done
  // sending the command.
  digitalWrite(_CS_PIN,HIGH);
}

		
//enable or disable internal voltage reference (1/True = use internal, 0/False = use external)
void AD56X4R::setIntRefV(boolean onoff){
	writeDAC(B00111000,0,onoff); //enable or disable internal voltage reference
}
		
//function to get current tuning value at specified DAC address
word AD56X4R::getVal(byte address){
	return _val[address];
}
		
//function to get set voltage at specified DAC address
double AD56X4R::getV(byte address){
	return _voltage[address];
}
		
		
		
