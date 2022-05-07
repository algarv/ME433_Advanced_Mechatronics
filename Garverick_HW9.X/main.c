#include<sys/attribs.h>  // __ISR macro
#include<stdio.h>
#include <math.h>
#include <time.h>
#include "ws2812b.h"

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
    
    ws2812b_setup();
    wsColor c[5];
    int a[360];
    int b[360];
    int d[360];
    int e[360];
    int f[360];
    
    for (int i = 0; i < 360; i += 1){
            a[i] = i;
            if (i + 15 < 360){
                b[i] = i + 15;
            }
            else{
                b[i] = i + 15 - 360;
            }
            if (i + 30 < 360){
                d[i] = i + 30;                
            }
            else{
                d[i] = i + 30 - 360;
            }
            if (i + 40 < 360){
                e[i] = i + 40;                
            }
            else{
                e[i] = i + 40 - 360;
            }
            if (i + 50 < 360){
                f[i] = i + 50;                
            }
            else{
                f[i] = i + 50 - 360;
            }
    }   
    
    
    while (1) {
        
        blink();
                
        for (int i = 0; i < 360; i += 1){
            
            c[0] = HSBtoRGB(a[i], 1.0, 1.0);
            c[1] = HSBtoRGB(b[i], 1.0, 1.0);
            c[2] = HSBtoRGB(d[i], 1.0, 1.0);
            c[3] = HSBtoRGB(e[i], 1.0, 1.0);
            c[4] = HSBtoRGB(f[i], 1.0, 1.0);
                
            ws2812b_setColor(c, 5);
            int t = 0;
            while (t < 500000){
                t++
                ;
            }
        }  
//        for (int i = 360; i > 0; i -= 1){
//            
//            c[0] = HSBtoRGB(a[i], 1.0, 1.0);
//            c[1] = HSBtoRGB(b[i], 1.0, 1.0);
//            c[2] = HSBtoRGB(d[i], 1.0, 1.0);
//            c[3] = HSBtoRGB(e[i], 1.0, 1.0);
//            c[4] = HSBtoRGB(f[i], 1.0, 1.0);
//                
//            ws2812b_setColor(c, 5);
//            int t = 0;
//            while (t < 500000){
//                t++
//                ;
//            }
//        } 
    }
}