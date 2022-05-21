#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<stdio.h>
#include <string.h> // for memset
#include "i2c_master_noint.h"
#include"font.h"
#include "ssd1306.h"

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

#define IODIR 0x00
#define GPIO 0x09
#define OLAT 0x0A

void setPin(unsigned char address, unsigned char reg, unsigned char value);
unsigned char readPin(unsigned char address, unsigned char reg);
unsigned char drawLetter(unsigned x, unsigned y, unsigned char letter);
unsigned char drawString(unsigned char x, unsigned char y, char * m);

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

    // do your TRIS and LAT commands here
    TRISBbits.TRISB4 = 1;
    TRISAbits.TRISA4 = 0;
    LATAbits.LATA4 = 0;
    LATBbits.LATB4 = 0;
    
    U1RXRbits.U1RXR = 0b0001; // U1RX is B6
    RPB7Rbits.RPB7R = 0b0001; // U1TX is B7
    
    // turn on UART3 without an interrupt
    U1MODEbits.BRGH = 0; // set baud to NU32_DESIRED_BAUD
    U1BRG = ((48000000 / 115200) / 16) - 1;

    // 8 bit, no parity bit, and 1 stop bit (8N1 setup)
    U1MODEbits.PDSEL = 0;
    U1MODEbits.STSEL = 0;

    // configure TX & RX pins as output & input pins
    U1STAbits.UTXEN = 1;
    U1STAbits.URXEN = 1;

    // enable the uart
    U1MODEbits.ON = 1;
  
    __builtin_enable_interrupts();
    
    i2c_master_setup();
    ssd1306_setup();
    unsigned char address = 0b01000000;
    int heartbeat = 1200000;
    unsigned char button;

    setPin(address,IODIR,0b01111111); // 1 input, 0 output
    setPin(address,OLAT,0b10000000); // 1 high, 0 low

    int t = 0;
    char m[50];
    char p[50];
    
    while (1) {
//        //heartbeat
//        LATAbits.LATA4 = 0;
        _CP0_SET_COUNT(0);
//        while (_CP0_GET_COUNT() < heartbeat) {
//            LATAbits.LATA4 = 0;
//        }
//        while (_CP0_GET_COUNT() < (heartbeat*2)) {
//            LATAbits.LATA4 = 1;
//        }
        sprintf(p,"HW 8 DEMO",t);
        drawString(0,0,p);
        sprintf(m,"UPDATE RATE: %d fps",t);
        drawString(0,12,m);
        ssd1306_update();
        t = 24000000/_CP0_GET_COUNT();
    }
}

void setPin(unsigned char address, unsigned char reg, unsigned char value) {
    i2c_master_start();
    i2c_master_send(address); 
    i2c_master_send(reg); // send register to change
    i2c_master_send(value); //send value to change register to
    i2c_master_stop();
}

unsigned char readPin(unsigned char address, unsigned char reg) {
    i2c_master_start();
    i2c_master_send(address);
    i2c_master_send(reg);
    i2c_master_restart();
    i2c_master_send(address|0b1);
    unsigned char r = i2c_master_recv();
    i2c_master_ack(1);
    i2c_master_stop();
    return r;
}

unsigned char drawLetter(unsigned x, unsigned y, unsigned char letter){
    for (int j = 0; j < 5; j++){
        for (int k = 0; k < 8; k++){
            if ((((ASCII[letter - 0x20][j])>>k)&1) == 1) {
                ssd1306_drawPixel(j + x, k + y, 1);
            }
            else{
                ssd1306_drawPixel(j + x, k + y, 0); 
            }
        }
    }
}

unsigned char drawString(unsigned char x, unsigned char y, char * m){
    int s = 0;
        while (m[s]!= 0) {
            drawLetter(x + (s * 6), y + 0, m[s]);
            s++;
        }
}