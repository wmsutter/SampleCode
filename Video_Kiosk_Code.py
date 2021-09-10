import RPi.GPIO as GPIO
import time
import os
import sys
from subprocess import Popen

GPIO.setmode(GPIO.BCM)
GPIO.setup(17, GPIO.IN, pull_up_down=GPIO.PUD_UP) #seek to 00m
GPIO.setup(18, GPIO.IN, pull_up_down=GPIO.PUD_UP) #seek to 55m
GPIO.setup(23, GPIO.IN, pull_up_down=GPIO.PUD_UP) #seek to 65m
GPIO.setup(24, GPIO.IN, pull_up_down=GPIO.PUD_UP) #quit player
GPIO.setup(22, GPIO.IN, pull_up_down=GPIO.PUD_UP) #play/pause
GPIO.setup(27, GPIO.IN, pull_up_down=GPIO.PUD_UP) #seek to 70m0.5s
GPIO.setup(12, GPIO.IN, pull_up_down=GPIO.PUD_UP) #seek fwd 1m
GPIO.setup(13, GPIO.IN, pull_up_down=GPIO.PUD_UP) #seek bwd 1m
GPIO.setup(5, GPIO.IN, pull_up_down=GPIO.PUD_UP)  #seek fwd 5m
GPIO.setup(6, GPIO.IN, pull_up_down=GPIO.PUD_UP)  #seek bwd 5m

#filepath for movie
movie1 = ("/home/pi/Videos/MoonriseTimerAndCredits.mov")

#initialize status variables
last_state00 = True
last_state55 = True
last_state65 = True
last_stateQ = True
last_stateP = True
last_state70 = True
last_stateF1 = True
last_stateB1 = True
last_stateF5 = True
last_stateB5 = True

input_state00 = True
input_state55 = True
input_state65 = True
input_stateQ = True
input_stateP = True
input_state70 = True
input_stateF1 = True
input_stateB1 = True
input_stateF5 = True
input_stateB5 = True

#open video player, wait 5s
omxc = Popen(['omxplayer','-l 00:00:00','--no-osd','--loop',movie1])
video_open = True
time.sleep(5)

#seek to 0s, wait 1s, then pause
os.system('dbuscontrol.sh setposition 0')
time.sleep(0.1)
os.system('dbuscontrol.sh pause')
paused = True
time.sleep(0.1)

