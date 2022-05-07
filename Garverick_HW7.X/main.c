#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro

#include <stdio.h>
#include <math.h>

#include "i2c_master_noint.h"
#include "mpu6050.h"

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
#pragma config WDTPS = PS1048576 // use largest wdt value
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

#define PIC32_SYS_FREQ 48000000ul // 48 million Hz
#define PIC32_DESIRED_BAUD 115200 // Baudrate for RS232

struct IMU_Data
{
    float accelX;
    float accelY;
    float accelZ;
    float gyroX;
    float gyroY;
    float gyroZ; 
};

typedef struct IMU_Data IMU_Data;

struct Orientation{
    float roll;
    float pitch;
    float yaw;
};

typedef struct Orientation Orientation;

void NU32_UART2_Startup(void);
void NU32_ReadUART2(char * string, int maxLength);
void NU32_WriteUART2(const char * string);
Orientation calc_RPY(IMU_Data IMU, Orientation saved_RPY, double dt);
void blink();

int main() {

    __builtin_disable_interrupts(); // disable interrupts while initializing things

    ANSELA = 0;
    ANSELB = 0;
    
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
    // Set B14 to U2TX
    RPB14Rbits.RPB14R = 0b0010;

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
    
    // init the imu
    init_mpu6050();
    
    char m_in[100]; // char array for uart data coming in
    char m_out[200]; // char array for uart data going out
    int i;
    #define NUM_DATA_PNTS 300 // how many data points to collect at 100Hz
    float ax[NUM_DATA_PNTS], ay[NUM_DATA_PNTS], az[NUM_DATA_PNTS], gx[NUM_DATA_PNTS], gy[NUM_DATA_PNTS], gz[NUM_DATA_PNTS], temp[NUM_DATA_PNTS], roll[NUM_DATA_PNTS], pitch[NUM_DATA_PNTS], yaw[NUM_DATA_PNTS];
    
    sprintf(m_out,"MPU-6050 WHO_AM_I: %X\r\n",whoami());
    NU32_WriteUART2(m_out);
    char who = whoami(); // ask if the imu is there
    if (who != 0x68){
        // if the imu is not there, get stuck here forever
        while(1){
            LATAbits.LATA4 = 1;
        }
    }
     
    char IMU_buf[IMU_ARRAY_LEN]; // raw 8 bit array for imu data

    while (1) {
//        blink();
        
        NU32_ReadUART2(m_in,100); // wait for a newline
        // don't actually have to use what is in m
        
        blink();

        Orientation saved_RPY;
        saved_RPY.roll = 0;
        saved_RPY.pitch = 0;
        saved_RPY.yaw = 0;
        // collect data
        for (i=0; i<NUM_DATA_PNTS; i++){
            _CP0_SET_COUNT(0);
            // read IMU
            burst_read_mpu6050(IMU_buf);
            ax[i] = conv_xXL(IMU_buf);
            ay[i] = conv_yXL(IMU_buf);
            az[i] = conv_zXL(IMU_buf);
            gx[i] = conv_xG(IMU_buf);
            gy[i] = conv_yG(IMU_buf);
            gz[i] = conv_zG(IMU_buf);
            temp[i] = conv_temp(IMU_buf);

            IMU_Data sample;
            sample.accelX = ax[i];
            sample.accelY = ay[i];
            sample.accelZ = az[i];
            sample.gyroX = gx[i];
            sample.gyroY = gy[i];
            sample.gyroZ = gz[i];
            
            Orientation calced_RPY = calc_RPY(sample, saved_RPY, .01);
            saved_RPY = calced_RPY;
            
            roll[i] = calced_RPY.roll;
            pitch[i] = calced_RPY.pitch;
            yaw[i] = calced_RPY.yaw;
            while(_CP0_GET_COUNT()<24000000/2/100){}
        }
        
        // print data
        for (i=0; i<NUM_DATA_PNTS; i++){
            sprintf(m_out,"%d %f %f %f %f %f %f %f %f %f %f\r\n",NUM_DATA_PNTS-i,ax[i],ay[i],az[i],gx[i],gy[i],gz[i],temp[i],roll[i], pitch[i], yaw[i]);
            NU32_WriteUART2(m_out);
        }
        
    }
}


Orientation calc_RPY(IMU_Data IMU, Orientation saved_RPY, double dt){
    Orientation RPY;
    
    float roll_acc = 180 * atan(IMU.accelY/sqrt(IMU.accelX*IMU.accelX + IMU.accelZ*IMU.accelZ))/M_PI; 
    float pitch_acc = 180 * atan(IMU.accelX/sqrt(IMU.accelY*IMU.accelY + IMU.accelZ*IMU.accelZ))/M_PI;
    
    float roll_gyro = IMU.gyroY * dt;  // Angle around the Y-axis
    float pitch_gyro = IMU.gyroX * dt; // Angle around the X-axis
    
    //Complimentary Filter
    RPY.roll = 0.7 * (saved_RPY.roll - roll_gyro) + 0.3 * roll_acc;
    RPY.pitch = 0.7 * (saved_RPY.pitch + pitch_gyro) + 0.3 * pitch_acc;
    RPY.yaw = 180 * atan(IMU.accelZ/sqrt(IMU.accelX*IMU.accelX + IMU.accelY*IMU.accelY))/M_PI;

    return RPY;
}

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