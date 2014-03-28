#include <18F26K80.h>
#device adc=16

#FUSES NOWDT                    //No Watch Dog Timer
#FUSES SOSC_DIG                 //Digital mode, I/O port functionality of RC0 and RC1
#FUSES NOXINST                  //Extended set extension and Indexed Addressing mode disabled (Legacy mode)
#FUSES HSH                      //High speed Osc, high power 16MHz-25MHz
#FUSES NOPLLEN                  //4X HW PLL disabled, 4X PLL enabled in software
#FUSES BROWNOUT               
#FUSES PUT
#FUSES NOIESO
#FUSES NOFCMEN
#FUSES NOPROTECT
#FUSES CANC  //Enable to move CAN pins to C6(TX) and C7(RX) 

//#FUSES NOWDT                    //No Watch Dog Timer
//#FUSES INTRC                    //Internal RC Osc
//#FUSES NOBROWNOUT               //No brownout reset
//#FUSES NOLVP                    //No low voltage prgming, B3(PIC16) or B5(PIC18) used for I/O

#use delay(clock = 20000000)
#include "../../Shared/Codeflex_lcd_PROT6.c"

void main()
{
////  lcd_init()   Must be called before any other function.               ////
////                                                                       ////
////  lcd_putc(c)  Will display c on the next position of the LCD.         ////
////                 \a  Set cursor position to upper left                 ////
////                 \f  Clear display, set cursor to upper left           ////
////                 \n  Go to start of second line                        ////
////                 \b  Move back one position                            ////
////              If LCD_EXTENDED_NEWLINE is defined, the \n character     ////
////              will erase all remanining characters on the current      ////
////              line, and move the cursor to the beginning of the next   ////
////              line.                                                    ////
////              If LCD_EXTENDED_NEWLINE is defined, the \r character     ////
////              will move the cursor to the start of the current         ////
////              line.                                                    ////
////                                                                       ////
////  lcd_gotoxy(x,y) Set write position on LCD (upper left is 1,1)     
   setup_comparator(NC_NC_NC_NC);// This device COMP currently not supported by the PICWizard
   
lcd_init();
unsigned int counter = 0;
here:
printf(lcd_putc, "\f1.BOOBS (.)(.) \n yolo ");

delay_ms(350);
while(1){
   OUTPUT_TOGGLE(PIN_B1);
   delay_ms(100);
   counter++;
   lcd_gotoxy(1,2);
   printf(lcd_putc, "%3u", counter);
   if (counter == 255)
   {goto here;}
   
}
}
