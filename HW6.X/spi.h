/* 
 * File:   spi.h
 * Author: aidan
 *
 * Created on May 17, 2016, 12:33 PM
 */

#ifndef SPI_H_
#define	SPI_H_

void initSPI1(void);
unsigned char spi_io(unsigned char o);
void ram_init(void) ;
void ram_write(unsigned short addr, const char data[], int len);
void ram_read(unsigned short addr, char data[], int len);

#endif