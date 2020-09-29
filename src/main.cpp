#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysinfo.h>

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()
#include <sys/time.h> // Used for timeout
#include <sys/types.h>//      -||-

#include "main.h"
#include "rpApp.h"

#define READ_AT_ONCE 128 // The number of bytes to read in a single read call
#define BAUD_RATE B230400

CStringParameter data_out ("dataout", CBaseParameter::RW, "", 0); // Data out of serial
CStringParameter data_in ("datain", CBaseParameter::RW, "", 0);   // Data in from serial

int serial_port;  // A file descriptor for the port
char read_buffer[READ_AT_ONCE + 1]; // Leave space for terminating character
fd_set readfs; // A set of file descriptors, used for select

int uart_init() {
    int serial_port = -1;

    // Open the PS1 serial port - on the extension header
    serial_port = open("/dev/ttyPS1", O_RDWR | O_NOCTTY);
    if (serial_port == -1) {
        fprintf(stderr, "Failed to open /dev/ttyPS1.\n");
        return -1;
    }

    // A termios struct is used for setting port parameters
    struct termios tty;

    // Read in existing settings, and handle any error
    if(tcgetattr(serial_port, &tty) != 0) {
        fprintf(stderr, "Error %i from tcgetattr: %s\n", errno, strerror(errno));
        return -1;
    }

    // Port settings:
    tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
    tty.c_cflag |= CS8; // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ECHOE; // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

    // If either of these limits is reached the read function returns
    tty.c_cc[VTIME] = 1; // Interbyte timeout (in deciseconds)
    tty.c_cc[VMIN] = READ_AT_ONCE; // Number of characters to read before returning

    // Set in/out baud rate
    cfsetispeed(&tty, BAUD_RATE);
    cfsetospeed(&tty, BAUD_RATE);

    // Save tty settings, also checking for error
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        fprintf(stderr, "Error %i from tcgetattr: %s\n", errno, strerror(errno));
        return -1;
    }

    return serial_port;
}

void uart_close(int serial_port) {
    tcflush(serial_port, TCIFLUSH);
    close(serial_port);
}


const char *rp_app_desc(void) {
    return (const char *)"Red Pitaya serial monitor.\n";
}


int rp_app_init(void) {
    fprintf(stderr, "Loading serial monitor\n");
	// Open serial port
	serial_port = uart_init();
	if (serial_port == -1) {
        return -1;
    }
    return 0;
}


int rp_app_exit(void) {
    fprintf(stderr, "Unloading serial monitor\n");
	uart_close(serial_port);
    return 0;
}


int rp_set_params(rp_app_params_t *p, int len) {
    return 0;
}


int rp_get_params(rp_app_params_t **p) {
    return 0;
}


int rp_get_signals(float ***s, int *sig_num, int *sig_len) {
    return 0;
}


void UpdateSignals(void){}


void UpdateParams(void){
	FD_SET(serial_port, &readfs); // Set port to be monitored
	struct timeval read_timeout = {0, 0}; // Return immediately if no data
	int res = select(serial_port + 1, &readfs, NULL, NULL, &read_timeout);
	if (res) {
		int read_length = read(serial_port, &read_buffer, READ_AT_ONCE);
		if (read_length > 0) {
			read_buffer[read_length] = 0;
			data_in.Set(read_buffer);
		}
	}
}


void OnNewParams(void) {
	data_out.Update();
	data_in.Update();
	const char *write_buffer = data_out.Value().c_str();
	size_t write_length = data_out.Value().length();
	write(serial_port, write_buffer, write_length);
	data_out.Set("");
}


void OnNewSignals(void){}


void PostUpdateSignals(void){}
