/*
   Teensy_TempController.h  - Pin mappings for the Teensy_TempController
   
   Created by Neal Pisenti, 2015, modified by Ben Reschovsky
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

/*!	\file	Teensy_TempController.h
 *	\brief	Pin mappings for Teensy_TempController PCB.
 *	\details Define statements for pin mappings on Teensy frequency shield.
 *          See https://github.com/JQIamo/mcu-accessories for the breakout
 *          shield, and https://www.pjrc.com/teensy/teensy31.html for info
 *			on the Teensy dev board.
 *
 *	\author	Ben Reschovsky <breschov@umd.edu>
 *  \version 0.1
 *	\date	2015-06-08
 */


/*! \name Pins for analog monitor
 *
 */
//! @{

//#define VMON4 		A1
//#define ACT_T4		A2

//#define VMON3 		A3
//#define ACT_T3		A4

//#define VMON2 		A5
//#define ACT_T2		A6

#define VMON1 		A10
#define ACT_T1		A11

//Are these needed: MON_OUT (A14/DAC), 2.5 Voltage ref (AREF), 
#define ANALOG_OUT_PIN    A14

//! @}

/*! \name Pins for digital potentionmeters and DAC:
 *
 */
//! @{
#define CS_POT1			8
//#define CS_POT2			9
//#define CS_POT3			10
//#define CS_POT4			12
#define CS_DAC			14


//!@}

/*! \name Pins for LCD & Switches/encoders
 *
 */
//! @{

#define RST_LCD			7
#define RS_LCD			6
#define CS_LCD			5

#define ENC_A1			2
#define ENC_B1			1
#define ENC_SW1			0

#define ENC_B2			3
#define ENC_A2			4

//! @}

/*! \name Pins for SPI 
 *
 */
//! @{

//#define MOSI		11
//#define SCK			13
#define SCK_B			22
#define MOSI_B			23

//! @}
