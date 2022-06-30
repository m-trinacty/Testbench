/*
 * Copyright (C) Aero4TE, s.r.o. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include "port.h"
#include <iostream>
#include <cstring>
#include <string>


// C library headers
#include <stdio.h>
#include <stdlib.h>

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()
#include <syslog.h>

port::port() {
	m_serial_port=-1;
}
port::port(std::string portName){
	m_port_name = portName;
    open_port(m_port_name);
}

int port::set_port_attribs(int fd, int speed, int parity){
	struct termios tty;
	if (tcgetattr (fd, &tty) != 0){
        syslog(LOG_ERR,"Error %d from tcgetattr  ",errno);

        return 0;
	}

	cfsetospeed (&tty, speed);
	cfsetispeed (&tty, speed);

	tty.c_cflag = (port_cfg.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
	// disable IGNBRK for mismatched speed tests; otherwise receive break
	// as \000 chars
	tty.c_iflag &= ~IGNBRK;         // disable break processing
	tty.c_lflag = 0;                // no signaling chars, no echo,
	                                        // no canonical processing
	tty.c_oflag = 0;                // no remapping, no delays
	tty.c_cc[VMIN]  = 0;            // read doesn't block
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

	tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
	                                        // enable reading
	tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
	tty.c_cflag |= parity;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr (fd, TCSANOW, &tty) != 0){
        syslog(LOG_ERR,"Error %d from tcsetattr  ",errno);
        return 0;
	}
    return EXIT_SUCCESS;
}
void port::set_port_block(int fd, int shouldBlock){
	//struct termios tty;
	memset (&port_cfg, 0, sizeof port_cfg);
    if (tcgetattr (fd, &port_cfg) != 0){
        syslog(LOG_ERR,"Error %d from tcgetattr  ",errno);
		return;
	}

	port_cfg.c_cc[VMIN]  = shouldBlock ? 1 : 0;
	port_cfg.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    if (tcsetattr (fd, TCSANOW, &port_cfg) != 0){
        syslog(LOG_ERR,"Error %d from tcsetattr  ",errno);
		return;
	}
}

int port::open_port(std::string port_name){
    const char * cportName = port_name.c_str();

	//char * cportName ="/dev/ttyACM0";
    m_serial_port = open(cportName,O_RDWR | O_NOCTTY | O_SYNC);

	if(m_serial_port < 0){
        syslog(LOG_ERR,"Error %d while opening port  ",errno);
        //std::cout << "Error "<< errno << " from open: " << strerror(errno) << std::endl;
        return 0;
	}
	set_port_attribs(m_serial_port,B115200,0);

	return EXIT_SUCCESS;
}
int port::write_port(std::string message){
	unsigned char msg[message.length()+1];
	for (int i = 0; i < (int)message.length(); i++) {
		msg[i]=message[i];
	}
	msg[message.length()]='\r';
	int numBytes = write(m_serial_port, msg, sizeof(msg));
	if(numBytes < 0){

        syslog(LOG_ERR,"Error %d while writing to port  ",errno);
        //std::cout << "Error "<< errno << " writing: " << strerror(errno) << std::endl<<std::flush;
        return 0;
    }
    /*Check for return*/
    char readBuf [256];
    if(message[0]=='w'){
        read(m_serial_port,&readBuf,sizeof(readBuf));
    }
    return numBytes;
}

std::string port::read_port(){
	char readBuf [256];
	int numBytes=read(m_serial_port,&readBuf,sizeof(readBuf));
    std::string output="NOT_READ";
    if (numBytes < 0) {
        syslog(LOG_ERR,"Error %d while reading from port  ",errno);
		return output;
	}
	if(numBytes>0){
        output = char_arr_to_string(readBuf,numBytes);
	}
	return output;
}


int port::close_port(){
    close(m_serial_port);
	return 0;
}

std::string port::char_arr_to_string(char * text,int size){
	//int size = sizeof(text)/sizeof(text[0]);
    std::string output = "";
	for (int i = 0; i < size; ++i) {
		output = output + text[i];
	}
	return output;

}

int port::set_port(std::string portName){
	m_port_name = portName;
    open_port(m_port_name);
	return EXIT_SUCCESS;
}
std::string port::get_port(){
	return m_port_name;
}

port::~port() {

    syslog(LOG_INFO,"Closing port");
    close_port();
	// TODO Auto-generated destructor stub
}

