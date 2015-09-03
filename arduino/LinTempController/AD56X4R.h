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

#ifndef AD56X4R_h
#define AD56X4R_h

#include "Arduino.h"

class AD56X4R
{
    public: 
        // Constructor function. 
        AD56X4R(byte CS_PIN, byte SCK_PIN, byte MOSI_PIN, byte num_bits, double volt_ref, double volt_offset, boolean int_ext_ref);
		
		//function that writes the voltage val to the DAC channel ch (executes immediately):
		void setVoltage(byte ch, double Vout);
		
		//function to send DATA to the DAC, this version uses a software emulated SPI protocol:
		void writeDAC(byte command, byte address, word data);
		
		//function to enable or disable internal voltage reference (1/True = use internal, 0/False = use external)
		void setIntRefV(boolean onoff);
		
		//function to get current tuning value at specified DAC address
		word getVal(byte address);
		
		//function to get set voltage at specified DAC address
		double getV(byte address);
		
	private:
		//Instance variable to hold pin mappings:
		byte _CS_PIN, _SCK_PIN, _MOSI_PIN;
		
		//Instance variable to hold DAC precision (12, 14, or 16 bits) and refernce voltage:
		byte _dac_precision;
		double _volt_ref, _volt_max, _volt_offset;
		
		//Instance variables to hold DAC set values and voltages for channels 0-3:
		word _val[4];
		double _voltage[4];
};

#endif
	
		
		
		
