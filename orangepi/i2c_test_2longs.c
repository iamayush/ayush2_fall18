#include <stdio.h>
#include <math.h>
#include "i2c.h"

#define ADDR  0x25

int main(void) {
    int bus;
    long tx1 = 0;
    long tx2 = 0;
    long rx1 = 0;
    long rx2 = 0;
    unsigned char tx[8]={0};
    unsigned char rx[8]={0};
    char i = 0; // loop index
    bus = i2c_start_bus(1);

    while (1) {
		for(i = 0; i < 8; i++){
			if(i < 4)
				tx[i] = (unsigned char)(tx1<<(i*8));
			else
				tx[i] = (unsigned char)(tx2<<((i-4)*8));
		}

        i2c_write_bytes(bus, ADDR, tx,8);
		
        i2c_read_bytes(bus, ADDR,rx,8);
		
		rx1 = (((long)rx[3])<<24)+(((long)rx[2])<<16)+(((long)rx[1])<<8)+((long)rx[0]);
        rx2 = (((long)rx[7])<<24)+(((long)rx[6])<<16)+(((long)rx[5])<<8)+((long)rx[4]);	
		
		printf("TX: %l %l \n", tx1,tx2);
		printf("RX: %l %l \n", rx1,rx2);

        usleep(200*1000);
		
	}
	
    i2c_close_bus(bus);
    return 0;
}
