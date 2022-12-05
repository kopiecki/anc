EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 7 7
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text HLabel 3900 1750 0    50   Input ~ 0
+5_VDC_D
$Comp
L power:GND #PWR?
U 1 1 643964FF
P 4750 1550
AR Path="/643964FF" Ref="#PWR?"  Part="1" 
AR Path="/63A64E34/643964FF" Ref="#PWR?"  Part="1" 
AR Path="/6437A1A7/643964FF" Ref="#PWR?"  Part="1" 
F 0 "#PWR?" H 4750 1300 50  0001 C CNN
F 1 "GND" H 4755 1377 50  0000 C CNN
F 2 "" H 4750 1550 50  0001 C CNN
F 3 "" H 4750 1550 50  0001 C CNN
	1    4750 1550
	0    1    1    0   
$EndComp
$Comp
L Connector:Conn_01x02_Male J?
U 1 1 64396505
P 4600 1250
AR Path="/64396505" Ref="J?"  Part="1" 
AR Path="/63A64E34/64396505" Ref="J?"  Part="1" 
AR Path="/6437A1A7/64396505" Ref="J?"  Part="1" 
F 0 "J?" H 4300 1300 50  0000 C CNN
F 1 "Conn_01x02_Male" H 4250 1200 50  0000 C CNN
F 2 "" H 4600 1250 50  0001 C CNN
F 3 "~" H 4600 1250 50  0001 C CNN
	1    4600 1250
	-1   0    0    1   
$EndComp
$Comp
L Connector:Jack-DC J?
U 1 1 6439650B
P 5050 1650
AR Path="/6439650B" Ref="J?"  Part="1" 
AR Path="/63A64E34/6439650B" Ref="J?"  Part="1" 
AR Path="/6437A1A7/6439650B" Ref="J?"  Part="1" 
F 0 "J?" H 5107 1975 50  0000 C CNN
F 1 "Output jack" H 5107 1884 50  0000 C CNN
F 2 "Connector_BarrelJack:BarrelJack_CUI_PJ-036AH-SMT_Horizontal" H 5100 1610 50  0001 C CNN
F 3 "~" H 5100 1610 50  0001 C CNN
	1    5050 1650
	-1   0    0    1   
$EndComp
Wire Wire Line
	4400 1750 4400 1250
Wire Wire Line
	4400 1750 4750 1750
$Comp
L power:GND #PWR?
U 1 1 64396513
P 4250 1150
AR Path="/64396513" Ref="#PWR?"  Part="1" 
AR Path="/63A64E34/64396513" Ref="#PWR?"  Part="1" 
AR Path="/6437A1A7/64396513" Ref="#PWR?"  Part="1" 
F 0 "#PWR?" H 4250 900 50  0001 C CNN
F 1 "GND" H 4255 977 50  0000 C CNN
F 2 "" H 4250 1150 50  0001 C CNN
F 3 "" H 4250 1150 50  0001 C CNN
	1    4250 1150
	1    0    0    -1  
$EndComp
Wire Wire Line
	4250 1150 4400 1150
Wire Wire Line
	4400 1750 3900 1750
Connection ~ 4400 1750
$EndSCHEMATC
