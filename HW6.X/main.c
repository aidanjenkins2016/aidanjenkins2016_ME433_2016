//#include "i2c_slave.h" //there is no slave.h
//#include "i2c_master_noint.h"
#include <xc.h>
#include <sys/attribs.h>  // __ISR macro
#include <math.h>
#include "ILI9163C.h"
#include "i2c_master_noint.h"

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


#define IMU_ADDRESS 0b1101011
#define OUT_TEMP_L 0x20

// CTRL1, CTRL2, CTRL3 initialize values
#define CTRL1_XL 0b10000001
#define CTRL2_G  0b10000000
#define CTRL3_C  0b00000100

void LCD_sprintf(unsigned short x, unsigned short y, unsigned short color, char c){
    char pixel;
    char LCD_ascii;
    LCD_ascii= c - 0x20;
    int i, j;
    for(i=0; i<5; i++){
        for(j=0; j<8; j++){
            pixel=ASCII[LCD_ascii][i];
            if((pixel >> (7-j)) & 1){
                LCD_drawPixel(x+i, y+(7-j),color);
            }
        }
    }
    
}

void initI2C2(void) {
    ANSELBbits.ANSB2=0; //turn off analog on SDA and SCL
    ANSELBbits.ANSB3=0;
    I2C2BRG = 233;   // I2CBRG = [1/(2*Fsck) - PGD]*Pblck - 2 
                    // [1/(2*100000Hz)-104ns]*48000000Hz -2
                                    // look up PGD for your PIC32
    I2C2CONbits.ON = 1;               // turn on the I2C2 module
}


//function initializations//
unsigned char readIMU(char reg);
void init_IMU(void);
void I2C_read_multiple(char address, char Register, unsigned char * data, char length);
void LCD_drawString(unsigned short x, unsigned short y, char *msg);


char msg[100];
unsigned char test;
unsigned char output[14];
signed short scale = 1000;
signed short accelX,accelY,accelZ,gyroX,gyroY,gyroZ,temp;
float accelXf,accelYf,accelZf,gyroXf,gyroYf,gyroZf;

   
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
   //TRISBbits.TRISB4 = 1; // pin 12 as an output
   TRISAbits.TRISA4 = 0; //pins 11 as an input
  // LATAbits.LATA4 = 0; //set pin 12 
      
   SPI1_init();
   LCD_init();
   initI2C2();
   init_IMU();
   
  
__builtin_enable_interrupts();
  

LCD_clearScreen(WHITE);

/*
sprintf(msg, "Hello World 1337!!");
int k=0;
while(msg[k]){
    LCD_sprintf(28+k*5, 32, BLACK, msg[k]);
    k++;
}
*/

 while(1) {
	    // use _CP0_SET_COUNT(0) and _CP0_GET_COUNT() to test the PIC timing
		// remember the core timer runs at half the CPU speed
        _CP0_SET_COUNT(0);                   // set core timer to 0
        
        while (_CP0_GET_COUNT() < 480000){;} // read at 50 Hz -- 480k / 24 MHz
               // intialize LED on
        
        
        I2C_read_multiple(IMU_ADDRESS<<1,OUT_TEMP_L,output,14);
        
        temp = (output[0] | (output[1] << 8));
        gyroX = (output[2] | (output[3] << 8));
        gyroY = (output[4] | (output[5] << 8));
        gyroZ = (output[6] | (output[7] << 8));
        accelX = (output[8] | (output[9] << 8));
        accelY= (output[10] | (output[11] << 8));
        accelZ= (output[14] | (output[13] << 8));
        
        //xl_x = xl_x/scale;
        accelXf = ((float)accelX )/16383;
        accelYf = ((float)accelY)/16383;
        accelZf = ((float)accelZ)/16383;
        gyroXf = ((float)gyroX)/134;
        gyroYf = ((float)gyroY)/134;
        gyroZf = ((float)gyroZ)/134;
        
        
        sprintf(msg,"accelX(g): %.2f   ",accelXf);
        LCD_drawString(2,2,msg);
        sprintf(msg,"accelY(g): %.2f   ",accelYf);
        LCD_drawString(2,12,msg);
        sprintf(msg,"accelZ(g): %.2f   ",accelZf);
        LCD_drawString(2,22,msg);
        sprintf(msg,"gyroX(dps): %.2f   ",gyroXf);
        LCD_drawString(2,32,msg);
        sprintf(msg,"gyroY(dps): %.2f   ",gyroYf);
        LCD_drawString(2,42,msg);
        sprintf(msg,"gyroZ(dps): %.2f   ",gyroZf);
        LCD_drawString(2,52,msg);
        sprintf(msg,"TEMP: %i   ",temp);
        LCD_drawString(2,62,msg);
        
    }

  

return 0;
}

void init_IMU(void){
    //init_XL
    i2c_master_start(); // make the start bit
    i2c_master_send(IMU_ADDRESS<<1); // write the address, write data
    i2c_master_send(0x10); // write location
    i2c_master_send(CTRL1_XL); // set register
    i2c_master_stop(); 
    
    //init_G
    i2c_master_start(); 
    i2c_master_send(IMU_ADDRESS<<1); 
    i2c_master_send(0x11); 
    i2c_master_send(CTRL2_G); 
    i2c_master_stop(); 
    
    //init_C
    i2c_master_start(); 
    i2c_master_send(IMU_ADDRESS<<1); 
    i2c_master_send(0x12); 
    i2c_master_send(CTRL3_C); 
    i2c_master_stop(); 
}


unsigned char readIMU(char reg){
   
    i2c_master_start();
    i2c_master_send(IMU_ADDRESS<<1);
    i2c_master_send(reg);
    i2c_master_restart();
    i2c_master_send(0b11010111);
    unsigned char r = i2c_master_recv();
    i2c_master_ack(1); 
    i2c_master_stop(); 
    return r;
}

void I2C_read_multiple(char address, char reg, unsigned char * data, char length){
    int i = 0;
    i2c_master_start();
    i2c_master_send(address);
    i2c_master_send(reg);
    i2c_master_restart();
    i2c_master_send(0b11010111);
    for (i=0;i<=length;i++){
    output[i] = i2c_master_recv();
    if (i<length){
        i2c_master_ack(0);
    }
    else if (i==length){
        i2c_master_ack(1);
    }
    } 
    i2c_master_stop(); 
}

void LCD_drawChar(unsigned short xx, unsigned short yy, char symbol){
    int set, x, y, ascii_row;
    int col = 0;
    int bit_index = 0;
    char bit_map;
    
    ascii_row = (int)(symbol - 32);
    
    while (col < 5){
        bit_index = 0;
        bit_map = ASCII[ascii_row][col];
        while (bit_index < 8){
            set = (bit_map >> bit_index) & 0x01;
            x = xx + col;
            y = yy + bit_index;
            if (set){
                LCD_drawPixel(x, y, BLACK);
            }
            else{
                LCD_drawPixel(x, y, WHITE);
            }
            bit_index++;
        }
        col++;
    }
}




void LCD_drawString(unsigned short x, unsigned short y, char *msg){
    
    int i = 0;
    int start = x;
    while (msg[i]!=0){
        if (msg[i]== '\n'){
            y = y + 10;
            x = start;
            i++;
            continue;
        }
        LCD_drawChar(x,y, msg[i]);
        x = x + 6;
        i++;
    }
}
