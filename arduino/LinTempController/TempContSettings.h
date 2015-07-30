#ifndef TempContSettings_h
#define TempContSettings_h

namespace Settings
{
        //single variables are global settings, variables in an array may vary for each channel:
        
	const float analog_vref(2.4974);  //value of voltage reference used for analog inputs
	const byte analog_read_bits(12); //precision for analog inputs
	const byte analog_read_avg(32); //number of analog measurements to average over, max of 32

	const float dac_vref(1.2478); //value of DAC's internal voltage reference
        const byte dac_bits(12); //number of bits used by DAC
	
	const byte prop_max(65);  //max allowed proportional gain (V/A)
	const byte prop_min(1);  //min allowed proportional gain (V/A)
	const float int_tc_min(0.5); //min allowed integrator time constant (sec)
	const float int_tc_max(10.0); //max allowed integrator time constant (sec) (above 3.4sec each turn of rot encoder doesn't always change pot value)

        const unsigned long save_interval(3600000UL); //1 hr = 3600000 milliseconds
        
        extern int addressTempSetPt[4];
        extern int addressItc[4];
        extern int addressPgain[4];
        extern int addressMinTemp[4];
        extern int addressMaxTemp[4];
        extern int addressFirstSave;
        
        //extern boolean firstSave;
        
        //extern float min_temp[4];
        //extern float max_temp[4];
        
        //Define constants used to convert temp to voltage
        //Current values use the Steinhart-Hart model and the following thermistor:
        //Epcos B57862S0103F040 (digikey PN 495-2166-ND), 10kOhm, NTC
        const double steinhart_A[4] = {1.1254e-3,  1.1254e-3,  1.1254e-3,  1.1254e-3};
        const double steinhart_B[4] = {2.3469e-4, 2.3469e-4, 2.3469e-4, 2.3469e-4};
        const double steinhart_C[4] = {0.8584e-7, 0.8584e-7, 0.8584e-7, 0.8584e-7};
        const double bias_current[4] = {100e-6, 100e-6, 100e-6, 100e-6};
        
        //Settings used to calibrate the digipot (resistance at max and min setting)
        const unsigned long pot_max[4] = {227500UL, 227500UL, 227500UL, 227500UL};
        const unsigned long pot_min[4] = {1100UL, 1100UL, 1100UL, 1100UL};
        
        //DAC address (0 -> DAC A, 1 -> DAC B, 2 -> DAC C, 3 -> DAC D)
        const byte dac_ch[4] = {0, 1, 2, 3};
        
        //Default initial settings if none are read from memory:
        const float default_min_temp(10.0);
        const float default_max_temp(40.0);
        const float default_set_temp(20.0);
        const byte default_prop(50);
        const float default_Itc(1.0);
        
}

namespace PinMappings
{
  const byte VMON4(A1);
  const byte ACT_T4(A2);
  const byte VMON3(A3);
  const byte ACT_T3(A4);
  const byte VMON2(A5);
  const byte ACT_T2(A6);
  const byte VMON1(A10);
  const byte ACT_T1(A11);
  
  const byte CS_POT1(8);
  const byte CS_POT2(9);
  const byte CS_POT3(10);
  const byte CS_POT4(12);
  const byte CS_DAC(14);
  
  const byte RST_LCD(7);
  const byte RS_LCD(6);
  const byte CS_LCD(5);
  
  const byte ENC_A1(2);
  const byte ENC_B1(1);
  const byte ENC_SW1(0);
  
  const byte ENC_A2(4);
  const byte ENC_B2(3);
  const byte ENC_SW2(21);
  
  const byte SCK_B(22);
  const byte MOSI_B(23);
  
}

#endif
