# Flog(g)r - a field data logger #

## Background ##

Flog(g)r is intended to be the basis for a robust field data logger for e.g. geography field studies. It was designed and prototyped as part of the first [Field Studies Hackday](http://fschackday.wordpress.com) in May 2012, and won the award for the prototype with the Most Potential. It uses a number of sensors attached to an Arduino and logs data to an SD card as a button is pressed at specific locations during field studies. It is lower-cost, lower-power, more versatile and more robust than smartphone-based platforms.

## Components ##

A number of hardware components are used in the prototype:

 * Arduino Uno
 * Sparkfun GPS shield with EMP406A GPS chip
 * BMP085 barometric sensor
 * SD card breakout board

## Design information ##

A Fritzing breadboard wiring diagram is included, in the design subdirectory. Note that not all of the components (specifically the Sparkfun SD card breakout board) are represented in Fritzing so wiring information is included in a sticky note instead with an approximate equivalent shown on the breadboard.

![wiring diagram](https://github.com/andypiper/fsc_flogr/raw/master/design/floggr_bb.png)

## Future ##

It is clear that this is an incomplete proof-of-concept and could be considerably enhanced:

 * some parts of the code are in need of attention to improve their efficiency (commented inline)
 * field and user testing could be carried out
 * a more elegant boxed version could be produced * it would be possible to add a simple LCD display to confirm that data has been recorded, or display current sensor values
 * a port (3.5mm jack?) could be added to enable temperature and soil analysis probes to be attached. 
 * although point-in-time data recording is a design point of the current design, by putting a simple config.txt file onto the SD card and having that read at startup, behaviour could be modified so that the logger did run continuously, log at configurable intervals, only log on demand, etc.
 * data analysis tools - both local and and webapp-based - could be developed to visualise and explore the recorded data points.

### Acknowledgements ###

[Andy Piper (@andypiper)](http://twitter.com/andypiper) wrote the code, so it's largely his fault.

Nathan, Katie and Barney from Bristol Hackspace gave amazing support with advice, lent components, and shared ideas.

Neil Ford wrote and tested the code for the BMP085 and assembled the final composite breadboard.

Every single person involved in #fschack was AMAZING.
