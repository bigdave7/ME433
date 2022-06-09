#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include <stdio.h>
#include "ws2812b.h"

// Timer2 delay times, you can tune these if necessary
#define LOWTIME 15 // number of 48MHz cycles to be low for 0.35uS
#define HIGHTIME 65 // number of 48MHz cycles to be high for 1.65uS

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
#define PIC32_DESIRED_BAUD 230400 // Baudrate for RS232

void ws2812b_setup();
void ws2812b_setColor(wsColor * c, int numLEDs);
void blink();

int main(){
    
    __builtin_disable_interrupts(); // disable interrupts while initializing things

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;
    
    __builtin_enable_interrupts();
    
    TRISAbits.TRISA4 = 0;
    LATAbits.LATA4 = 1;
    
    ws2812b_setup();
    
    wsColor c[4];
    
    c[0] = HSBtoRGB(0, 1, 0.5);
    c[1] = HSBtoRGB(60, 1, 0.5);
    c[2] = HSBtoRGB(120, 1, 0.5);
    c[3] = HSBtoRGB(180, 1, 0.5);
    
    int i = 0;
    int j = 0;
    int k = 0;
    int m = 0;
    
    while (1) {
        blink();
        for (k = 0; k <= 360; k++) {
          for (m = 0; m <= 3; m++){
            c[m]= HSBtoRGB(((60*m)+k)%360, 1, 0.5);//% is MOD, which means /360 and keep the remainder
            ws2812b_setColor(c, 4);
        }
          _CP0_SET_COUNT(0);
          while(_CP0_GET_COUNT()<24000000/200){}
        }
    }
}

void blink(){
    LATAbits.LATA4 = 1;
    _CP0_SET_COUNT(0);
    while(_CP0_GET_COUNT()<24000000/2/20){}
    LATAbits.LATA4 = 0;
    _CP0_SET_COUNT(0);
    while(_CP0_GET_COUNT()<24000000/2/20){}
}