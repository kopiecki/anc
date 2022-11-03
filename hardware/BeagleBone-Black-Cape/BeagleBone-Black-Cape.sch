EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
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
L BeagleBone-Black-Cape-rescue:BeagleBone_Black_Header P8
U 1 1 55DF7DE1
P 3500 2250
F 0 "P8" H 3500 3450 50  0000 C CNN
F 1 "BeagleBone_Black_Header" V 3500 2250 50  0000 C CNN
F 2 "Socket_BeagleBone_Black:Socket_BeagleBone_Black" H 3500 1400 60  0001 C CNN
F 3 "" H 3500 1400 60  0000 C CNN
	1    3500 2250
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 6357A638
P 1750 3550
F 0 "#PWR?" H 1750 3300 50  0001 C CNN
F 1 "GND" H 1755 3377 50  0000 C CNN
F 2 "" H 1750 3550 50  0001 C CNN
F 3 "" H 1750 3550 50  0001 C CNN
	1    1750 3550
	1    0    0    -1  
$EndComp
Wire Wire Line
	2000 3550 1750 3550
Connection ~ 1750 3550
Wire Wire Line
	1750 3550 1500 3550
Wire Wire Line
	2000 3550 2000 3350
Wire Wire Line
	2000 3250 2000 3350
Connection ~ 2000 3350
Wire Wire Line
	1500 3350 1500 3250
Wire Wire Line
	1500 3550 1500 3350
Connection ~ 1500 3350
$Comp
L BeagleBone-Black-Cape-rescue:BeagleBone_Black_Header P9
U 1 1 55DF7DBA
P 1750 2250
F 0 "P9" H 1750 3450 50  0000 C CNN
F 1 "BeagleBone_Black_Header" V 1750 2250 50  0000 C CNN
F 2 "Socket_BeagleBone_Black:Socket_BeagleBone_Black" H 1750 1400 60  0001 C CNN
F 3 "" H 1750 1400 60  0000 C CNN
	1    1750 2250
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR?
U 1 1 635918DF
P 1750 900
F 0 "#PWR?" H 1750 650 50  0001 C CNN
F 1 "GND" H 1755 727 50  0000 C CNN
F 2 "" H 1750 900 50  0001 C CNN
F 3 "" H 1750 900 50  0001 C CNN
	1    1750 900 
	1    0    0    1   
$EndComp
Wire Wire Line
	1500 1150 1500 900 
Wire Wire Line
	1500 900  1750 900 
Wire Wire Line
	2000 900  2000 1150
Connection ~ 1750 900 
Wire Wire Line
	1750 900  2000 900 
$Comp
L power:GND #PWR?
U 1 1 63597CCF
P 3500 900
F 0 "#PWR?" H 3500 650 50  0001 C CNN
F 1 "GND" H 3505 727 50  0000 C CNN
F 2 "" H 3500 900 50  0001 C CNN
F 3 "" H 3500 900 50  0001 C CNN
	1    3500 900 
	1    0    0    1   
$EndComp
Wire Wire Line
	3250 1150 3250 900 
Wire Wire Line
	3250 900  3500 900 
Wire Wire Line
	3750 900  3750 1150
Connection ~ 3500 900 
Wire Wire Line
	3500 900  3750 900 
Text GLabel 1200 1350 0    50   Input ~ 0
Vref
Wire Wire Line
	1200 1350 1500 1350
Wire Wire Line
	1350 4250 1900 4250
Wire Wire Line
	1350 4450 1900 4450
Wire Wire Line
	1350 4650 1900 4650
Wire Wire Line
	1350 4850 1900 4850
Entry Wire Line
	1900 4250 2000 4350
Entry Wire Line
	1900 4450 2000 4550
Entry Wire Line
	1900 4650 2000 4750
Entry Wire Line
	1900 4850 2000 4950
Wire Bus Line
	2000 4950 2150 4950
