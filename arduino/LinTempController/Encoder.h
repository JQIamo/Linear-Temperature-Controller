#ifndef Encoder_h
#define Encoder_h

class Encoder {
 public:
	Encoder(int pin_A, int pin_B, int pin_button, void (*button_press_event)(Encoder *this_encoder), void (*button_hold_event)(Encoder *this_encoder));
	void init(int initial_position, int min_position, int max_position);
	void define_step_sizes(int num_step_sizes, int *step_sizes, String *step_size_labels);
	void reverse_polarity();
	int position();
	String step_size_label();
	void change_step_size();
	void change_hold_time(int hold_time);
	void interrupt();
	void button_state();

 private:
	// Encoder configuration variables:
	int pin_A_, pin_B_, pin_button_;

	int max_position_;
	int min_position_;

	int num_step_sizes_;
	int step_sizes_[5]; // Note: the max number of step sizes is 5.
	String step_size_labels_[5]; // Contains labels for each step size.
	int step_size_index_; // Selects the current step size.

	unsigned long bounce_time_; // (ms)
	unsigned long button_hold_time_; // (ms)

	// Rotation state variables:
	long position_, prev_position_;
	bool prev_state_A_;

	// Encoder button state variables:
	bool prev_button_state_;
	bool button_state_;
	bool button_held_;
	unsigned long button_press_time_;
	unsigned long button_release_time_;
	unsigned long button_change_time_;
	bool bouncing_;
	void (*button_press_event_)(Encoder *this_encoder);
	void (*button_hold_event_)(Encoder *this_encoder);
};

#endif

