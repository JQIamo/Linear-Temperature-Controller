#ifndef TempContSettings_h
#define TempContSettings_h

namespace Settings
{
	const float analog_vref(2.5);  //value of voltage reference used for analog inputs
	const byte analog_read_bits(12); //precision for analog inputs
	const byte analog_read_avg(32); //number of analog measurements to average over, max of 32

	const float dac_vref(1.247); //value of DAC's internal voltage reference
        const byte dac_bits(12); //number of bits used by DAC
	
	const byte prop_max(65);  //max allowed proportional gain (V/A)
	const byte prop_min(1);  //min allowed proportional gain (V/A)
	const float int_tc_min(0.5); //min allowed integrator time constant (sec)
	const float int_tc_max(10.0); //max allowed integrator time constant (sec) (above 3.4sec each turn of rot encoder doesn't always change pot value)

        const unsigned long save_interval(3600000UL); //1 hr = 3600000 milliseconds
        
        extern int addressTempSetPt;
        extern int addressItc;
        extern int addressPgain;

}

namespace Ch1
{
        //Settings used to calibrate the digipot (resistance at max and min setting)
        const unsigned long pot_max(227500UL);
        const unsigned long pot_min(1100UL);
        
        //Define constants used to convert temp to voltage
        //Current values use the Steinhart-Hart model and the following thermistor:
        //Epcos B57862S0103F040 (digikey PN 495-2166-ND), 10kOhm, NTC
        const double steinhart_A(1.1254e-3);
        const double steinhart_B(2.3469e-4);
        const double steinhart_C(0.8584e-7);
        const double bias_current(100e-6);
        
        //DAC address (0 -> DAC A, 1 -> DAC B, 2 -> DAC C, 3 -> DAC D)
        const byte dac_ch(0);
        
        //Define min and max allowed commandable temperatures:
        const float min_temp(10.0);
        const float max_temp(35.0);
        
}
#endif
