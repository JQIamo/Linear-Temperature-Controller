#include "Menu.h"

Menu::Menu(int num_of_modes, String *mode_labels) {
	num_of_modes_ = num_of_modes;
	menu_select_ = false;
	current_mode_ = 0;
	for(int i = 0; i < num_of_modes_; i++) {
		mode_labels_[i] = mode_labels[i];
	}
}

void Menu::attach_mode(int mode_num, void (*mode_loop)()) {
	mode_loops_[mode_num] = mode_loop;
}

void Menu::attach_menu_knob(Encoder *knob_select) {
	knob_select_ = knob_select;
}

void Menu::attach_knob_to_mode(int mode, Encoder *knob) {
	knobs_[mode] = knob;
}

void Menu::attach_mode_select(void (*mode_select)(Menu *this_menu)) {
	mode_select_ = mode_select;
}

void Menu::run_current_mode() {
	if (menu_select_ == false) {
		(mode_loops_[current_mode_])();
	}
	else {
		mode_select_(this);
	}
}

void Menu::run_mode(int mode_number) {
	mode_loops_[mode_number]();
}

void Menu::switch_to_mode(int mode_number) {
	menu_select_ = false;
	current_mode_ = mode_number % num_of_modes_;
}

void Menu::switch_to_select() {
	menu_select_ = true;
}

void Menu::increment_mode_number() {
	current_mode_ = (current_mode_ + 1) % num_of_modes_;
}

bool Menu::menu_select() {
	return menu_select_;
}

int Menu::current_mode() {
	return current_mode_;
}

String Menu::current_mode_label() {
	return mode_labels_[current_mode_];
}

String Menu::mode_label(int i) {
	return mode_labels_[i];
}

Encoder* Menu::current_knob() {
	if (menu_select_) {
		return knob_select_;
	}
	else {
		return knobs_[current_mode_];
	}
}
