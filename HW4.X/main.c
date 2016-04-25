//#include "i2c_slave.h" //there is no slave.h
#include "i2c_master_noint.h"
#include <xc.h>
#include <sys/attribs.h>  // __ISR macro


void toggleGPIO(char pin, char hi_lo);
void initGPIO();
char getGPIO();
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


#define SLAVE_ADDR 0b010000000 //slave address for pin expander

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
       
   
    __builtin_enable_interrupts();
   
     // some initialization function to set the right speed setting
  char buf[100] = {};                       // buffer for sending messages to the user
  
  unsigned char gpio_addr= 0x09;            //gpio address location
  
  
   
    _CP0_SET_COUNT(0);
    
    //stuff from HW1
    /*while(1) {
	    // use _CP0_SET_COUNT(0) and _CP0_GET_COUNT() to test the PIC timing
		// remember the core timer runs at half the CPU speed
        
        if (PORTBbits.RB4==0) {          
            if (_CP0_GET_COUNT() >= 12000){
                LATAbits.LATA4= ~LATAbits.LATA4; //toggle
                _CP0_SET_COUNT(0);
            }
            else {
            //do nothing
            }
    
        }
        else 
            LATAbits.LATA4=0;//no button then LED off
        }
    }*/
  
    
    i2c_master_setup();
    
    initGPIO(); //start expander
       
  while(1) {
     
      if ( (getGPIO() & 0b01000000) == 0b01000000) {
          toggleGPIO( 0b00000001,1 ); 
      }
      else {
          toggleGPIO(0b00000001,0);
      }
  }
  return 0;
}

void initGPIO() {
    //setup the expander with GPIO0-3 off
    //inputs low
    i2c_master_start();
    i2c_master_send(0b01000000); //i2c slave address
    i2c_master_send(0x0A);      // OLAT address
    i2c_master_send(0x0);       //all pins low
    i2c_master_stop();
    
}

char getGPIO() {
    i2c_master_start();
    i2c_master_send(0b01000000);
    i2c_master_send(0x09); //select GPIO byte
    i2c_master_restart();
    i2c_master_send(0b01000001); //read
    char read= i2c_master_recv(); //store read value
    i2c_master_ack(1); //received bit
    i2c_master_stop(); // 
    
    return read;
        
}

void toggleGPIO(char pin, char hi_lo) {
    char set_hi_lo=0b1111111; //pin high
    if (hi_lo==0) {
        set_hi_lo=0b0000000; //set pin low
    }
    
    i2c_master_start();
    i2c_master_send(0b01000000);
    i2c_master_send(0x0A); //GPIO output latch
    i2c_master_send(pin); //selected pins high, or all low
    i2c_master_send(set_hi_lo);
    i2c_master_stop();
    
}