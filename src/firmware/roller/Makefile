# Arduino Make file. Refer to https://github.com/sudar/Arduino-Makefile
BOARD_TAG    = uno
MONITOR_PORT = /dev/ttyACM0
#MONITOR_PORT = $(shell list_arduinos.py torso)
AVRDUDE_OPTS = -v
# Note, you must copy these to USER_LIB_PATH, TODO:fixme
#ARDUINO_LIBS = CapacitiveSensor
#ARDUINO_SKETCHBOOK = /home/$(USER)/sketchbook
ARDUINO_SKETCHBOOK = $(realpath .)/lib
USER_LIB_PATH = $(realpath .)/lib
include /usr/share/arduino/Arduino.mk
