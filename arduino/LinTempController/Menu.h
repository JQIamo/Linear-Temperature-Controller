#ifndef Menu_h
#define Menu_h
#include "Arduino.h"

class Menu {
 public:
	Menu(int num_of_modes);
	void attach_mode(String menu_name, void (*mode_select)(Menu* this_menu));
	void attach_mode(int mode_num, String mode_name, void (*mode_loop)());
	void attach_mode(int mode_num, String submenu_name, Menu *submenu);

	// Run the current mode or a given mode once:
	void run_mode();
	void run_mode(int mode_number);

	// Switch various modes or increment mode number:
	void switch_to_menu();
	void increment_mode_number();
	void switch_to_mode(int mode_number);

	// Return the in_menu_ state, mode names, and other info:
	bool in_menu();
	int current_mode();
	String mode_name();
	String mode_name(int i);
	Menu* point_to_self();

 private:
 	int num_of_modes_;
 	int current_mode_;
 	bool in_menu_;
 	String menu_name_;
 	String mode_names_[15]; // Note: the max number of modes is 15.
 	void (*menu_function_)(Menu* this_menu);
 	void (*mode_functions_[15])();
};

#endif