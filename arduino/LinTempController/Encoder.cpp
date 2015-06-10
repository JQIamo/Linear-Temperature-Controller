#include "Arduino.h"
#include "Encoder.h"

Encoder::Encoder(int pin_A, int pin_B, int pin_button, void (*button_press_event)(Encoder *this_encoder), void (*button_hold_event)(Encoder *this_encoder)) {
	pin_A_ = pin_A;
	pin_B_ = pin_B;
	pin_button_ = pin_button;

	bounce_time_ = 20;
	button_hold_time_ = 1500;

	num_step_sizes_ = 1;
	step_sizes_[0] = 1;
	step_size_labels_[0] = "LSB";
	step_size_index_ = 0;

	// Encoder button states:
	button_state_ = HIGH;
	prev_button_state_ = HIGH;
	button_held_ = false;
	bouncing_ = false;

	button_press_event_ = button_press_event;
	button_hold_event_ = button_hold_event;
}


void Encoder::init(int initial_position, int min_position, int max_position) {
	// Note: pinMod cannot be called multiple times without interfering with Teensy's interrupt routines. In the future these should be moved to setup().
	// pinMode(pin_A_,INPUT);
	// pinMode(pin_B_,INPUT);
	pinMode(pin_button_,INPUT);

	position_ = initial_position;
	prev_position_ = initial_position;
	min_position_ = min_position;
	max_position_ = max_position;
	prev_state_A_ = digitalRead(pin_B_);
}


void Encoder::define_step_sizes(int num_step_sizes, int *step_sizes, String *step_size_labels) {
	num_step_sizes_ = num_step_sizes;
	for(int i = 0; i < num_step_sizes_; i++) {
		step_sizes_[i] = step_sizes[i];
		step_size_labels_[i] = step_size_labels[i];
	}
}


void Encoder::reverse_polarity() {
	for(int i = 0; i < num_step_sizes_; i++) {
		step_sizes_[i] *= -1;
	}
}

void Encoder::interrupt() {
	bool current_state_A = digitalRead(pin_A_);
	bool current_state_B = digitalRead(pin_B_);

	// As the encoder knob is rotated clockwise, pin_A changes state slightly before pin_B.
	if (current_state_B == LOW) {
		if (prev_state_A_ == LOW && current_state_A == HIGH) {
			position_ += step_sizes_[step_size_index_];
		}
		else if (prev_state_A_ == HIGH && current_state_A == LOW) {
			position_ -= step_sizes_[step_size_index_];
		}
	}
	else {
		if (prev_state_A_ == HIGH && current_state_A == LOW) {
			position_ += step_sizes_[step_size_index_];
		}
		else if (prev_state_A_ == LOW && current_state_A == HIGH) {
			position_ -= step_sizes_[step_size_index_];
		}
	}

	prev_state_A_ = current_state_A;

	if (position_ < min_position_) {
		position_ = min_position_;
	}
	else if (position_ > max_position_) {
		position_ = max_position_;
	}
}


int Encoder::position(){
	return position_;
}


String Encoder::step_size_label() {
	return step_size_labels_[step_size_index_];
}


void Encoder::change_step_size() {
	step_size_index_ = (step_size_index_ + 1) % num_step_sizes_;
}


void Encoder::button_state() {
	button_state_ = digitalRead(pin_button_);

	if (button_state_ == LOW && prev_button_state_ == HIGH && bouncing_ == false) {
		button_press_time_ = millis();
		button_change_time_ = millis();
		prev_button_state_ = button_state_;
		bouncing_ = true;
	}

	if (button_state_ == HIGH && prev_button_state_ == LOW && bouncing_ == false) {
		button_release_time_ = millis();
		button_change_time_ = millis();
		prev_button_state_ = button_state_;
		bouncing_ = true;

		if (!button_held_) {
			button_press_event_(this);
		}
		else {
			button_held_ = false;
		}
	}

	if (button_state_ == LOW && (millis() - button_change_time_) > button_hold_time_) {
		button_held_ = true;
		button_hold_event_(this);
	}

	if ((millis() - button_change_time_) > bounce_time_) bouncing_ = false;

}

