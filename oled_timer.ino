#define RST_PIN 7
#define UP_PIN 5
#define DOWN_PIN 6
#define START_PIN 4

#define BUZ_PIN 10
#define BUZ_PERIOD 250
#define BUZ_FREQ 1200
#define BEEP_FREQ 1500
#define BEEP_PERIOD 200

#define DEBOUNE_TIME 200
#define DELTA_TIME 30
#define LONG_DELTA_TIME 5*60
#define LONG_PRESS 3000

#include "U8glib.h"


// setup u8g object, please remove comment from one of the following constructor calls
// IMPORTANT NOTE: The following list is incomplete. The complete list of supported 
// devices with all constructor calls is here: https://github.com/olikraus/u8glib/wiki/device
U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NONE);	// I2C / TWI 

void beep()
{
	tone(BUZ_PIN, BEEP_FREQ, BEEP_PERIOD);
}


void buzz()
{
	static long buz_time = 0;
	if (millis() - buz_time < BUZ_PERIOD)
	{
		tone(BUZ_PIN, BUZ_FREQ, BUZ_PERIOD);
	}
	else if (millis() - buz_time > 2*BUZ_PERIOD)
	{
		buz_time = millis();
	}
	else if(millis()-buz_time>BUZ_PERIOD)
	{
		noTone(BUZ_PIN);
	}
}

void drawEnd()
{
	static long last_show = 0;
	u8g.setFont(u8g_font_osb21);
	if (millis() - last_show > 2000)
	{
		last_show = millis();
	}
	if (millis() - last_show > 1000)
	{
		u8g.drawStr(10, 40, "FINISH");
		
	}
	else if (millis() - last_show > 0)
	{
		u8g.drawStr(10, 40, "");
	}
	
	
}
void draw(int time) 
{
	int hour, minute, second;
	hour = time / 3600;
	minute = (time % 3600) / 60;
	second = (time % 3600) % 60;
	// graphic commands to redraw the complete screen should be placed here  
	u8g.setFont(u8g_font_fub20n);
	char buf[9];
	sprintf(buf, "%02d:%02d:%02d", hour,minute,second);
	u8g.drawStr(10, 40, buf);
}

int set_legal_time(int time)
{
	if (time < 0)
	{
		return 0;
	}
	else
	{
		return time;
	}
}

int timer1_counter;
int time = 0;

ISR(TIMER1_OVF_vect)        // interrupt service routine 
{
	TCNT1 = timer1_counter;   // preload timer
	time--;	
}



void set_interrupt(int flag)
{
	noInterrupts();           // disable all interrupts
	TCCR1A = 0;
	TCCR1B = 0;
	timer1_counter = 34286;   // preload timer 65536-16MHz/256/2Hz
	TCNT1 = timer1_counter;   // preload timer
	TCCR1B |= (1 << CS12);    // 256 prescaler 
	if (flag == 1)
	{
		TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
	}
	else
	{
		TIMSK1 &= ~(1 << TOIE1); // disable timer overflow interrupt
	}
	interrupts();             // enable all interrupts
}

void setup(void) 
{
	

	pinMode(RST_PIN, INPUT_PULLUP);
	pinMode(UP_PIN, INPUT_PULLUP);
	pinMode(DOWN_PIN, INPUT_PULLUP);
	pinMode(START_PIN, INPUT_PULLUP);
	// flip screen, if required
	// u8g.setRot180();

	// assign default color value
	if (u8g.getMode() == U8G_MODE_R3G3B2) 
	{
		u8g.setColorIndex(255);     // white
	}
	else if (u8g.getMode() == U8G_MODE_GRAY2BIT) 
	{
		u8g.setColorIndex(3);         // max intensity
	}
	else if (u8g.getMode() == U8G_MODE_BW) 
	{
		u8g.setColorIndex(1);         // pixel on
	}
	else if (u8g.getMode() == U8G_MODE_HICOLOR) 
	{
		u8g.setHiColorByRGB(255, 255, 255);
	}
}

void loop(void) 
{
	int rst_state;
	int up_state;
	int down_state;
	int start_state;
	long time_diff;

	static long start_key_press = 0;
	static long last_disp_update = 0;
	static int prev_start_button_state = 0;
	static int timer_state = 0;
	
	static int prev_up_button_state = 0;
	static long last_up_key_press = 0;
	static int prev_down_button_state = 0;
	static long last_down_key_press = 0;
	
	static int delta_time = DELTA_TIME;

	rst_state = digitalRead(RST_PIN);
	up_state = digitalRead(UP_PIN);
	down_state = digitalRead(DOWN_PIN);
	start_state = digitalRead(START_PIN);
//	block_key = 0;
	//handle reset button
	if (rst_state == LOW)
	{
		noTone(BUZ_PIN);
		time = 0;
		timer_state = 0;
		delta_time = DELTA_TIME;
		set_interrupt(0);
	}

	//handle up/down button
	if ((up_state == LOW) &&(prev_up_button_state==0))
	{
		prev_up_button_state = 1;
		last_up_key_press = millis();
		if ((time == 0) && (timer_state == 1))
		{
			set_interrupt(0);
			timer_state = 0;
		}
		time += delta_time;
	}
	else if ((up_state == LOW) && (prev_up_button_state == 1))
	{
		time_diff = millis() - last_up_key_press;
		if (time_diff>LONG_PRESS)
		{
			delta_time = LONG_DELTA_TIME;
		}
		if (time_diff > DEBOUNE_TIME)
		{
			time += delta_time;
		}
	}
	else if (up_state == HIGH)
	{
		delta_time = DELTA_TIME;
		prev_up_button_state = 0;
	}

	//handle up/down button
	if ((down_state == LOW) && (prev_down_button_state == 0))
	{
		prev_down_button_state = 1;
		last_down_key_press = millis();
		time -= delta_time;
	}
	else if ((down_state == LOW) && (prev_down_button_state == 1))
	{
		time_diff = millis() - last_down_key_press;
		if (time_diff>LONG_PRESS)
		{
			delta_time = LONG_DELTA_TIME;
		}
		if (time_diff > DEBOUNE_TIME)
		{
			time -= delta_time;
		}
	}
	else if (down_state == HIGH)
	{
		delta_time = DELTA_TIME;
		prev_down_button_state = 0;
	}

	time = set_legal_time(time);

	//handle start/stop button
	if ((start_state == LOW) && (prev_start_button_state==0))
	{
		if (millis() - start_key_press > DEBOUNE_TIME)
		{
			if ((timer_state == 0) && (time > 0))
			{
				timer_state = 1;
				set_interrupt(1);
			}
			else
			{
				timer_state = 0;
				set_interrupt(0);
			}
			beep();
			start_key_press = millis();
			prev_start_button_state = 1;
		}
	}
	else if (start_state == HIGH)
	{
		prev_start_button_state = 0;
	}
	

	// picture loop
	u8g.firstPage();
	do
	{
		if ((timer_state == 1) && (time<=0))
		{
			drawEnd();
			buzz();
		}
		else
		{
			draw(time);
		}
	} while (u8g.nextPage());
}
