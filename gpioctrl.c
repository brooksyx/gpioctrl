#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev-user.h>


#define KEY_UP                  103
#define KEY_LEFT                105
#define KEY_RIGHT               106
#define KEY_DOWN                108

#define KEY_A                   30
#define KEY_S                   31
#define KEY_D                   32
#define KEY_F                   33

#define KEY_Z                   44
#define KEY_X                   45

#define KEY_Q                   16
#define KEY_W                   17

struct GPIO_KEY {

	char keyCode;
	int isPressed;
	char bankAddr;
	char bitPosition;
};


int main () {
	int                   	fd;
	struct uinput_user_dev 	uidev;
	struct input_event     	ev;
	char					currentAddr;
	char					currentVal[20];
	char 					lowerValue;
	char					upperValue;
	struct GPIO_KEY 		keys[12];
	int x;
	FILE *pf;
	int temp;
	int sync;
	char buf[2];
	int file;
	char *filename = "/dev/i2c-2";


	if ((file = open(filename, O_RDWR)) < 0) {
	    /* ERROR HANDLING: you can check errno to see what went wrong */
	    perror("Failed to open the i2c bus");
	    exit(1);
	}

	int addr = 0x20;          // The I2C address of the ADC
	if (ioctl(file, I2C_SLAVE, addr) < 0) {
	    printf("Failed to acquire bus access and/or talk to slave.\n");
	    /* ERROR HANDLING; you can check errno to see what went wrong */
	    exit(1);
	}

	buf[0] = 0x0C;
	buf[1] = 0xFF;

	if (write(file,buf,2) != 2) {
		printf("Error");
	}


	buf[0] = 0x0D;
	buf[1] = 0xFF;

	if (write(file,buf,2) != 2) {
		printf("Error");
	}



	keys[0].keyCode = KEY_UP;
	keys[0].isPressed = 0;
	keys[0].bankAddr = 0x12; 
	keys[0].bitPosition = 7;

	keys[1].keyCode = KEY_DOWN;
	keys[1].isPressed = 0;
	keys[1].bankAddr = 0x12; 
	keys[1].bitPosition = 5;

	keys[2].keyCode = KEY_LEFT;
	keys[2].isPressed = 0;
	keys[2].bankAddr = 0x12; 
	keys[2].bitPosition = 4;

	keys[3].keyCode = KEY_RIGHT;
	keys[3].isPressed = 0;
	keys[3].bankAddr = 0x12; 
	keys[3].bitPosition = 6;

	keys[4].keyCode = KEY_Z; //Start
	keys[4].isPressed = 0;
	keys[4].bankAddr = 0x12; 
	keys[4].bitPosition = 2;

	keys[5].keyCode = KEY_X; //Select
	keys[5].isPressed = 0;
	keys[5].bankAddr = 0x12; 
	keys[5].bitPosition = 3;

	keys[6].keyCode = KEY_Q; // L
	keys[6].isPressed = 0;
	keys[6].bankAddr = 0x13; 
	keys[6].bitPosition = 5;

	keys[7].keyCode = KEY_W; //R 
	keys[7].isPressed = 0;
	keys[7].bankAddr = 0x13; 
	keys[7].bitPosition = 4;

	keys[8].keyCode = KEY_A; //Y
	keys[8].isPressed = 0;
	keys[8].bankAddr = 0x13; 
	keys[8].bitPosition = 3;

	keys[9].keyCode = KEY_S; // X
	keys[9].isPressed = 0;
	keys[9].bankAddr = 0x13; 
	keys[9].bitPosition = 0;

	keys[10].keyCode = KEY_D; // B
	keys[10].isPressed = 0;
	keys[10].bankAddr = 0x13; 
	keys[10].bitPosition = 2;

	keys[11].keyCode = KEY_F; // A
	keys[11].isPressed = 0;
	keys[11].bankAddr = 0x13; 
	keys[11].bitPosition =1;

	fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if(fd < 0)
		return 1;

	if(ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0)
		return 1;

	if(ioctl(fd, UI_SET_EVBIT,EV_SYN) < 0)
		return 1;


	for(x = 0; x < 12 ; x++) {
		if ( ioctl(fd, UI_SET_KEYBIT,keys[x].keyCode) < 0)
			return 1;

	}

	memset(&uidev, 0, sizeof(uidev));
	snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "uinput-sample");
	uidev.id.bustype = BUS_USB;
	uidev.id.vendor  = 0x1;
	uidev.id.product = 0x1;
	uidev.id.version = 1;

	if(write(fd, &uidev, sizeof(uidev)) < 0)
		return 1;

	if(ioctl(fd, UI_DEV_CREATE) < 0)
		return 1;

	printf("uinput entry created and running");
	while (1) {
		sync = 0;


		lowerValue =  i2c_smbus_read_byte_data(file,0x12);

		lowerValue = ~lowerValue;


		buf[0] = 0x13;


		upperValue =  i2c_smbus_read_byte_data(file,0x13);

		upperValue = ~upperValue;


		for(x = 0; x < 12; x++) {

			if(keys[x].bankAddr == 0x12)
				temp = lowerValue & ( 1 << keys[x].bitPosition);
			else
				temp = upperValue & ( 1 << keys[x].bitPosition);
			
			if (temp != 0 )
				temp = 1;


			if ( temp != keys[x].isPressed) {
	
				if(keys[x].isPressed == 0) {
					keys[x].isPressed = 1;

				}
				else {
					keys[x].isPressed = 0;

				}

				sync = 1;
				memset(&ev, 0, sizeof(struct input_event));
				ev.type = EV_KEY;
				ev.code = keys[x].keyCode;
				ev.value = keys[x].isPressed;
				if(write(fd, &ev, sizeof(struct input_event)) < 0)
					return 1;
	
			}

		}

		if(sync == 1) {
			ev.type = EV_SYN;
			ev.code = SYN_REPORT;
			ev.value = 0;
			if(write(fd, &ev, sizeof(struct input_event)) < 0)
				return 1;
		}
		usleep(16670);

	}
}
