//Code Written by Brennan McTernan and Tianyi Zhou
//Final Project for Real-time and Embedded Systems
/////////////
//This is a controller for the Arduino Uno R3 microcontroller
//Code is compiled using the Arduino IDE
//In order to use this controller three items are required:
//1 Arduino Uno R3
//1 Bricktronics Shield - interface with between the Arduino and the lego motors and sensors
//1 Wii nunchuk - modified to work with the Arduino system
//A differential drive lego vehicle with 1 color detector. 
///
//Please note that this controller was developed for the specific vehicle we had built
//for this project. It no longer exists. 
///////////////////////////////////////////////////////////////////////

#include <ArduinoNunchuk.h>

#include <Bricktronics.h>
#include <Wire.h>
#include <Button.h>
#include <ColorSensor.h>
#include <Motor.h>
#include <Ultrasonic.h>

//Define////////////////
#define HIGHFTHRESH 710
#define LOWFTHRESH 300
#define LEFTTHRESH 300
#define RIGHTTHRESH 730
#define ENCODERCHECK 720
#define ENCODERTHRESH 60
#define DEFAULTBAUD 9600
#define NUNCHUKPRESENT 694
#define MAXRANGE 510
#define STARTRANGE 255
#define BEGINDEADZONE 50
#define ENDDEADZONE -50
#define BEGINMOTORDEADZONE 50
#define ENDMOTORDEADZONE -50
#define MAXSPEED 255
#define MINSPEED -255
#define LEFTTURNSPEED 120
#define RIGHTTURNSPEED 120
////////////////////////

//Initialize
ArduinoNunchuk nunchuk = ArduinoNunchuk();
Bricktronics brick = Bricktronics();
Motor m1 = Motor(&brick, 1);
Motor m2 = Motor(&brick, 2);
ColorSensor color = ColorSensor(&brick,3);

double speedY = 0;
double speedX = 0;
double speedL = 0;
double speedR = 0;
double nunchukY = 0;
double nunchukX = 0;
double lowBound = LOWFTHRESH + 10;
double highBound = HIGHFTHRESH - 10;
double leftBound = LEFTTHRESH + 10;
double rightBound = RIGHTTHRESH - 10;
int lEncoder = 0;
int rEncoder = 0;
boolean nunchukPresent = false;

//Begin serial connection and initialize the motors and sensors
//setup() must be named setup() and is initially called by the Arduino system
void setup()
{
  Serial.begin(DEFAULTBAUD);						//setup debug communication
  brick.begin();												//Initialize bricktronics shield, interface between lego system and Arduino
  m1.begin();														//Initialize motor 1
  m2.begin();														//Initialize motor 2
  color.begin();												//Initialize the color sensor
  nunchuk.init();												//Initialize the Wii Nunchuk
  if(analogRead(A5) == NUNCHUKPRESENT)  //Detects if the Wii Nunchuk is present
  {
    nunchukPresent = true; 
  }
  Serial.println(analogRead(A5));
}

//loop() must be called loop and is the main cycle of the Arduino system
void loop()
{
  int new_color = color.get_color();		//Read color sensor
  if(nunchukPresent)
  {
    Serial.println("In Normal");
    operateNormal(new_color);
  }
  else
  {
    Serial.println("In auto");
    operateAutomatic(new_color);
  }

  Serial.println("***");
  delay(1);															//Allows Arduino to work
}

//getForwardSpeed(nunchukY) calculates forward speed based on y axis tilt of nunchuk
int getForwardSpeed(double nunchukY)
{
  if((nunchukY >= LOWFTHRESH)&&(nunchukY <= HIGHFTHRESH))
  {
    //Dynamically adjust thresholds 
    if(nunchukY < lowBound)
    {
      lowBound = nunchukY;
    }
    
    if(nunchukY > highBound)
    {
      highBound = nunchukY;
    }
  }
  return (((nunchukY-lowBound)/(highBound - lowBound))*MAXRANGE)-STARTRANGE;
}

//getTurnSpeed(nunchukX) calculate turn from left or right tilt of Nunchuk
int getTurnSpeed(double nunchukX)
{
  if((nunchukX >= LEFTTHRESH)&&(nunchukX <= RIGHTTHRESH))
  {
    //Dynamically adjust thresholds 
    if(nunchukX < leftBound)
    {
      leftBound = nunchukX;
    }
    
    if(nunchukX > rightBound)
    {
      rightBound = nunchukX;
    }
  }
  return (((nunchukX-leftBound)/(rightBound - leftBound))*MAXRANGE)-STARTRANGE;
}

void operateNormal(int new_color) 				//Wii Nunchuk is present and will be controller by the user
{
  nunchuk.update();												//Read nunchuk data
  nunchukY = nunchuk.accelY;
  nunchukX = nunchuk.accelX;
  if(new_color != WHITE)									//If no longer over white floor then stop
  {
    m1.set_speed(0);
    m2.set_speed(0);
  }
  else
  {
		//Define deadzone for controller
		if(((speedX < BEGINDEADZONE)&&(speedX > ENDDEADZONE))) 
		{
			speedX = 0;
		}

		speedY = getForwardSpeed(nunchukY);
		speedX = getTurnSpeed(nunchukX);

		//If making right turn adjust right wheel speed
		//If making left turn adjust left wheel speed
		if(speedX >= 0)
		{
			speedR = speedY + (-1 * speedX);
			speedL = speedY;
		}
		else
		{
			speedR = speedY;
			speedL = speedY + (speedX);
		}

		//Cap the speed of the motors
		if(speedL > MAXSPEED)
		{
			speedL = MAXSPEED;
		}
		else if (speedL < MINSPEED)
		{
			speedL = MINSPEED;
		}

		if(speedR > MAXSPEED)
		{
			speedR = MAXSPEED;
		}
		else if (speedR < MINSPEED)
		{
			speedR = MINSPEED;
		}

		if(((speedL < BEGINMOTORDEADZONE)&&(speedL > ENDMOTORDEADZONE))) // Minimum Speed
		{
			speedL = 0;
		}

		if(((speedR < BEGINMOTORDEADZONE)&&(speedR > ENDMOTORDEADZONE))) // Minimum Speed
		{
			speedR = 0;
		}

		m1.set_speed(speedL);
		m2.set_speed(speedR);
	}
}

void operateAutomatic(int new_color) 						//No Wii Nunchik, vehicle will move on its own
{
  if (new_color == GREEN)												//GREEN is the color of the goal. Goal reached, stop motors.
  {
    speedL = 0;
    speedR = 0;
  }
  else 
  {
    if (new_color == BLUE)    									//blue on the left side, turn right a little
    {
      speedL = LEFTTURNSPEED;
      speedR = 0; 
      delay(10);
    }
    else if(new_color == YELLOW)  							//yellow on the right side, turn left a little
    {
      speedL = 0;
      speedR = RIGHTTURNSPEED;   
      delay(10);   
    }
    else
    {
      speedL = LEFTTURNSPEED;
      speedR = RIGHTTURNSPEED;
    }
  }
    delay(1);									//Allow Arduino to work
    m1.set_speed(speedL);     // half speed
    m2.set_speed(speedR);     // half speed
}
