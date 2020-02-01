EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 4 21
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L RF_Module:ESP-12F U7
U 1 1 5E359EF4
P 5050 3700
F 0 "U7" H 5050 4681 50  0000 C CNN
F 1 "ESP-12F" H 5050 4590 50  0000 C CNN
F 2 "RF_Module:ESP-12E" H 5050 3700 50  0001 C CNN
F 3 "http://wiki.ai-thinker.com/_media/esp8266/esp8266_series_modules_user_manual_v1.1.pdf" H 4700 3800 50  0001 C CNN
	1    5050 3700
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR0105
U 1 1 5E35C1D0
P 5050 2600
F 0 "#PWR0105" H 5050 2450 50  0001 C CNN
F 1 "+3.3V" H 5065 2773 50  0000 C CNN
F 2 "" H 5050 2600 50  0001 C CNN
F 3 "" H 5050 2600 50  0001 C CNN
	1    5050 2600
	1    0    0    -1  
$EndComp
Wire Wire Line
	5050 2600 5050 2900
$EndSCHEMATC
