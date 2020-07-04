/* Mustafa Murat Akçayiğit & Ayberk Hami Bulut Bitirme Projesi'ne ilişkin kodları içerir */

/* NEXTION Display Configurations */
#include <Nextion.h>    // Main Library for Nextion TFT Display
#include <NexNumber.h>  // Library for displaying Numerals on Nextion TFT Display
#include <NexTimer.h>   // Library of timer

SoftwareSerial HMISerial(10, 11); // Software serial connection to Nextion TFT Display RX-TX

/* Initializing Required Variables */
volatile int sensorValue = 0;       // Pressure Transmitter Output Value
int valfPin = 12;                   // Selenoid Valf Control pin is attached to digital pin 13
int compressPin = 7;                // Compressor Control pin is attached to digital pin 7
volatile double analogValue = 0.0;  // Analog Voltage value of the Sensor Output data
volatile double chamberPressure = 0.0; // Actual Chamber Pressure
volatile double userPressure = 0.0; // Obtained desired pressure from Screen
boolean controlOperating = false;

// Start Button Decleration
NexPicture startBut = NexPicture(0, 13, "p1");    /* Nextion Picture information as a Button for START */
uint32_t number;  /* Keeps value of the set picture numbers */

// Up-Down Button Decleration
NexButton uP = NexButton(0, 7, "b1");
NexButton dowN = NexButton(0, 8, "b2");
boolean stopPresRead = false;

// Get Desired Pressure Value From USER
NexNumber pressureReadW = NexNumber(0, 5, "n2"); //Whole Part of the given pressure by USER
NexNumber pressureReadF = NexNumber(0, 6, "n3"); //Fraction Part of the given pressure by USER
uint32_t pressureReadWh;
uint32_t pressureReadFr;

// PRESSURE DISPLAY SETUP FOR MAIN PAGE
/*  In order to Send a non integer value we need to divide
     the Chamber Pressure
    value into two integer values and then send to the TFT Display
*/
int32_t wholePressure = 0 ;
int32_t fractPressure = 0 ;
NexNumber pressureWriteW = NexNumber(0, 2, "n0");
NexNumber pressureWriteF = NexNumber(0, 3, "n1");

// PRESSURE DISPLAY SETUP FOR OPERATION PAGE
NexNumber pressureWriteW1 = NexNumber(1, 7, "n4");
NexNumber pressureWriteF1 = NexNumber(1, 8, "n5");

// Pressure System is Completed Successfully
NexPicture donePic = NexPicture(0, 11, "p3");    /* Nextion Picture information as a Button for START */
uint32_t doneNumber = 12; /* Keeps value of the set picture numbers */
NexTimer tm1 = NexTimer(1, 16, "tm1");

void uPPopCallback(void *ptr)
{
  Serial.println("Yukari Basildi");
  stopPresRead = true;
}

void dowNPopCallback(void *ptr)
{
  Serial.println("Asagi Basildi");
  stopPresRead = true;
}

// START BUTTON FUNCTION
void startButPopCallback(void *ptr)
{
  Serial.println("Butona Basildi");
  delay(1000);
  startBut.getPic(&number);

  if (number == 1)
  {
    Serial.println("startin icine geldi");
    stopPresRead = false;
    controlOperating = true;
    pressureReadW.getValue(&pressureReadWh);
    pressureReadWh = pressureReadWh * 10;

    pressureReadF.getValue(&pressureReadFr);

    userPressure = pressureReadWh + pressureReadFr;
  }
}

NexTouch *nexListenList[] =
{
  &startBut,
  &uP,
  &dowN,
  NULL
};

// SETUP
void setup()
{
  Serial.begin(9600);
  pinMode(valfPin, OUTPUT);
  pinMode(compressPin, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  analogReference(INTERNAL);

  startBut.attachPop(startButPopCallback, &startBut);
  uP.attachPop(uPPopCallback, &uP);
  dowN.attachPop(dowNPopCallback, &dowN);

  nexInit();

  Serial.println("Setup Completed");
}

// Main Loop
void loop()
{
  nexLoop(nexListenList);

  if (stopPresRead == false)
  {
    readPressure(10);
  }
  if (controlOperating == true)
  {
    operating();
  }
}

// ANALOG READ PRESSURE (Resolution : 0.1)
//        &
// Transmit to the Screen
void readPressure(int iter)
{
  int val;
  float val2;
  short int i = 0;
  val2 = 0;
  for (int c = 0; c <10; c++)

  {
    val = analogRead(A5);
    val2 += (val/10.0);
  }
  
  sensorValue = int(val2);
  analogValue = sensorValue * (1.56 / 1023);
  chamberPressure = (analogValue / 0.1085) * 10;

  fractPressure = chamberPressure;
  wholePressure = chamberPressure / 10;
  fractPressure = fractPressure % 10;

  if (controlOperating == true)
  {
    pressureWriteW1.setValue(wholePressure);
    pressureWriteF1.setValue(fractPressure);
  }else
  {
    pressureWriteW.setValue(wholePressure);
    pressureWriteF.setValue(fractPressure);
  }
}

void operating()
{
  Serial.println("Operating Basladi");
  if (int(chamberPressure) < userPressure)
  {
    digitalWrite(compressPin, HIGH);
  }
  else if ((int)chamberPressure > userPressure)
  {
    digitalWrite(valfPin, HIGH);
  }
  else
  {
    digitalWrite(valfPin, LOW);
    digitalWrite(compressPin, LOW);
    donePic.setPic(doneNumber);
    tm1.enable();
    controlOperating = false;
  }
}
