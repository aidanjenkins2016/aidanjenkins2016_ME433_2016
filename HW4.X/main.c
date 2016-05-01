//#include "i2c_slave.h" //there is no slave.h
#include "i2c_master_noint.h"
#include <xc.h>
#include <sys/attribs.h>  // __ISR macro
#include <math.h>
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

//Functions

int sinewave[100];
int trianglewave[200];

// initialize spi4 and the ram module
void initSPI1() {
  // set up the chip select pin (B8) as an output
  // the chip select pin is used by the sram to indicate
  // when a command is beginning (clear CS to low) and when it
  // is ending (set CS high)
    //ANSELBbits.ANSB8=0; //not analog capable
    TRISBbits.TRISB8 = 0;//B8 as an output
    CS = 1; //B8 high
  //SDI/DSO
  SDI1Rbits.SDI1R=0;      //A1 as SDI  
  RPB7Rbits.RPB7R=0b0011; //SDO1 as B7
  
  // Master - SPI1, pins are: SDI1(A1), SDO1(), SCK1(25).  
  // we manually control SS4 as a digital output (F12)
  // since the pic is just starting, we know that spi is off. We rely on defaults here
 
  // setup spi1
  SPI1CON = 0;              // turn off the spi module and reset it
  SPI1BUF;                  // clear the rx buffer by reading from it
  SPI1BRG = 300;            // baud rate to 10 MHz [SPI1BRG = (48000000/(2*desired))-1]
  SPI1STATbits.SPIROV = 0;  // clear the overflow bit
  SPI1CONbits.CKE = 1;      // data changes when clock goes from hi to lo (since CKP is 0)
  SPI1CONbits.MSTEN = 1;    // master operation
  SPI1CONbits.ON = 1;       // turn on spi 1
 }
 unsigned char SPI1_IO(unsigned char write) {
  SPI1BUF = write;
  while(!SPI1STATbits.SPIRBF) { // wait to receive the byte
    ;
  }
  return SPI1BUF;
}



void setVoltage( unsigned char channel, unsigned char voltage){
    unsigned short value=0;
    if (channel ==0){
    value = (0b0011 << 12) + (voltage << 4);
        CS = 0;                                 
        SPI1_IO((value & 0xFF00) >> 8 ); 
        SPI1_IO(value & 0x00FF);         
        CS = 1;                          
    }
    
    if (channel == 1){
        
      value = (0b1011 << 12) + (voltage << 4);
        CS = 0;
        SPI1_IO((value & 0xFF00) >> 8 ); 
        SPI1_IO(value & 0x00FF);         
        CS = 1;
    }
}

void waveform(void){
    
  int i;
  for(i = 0; i < 100; i++){
    sinewave[i] = (int)(128.0 + 127.5 * sin(M_PI * 0.02 * i));
  }
  for(i = 0; i < 200; i++){
    trianglewave[i] = (int)(1.28 * i);
  }
}


void initExpander() {
    //setup the expander with GPIO0-3 off
    //inputs low
    i2c_master_start();
    i2c_master_send(0x40); //i2c slave address
    i2c_master_send(0x00);      //I/o address
    i2c_master_send(0xF0);       // set I/O 0b11110000
    i2c_master_stop();
    
}

unsigned char getExpander() {
    i2c_master_start();
    i2c_master_send(0x40);
    i2c_master_send(0x09); //select GPIO addr
    i2c_master_restart();
    i2c_master_send(0x41); //read
    unsigned char read= i2c_master_recv() >> 7; //store read value from GP7
    i2c_master_ack(1); //received bit stop sending
    i2c_master_stop(); // 
    
    return read;
        
}

void setExpander(char pin, char level) {
  
    
    i2c_master_start();
    i2c_master_send(0x40);
    i2c_master_send(0x0A); //GPIO output latch
    //i2c_master_send(0x01);
       
    if(level==1) 
    {
        i2c_master_send(0x01); //set GP0 high
       
    }

    
    if (level == 0) 
    {
        i2c_master_send(0x00);
    }
    
    
    i2c_master_stop();
    
}

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
  
    
    i2c_master_setup(); //setup i2c   
    initExpander(); //start expander config I/O pins
    char data;
    int countA=0;
    int countB=0;
    initSPI1();
    
  while(1){
      
      _CP0_SET_COUNT(0);
      setVoltage(0, sinewave[countA]);
      setVoltage(1, trianglewave[countB]);
      countA++;
      countB++;
      
      //reset counters
      if(countA>=100){
          countA=0;
      }
      
      if(countB>=200){
          countB=0;
      }
     
       //delay
    while(_CP0_GET_COUNT()<= 24000){
        ;
    }
      
    data=getExpander();
    setExpander(0,data);
      
    
   
    }
  return 0;
}

