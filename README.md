MPL3115A2 Pressure Sensor library with calibration functions
==================

This library is a modified and extended version of the original at https://github.com/sparkfun/MPL3115A2_Breakout/

List of improvements / changes
------------------

Sensor calibration functions added by https://github.com/mariocannistra
Note: I've put together this code to obtain more precise values and a stable behavior from the sensor when using it both for altitude and pressure measurements

List of code merges and improvements:
- slightly customized the code by http://www.henrylahr.com/?p=99
- used the offset routines by Michael Lange (see https://developer.mbed.org/users/sophtware/code/MPL3115A2/ )
- completed with elevation_offset calculation by digitalmisery comment at https://www.sparkfun.com/products/11084
- then packaged the calibration function within the original 
library code by Nathan Seidle - SparkFun Electronics
- merged with latest Sparkfun fixes by https://github.com/ToniCorinne
- added some () in a couple of IFs per compiler warnings/suggestions
- added the proper register config calls sequences using setModeStandby() and setModeActive()

New example for calibration usage
------------------

I added barometer_calibration.ino in the examples folder.
It shows proper usage of the calibration functions.

License
------------------

Please see below and refer to the original sources for the license stuff. I guess it's all public domain but please check. And if you meet with Nathan for a beer let me know and consider as beerware my code as well.

Original library:
by Nathan Seidle
SparkFun Electronics
Date: September 22nd, 2013
License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
