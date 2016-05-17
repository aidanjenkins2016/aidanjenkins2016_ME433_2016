//#include "i2c_slave.h" //there is no slave.h
#include "i2c_master_noint.h"
#include <xc.h>
#include <sys/attribs.h>  // __ISR macro
#include <math.h>
#include "ILI9163C.h"
#define CS LATBbits.LATB8 //chip select pin



// Demonstrate I2C by having the I2C2 talk to the pin expander
// Master will use SDA1 (D9) and SCL1 (D10).  Connect these through resistors to
// Vcc (3.3 V) (2.4k resistors recommended, but around that should be good enough)
// Two bytes will be written to the slave and then read back to the slave.
//#define SLAVE_ADDR 0x32

// DEVCFG3
#pragma config USERID = 0 //No Setting
#pragma config PMDL1WAY = OFF           // Peripheral Module Disable Configuration (Allow multiple reconfigurations)
#pragma config IOL1WAY = OFF            // Peripheral Pin Select Configuration (Allow multiple reconfigurations)
#pragma config FUSBIDIO = ON            // USB USID Selection (Controlled by the USB Module)
#pragma config FVBUSONIO = ON           // USB VBUS ON Selection (Controlled by USB Module)

// DEVCFG2
#pragma config FPLLIDIV = DIV_2         // PLL Input Divider (2x Divider)
#pragma config FPLLMUL = MUL_24         // PLL Multiplier (24x Multiplier)
#pragma config UPLLIDIV = DIV_2         // USB PLL Input Divider (2x Divider)
#pragma config UPLLEN = ON              // USB PLL Enable (Enabled)
#pragma config FPLLODIV = DIV_2         // System PLL Output Clock Divider (PLL Divide by 2)

// DEVCFG1
#pragma config FNOSC = PRIPLL           // Oscillator Selection Bits (Primary Osc w/PLL (XT+,HS+,EC+PLL))
#pragma config FSOSCEN = OFF            // Secondary Oscillator Enable (Disabled)
#pragma config IESO = OFF               // Internal/External Switch Over (Disabled)
#pragma config POSCMOD = HS             // Primary Oscillator Configuration (HS osc mode)
#pragma config OSCIOFNC = OFF           // CLKO Output Signal Active on the OSCO Pin (Disabled)
#pragma config FPBDIV = DIV_1           // Peripheral Clock Divisor (Pb_Clk is Sys_Clk/1)
#pragma config FCKSM = CSDCMD           // Clock Switching and Monitor Selection (Clock Switch Disable, FSCM Disabled)
#pragma config WDTPS = PS1048576        // Watchdog Timer Postscaler (1:1048576)
#pragma config WINDIS = OFF             // Watchdog Timer Window Enable (Watchdog Timer is in Non-Window Mode)
#pragma config FWDTEN = OFF             // Watchdog Timer Enable (WDT Disabled (SWDTEN Bit Controls))
#pragma config FWDTWINSZ = WINSZ_25     // Watchdog Timer Window Size (Window Size is 25%)

// DEVCFG0
#pragma config JTAGEN = OFF             // JTAG Enable (JTAG Disabled)
#pragma config ICESEL = ICS_PGx1        // ICE/ICD Comm Channel Select (Communicate on PGEC1/PGED1)
#pragma config PWP = OFF                // Program Flash Write Protect (Disable)
#pragma config BWP = OFF                // Boot Flash Write Protect bit (Protection Disabled)
#pragma config CP = OFF                 // Code Protect (Protection Disabled)

void LCD_sprintf(unsigned short x, unsigned short y, unsigned short color, char c);
char msg[100];
int i, l, m;

int main() {
    
__builtin_disable_interrupts();
 
    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;
    
    // do your TRIS and LAT commands here
   TRISBbits.TRISB4 = 1; // pin 12 as an output
   TRISAbits.TRISA4 = 0; //pin 11 as an input
   LATAbits.LATA4 = 0; //set pin 12 
      
   SPI1_init();
   LCD_init();
   LCD_clearScreen(BLACK);
  
__builtin_enable_interrupts();
  



/*
sprintf(msg, "hello world");
int n=0;
while(msg[n]){
    LCD_sprintf(28+n*5, 32, WHITE, msg[n]);
}
*/
    
 
  return 0;
}

void LCD_sprintf(unsigned short x, unsigned short y, unsigned short color, char c){
    char pixel;
    char LCD_ascii;
    LCD_ascii= c - 0x20;
    for(l=0; l<5; l++){
        for(m=0; m<8; m++){
            pixel=ASCII[LCD_ascii][1];
            if((pixel>>(7-m))&1){
                LCD_drawPixel(x+1, y+(7-m),color);
            }
        }
    }
    
}