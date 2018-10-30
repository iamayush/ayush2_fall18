#include <stdio.h>
#include <opencv/cv.h>
#include <highgui.h>
#include <math.h>
#include <sys/select.h>
#include <termios.h>
#include <stropts.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <time.h>


#define BILLION 1000000000L
#define I2C
#define ADDR  0x25
#define MIN_SERVO_TBCCR1 1400
#define MAX_SERVO_TBCCR1 5000

#ifdef I2C
#include "i2c.h"
#endif

// Simple structure to hold details about the framebuffer device after it has
// been opened.
typedef struct {
	// Pointer to the pixels. Try to not write off the end of it.
	uint32_t * buffer;
	// The file descriptor for the device.
	int fd;
	// Number of bytes in the buffer. To work out how many elements are in the buffer, divide this by 4 (i.e. sizeof(uint32_t))
	size_t screen_byte_size;
	// Structs providing further information about the buffer configuration. See https://www.kernel.org/doc/Documentation/fb/api.txt
	struct fb_fix_screeninfo fix_info;
	struct fb_var_screeninfo var_info;
} screen_t;

// Because you can't have too many variants of NULL
static const screen_t NULL_SCREEN = {0};
// Indicates if the passed screen_t struct is valid.
#define valid_screen(s) ((s).buffer != NULL)

#define NUMFRAME_ROWS 480
#define NUMFRAME_COLS 720
#define FRAMEOFFSET_ROWS 150
#define FRAMEOFFSET_COLS 300

#define NUM_ROWS 120
#define NUM_COLS 160

screen_t open_fb(void);
void close_fb(screen_t *screen);
int my_min(int a, int b, int c);
int my_max(int a, int b, int c);
void rgb2hsv(int red,int green,int blue,int *hue,int *sat,int *value);
int _kbhit(void);



int main(void) { 
#ifdef I2C
	int bus;
	// add any other I2C variables here
	long tx1 = MIN_SERVO_TBCCR1; // TBCCR1 value to be sent to MSP430
    long tx2 = 0;
    long rx1 = 0;
    long rx2 = 0;
    unsigned char tx[8]={0};
    unsigned char rx[8]={0};
    int delta = 80; //changing TBCCR1 (tx1) value
#endif

	uint64_t diff;
	struct timespec current_time,previous_time;

	int key=0;
	CvCapture* capture;

	IplImage *frame, *frame_hsv;

	capture= cvCaptureFromCAM(-1);

	cvSetCaptureProperty(capture,CV_CAP_PROP_FRAME_WIDTH, NUM_COLS);  // Number of columns
	cvSetCaptureProperty(capture,CV_CAP_PROP_FRAME_HEIGHT, NUM_ROWS); // Number of rows

	if(!capture) printf("No camera detected \n");

	frame=cvQueryFrame(capture);  
	frame_hsv=cvCreateImage(cvGetSize(frame),8,3);

#ifdef I2C
	bus = i2c_start_bus(1);
#endif
  
	screen_t screen = open_fb();
	if (!valid_screen(screen)) {
		printf("ERROR: Failed to open screen\n");
		return 1;
	}

	float period=0,Hz=0;  
	unsigned char *data_hsv, *data_bgr, *current_pixel;
	int r=0;
	int c=0;
	int nr;
	int nc;
	int celements;
	nr=frame_hsv->height; //number of rows in image should be 120 
	nc=frame_hsv->width; //number of collumns
	celements = nc*3;  // b,g,r elements in each column
	  
	int vh=255, vl=150, sh=255, sl=150, hh=254, hl=235; //vh=255, vl=20, sh=184, sl=63, hh=165, hl=149;
	int h=0, s=0, v=0;
	int blue=0, green=0, red=0;

	clock_gettime(CLOCK_MONOTONIC, &previous_time);

	while(! _kbhit()) {  //27 is the code corresponding to escape key

		frame=cvQueryFrame(capture);

		if(!frame) break;
		 
		// -----loop through image pixels-----  
		data_bgr=(unsigned char *)(frame->imageData); 
		
		// variables declared by Ayush
		unsigned int numpix = 0;
		unsigned int centroid_row = 0;
		unsigned int centroid_col = 0;
		
		for(r=0; r<nr; r++) {  
			for(c=0; c<nc;c++) {
				blue = (int)data_bgr[r*celements+c*3];
				green = (int)data_bgr[r*celements+c*3+1];
				red = (int)data_bgr[r*celements+c*3+2];
				rgb2hsv(red,green,blue,&h,&s,&v);

				//--------------process each thresholded pixel here------------------
				if( (h>=hl && h <= hh) && (s>=sl && s<=sh) && (v>=vl && v<=vh) ) {
					green = 255;
					blue = 0; 
					red = 0;
					centroid_row += r;
					centroid_col += c;
					numpix++;
				} 
				
				screen.buffer[(r+FRAMEOFFSET_ROWS)*NUMFRAME_COLS + (FRAMEOFFSET_COLS + c)] = (int)((red<<16) | (green<<8) | blue);
 				
			}
		} //--------------end pixel processing---------------------
        
		// Finding Centroid and Printing Crosshair
		if(numpix > 0){
			centroid_row /= numpix;
			centroid_col /= numpix;
			green = 255;
			blue = 255;
			red = 255; // white crosshair

			//char ri[13] = {0,0,0,0,0,0,1,2,3,-1,-2,-3};
			for (r = centroid_row - 3; r <= centroid_row + 3; r++){
				for (c = centroid_col - 3; c <= centroid_col + 3; c++){
					if((r>=0)&&(r<nc))
						if((c>=0)&&(c<nc))
							screen.buffer[(r+FRAMEOFFSET_ROWS)*NUMFRAME_COLS + (FRAMEOFFSET_COLS + c)] = (int)((red<<16) | (green<<8) | blue);
				}
			}
		}
		
		// Moving servo 
		#ifdef I2C
		if(numpix < 100)
			tx1 = MIN_SERVO_TBCCR1;
		else
			tx1 = MIN_SERVO_TBCCR1+(centroid_row*(long)(MAX_SERVO_TBCCR1 - MIN_SERVO_TBCCR1)/nc);
		
		for(char i = 0; i < 8; i++){
			if(i < 4)
				tx[i] = (unsigned char)(tx1>>(i*8));
			else
				tx[i] = (unsigned char)(tx2>>((i-4)*8));
		}

        i2c_write_bytes(bus, ADDR, tx,8);
		
        i2c_read_bytes(bus, ADDR,rx,8);
		
		rx1 = (((long)rx[3])<<24)+(((long)rx[2])<<16)+(((long)rx[1])<<8)+((long)rx[0]);
        rx2 = (((long)rx[7])<<24)+(((long)rx[6])<<16)+(((long)rx[5])<<8)+((long)rx[4]);	
		#endif
		
		clock_gettime(CLOCK_MONOTONIC, &current_time);
		diff = BILLION * (current_time.tv_sec - previous_time.tv_sec) + current_time.tv_nsec - previous_time.tv_nsec;
		period = diff * 1e-9;
		if (period > 0.0) {
			Hz = 1.0/period;
		} else {
			Hz = 0.0;
		}
		//printf("Hz = %.4f\n",Hz);
		printf("h=%d,s=%d,v=%d\n",h,s,v);
		previous_time = current_time;
	}  // end while

	cvReleaseCapture(&capture);

#ifdef I2C
	i2c_close_bus(bus);
#endif

	printf("All done, bye bye.\n");
	close_fb(&screen);

	return 0;
}

