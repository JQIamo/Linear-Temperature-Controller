/*
   AD5262.h  - Library for controlling Analog Devices AD5262 digital potentiometer
   
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

/*!	\file	AD5262.h
 *	\brief	Library for controlling AD5262 chips
 *	\details Library for controlling a AD5262 digital potentiomer. 
 *          See http://www.analog.com/media/en/technical-documentation/data-sheets/AD5260_5262.pdf
 *          for the datasheet. 
 *
 *	\author	Ben Reschovsky <breschov@umd.edu>
 *  \version 0.1
 *	\date	2015-06-10
 */

#ifndef AD5262_h
#define AD5262_h

#include "Arduino.h"

class AD5262
{
    public: 
        // Constructor function. 
        AD5262(byte, byte, byte, unsigned long, unsigned long);
		
	// write a value
	void writeDigiPOT(byte, byte);

        //write a specific resistance value (in ohms)
        void setR(byte, unsigned long);
		
	// read current value
	byte getVal(byte);

        // read current Resistance
        unsigned long getR(byte);
		
    private:
        // Instance variables that hold pinout mapping
	byte _CS_PIN, _SCK_PIN, _MOSI_PIN;
		
	// Instance variables that hold the current set value for channel 0 and channel 1
	byte _val[2];

        // Instance variables that are used to calibrate and store the desired proportional, integral, or resistance value
	unsigned long _R_MIN, _R_MAX, _R[2];

};

#endif
