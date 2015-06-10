#include "Arduino.h"
#include "Encoder.h"
#define Encoder_h

class Menu {
 public:
 	// Constructor and initialization methods:
	Menu(int num_of_modes, String *mode_labels);
	void attach_mode_select(void (*mode_select)(Menu* this_menu));
	void attach_mode(int mode_num, void (*mode_loop)());
	void attach_menu_knob(Encoder *knob_select);
	void attach_knob_to_mode(int mode, Encoder *knob);

	// Run a particular mode:
	void run_current_mode();
	void run_mode(int mode_number);

	// Switch various modes or increment mode number:
	void switch_to_select();
	void increment_mode_number();
	void switch_to_mode(int mode_number);

	// Return the menu_select_ state and current_mode_label:
	bool menu_select();
	int current_mode();
	String current_mode_label();
	String mode_label(int i);
	Encoder* current_knob();



 private:
 	int num_of_modes_;
 	int current_mode_;
 	bool menu_select_;
 	Encoder *knob_select_;
 	String mode_labels_[6]; // Note: the max number of modes is 6.
 	Encoder *knobs_[6]; //
 	void (*mode_loops_[6])();
 	void (*mode_select_)(Menu* this_menu);

};