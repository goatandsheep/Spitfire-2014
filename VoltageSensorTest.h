#include <16F883.h>
#device adc=16

#FUSES NOWDT                 	//No Watch Dog Timer
#FUSES INTRC                 	//Internal RC Osc
#FUSES NOBROWNOUT            	//No brownout reset
#FUSES NOLVP                 	//No low voltage prgming, B3(PIC16) or B5(PIC18) used for I/O

#use delay(int=8000000)
#use rs232(baud=9600,parity=N,xmit=PIN_C6,rcv=PIN_C7,bits=8,stream=PORT1)
#use i2c(Master,Fast,sda=PIN_C4,scl=PIN_C3)

