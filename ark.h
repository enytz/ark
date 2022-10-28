#pragma once
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <math.h>
#include <string>

#include <ncurses.h>

#include <fcntl.h>      // Contains file control like O_RDWR
#include <errno.h>      // Error integer
#include <termios.h>    // Contains POSIX terminal control definitions
#include <unistd.h>     // write(), read(), close()

#define M_PI				3.14159265358979323846
#define REPS_FOR_QUIT		30
#define S_VERT				40
#define S_HOR				60

double degtorad(int& angle);
std::string itos(int val);
int char_to_int(const char* buf);
void game_over(std::atomic<bool>& state, int size_hor=S_HOR, int size_vert=S_VERT);

struct windows_parameters
{
	windows_parameters()
		:size_hor(S_HOR), size_vert(S_VERT){}
	~windows_parameters() {std::cout<<" ~windows_param "; std::cout.flush(); std::cout<<"\033[8;40;80;t";}
	int get_size_hor() const { return size_hor; }
	int get_size_vert() const { return size_vert; }
	void set_size_window() {std::cout<<"\033[8;"<<itos(size_vert)<<';'<<itos(size_hor)<<";t";}
private:
	int size_hor;
	int size_vert;
};

struct tty_init
{
    public:
    	tty_init()
    		:buf_int(0)
     		{serial_port = open("/dev/ttyACM0",O_RDWR);}
    	~tty_init() {std::cout<<"~tty"; std::cout.flush(); close(serial_port);}
    	void set_settings_tty();
    	void read_data(std::atomic<bool>& state);
    	int get_value_sensor() const;
		void write_log_sensor_data(const char* buf);
    private:
        int serial_port;
        termios tty;
        char buf[3];            // max bit depth for sensor (~300 cm)
        int buf_int;
        

};

struct desk
{
	desk(windows_parameters& param_,int size_desk_ = 10)
		:param(param_), x(param_.get_size_hor() / 2 - 1- size_desk_/2), size_desk(size_desk_) {}
	void draw_desk();
	void move_desk();
	void move_desk_with_sensor(int value_sensor, int& cnt);
	int get_size_desk() const { return size_desk; }
	int get_coordX() const { return x; }
private:
	windows_parameters param;
	int x;
	int size_desk;
};


struct ball
{
	ball(windows_parameters& param_,int angle_ = 30)
		:param(param_),angle(angle_),x(param.get_size_hor()/2),y(2), speedx(1),speedy(1), dx(speedx* cos(degtorad(angle))),dy(speedy* sin(degtorad(angle))) {}
	void set_cursor_position(int x, int y);
	windows_parameters param;
	int angle;
	double x;
	double y;
	int speedx;
	int speedy;
	double dx;
	double dy;
};

struct field
{
	field(ball& B_,desk& D_)
		:B(B_),D(D_),cnt(0) {}
	void draw_field(const windows_parameters& param);
	//void move_ball(std::atomic<bool>& state);
	//void move_desk_with_use_sensor(std::atomic<bool>& state);
	void move_ball_and_desk(std::atomic<bool>& state);
	void increment(std::atomic<bool>& state);
	bool collision_with_field(std::atomic<bool>& state);
	void collision_desk_and_ball(std::atomic<bool>& state);
	ball B;
	desk D;
	tty_init T;
private:
	windows_parameters param;
	//std::mutex mtx;
	int cnt;		// this variable for count waiting for quit work thread, now if cnt = 50 (if T request sensor = 5 ms, and T request 30 ms)
};