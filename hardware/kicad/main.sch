EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 5
Title "Active noise reduction development board - main"
Date "2022-11-20"
Rev "0.1"
Comp "Daniel Kopiecki"
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Sheet
S 4850 3300 1150 350 
U 63A8EBCD
F0 "Input" 50
F1 "input.sch" 50
F2 "MIC_IN" O R 6000 3400 50 
F3 "MIC_ERR" O R 6000 3550 50 
F4 "+1.25_OFFSET" I L 4850 3400 50 
$EndSheet
$Sheet
S 4850 3850 1150 350 
U 63A64E34
F0 "Output" 50
F1 "output.sch" 50
F2 "DAC_OUT" I R 6000 3950 50 
F3 "DAC_NOISE" I R 6000 4100 50 
$EndSheet
$Sheet
S 3300 3300 1300 200 
U 63A2DB78
F0 "Power supply" 50
F1 "psu.sch" 50
F2 "+1.25_OFFSET" O R 4600 3400 50 
$EndSheet
$Sheet
S 6200 3300 1150 900 
U 63B0C660
F0 "ADC/DAC" 50
F1 "adc_dac.sch" 50
F2 "MIC_IN" I L 6200 3400 50 
F3 "DAC_OUT" O L 6200 3950 50 
F4 "DAC_NOISE" O L 6200 4100 50 
F5 "MIC_ERR" I L 6200 3550 50 
$EndSheet
Text Notes 550  850  0    197  ~ 39
Main
Wire Wire Line
	6000 3400 6200 3400
Wire Wire Line
	6000 3550 6200 3550
Wire Wire Line
	6000 3950 6200 3950
Wire Wire Line
	6000 4100 6200 4100
Wire Wire Line
	4600 3400 4850 3400
$EndSCHEMATC
