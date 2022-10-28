#include "ark.h"

struct FileDeleter {
	void operator() (FILE* file)
	{
		std::fclose(file);
	}
};

void tty_init::write_log_sensor_data(const char* buf)
{
	std::unique_ptr<FILE,FileDeleter> file(std::fopen("data_sensor.txt","a+"));
	fputs("\n",file.get());
	fputs(std::to_string(buf_int).c_str(),file.get());
}

void tty_init::set_settings_tty()
{
    // https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/
    if (tcgetattr(serial_port,&tty) !=0)
    {
        perror("Error ");
        return;
    }

    tty.c_cflag &= ~PARENB; // clear parity bit, disabling parity
    tty.c_cflag &= ~CSTOPB; // clear stop field, 1 stop bit
    tty.c_cflag &= ~CSIZE;  // clear all bits for set data size
    tty.c_cflag |= CS8;     // set data size  = 8 bit
    tty.c_cflag &= ~CRTSCTS;    // disable RTS/CTS hardware flow control
    tty.c_cflag |= CREAD | CLOCAL;  // turn on read& ignore ctrl lines

    tty.c_iflag &= ~ICANON;
    tty.c_iflag &= ~ECHO;   // disable echo
    tty.c_iflag &= ~ECHOE;  // disable earsure
    tty.c_iflag &= ~ECHONL; // disable new-line echo
    tty.c_iflag &= ~ISIG;   // disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); //turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);

    tty.c_oflag &= ~OPOST;  // prevent special interpretator of output bytes
    tty.c_oflag &= ~ONLCR;  // prevent conversion of newline to carriage return/line feed

    // setting time delay for waiting data
    tty.c_cc[VTIME] = 10;      // wait for up 1s (10*100ms)
    tty.c_cc[VMIN] = 0;

    cfsetispeed(&tty,B9600);
    cfsetospeed(&tty,B9600);

    if (tcsetattr(serial_port,TCSANOW,&tty) !=0)
    {
        perror("Error from tcsetattr ");
        return;
		//exit(1);
    }
}

void tty_init::read_data(std::atomic<bool>& state)
{
    int i =0;
    while (state)
    {
        //Simple_timer st;
        tcflush(serial_port,TCIFLUSH);
        int num_bytes = read(serial_port,&buf,sizeof(buf));
        if (num_bytes <0)
            {
                perror("Error reading ");
				state = false;
                return;
				//exit(1);
            }
        buf_int = char_to_int(buf);
		write_log_sensor_data(buf);
    }
};

int tty_init::get_value_sensor() const
{
	return buf_int;
}

void field::draw_field(const windows_parameters& param)
{
	initscr();
	cbreak();
	noecho();
	mvaddch(0,0,ACS_BSSB);
	for (int i=1;i<param.get_size_hor()-1;++i)
	{
		addch(ACS_HLINE);
	}
	addch(ACS_BBSS);
	for (int i=1;i<param.get_size_vert()-1;++i)
	{
		mvaddch(i,0,ACS_VLINE);
		mvaddch(i,param.get_size_hor()-1,ACS_VLINE);
	}
	mvaddch(param.get_size_vert()-1,0,ACS_SSBB);
	for (int i=1;i<param.get_size_hor()-1;++i)
	{
		addch(ACS_HLINE);
	}
	addch(ACS_SBBS);
}

bool field::collision_with_field(std::atomic<bool>& state)
{
	bool flag = 0;
	if ((round(B.x) >= param.get_size_hor() - 3) || round(B.x) < 2)
	{
		B.angle *= -1;
		flag = 1;
	}
	if ((round(B.y) >= param.get_size_vert()-2))
	{
		collision_desk_and_ball(state);
		if (B.angle < 0)
			B.angle -= 180;
		else
			B.angle += 180;
		B.speedx *= -1;
		flag = 1;
	}
	else if (round(B.y) <= 1)
	{
		if (B.angle < 0)
			B.angle -= 180;
		else
			B.angle += 180;
		B.speedx *= -1;
		B.y += 0.5;	// for correct work in the boundary condition
		flag = 1;
	}
	// debug
	//std::unique_ptr<FILE,FileDeleter> file_coord_x(std::fopen("data_coord.txt","a+"));
	//fputs("\n",file_coord_x.get());
	//std::string str = "desk X: "+std::to_string(D.get_coordX())+" ball X "+std::to_string(B.x);
	//fputs(str.c_str(),file_coord_x.get());
	//--------------------------------------------------------------------------------------
 	return flag ? 1 : 0;
}

void field::collision_desk_and_ball(std::atomic<bool>& state)
{
	//std::lock_guard<std::mutex> lck(mtx);
	if (!((round(B.x) >= D.get_coordX()) && (round(B.x) <= D.get_coordX() + D.get_size_desk())))
	{
		game_over(state);
	}
}

