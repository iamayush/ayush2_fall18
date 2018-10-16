#include <stdio.h>
#include <math.h>
#include "i2c.h"

#define ADDR  0x25

int main(void) {
    int bus;
    unsigned char tx[8]={0};
    unsigned char rx[8]={0};
    char i = 0; // loop index
    bus = i2c_start_bus(1);

    while (1) {
        i2c_write_bytes(bus, ADDR, tx,8);
		
        i2c_read_bytes(bus, ADDR,rx,8);
		
	printf("TX: ");
	for (i = 0; i<8 ; i++)
		printf("%03d ",tx[i]);
	printf("\n");

	printf("RX: ");
	for (i = 0; i< 8; i++)
		printf("%03d ",rx[i]);
	printf("\n");

        usleep(200*1000);
		
		tx[0] += 1;
		tx[7] -= 1;
	}
	
    i2c_close_bus(bus);
    return 0;
}