int my_min(int a, int b, int c) {
  
	int min;
	min = a;

	if (b < min) {
		min = b;
	}
	if (c < min) {
		min = c;
	}
	return (min);
}

int my_max(int a, int b, int c) {
	int max;
	max = a;
	if (b > max) {
		max = b;
	}
	if (c > max) {
		max = c;
	}
	return (max);
}

void rgb2hsv(int red,int green,int blue,int *hue,int *sat,int *value) {
	int min, delta;
	min = my_min( red, green, blue );

	*value = my_max( red, green, blue );

	delta = *value - min;

	if( *value != 0 ) {
    
		*sat = (delta*255) / *value; // s
    
		if (delta != 0) {
      
			if( red == *value )   
				*hue = 60*( green - blue ) / delta; // between yellow & magenta
      
			else if( green == *value )
				*hue = 120 + 60*( blue - red ) / delta;  // between cyan & yellow
			else
				*hue = 240 + 60*( red - green ) / delta; // between magenta & cyan
      
			if( *hue < 0 )
				*hue += 360;
      
		} else {   
			*hue = 0;
			*sat = 0;
		}
    
	} else {
		// r = g = b = 0                    
		// s = 0, v is undefined
		*sat = 0;
		*hue = 0;
	}
	
	*hue = (*hue*255)/360;
	
}

/**
 * Opens the first frame buffer device and returns a screen_t struct
 * for accessing it. If the framebuffer isn't in the expected format
 * (32 bits per pixel), NULL_SCREEN will be returned.
 */
screen_t open_fb(void) {
	const char * const SCREEN_DEVICE = "/dev/fb0";
	int screen_fd = open(SCREEN_DEVICE, O_RDWR);
	if (screen_fd == -1) {
		printf("ERROR: Failed to open %s\n", SCREEN_DEVICE);
		return NULL_SCREEN;
	}

	struct fb_var_screeninfo var_info = {0};
	if (ioctl(screen_fd, FBIOGET_VSCREENINFO, &var_info) == -1) {
		printf("ERROR: Failed to get variable screen info\n");
		return NULL_SCREEN;
	}

	struct fb_fix_screeninfo fix_info = {0};
	if (ioctl(screen_fd, FBIOGET_FSCREENINFO, &fix_info) == -1) {
		printf("ERROR: Failed to get fixed screen info\n");
		return NULL_SCREEN;
	}

	if (var_info.bits_per_pixel != 32) {
		printf("ERROR: Only support 32 bits per pixel. Detected bits per pixel: %d\n", var_info.bits_per_pixel);
		return NULL_SCREEN;
	}

	const size_t screen_byte_size = var_info.xres * var_info.yres * var_info.bits_per_pixel / 8;
	uint32_t * const buffer = (uint32_t *)mmap(NULL, screen_byte_size, PROT_READ | PROT_WRITE, MAP_SHARED, screen_fd, 0 /* offset */);

	screen_t screen = {
		.buffer = buffer,
		.fd = screen_fd,
		.screen_byte_size = screen_byte_size,
		.fix_info = fix_info,
		.var_info = var_info
	};
	return screen;
}

/**
 * Closes the framebuffer when you are finished with it. Don't try
 * to access things in the struct after calling this or else a
 * kitten will die.
 */
void close_fb(screen_t *screen) {
	munmap(screen->buffer, screen->screen_byte_size);
	close(screen->fd);
	*screen = NULL_SCREEN;
}

int _kbhit() {
    static const int STDIN = 0;
    static int initialized = 0;

    if (! initialized) {
        // Use termios to turn off line buffering
        struct termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        initialized = 1;
    }

    int bytesWaiting;
    ioctl(STDIN, FIONREAD, &bytesWaiting);
    return bytesWaiting;
}