void field::increment(std::atomic<bool>& state)
{
	if (collision_with_field(state))
	{
		double val = degtorad(B.angle);
		B.dx = B.speedx * cos(val);
		B.dy = B.speedy * sin(val);
	}
	B.x += B.dx;
	B.y += B.dy;
}
/*
void field::move_ball(std::atomic<bool>& state)
{
 	char sym_ball = '*';
	while (state)
	{
		B.set_cursor_position(round(B.x), round(B.y));					
		addch(' ');
		increment(state);
		B.set_cursor_position(round(B.x), round(B.y));
		addch('*');
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		refresh();
	}
}

void field::move_desk_with_use_sensor(std::atomic<bool>& state)
{	
	while((cnt !=REPS_FOR_QUIT) && state)
		{	
			mtx.lock();
			D.move_desk_with_sensor(T.get_value_sensor(),cnt);
			mtx.unlock();
			std::this_thread::sleep_for(std::chrono::milliseconds(30));
		}
	//game_over(state ,param.get_size_hor(),param.get_size_vert());
	game_over(state);
}
*/
void field::move_ball_and_desk(std::atomic<bool>& state)
{
	char sym_ball = '*';
	while ((cnt !=REPS_FOR_QUIT) && state)
	{
		D.move_desk_with_sensor(T.get_value_sensor(),cnt);
		refresh();
		std::this_thread::sleep_for(std::chrono::milliseconds(30));
		B.set_cursor_position(round(B.x), round(B.y));					
		addch(' ');
		increment(state);
		B.set_cursor_position(round(B.x), round(B.y));
		addch('*');
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		refresh();
	}
	game_over(state);
}

void desk::draw_desk()
{
	if (x <= 0)
	{
		x = 1;
		return;
	}
	else if (x + size_desk >= param.get_size_hor() -1)
	{
		x = param.get_size_hor()-size_desk;
		return;
	}
	move(param.get_size_vert()-2,1);
	for (int i = 0; i < param.get_size_hor()-2;++i)
	{
		addch(' ');
	}
	move(param.get_size_vert()-2,x);
	for (int i = 0; i < size_desk;++i)
	{
		addch(ACS_CKBOARD);
	}
}

void desk::move_desk()
{
	char sym{ ' ' };
	while (sym != 'q')
	{
		sym = getch();
		switch (sym)
		{
		case 'a':
		{
			--x;
			draw_desk();
			break;
		}
		case 'd':
		{
			++x;
			draw_desk();
			break;
		}
		case 'q':
		{
			endwin();
			exit(0);
		}
		default:
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

void desk::move_desk_with_sensor(int value_sensor, int& cnt)
{
	// cnt is variable for count value, if value_sensor > 40 cm N reps then quit
		if (value_sensor>= 5 && value_sensor < 10)			
		{
			x -=3;
			cnt=0;
			draw_desk();
		}
		else if (value_sensor >=10 && value_sensor <15)
		{
			x -=2;
			cnt=0;
			draw_desk();
		}
		else if (value_sensor >= 15 && value_sensor <20)
		{
			x -=1;
			cnt=0;
			draw_desk();
		}
		else if (value_sensor >= 20 && value_sensor <25)
		{
			cnt =0;
		}
		else if (value_sensor >= 25 && value_sensor <30)
		{
			x +=1;
			cnt=0;
			draw_desk();
		}
		else if (value_sensor >= 30 && value_sensor <35)
		{
			x +=2;
			cnt=0;
			draw_desk();
			
		}
		else if (value_sensor >=35 && value_sensor <40)
		{
			x +=3;
			cnt=0;
			draw_desk();
		}
		else
			{
				cnt++;
			}
}

void ball::set_cursor_position(int x, int y)
{
	move(y,x);
}

std::string itos(int val)
{
	std::string ans{""};
	ans+=val/10 +48;
	ans+=val%10 +48;
	return ans;
}

int char_to_int(const char* buf)
{
    int ans=0;
    int n =1;
    for (int i=2; i>=0;--i)
    {
        if (buf[i]>=48)
        {
            ans += (buf[i]-'0')*n;
            n *=10;
        }
    }
    return ans; 
}

double degtorad(int& angle)
{
	if (angle >= 360)
		angle -= 360;
	return angle > 0 ? (angle * M_PI / 180) : (180 + angle) * M_PI / 180;
}

void game_over(std::atomic<bool>& state, int size_hor, int size_vert)
{
	move(size_vert/2,size_hor/2-4);
	printw("Game over");
	getch();
	endwin();
	echo();
	clear();
	state = false;
	return;
}
