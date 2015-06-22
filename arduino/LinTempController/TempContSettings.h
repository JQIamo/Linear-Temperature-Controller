#ifndef TempContSettings_h
#define TempContSettings_h

namespace Settings
{
	const float analog_vref(2.5);  //value of voltage reference used for analog inputs
	const byte analog_read_bits(12); //precision for analog inputs
	const byte analog_read_avg(10); //number of analog measurements to average over

	const float dac_vref(1.25); //value of DAC's internal voltage reference
	
	const byte prop_max(50);  //max allowed proportional gain (V/A)
	const byte prop_min(1);  //min allowed proportional gain (V/A)
	const float int_tc_min(0.5); //min allowed integrator time constant (sec)
	const float int_tc_max(15.0); //max allowed integrator time constant (sec)

}
#endif
