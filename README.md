# EFIS_ModuleA

This is Arduino software for the Module A

Details are here: http://experimentalavionics.com/efis-module-a/

Unzip libraries.zip to the "libraries" folder in your Arduino environment.

## Release Notes: ##

### 2023-06-27 ###
* Removed QNH re-broadcasting as unnecessary
* Added code to read and broadcast backup battery data from CAN HUB ver 3

### 2021-06-08 ###
* Bug fix for QNH setting

### 2021-06-01 ###

* Altitude sensor filtering improvements. Moving Average used for filtering. Depth of the filtering is 60 points (about 1.5 seconds span)
* Linear regression used for VSI calculations. VSI stability improved significantly. Regression calculated over 15 point spaced about 200ms apart. Altitude points are taken from filtered altitude data (Moving Average).
* Single dimension Kalman filter has been fully removed from the code.
