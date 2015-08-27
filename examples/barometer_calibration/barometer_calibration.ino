/*
 MPL3115A2 Barometric Pressure Sensor - Calibration Code
 by https://github.com/mariocannistra
 
 i've put together this code to obtain more precise values 
 from the sensor when using it both for altitude and pressure measurements
 
 How I glued things together:
 - slightly customized the code by http://www.henrylahr.com/?p=99
 - used the offset routines from Michael Lange on mbed.org
 - completed with elevation_offset calculation by digitalmisery comment at https://www.sparkfun.com/products/11084
 - then packaged the calibration function within the original 
   library code by Nathan Seidle - SparkFun Electronics
 - merged with latest Sparkfun fixes
 - added some () in a couple of IFs per compiler warnings/suggestions
 - added the proper register config calls sequences using setModeStandby() and setModeActive()

 License: please refer to the original sources for the license stuff. I guess it's all public domain but please check. About my additions: you can do what you want with the code, I've no time to figure out the corresponding license name :-)

 */

#include <Wire.h>
#include "MPL3115A2.h"

//Create an instance of the object
MPL3115A2 baro3115;

// start altitude to calculate mean sea level pressure in meters
// for barometer calibration purposes:
#define ALTBASIS 331.10	// altitude of my house lab

// I got this altitude value using a specific GPS setup and config:
//   received D-GPS messages from ESA EGNOS satellite correction data
//   selected the elevation values with longer age over 30 minutes measurement timeframe
//   where age means: continuous reception of same elevation value for 
//   the longest time with highest number of satellites and lowest vertical error estimation

void doCalibration()
{
	//calculate pressure for current ALTBASIS altitude by averaging a few readings
	Serial.println("Starting pressure calibration...");

	// this function calculates a new sea level pressure ref value
	// I adde this function to the original library and it will NOT change the sensor registers
	// see below setBarometricInput() where that value is actually set
	// in the registers. the sensor will start using it just after.
	baro3115.runCalibration(ALTBASIS);

	Serial.print("calculated sea level pressure: ");
	Serial.print(baro3115.calculated_sea_level_press, 2);
	Serial.println(" Pa");

	Serial.print("calculated elevation_offset: ");
	Serial.print(baro3115.elevation_offset, 2);
	Serial.println(" Pa");

	// i originally had big errors in pressure and altitude readouts,
	// once i added the elevation_offset calculation the error
	// decreased and is now close to that of another barometer
	// (whose manual calibration for altitude should be checked anyway)

	// I initally implemented this code without using the calls to setModeStandby() and setModeActive()
	// the results were polluted by occasional weird behavior with exceedingly large and wrong values.
	// Also, the temperature measurements were constantly increasing, probably due to the 
	// lower oversample rate and faster reading loop
	// I decided to compare the various libraries on github for this sensor and found out that
	// both Sparkfun's and Adafruit's versions did NOT use a specific calls sequence when setting
	// mode and registers !
	// Michael Lange's sequence instead, is the one that finally gave me correct, smooth and repeatable measurement sessions. See below:
	
	baro3115.setModeStandby();		// <-- this one starts a config sequence
	baro3115.setModeBarometer();
	baro3115.setBarometricInput(baro3115.calculated_sea_level_press);
	baro3115.setOversampleRate(7);
	baro3115.enableEventFlags();
	baro3115.setModeActive();		// <-- this one ends the sequence and starts the measurement mode

	// calibration is now completed
	//
	// setBarometricInput() :
	// This configuration option calibrates the sensor according to
	// the sea level pressure for the measurement location (2 Pa per LSB)
	// The default value for "Barometric input for Altitude calculation" is 101,326 Pa

	// About the oversample rate:
	// Set the # of samples from 1 to 128. See datasheet.
	// Integer values between 0 < n < 7 give oversample ratios 2^n and 
	// sampling intervals of 0=6 ms , 1=10, 2=18, 3=34, 4=66, 5=130, 6=258, and 7=512
	// Seems that the suggested value is 7 and the measurement could take 512ms
	// I'm using it with good results, but I'm still trying to understand why the time taken
	// for the pressure reading is just 3ms (measured with the millis() function)
	
	// add temperature offset for my tested sensor
	// seems the temperature probe is within the ADC and should not be used
	// to measure the environment. Will try adding the offset i got by comparison
	// with another thermometer
	// you can enable this if you need it:
	// baro3115.setModeStandby();
	// baro3115.setOffsetTemperature((char) (0.65 / 0.0625) );
	// baro3115.setModeActive();

	Serial.println("Pressure calibration completed.");

	// let's have a look, just in case:
	//Serial.println("OFFSETS:");
	//Serial.print("  pressure: ");
	//Serial.println(baro3115.offsetPressure() ,2);
	//Serial.print("  altitude: ");
	//Serial.println((float)baro3115.offsetAltitude() ,2);
	//Serial.print("  temperature(C): ");
	//Serial.println(baro3115.offsetTemperature() ,2);

}

void setup()
{
	Wire.begin();        // Join i2c bus
	Serial.begin(115200);  // Start serial for output

	baro3115.begin(); // Get sensor online

	baro3115.setModeStandby();		// <-- this one starts a config sequence
	baro3115.setModeBarometer();
	baro3115.setOffsetPressure(0);
	baro3115.setOffsetTemperature(0);
	baro3115.setOffsetAltitude(0);
	baro3115.setBarometricInput(0.0);
	baro3115.elevation_offset = 0;
	baro3115.calculated_sea_level_press = 0;
	baro3115.setOversampleRate(7);
	baro3115.enableEventFlags();
	baro3115.setModeActive();		// <-- this one ends the sequence and starts the measurement mode

	Serial.println("BEFORE calibration...");

	// show sample data readouts before starting the calibration:
	outData();

	// now perform the calibration
	doCalibration();
	// calibration performed
}

void outData()
{
	baro3115.setModeStandby();
	baro3115.setModeBarometer();
	baro3115.setOversampleRate(7);
	baro3115.enableEventFlags();
	baro3115.setModeActive();
	// when we are using the calibration then we also have to add the
	// calculated elevation related pressure offset to our readings:
	float pressure = baro3115.readPressure() + baro3115.elevation_offset;

	// output is in Pa
	// 1 kPa = 10 hPa = 1000 Pa
	// 1 hPa = 100 Pascals = 1 mb
	pressure = (pressure / 100) ;		//  ... / 1000 * 10 ;
	Serial.print("Pressure(hPa): ");
	Serial.print(pressure, 2);

	float temperature = baro3115.readTemp();
	Serial.print(" Temperature(C): ");
	Serial.print(temperature, 2);

	baro3115.setModeStandby();
	baro3115.setModeAltimeter();
	baro3115.setOversampleRate(7);
	baro3115.enableEventFlags();
	baro3115.setModeActive();
	float altitude = baro3115.readAltitude();
	Serial.print(" Altitude above sea(m): ");
	Serial.print(altitude, 2);

	Serial.println();
}

void loop()
{
	outData();
}
