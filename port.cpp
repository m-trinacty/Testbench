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


port::port() {
	m_serialPort=-1;
}
port::port(std::string portName){
	m_portName = portName;
	openPort(m_portName);
}

int port::setInterfaceAttribs(int fd, int speed, int parity){
	struct termios tty;
	if (tcgetattr (fd, &tty) != 0){
        std::cout << "Error "<< errno << " from tcgetattr: " << strerror(errno) << std::endl;
        return 0;
	}

	cfsetospeed (&tty, speed);
	cfsetispeed (&tty, speed);

	tty.c_cflag = (portSettings.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
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
        std::cout << "Error "<< errno << " from tcsetattr: " << strerror(errno) << std::endl;
        return 0;
	}
    return EXIT_SUCCESS;
}
void port::setBlocking(int fd, int shouldBlock){
	//struct termios tty;
	memset (&portSettings, 0, sizeof portSettings);
	if (tcgetattr (fd, &portSettings) != 0){
        std::cout << "Error "<< errno << " from tcgetattr: " << strerror(errno) << std::endl;
		return;
	}

	portSettings.c_cc[VMIN]  = shouldBlock ? 1 : 0;
	portSettings.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

	if (tcsetattr (fd, TCSANOW, &portSettings) != 0){
        std::cout << "Error "<< errno << " from tcsetattr: " << strerror(errno) << std::endl;
		return;
	}
}

int port::openPort(std::string portName){
	const char * cportName = portName.c_str();

	//char * cportName ="/dev/ttyACM0";
	m_serialPort = open(cportName,O_RDWR | O_NOCTTY | O_SYNC);

	if(m_serialPort < 0){
        std::cout << "Error "<< errno << " from open: " << strerror(errno) << std::endl;
        return 0;
	}
	setInterfaceAttribs(m_serialPort,B115200,0);

	return EXIT_SUCCESS;
}
int port::writeToPort(std::string message){
	unsigned char msg[message.length()+1];
	for (int i = 0; i < (int)message.length(); i++) {
		msg[i]=message[i];
	}
	msg[message.length()]='\r';
	int numBytes = write(m_serialPort, msg, sizeof(msg));
	if(numBytes < 0){
        std::cout << "Error "<< errno << " writing: " << strerror(errno) << std::endl<<std::flush;
        return 0;
    }
	return numBytes;
}
//cant read
std::string port::readFromPort(){
	char readBuf [256];
	int numBytes=read(m_serialPort,&readBuf,sizeof(readBuf));
    std::string output="NOT_READ";
	if (numBytes < 0) {
        std::cout << "Error "<< errno << " reading: " << strerror(errno) << std::endl;
		return output;
	}
	if(numBytes>0){
		output = charArrayToString(readBuf,numBytes);
	}
	return output;
}


int port::closePort(){
	close(m_serialPort);
	return 0;
}

std::string port::charArrayToString(char * text,int size){
	//int size = sizeof(text)/sizeof(text[0]);
    std::string output = "";
	for (int i = 0; i < size; ++i) {
		output = output + text[i];
	}
	return output;

}

int port::setPort(std::string portName){
	m_portName = portName;
	openPort(m_portName);
	return EXIT_SUCCESS;
}
std::string port::getPort(){
	return m_portName;
}

port::~port() {
    std::cout <<"Closing "<< m_portName << std::endl;
	closePort();
	// TODO Auto-generated destructor stub
}

