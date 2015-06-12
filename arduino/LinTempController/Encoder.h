#ifndef Encoder_h
#define Encoder_h

class Encoder {
 public:
	Encoder(int pin_A, int pin_B, int pin_button);
	void init(double initial_position, double min_position, double max_position);
	// Note: pinMode(pin_A, INPUT) and pinMod(pin_B, INPUT) should be called (once, and only once if these pins handle multiple instances of Encoder) somewhere in setup().

	// To get the encoder's states without attached event functions, use the following:
	void interrupt();
	double position();
	bool button_pressed();
	bool button_held();

	// If button events are attached, these will call the attached events. This is preferred in simple programs using a standalone encoder. If encoder works with an instance of Menu, then it's best to let the menu handle the event functions.
	void attach_button_press_event(void (*button_press_event)(Encoder *this_encoder));
	void attach_button_hold_event(void (*button_hold_event)(Encoder *this_encoder));
	void button_events();
	void button_press_event();
	void button_hold_event();

	// To change polarity or step size at initialization, use these:
	void reverse_polarity();
	void change_step_size(double step_size, int step_size_index);
	void change_hold_time(int hold_time);

	// To allow the user to cycle through step sizes, use the following:
	void define_step_sizes(int num_step_sizes, double *step_sizes, String *step_size_labels);
	String step_size_label();
	void increment_step_size();


 private:
	// Encoder configuration variables:
	int pin_A_, pin_B_, pin_button_;

	double max_position_;
	double min_position_;

	int num_step_sizes_;
	double step_sizes_[5]; // Note: the max number of step sizes is 5.
	String step_size_labels_[5]; // Contains labels for each step size.
	int step_size_index_; // Selects the current step size.

	unsigned long bounce_time_; // (ms)
	unsigned long hold_time_; // (ms)

	// Rotation state variables:
	double position_; //prev_position_;
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

