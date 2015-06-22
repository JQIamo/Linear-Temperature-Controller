/*
   WTC3243.h  - Library for controlling a WTC3243 linear temperature controller from Team Wavelength
   
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

/*!	\\file	WTC3242.h
 *	\\brief	Library for controlling WTC3243 temperature controller
 *	\\details  Depends on AD5262 to control P
 *          See http://www.teamwavelength.com/downloads/datasheets/wtc3243.pdf#page=1 for datasheet. 
 *
 *	\\author	Ben Reschovsky <breschov@umd.edu>
 *  \\version 0.1
 *	\\date	2015-06-10
 */

#ifndef WTC3243_h
#define WTC3243_h

#include "Arduino.h"
#include "AD5262.h"
#include "AD56X4R.h"

class WTC3243
{
	public:
		//Constructor function
		//Do I need the DAC_CS pin here or can I use a global variable?
		WTC3243(byte CS_POT, byte SCK_PIN, byte MOSI_PIN, byte CS_DAC_PIN , byte DAC_Ch, byte VMON, byte ACT_T, unsigned long R_MIN, unsigned long R_MAX, AD56X4R &dac);

		void init(double _BIAS_CURRENT, double _STEINHART_A, double _STEINHART_B, double _STEINHART_C, float _MIN_TEMP, float _MAX_TEMP);


		//other variables needed for calibration:
		// max/min resistance of pot: _R_MIN, _R_MAX;
		// max/min allowed value of P, I: _P_MAX, _P_MIN, _Itc_Max, _Itc_Min (global?)
		//actual value of 2.5 voltage reference: VREF	(global)
		//bits used for analog read: ANALOG_READ_BITS	(global)
		//bias current	
		//steinhard coefficients
		//min temp
		//max temp
		//Dac reference voltage (global)

		//Set the proportional gain in (A/V)
		void setP(byte);

		//Set the integrator time constant (in sec)
		void setI(float);

		//Read current prop gain
		byte getP();

		//read current integrator time constant 
		float getI();

		//Set desired temperature voltage
		void setTempV(double V);

		//Set desired temperature
		void setTemp(double temp);

                //Read voltage at specified analog input pin
                float readVoltage(byte analogPin);

		//Get the measured temperature
		float getActTemp();

		//Get the temperature set point
		float getTempSetPt();

		//Get the temperature set voltage();
		double getTempSetV();

		//Get the error signal (act - set temp)
		float getTempDev();

                //Instance digipot object
		AD5262 _dPOT;

                //Instance DAC object
                AD56X4R* _dac;

	private:
		//Instance variables that hold pin mappings
		byte _CS_POT, _SCK_PIN, _MOSI_PIN, _CS_DAC, _VMON_PIN, _ACT_T_PIN;

		//Instance variables used for DAC communication
		byte _DAC_CH;

		//Instance variables used to calibrate the max/min resistance of the digit:
		unsigned long _R_MIN, _R_MAX;

		//Instance variables that store the current proportional and integral settings:
		byte _Pgain, _P_MAX, _P_MIN;
		float _Itc, _Itc_MAX, _Itc_MIN;

		//Instance variables used to store the temperature set voltage:
		double _setV;

		//Instance variables used to calibrate the voltage across the thermistor to a temperature
		double _BIAS_CURRENT, _STEINHART_A, _STEINHART_B, _STEINHART_C;

		//Instance variables used to store the set, actual, min/max temperatures:
		float _actT, _setT, _MAX_TEMP, _MIN_TEMP, _tempDev;

};

#endif

