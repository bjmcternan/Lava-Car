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
////////////////////////

//Initialize
ArduinoNunchuk nunchuk = ArduinoNunchuk();
Bricktronics brick = Bricktronics();
Motor m1 = Motor(&brick, 1);
Motor m2 = Motor(&brick, 2);
ColorSensor color = ColorSensor(&brick,3);

//Button b= Button(&brick,1); //Was used for testing

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
void setup()
{
  Serial.begin(9600);
  brick.begin();
  m1.begin();
  m2.begin();
  color.begin();
  //b.begin();
  nunchuk.init();
  if(analogRead(A5) == 694) //Detects if the Wii Nunchuk is present
  {
    nunchukPresent = true; //
  }
  Serial.println(analogRead(A5));
}


void loop()
{
  int new_color = color.get_color(); 
  if(nunchukPresent)
  {
    Serial.println("In Normal");
    normal(new_color);
  }
  else
  {
    Serial.println("In auto");
    automatic(new_color);
  }

  Serial.println("***");
  delay(1);
}

int getForwardSpeed(double nunchukY)
{
  if((nunchukY > HIGHFTHRESH)||(nunchukY < LOWFTHRESH))
  {
  }
  else
  {
    if(nunchukY < lowBound)
    {
      lowBound = nunchukY;
    }
    
    if(nunchukY > highBound)
    {
      highBound = nunchukY;
    }
  }
  /*if(hold == 1)
  {
    return 0;
  }
  else
  {*/
    return (((nunchukY-lowBound)/(highBound - lowBound))*510)-255;
  //}
}

int getTurnSpeed(double nunchukX)
{
  if((nunchukX > RIGHTTHRESH)||(nunchukX < LEFTTHRESH))
  {
    //return speedX;
  }
  else
  {
    if(nunchukX < leftBound)
    {
      leftBound = nunchukX;
    }
    
    if(nunchukX > rightBound)
    {
      rightBound = nunchukX;
    }
  }
  /*if(hold == 1)
  {
    return 0;
  }
  else
  {*/
    return (((nunchukX-leftBound)/(rightBound - leftBound))*510)-255;
  //}
}

void normal(int new_color) //Wii Nunchuk is present and will be controller by the user
{
  nunchuk.update();
  nunchukY = nunchuk.accelY;
  nunchukX = nunchuk.accelX;
  if(new_color != WHITE)
  {
    m1.set_speed(0);
    m2.set_speed(0);
  }
  else
  {
  if(((speedX < 50)&&(speedX > -50))) 
  {
    speedX = 0;
  }
  
  speedY = getForwardSpeed(nunchukY);
  speedX = getTurnSpeed(nunchukX);
  
  if(speedY >= 0) 
  {
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
  }
  else
  {
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
  }
 
  /* if(b.is_pressed()) //Used for testing
  {
  hold *= -1;
  
  }
  */
 
  /*if(((speed < 40)&&(speed > -45)) || (new_color != RED)) //Used for testing
  {
    speed = 0;
    nunchuk.init();
    
  }
  previousNunchukY = nunchukY;
  */
  
  //Cap the speed of the motors
  if(speedL > 255)
  {
    speedL = 255;
  }
  else if (speedL < -255)
  {
    speedL = -255;
  }
  
  if(speedR > 255)
  {
    speedR = 255;
  }
  else if (speedR < -255)
  {
    speedR = -255;
  }
  
  if(((speedL < 50)&&(speedL > -50))) // Minimum Speed
  {
    speedL = 0;
  }
  
  if(((speedR < 50)&&(speedR > -50))) // Minimum Speed
  {
    speedR = 0;
  }
  
  m1.set_speed(speedL);
  m2.set_speed(speedR);
  }
}

void automatic(int new_color) //No Wii Nunchik, vehicle will move on its own
{
  if (new_color == GREEN)
  {
    speedL = 0;
    speedR = 0;
  }
  else 
  {
    if (new_color == BLUE)    //blue on the left side, turn right a little
    {
      speedL = 120;
      speedR = 0; 
      delay(10);
    }
    else if(new_color == YELLOW)   //yellow on the right side, turn left a little
    {
      speedL = 0;
      speedR = 120;   
      delay(10);   
    }
    else
    {
      speedL = 120;
      speedR = 120;
    }
    /*else if(new_color == GREEN)
    {
      speedL=0;
      speedR=0;
    }*/
  }
    delay(1);
    m1.set_speed(speedL);    // half speed
    m2.set_speed(speedR);    // half speed
}
