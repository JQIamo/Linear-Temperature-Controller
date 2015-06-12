#include "Menu.h"


Menu::Menu(int num_of_modes) {
	num_of_modes_ = num_of_modes;
	in_menu_ = false;
	current_mode_ = 0;
}


/*
	Setup Methods
*/
void Menu::attach_mode(String menu_name, void (*menu_function)(Menu* this_menu)) {
	menu_name_ = menu_name;
	menu_function_ = menu_function;
}

void Menu::attach_mode(int mode_num, String mode_name, void (*mode_function)()) {
	mode_names_[mode_num] = mode_name;
	mode_functions_[mode_num] = mode_function;
}

void Menu::attach_mode(int mode_num, String submenu_name, Menu *submenu) {
	// mode_names_[mode_num] = submenu_name;
	// mode_functions_[mode_num] = submenu->point_to_self;
}


/*
	Execute modes
*/
void Menu::run_mode() {
	if (in_menu_ == false) {
		mode_functions_[current_mode_]();
	}
	else {
		menu_function_(this);
	}
}

void Menu::run_mode(int mode_number) {
	mode_functions_[mode_number]();
}


/*
	Switching between modes
*/
void Menu::switch_to_menu() {
	in_menu_ = true;
}

void Menu::increment_mode_number() {
	current_mode_ = (current_mode_ + 1) % num_of_modes_;
}

void Menu::switch_to_mode(int mode_number) {
	in_menu_ = false;
	current_mode_ = mode_number % num_of_modes_;
}


/*
	Getting the menu state
*/
bool Menu::in_menu() {
	return in_menu_;
}

int Menu::current_mode() {
	return current_mode_;
}

String Menu::mode_name() {
	if (in_menu_)
		return menu_name_;
	else {
		return mode_names_[current_mode_];
	}
}

String Menu::mode_name(int i) {
	return mode_names_[i];
}

Menu* Menu::point_to_self() {
	return this;
}

