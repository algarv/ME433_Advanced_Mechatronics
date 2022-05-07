#include<sys/attribs.h>  // __ISR macro
#include<stdio.h>
#include <math.h>
#include <time.h>
#include "ws2812b.c"

// DEVCFG0
#pragma config DEBUG = OFF // disable debugging
#pragma config JTAGEN = OFF // disable jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // disable flash write protect
#pragma config BWP = OFF // disable boot write protect
#pragma config CP = OFF // disable code protect

// DEVCFG1
#pragma config FNOSC = FRCPLL // use fast frc oscillator with pll
#pragma config FSOSCEN = OFF // disable secondary oscillator
#pragma config IESO = OFF // disable switching clocks
#pragma config POSCMOD = OFF // primary osc disabled
#pragma config OSCIOFNC = OFF // disable clock output
#pragma config FPBDIV = DIV_1 // divide sysclk freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // disable clock switch and FSCM
#pragma config WDTPS = PS1 // use largest wdt value
#pragma config WINDIS = OFF // use non-window mode wdt
#pragma config FWDTEN = OFF // wdt disabled
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz fast rc internal oscillator
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz

// DEVCFG3
#pragma config USERID = 0 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations

void blink(){
    _CP0_SET_COUNT(0);
    LATAbits.LATA4 = 1;
    while(_CP0_GET_COUNT()<12000000){
        ;
    }
    _CP0_SET_COUNT(0);
    LATAbits.LATA4 = 0;
    while(_CP0_GET_COUNT()<12000000){
        ;
    }
    _CP0_SET_COUNT(0);
    LATAbits.LATA4 = 1;
    while(_CP0_GET_COUNT()<12000000){
        ;
    }
    _CP0_SET_COUNT(0);
    LATAbits.LATA4 = 0;
    while(_CP0_GET_COUNT()<12000000){
        ;
    }
    _CP0_SET_COUNT(0);
}


int main() {

    TRISAbits.TRISA4 = 0;
    LATAbits.LATA4 = 0;
    
    TRISBbits.TRISB10 = 0;
    LATBbits.LATB10 = 0;
    
    ws2812b_setup();
    wsColor c[5];

    while (1) {
        
        blink();
        
        for (int i = 0; i < 5; i++){
            c[i].r = 125;
            c[i].b = 0;
            c[i].g = 0;
        }   
           
        ws2812b_setColor(c, 5);

    }
}