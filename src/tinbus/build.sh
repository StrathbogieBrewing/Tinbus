#!/bin/bash
arduino-cli compile --fqbn arduino:avr:uno Tinbus
arduino-cli upload -p /dev/ttyACM1 --fqbn arduino:avr:uno Tinbus