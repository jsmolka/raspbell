#!/bin/bash

pkill -f "python3 -B -u server.py -port 8844 -pin 4"
nohup python3 -B -u server.py -port 8844 -pin 4 > server.log &
