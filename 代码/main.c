#include "stc8h.h" //you can also put this head file under the folder: "C:\Program Files (x86)\keil5\C51\INC\STC\"
sbit Hsensor=P3^2;
sbit Touchout=P3^3;

#define Pmotor(x) (P1=P1&0xf0|x)
#define MOTOR_DELAY 30

#define CONTROL_AUTO 1
#define CONTROL_MANUAL 0

#define LIGHT_ON 1
#define LIGHT_OFF 0

#define TOUCH_SHORT 2
#define TOUCH_LONG 1
#define TOUCH_INACTIVE 0

#define SENSOR_ACTIVE 1
#define SENSOR_INACTIVE 0

#define WAIT_ON 1
#define WAIT_OFF 0

#define MOTOR_A 0x01
#define MOTOR_AB 0x03
#define MOTOR_B 0x02
#define MOTOR_BC 0x06
#define MOTOR_C 0x04
#define MOTOR_CD 0x0C
#define MOTOR_D 0x08
#define MOTOR_DA 0x09
#define MOTOR_DOWN 0x00

#define MOTOR_STEP_GO 25
#define MOTOR_STEP_BACK 45

unsigned char light_state=LIGHT_OFF;
unsigned char sensor_state=SENSOR_INACTIVE;
unsigned char touch_state=TOUCH_INACTIVE;
unsigned char control_mode=CONTROL_MANUAL;
unsigned char wait_state=WAIT_OFF;

void delay_ms(unsigned int interval)
{
	int i,j;
		for(i=interval;i>0;i--)
			for(j=0;j<400;j++);
}

void motor(int step)
{
	int ctn=0;
	P1M0|=0x0F;
	P1M1&=~0x0F;
	delay_ms(100); 
	if(step>0)
	{
		while(++ctn<=step){
			Pmotor(MOTOR_A);
			delay_ms(MOTOR_DELAY);
			Pmotor(MOTOR_AB);
			delay_ms(MOTOR_DELAY);
			Pmotor(MOTOR_B);
			delay_ms(MOTOR_DELAY);
			Pmotor(MOTOR_BC);
			delay_ms(MOTOR_DELAY);
			Pmotor(MOTOR_C);
			delay_ms(MOTOR_DELAY);
			Pmotor(MOTOR_CD);			
			delay_ms(MOTOR_DELAY);
			Pmotor(MOTOR_D);
			delay_ms(MOTOR_DELAY);
			Pmotor(MOTOR_DA);			
			delay_ms(MOTOR_DELAY);
		}
	}
	else{
		while(++ctn<=-step){
			Pmotor(MOTOR_D);
			delay_ms(MOTOR_DELAY);
			Pmotor(MOTOR_CD);			
			delay_ms(MOTOR_DELAY);
			Pmotor(MOTOR_C);
			delay_ms(MOTOR_DELAY);
			Pmotor(MOTOR_BC);
			delay_ms(MOTOR_DELAY);
			Pmotor(MOTOR_B);
			delay_ms(MOTOR_DELAY);
			Pmotor(MOTOR_AB);
			delay_ms(MOTOR_DELAY);
			Pmotor(MOTOR_A);
			delay_ms(MOTOR_DELAY);
			Pmotor(MOTOR_DA);
			delay_ms(MOTOR_DELAY);
		}
	}
	Pmotor(MOTOR_DOWN);
	P1M0&=~0x0F;
	P1M1|=0x0F;
}
	

void init_all()
{
	// P3.2 for sensor input. interrup.t (rising or fulling edge trigger)
	// high restance state by default
	IT0=0;
	EX0=1;
	
	// P3.3 for touch pad input. interrup.t (rising edge trigger)
	// high restance state by default
	IT1=0;
	EX1=1;
	// P1 port should be set as output state before being used, and after that P1 should be set as high resitance state.
	// P1 port is at high resitance state by default.

	// TIMER
	//TMOD=0x00;

	EA=1;
}

void timer_start()
{
	// lowpower timer,period: 2s
	WKTCL=0xff;
	WKTCH=0x0f;
	WKTCH|=0x80;
}

void timer_stop()
{
	WKTCH&=~0x80;
}

// sensor's interrup.t
void int0_int() interrupt 0
{
	EX0=0;
	//no operation, as the main program will judge the Hsensor pin
	EX0=1;
}

// touchpad's interrup.t
void int1_int() interrupt 2
{
	unsigned int ctn=0;
	EX1=0;
	delay_ms(30);
	if(Touchout==1){
		for(ctn=0;ctn<500;ctn++){
			delay_ms(30);
			if(Touchout==0){
				break;
			}
		}
		if(ctn<100){
			touch_state=TOUCH_SHORT;
		}
		else{
			touch_state=TOUCH_LONG;
		}
	}
	EX1=1;
}

void light_on()
{
	if(light_state==LIGHT_OFF){
		light_state=LIGHT_ON;
		motor(-MOTOR_STEP_GO);
		motor(MOTOR_STEP_BACK);
	}
}
void light_off()
{
	if(light_state==LIGHT_ON){
		light_state=LIGHT_OFF;
		motor(MOTOR_STEP_GO);
		motor(-MOTOR_STEP_BACK);
	}
}
void go_automode()
{
	control_mode=CONTROL_AUTO;
	if(Hsensor==1){
		light_state=LIGHT_OFF;
		light_on();
	}else{
		light_state=LIGHT_ON;
		light_off();
	}
}
void go_manualmode()
{
	timer_stop();
	wait_state=WAIT_OFF;
	control_mode=CONTROL_MANUAL;
	motor(-120);
}



void lowpower()
{
	PCON=0x02;  // turn to "power down" mode
}


void main()
{
	unsigned int sensor_inact_ctn=0;
//	P1M0|=0x0F;
//	P1M1&=~0x0F;
//	P1=0xFF;
//	while(1);
//	while(1){
	delay_ms(1000);
	init_all();
//	light_on();
//	}

	while(1){
		switch(touch_state)
		{
			case TOUCH_SHORT:
				touch_state=TOUCH_INACTIVE;
				if(control_mode==CONTROL_MANUAL)
				{
					switch(light_state)
					{
						case LIGHT_ON:
							light_off();
						break;
						case LIGHT_OFF:
							light_on();
						break;
					}
				}
			break;
			case TOUCH_LONG:
				touch_state=TOUCH_INACTIVE;
				switch(control_mode)
				{
					case CONTROL_AUTO:
						go_manualmode();
					break;
					case CONTROL_MANUAL:
						go_automode();
					break;
				}
			break;
		}
		
		if(control_mode==CONTROL_AUTO){
				if(Hsensor==1){
					if(wait_state==WAIT_ON){  
						timer_stop();
						wait_state=WAIT_OFF;
					}						
					light_on();	
				}
				else if(Hsensor==0){
					if(wait_state==WAIT_OFF){
						wait_state=WAIT_ON; // wait for several seconds or minutes to detecing the sensor state after the sensor sending the low-voltage signal
						sensor_inact_ctn=0;
						timer_start();
					}
					if (++sensor_inact_ctn>15){  // you can change 15 to set other waiting & detecing time.
						timer_stop();
						light_off();
					}
				}
		}
		lowpower();
		delay_ms(10); 
	}
	
}
