#include "ark.h"

int main()
{
	int angle = 20;
	std::atomic<bool> state = true;		// for correct end of loop
	windows_parameters param;
	param.set_size_window();
	ball B(param,angle);
	desk D(param);
	Game game(B, D,param);
	game.T.set_settings_tty();
	
	std::thread thread_1;
	std::thread thread_2;
	do
	{
		std::thread thread_1 ([&game, &state]() {game.T.read_data(state);});
		std::thread thread_2 ([&game, &state]() {game.move_ball_and_desk(state);});
		thread_1.join();
		thread_2.join();
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	} while (game.is_running(state));
	
 	return 0;
}