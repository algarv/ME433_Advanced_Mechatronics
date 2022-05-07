#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<stdio.h>
#include <math.h>
#include <time.h>

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

char buffer[500];

#define ADD_W 
#define ADD_R
#define GPIO //SFR for GPIO
#define OLAT

// Read from UART3
// block other functions until you get a '\r' or '\n'
// send the pointer to your char array and the number of elements in the array
void NU32_ReadUART2(char * message, int maxLength) {
  char data = 0;
  int complete = 0, num_bytes = 0;
  // loop until you get a '\r' or '\n'
  while (!complete) {
    if (U2STAbits.URXDA) { // if data is available
      data = U2RXREG;      // read the data
      if ((data == '\n') || (data == '\r')) {
        complete = 1;
      } else {
        message[num_bytes] = data;
        ++num_bytes;
        // roll over if the array is too small
        if (num_bytes >= maxLength) {
          num_bytes = 0;
        }
      }
    }
  }
  // end the string
  message[num_bytes] = '\0';
}

// initialize SPI1
void initSPI() {
    // Pin B14 has to be SCK1
    // Turn off analog pins
    ANSELA = 0;
    // Make an output pin for CS
    TRISBbits.TRISB13 = 0;
    LATBbits.LATB13 = 0;
    
    // Set SDO1 (B5)
    RPB5Rbits.RPB5R = 0b0011; 
    // Set SDI1 (B14)
    RPB11Rbits.RPB11R = 0b0011;
    
    // setup SPI1
    SPI1CON = 0; // turn off the spi module and reset it
    SPI1BUF; // clear the rx buffer by reading from it
    SPI1BRG = (48000000/(2000000))-1; // 1000 for 24kHz, 1 for 12MHz; // baud rate to 10 MHz [SPI1BRG = (48000000/(2*desired))-1]
    SPI1STATbits.SPIROV = 0; // clear the overflow bit
    SPI1CONbits.CKE = 1; // data changes when clock goes from hi to lo (since CKP is 0)
    SPI1CONbits.MSTEN = 1; // master operation
    SPI1CONbits.ON = 1; // turn on spi 
}

// Write a character array using UART3
void NU32_WriteUART2(const char * string) {
  while (*string != '\0') {
    while (U2STAbits.UTXBF) {
      ; // wait until tx buffer isn't full
    }
    U2TXREG = *string;
    ++string;
  }
}

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

unsigned char spi_io(unsigned char o) {
  SPI1BUF = o;
  while(!SPI1STATbits.SPIRBF) { // wait to receive the byte
    ;
  }
  return SPI1BUF;
}

const char* reaction[] = {"hey", "stop", "why", "excuse me", "ma'am", "bro", "ouch"};

unsigned short trig_traj(double A,double B,double C,double D,int i, int c) {
    
    unsigned short dec = A * sin(B * i + C) + D;
  
    unsigned short p;
    p = c << 15;
    p = p | (0b111 << 12);
    p = p | dec << 4;
    
    return p;
}

unsigned short triangle_traj(double A,double B,double C,double D, int i,int c) {
    
    unsigned short dec;
    int steps = 100;
    int num_triangles = B;
    int i_per_triangle = steps / num_triangles;
    double slope = A / (i_per_triangle/2);
    while(i > i_per_triangle){
        i -= i_per_triangle;
    }
    if (i < i_per_triangle/2){
        dec = slope * i;
    }
    else {
        dec = A - (slope*(i-i_per_triangle/2));
    }
    
    unsigned short p;
    p = c << 15;
    p = p | (0b111 << 12);
    p = p | dec << 4;
    
    return p;
}

int main() {

    __builtin_disable_interrupts(); // disable interrupts while initializing things

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;
    
    // Set A1 to U2RX
    U2RXRbits.U2RXR = 0b0000; 
    // Set B15 to U2TX
    RPB15Rbits.RPB15R = 0b0010;

    // initialize B4 as input 
      TRISBbits.TRISB4 = 1;
    // initialize A4 as output, initially off
      TRISAbits.TRISA4 = 0;
      LATAbits.LATA4 = 0;
    
    // turn on UART2 without an interrupt
    U2MODEbits.BRGH = 0; // set baud to 230400
    U2BRG = ((48000000 / 230400) / 16) - 1;

    // 8 bit, no parity bit, and 1 stop bit (8N1 setup)
    U2MODEbits.PDSEL = 0;
    U2MODEbits.STSEL = 0;

    // configure TX & RX pins as output & input pins
    U2STAbits.UTXEN = 1;
    U2STAbits.URXEN = 1;

    // enable the uart
    U2MODEbits.ON = 1;  
          
    __builtin_enable_interrupts();

    initSPI();
    
    /*init I2C
     * start bit
     * write the address with write bit
     * write the register
     * write the value to the register
     * stop bit
    */
    
    
    while (1) {
        //if B4 is pushed, turn on A4 for 0.5s, off for 0.5s, on for 0.5s, and off for 0.5s
        
        if (PORTBbits.RB4 == 0){
            int reply_num = rand() % 7;
            sprintf(buffer, "%s\n", reaction[reply_num]);
            NU32_WriteUART2(buffer);
            blink();
        }

        for (int i=0; i<10000; i++){
            LATBbits.LATB13 = 0; // Chip select low
            unsigned short p = trig_traj(128, (350*M_PI)/10000, 0, 128, i, 1);
            spi_io(p >> 8);
            spi_io(p);
            LATBbits.LATB13 = 1; // Chip select high   

            LATBbits.LATB13 = 0; // Chip select low
            p = triangle_traj(255, 1, 0, 0, i, 0);
            spi_io(p >> 8);
            spi_io(p);
            LATBbits.LATB13 = 1; // Chip select high

//            for (int j = 0; j < 500000; j++) {
//                ;
//            }
        }
        
        /* mcp23008_write(IODIR,0b01111111)
         * mcp23008_write(OLAT,0b100000000)
         * unsigned char r = mcp23008_read(GPIO)
         * r = r & 0b1
         * if (r){
         *  //button is pushed
         * }
         * 
         */
        
    }
}