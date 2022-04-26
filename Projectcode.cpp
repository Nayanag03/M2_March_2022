//MOTOR_A
#define ENA        DDB3		 //PWM Enable Motor 1				-- Pin 11 / OCOA
#define IN1_DDR    DDC4		 //DATA DIRECTION REGISTER~~~Control direction  of Motor 1	-- Pin A4
#define IN2_DDR    DDC5		 //DATA DIRECTION REGISTER~~~Control direction of Motor 1    -- Pin A5
#define IN1_PORT   PORTC4    //PORT~~~Control direction of Motor 1	-- Pin A4
#define IN2_PORT   PORTC5    //PORT~~~Control direction of Motor 1    -- Pin A5

//MOTOR_B
#define ENB		   DDD3      //PWM Enable Motor 2				-- Pin 3 / OCOB
#define IN3_DDR    DDC3      //DATA DIRECTION REGISTER~~~Control direction of Motor 2	-- Pin A3
#define IN4_DDR    DDC2      //DATA DIRECTION REGISTER~~~Control direction of Motor 2    -- Pin A2
#define IN3_PORT   PORTC3    //PORT~~~Control direction of Motor 1	-- Pin A3
#define IN4_PORT   PORTC2    //PORT~~~Control direction of Motor 1    -- Pin A2

//SERVO
#define SERVO	   DDB1		//SERVO SIGNAL	pin 9

//ULTRASONIC SENSOR
#define echoPin 7
#define trigPin 8

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <Arduino.h>


#line 1
//#line 1 "C:\\Users\\Alexandru\\Documents\\Arduino\\ultra\\ultra.ino"



void setup();
void loop();
void forward();
void push_break();
void scan();
void navigate();

#line 5

double dutyA = 100;
double dutyB = 100;
uint16_t FrontDistance;
uint16_t LeftDistance;
uint16_t RightDistance;
int distance,travel_time;


void setup()
{
	//Serial.begin(9600);
	pinMode(trigPin, OUTPUT);
	pinMode(echoPin, INPUT);
	
	DDRB |= (1<<DDB5); //Led output
	
	DDRB |= (1<<SERVO);
	
	// motor A
	DDRB |= (1<<ENA); //pin 6 output PWM
	DDRC = (1<<IN1_DDR) | (1<<IN2_DDR); // A4,A5 pins for controlling the direction of motor
	
	//motor B
	DDRD |= (1<<ENB); //pin 5 output PWM
	DDRC = (1<<IN3_DDR) | (1<<IN4_DDR); //2,4 pins for controlling the direction of motor B
	
	TCCR2A = (1<<COM2A1) | (1<<COM2B1) | (1<<WGM21) | (1<<WGM20); //clear pin on compare match, update pin on BOTTOM, TOPT= 0xFF
	TCCR1A = (1<<COM1A1) | (1<<COM1A0) | (1<<WGM11); // inverting mode fast pwm with icr1 reg as top
	
	
	TIMSK2 = (1<<TOIE2); // intterupt on overflow
	
	
	OCR2A = (dutyA/100) *255;
	OCR2B = (dutyB/100) *255;
	ICR1 = 39999; 
	
	sei();
	
	TCCR2B = (1<<CS22) | (1<<CS21) | (1<<CS20); //clk source, prescalar 1024
	TCCR1B  = (1<<WGM13) |(1<<WGM12)|(1<<CS11); //Prescalar = 8
	
	delay(3000);
}

void loop()
{
	
	 OCR1A = ICR1 - 2600; //~90 deg center
	 scan();
	 FrontDistance = distance;
	 //Serial.println("Front distance = ");
	 //Serial.print(distance);
	
	if(distance >=20) // for going forward
	{
	    forward();
		PORTB &= ~(1<<PORTB5);
	}
	else // obstacle detected turn left
	{		
		PORTB |= (1<<PORTB5);
		push_break();
		navigate();
	}
	
	
}

void motorA_forward()
{
	PORTC |= (1<<IN1_PORT); // 4 is High
	PORTC &= ~(1<<IN2_PORT);// 5 is Low
}

void motorA_backward()
{
	PORTC &= ~(1<<IN1_PORT); //4 is LOW
	PORTC |= (1<<IN2_PORT);// 5 is High
}

void motorA_stop()
{
	PORTC &= ~(1<<IN1_PORT); //4 is LOW
	PORTC &= ~(1<<IN2_PORT);// 5 is LOW
}

void motorB_forward()
{
	PORTC |= (1<<IN3_PORT); // 2 is High
	PORTC &= ~(1<<IN4_PORT);// 4 is Low
}

void motorB_backward()
{
	PORTC &= ~(1<<IN3_PORT); // 2 is low
	PORTC |= (1<<IN4_PORT);// 4 is high
}

void motorB_stop()
{
	PORTC &= ~(1<<IN3_PORT); //2 is LOW
	PORTC &= ~(1<<IN4_PORT);// 4 is LOW
}

void motorA_on()
{
	OCR2A = (dutyA/100) *255;
}

void motorA_off()
{
	OCR2A = 0;
}

void motorB_on()
{
	OCR2B = (dutyB/100) *255;
}

void motorB_off()
{
	OCR2B = 0;
}

//Movment Functions
void forward()
{
	motorA_forward();
	motorB_forward();
	//delay_ms(duration);
}

void backward(int duration)
{
	motorA_backward();
	motorB_backward();
	delay(duration);
}
void spin(int duration)
{
	motorA_forward();
	motorB_backward();
	delay(duration);
}

void right(int duration)
{
	motorA_stop();
	motorB_forward();
	delay(duration);
}

void left(int duration)
{
	motorA_forward();
	motorB_stop();
	delay(duration);
} 
void push_break()
{
	motorA_stop();
	motorB_stop();

}



void enableMotor()
{
	motorA_on();
	motorB_on();
}


ISR(TIMER2_OVF_vect)
{
	OCR2A = (dutyA/100) *255;
	OCR2B = (dutyB/100) *255;
}

void scan()
{
	digitalWrite(trigPin, HIGH);
	delay(50);
	digitalWrite(trigPin, LOW);
	travel_time=pulseIn(echoPin, HIGH,16000000L);
	distance=(travel_time/2)/29.1;
	delay(50);
}

void navigate()
{

	OCR1A = ICR1 - 3800; // move servo to left ~155 DEG
	delay(2000);
	scan();
	LeftDistance = distance;

	OCR1A = ICR1 - 1700 // ~15-20 degrees
	delay(2000);
	scan();
	RightDistance = distance;

	if( (RightDistance - LeftDistance) < 7)
	{
		//backward(700);
		//right(700);
				spin(500);
	}
	else if(RightDistance < LeftDistance)                  
	{
		left(500);                                  
	}
	else if(LeftDistance < RightDistance)             
		right(500);                                     
	}
}