while True:
    #Read states of inputs
    input_state00 = GPIO.input(17) #seeks to 0 and pauses
    input_state55 = GPIO.input(18) #seeks to 55 and pauses
    input_state65 = GPIO.input(23) #seeks to 65 and pauses
    input_stateQ = GPIO.input(24) #exits omxplayer
    input_stateP = GPIO.input(22) #plays or pauses
    input_state70 = GPIO.input(27) #seeks to 70 and pauses
    input_stateF1 = GPIO.input(12) #seeks fwd 1 min
    input_stateB1 = GPIO.input(13) #seeks bwd 1 min
    input_stateF5 = GPIO.input(5) #seeks fwd 5 min
    input_stateB5 = GPIO.input(6) #seeks bwd 5 min
    
    time.sleep(0.005)

    #if GPIO(17) is grounded seek to 0, play 0.71s, and pause
    if input_state00 != last_state00:
        if (video_open and not input_state00):
            if paused:
                os.system('dbuscontrol.sh pause')
                time.sleep(0.01)
                paused = False
            os.system('dbuscontrol.sh setposition 1')
            video_open = True
            time.sleep(0.7)
            if not paused:
                os.system('dbuscontrol.sh pause')
                time.sleep(0.01)
                paused = True
        elif not input_state00:
            omxc = Popen(['omxplayer','-l 00:00:01','--no-osd',movie1])
            video_open = True
            time.sleep(4)
            os.system('dbuscontrol.sh pause')            
            paused = True

    #if GPIO(18) is grounded, seek to 55:19, play 0.71s, and pause
    if input_state55 != last_state55:
        if (video_open and not input_state55):
            if paused:
                os.system('dbuscontrol.sh pause')
                time.sleep(0.01)
                paused = False
            os.system('dbuscontrol.sh setposition 3319000000')
            video_open = True
            time.sleep(0.7)
            if not paused:
                os.system('dbuscontrol.sh pause')
                time.sleep(0.01)
                paused = True            
        elif not input_state55:
            omxc = Popen(['omxplayer','-l 00:55:19','--no-osd',movie1])
            video_open = True
            time.sleep(4)
            os.system('dbuscontrol.sh pause')
            paused = True

    #if GPIO(23) is grounded, seek to 65, play 0.71s, and pause
    if input_state65 != last_state65:
        if (video_open and not input_state65):
            if paused:
                os.system('dbuscontrol.sh pause')
                time.sleep(0.01)
                paused = False
            os.system('dbuscontrol.sh setposition 3900000000')
            video_open = True
            time.sleep(0.7)
            if not paused:
                os.system('dbuscontrol.sh pause')
                time.sleep(0.01)
                paused = True            
        elif not input_state65:
            omxc = Popen(['omxplayer','-l 01:05:00','--no-osd',movie1])
            video_open = True
            time.sleep(4)
            os.system('dbuscontrol.sh pause')
            paused = True

    #if GPIO(27) is grounded, seek to 70, play 0.71s, and pause
    if input_state70 != last_state70:
        if (video_open and not input_state70):
            if paused:
                os.system('dbuscontrol.sh pause')
                time.sleep(0.01)
                paused = False
            os.system('dbuscontrol.sh setposition 4200000000')
            video_open = True
            time.sleep(0.7)
            if not paused:
                os.system('dbuscontrol.sh pause')
                time.sleep(0.01)
                paused = True            
        elif not input_state65:
            omxc = Popen(['omxplayer','-l 01:10:00','--no-osd',movie1])
            video_open = True
            time.sleep(4)
            os.system('dbuscontrol.sh pause')
            paused = True

    #if GPIO(12) is grounded, seek fwd 1 min
    if input_stateF1 != last_stateF1:
        if(video_open and not input_stateF1):
            os.system('dbuscontrol.sh seek 60000000')
            time.sleep(0.01)

    #if GPIO(13) is grounded, seek bwd 1 min
    if input_stateB1 != last_stateB1:
        if(video_open and not input_stateB1):
            os.system('dbuscontrol.sh seek -60000000')
            time.sleep(0.01)

    #if GPIO(5) is grounded, seek fwd 5 min
    if input_stateF5 != last_stateF5:
        if(video_open and not input_stateF5):
            os.system('dbuscontrol.sh seek 300000000')
            time.sleep(0.01)
    
    #if GPIO(6) is grounded, seek bwd 5 min
    if input_stateB5 != last_stateB5:
        if(video_open and not input_stateB5):
            os.system('dbuscontrol.sh seek -300000000')
            time.sleep(0.01)

    #if GPIO(24) is grounded, exit video player
    if (input_stateQ != last_stateQ and not input_stateQ):
        time.sleep(0.1)
        input_stateQ = GPIO.input(24)
        if video_open and not input_stateQ:
            if paused:
                os.system('dbuscontrol.sh pause')
                time.sleep(0.005)
                paused = False
            os.system('dbuscontrol.sh stop')
            time.sleep(0.1)
            video_open = False

    #if GPIO(22) is grounded, toggle between pause / play
    if (input_stateP != last_stateP and not input_stateP):
        if paused:
            os.system('dbuscontrol.sh pause')
            time.sleep(0.005)
            paused = False
        elif not paused:
            os.system('dbuscontrol.sh pause')
            time.sleep(0.005)
            paused = True

    #Update most recent input states
    last_state00 = input_state00
    last_state55 = input_state55
    last_state65 = input_state65
    last_stateQ = input_stateQ
    last_stateP = input_stateP
    last_state70 = input_state70
    last_stateF1 = input_stateF1
    last_stateB1 = input_stateB1
    last_stateF5 = input_stateF5
    last_stateB5 = input_stateB5
