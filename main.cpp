#include "ark.h"

int main()
{
	int angle = 20;
	std::atomic<bool> state = true;
	windows_parameters param;
	param.set_size_window();
	ball B(param,angle);
	desk D(param);
	field field_ark(B, D);
	field_ark.draw_field(param);
	field_ark.T.set_settings_tty();

	std::thread thread_1 ([&field_ark,&state]() {field_ark.T.read_data(state);});
	std::thread thread_2 ([&field_ark, &state]() {field_ark.move_ball_and_desk(state);});

	thread_1.join();
	thread_2.join();
 	return 0;
}