Text Label 1500 4250 0    50   ~ 0
MOSI
Text Label 1500 4450 0    50   ~ 0
SCLK
Text Label 1500 4650 0    50   ~ 0
MISO
Text Label 1500 4850 0    50   ~ 0
CS
Text GLabel 2150 4950 2    50   Output ~ 0
SPI0
Text GLabel 1400 1950 0    50   Output ~ 0
SPIO_CS0
Text GLabel 1400 2050 0    50   Output ~ 0
SPIO_CS1
Text GLabel 1400 2150 0    50   Input ~ 0
SPIO_D0
Text GLabel 2100 1950 2    50   Output ~ 0
SPIO_D1
Text GLabel 2100 2050 2    50   Output ~ 0
SPIO_CS0
Text GLabel 2100 2150 2    50   Output ~ 0
SPIO_SCLK
Wire Wire Line
	1400 1950 1500 1950
Wire Wire Line
	1400 2050 1500 2050
Wire Wire Line
	1400 2150 1500 2150
Wire Wire Line
	2000 1950 2100 1950
Wire Wire Line
	2000 2050 2100 2050
Wire Wire Line
	2000 2150 2100 2150
Text GLabel 1350 4250 0    50   Input ~ 0
SPIO_D0
Text GLabel 1350 4450 0    50   Output ~ 0
SPIO_SCLK
Text GLabel 1350 4650 0    50   Output ~ 0
SPIO_D1
Text GLabel 1350 4850 0    50   Output ~ 0
SPIO_CS0
Wire Wire Line
	3150 4250 3700 4250
Wire Wire Line
	3150 4450 3700 4450
Wire Wire Line
	3150 4650 3700 4650
Wire Wire Line
	3150 4850 3700 4850
Entry Wire Line
	3700 4250 3800 4350
Entry Wire Line
	3700 4450 3800 4550
Entry Wire Line
	3700 4650 3800 4750
Entry Wire Line
	3700 4850 3800 4950
Wire Bus Line
	3800 4950 3950 4950
Text Label 3300 4250 0    50   ~ 0
MOSI
Text Label 3300 4450 0    50   ~ 0
SCLK
Text Label 3300 4650 0    50   ~ 0
MISO
Text Label 3300 4850 0    50   ~ 0
CS
Text GLabel 3950 4950 2    50   Output ~ 0
SPI1
Text GLabel 3150 4250 0    50   Input ~ 0
SPI1_D0
Text GLabel 3150 4450 0    50   Output ~ 0
SPI1_SCLK
Text GLabel 3150 4650 0    50   Output ~ 0
SPI1_D1
Text GLabel 3150 4850 0    50   Output ~ 0
SPI1_CS0
Text GLabel 2100 2450 2    50   Output ~ 0
SPI1_CS0
Text GLabel 2100 2550 2    50   Output ~ 0
SPI1_D1
Text GLabel 1400 2650 0    50   Output ~ 0
SPI1_SCLK
Text GLabel 1400 2550 0    50   Input ~ 0
SPI1_D0
Wire Wire Line
	1400 2550 1500 2550
Wire Wire Line
	1400 2650 1500 2650
Wire Wire Line
	2000 2450 2100 2450
Wire Wire Line
	2000 2550 2100 2550
Text GLabel 2100 3150 2    50   Output ~ 0
SPI1_CS0
Wire Wire Line
	2000 3150 2100 3150
Text Notes 2950 700  2    50   ~ 10
BeagleBone Black cape
Text Notes 1750 4050 2    50   ~ 10
SPIO
Text Notes 3550 4050 2    50   ~ 10
SPI1
Wire Notes Line
	850  3800 850  550 
Wire Notes Line
	850  3950 2450 3950
Wire Notes Line
	2450 3950 2450 5100
Wire Notes Line
	2450 5100 850  5100
Wire Notes Line
	850  5100 850  3950
Wire Notes Line
	2600 3950 2600 5100
Wire Notes Line
	4250 3950 4250 5100
Wire Notes Line
	2600 5100 4250 5100
Wire Notes Line
	2600 3950 4250 3950
Wire Notes Line
	4250 3800 4250 550 
Wire Notes Line
	850  550  4250 550 
Wire Notes Line
	850  3800 4250 3800
Text GLabel 1200 1450 0    50   Output ~ 0
Vio
Wire Wire Line
	1200 1450 1500 1450
Wire Bus Line
	2000 4350 2000 4950
Wire Bus Line
	3800 4350 3800 4950
$EndSCHEMATC
