/*
 * Copyright (C) Aero4TE, s.r.o. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include "port.h"
#include <iostream>
#include <cstring>
#include <string>


#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <syslog.h>
/*!
 * \brief port::port
 * \details Port class constructor without parameter only declare port object, file descriptor is
 *          set to invalid value, and needs to be set with port::set_port
 */
port::port() {
    m_serial_port=-1;   /*!< Sets port file descriptor to invalid value*/
}
/*!
 * \brief   port::port
 * \details Function sets private member with port name and an tries to open port with defined port name
 * \param   portName Name of port to which is ODrive connected
 */
port::port(std::string portName){
	m_port_name = portName;
    open_port(m_port_name);
}
/*!
 * \brief port::set_port_attribs
 * \details         Function does addition configuration of port, disables echod, nonblocking reading, sets reading timeout
 * \param fd        File descriptor of opened port
 * \param speed     Baud rate defining communication speed
 * \param parity    Parameter determines if communication use parity bit
 *
 * \return          Function return status code indicating if configuration was succesful or not
 *
 * \retval EXIT_SUCCESS    Function executed succesfully
 * \retval EXIT_FAILURE    An error occured, function execution failed
 */
int port::set_port_attribs(int fd, int speed, int parity){
	struct termios tty;
    if (tcgetattr (fd, &tty) != 0){                         /** Getting basic attributes from opened port*/
        syslog(LOG_ERR,"Error %d from tcgetattr  ",errno);

        return EXIT_FAILURE;
	}

    cfsetospeed (&tty, speed);          /** Setting speed*/
	cfsetispeed (&tty, speed);

    tty.c_cflag = (m_port_cfg.c_cflag & ~CSIZE) | CS8;     /** 8-bit chars*/
    /** disable IGNBRK for mismatched speed tests; otherwise receive break*/
    /** as \000 chars*/
    tty.c_iflag &= ~IGNBRK;         /** disable break processing*/
    tty.c_lflag = 0;                /** no signaling chars, no echo,
                                             no canonical processing*/
    tty.c_oflag = 0;                /** no remapping, no delays*/
    tty.c_cc[VMIN]  = 0;            /** read doesn't block*/
    tty.c_cc[VTIME] = 5;            /** 0.5 seconds read timeout*/

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); /** shut off xon/xoff ctrl*/

    tty.c_cflag |= (CLOCAL | CREAD);/** ignore modem controls,*/
                                            /** enable reading*/
    tty.c_cflag &= ~(PARENB | PARODD);      /** shut off parity*/
	tty.c_cflag |= parity;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr (fd, TCSANOW, &tty) != 0){        /** Saving updated attributes*/
        syslog(LOG_ERR,"Error %d from tcsetattr  ",errno);
        return EXIT_FAILURE;
	}
    return EXIT_SUCCESS;
}

/*!
 * \brief port::set_port_block
 * \details Function can be used to set port as blocking
 * \param fd            File descriptor of opened port
 * \param shouldBlock   Parameter describing if port should be blocking and waiting for user input
 *
 * \return          Function return status code indicating if configuration was succesful or not
 *
 * \retval EXIT_SUCCESS    Function executed succesfully
 * \retval EXIT_FAILURE    An error occured, function execution failed
 */
int port::set_port_block(int fd, int shouldBlock){
	//struct termios tty;
    memset (&m_port_cfg, 0, sizeof m_port_cfg);
    if (tcgetattr (fd, &m_port_cfg) != 0){
        syslog(LOG_ERR,"Error %d from tcgetattr  ",errno);
        return EXIT_FAILURE;
	}

    m_port_cfg.c_cc[VMIN]  = shouldBlock ? 1 : 0;
    m_port_cfg.c_cc[VTIME] = 5;            /* 0.5 seconds read timeout*/

    if (tcsetattr (fd, TCSANOW, &m_port_cfg) != 0){
        syslog(LOG_ERR,"Error %d from tcsetattr  ",errno);
        return EXIT_FAILURE;
	}
    return EXIT_SUCCESS;
}
/*!
 * \brief   port::open_port
 * \details Functions open port. It embbeds system call open() to open port and config functions
 *          port::set_port_attribs.
 * \param   port_name   Name of port that has to be opened
 *
 * \return  Function return status code indicating if function was succesful or not
 *
 * \retval  EXIT_SUCCESS    Function executed succesfully
 * \retval  EXIT_FAILURE    An error occured, function execution failed
 */
int port::open_port(std::string port_name){
    const char * cportName = port_name.c_str();

    m_serial_port = open(cportName,O_RDWR | O_NOCTTY | O_SYNC);

	if(m_serial_port < 0){
        syslog(LOG_ERR,"Error %d while opening port  ",errno);
        return EXIT_FAILURE;
	}
	set_port_attribs(m_serial_port,B115200,0);

	return EXIT_SUCCESS;
}
/*!
 * \brief   port::write_port
 * \details Public function to write commands to port with ODrive connected. Function adds ending symbol
 *          "\r" to message, wchich is equvivalent to enter. Then message is written to port with
 *          system call write()
 * \param   message String command that should be written to port
 * \return  Function return number of bytes written to port or failure status code
 */
int port::write_port(std::string message){
	unsigned char msg[message.length()+1];
	for (int i = 0; i < (int)message.length(); i++) {
		msg[i]=message[i];
	}
	msg[message.length()]='\r';
	int numBytes = write(m_serial_port, msg, sizeof(msg));
	if(numBytes < 0){

        syslog(LOG_ERR,"Error %d while writing to port  ",errno);
        return EXIT_FAILURE;
    }
    /*Check for return*/
    char readBuf [256];
    if(message[0]=='w'){
        read(m_serial_port,&readBuf,sizeof(readBuf));
    }
    return numBytes;
}
/*!
 * \brief port::read_port
 *
 * \details Function reads response message written to port by ODrive
 *
 * \return  String with response message, otherwise return failure reading
 *
 * \retval  Response message
 * \retval  "NOT_READ"
 */
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
/*!
 * \brief port::close_port
 * \details Function closing opened port. It simply wraps system call close()
 *
 * \return  Function return status code indicating if function was succesful or not
 *
 * \retval  EXIT_SUCCESS    Function executed succesfully
 * \retval  EXIT_FAILURE    An error occured, function execution failed
 */

int port::close_port(){
    if(m_serial_port>=0){
        close(m_serial_port);
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}
/*!
 * \brief port::char_arr_to_string
 * \details This function convert char array to string
 * \param text  Char array with message
 * \param size  Size/Length of message
 * \return String representation of message
 */
std::string port::char_arr_to_string(char * text,int size){
    std::string output = "";
	for (int i = 0; i < size; ++i) {
		output = output + text[i];
	}
	return output;

}
/*!
 * \brief   port::set_port
 * \details This function can set port name and open port if object of class is declared with
 *          empty constructor
 * \param   portName Name of port where is ODrive connected
 * \return  Function return status code indicating if function was succesful or not
 *
 * \retval  EXIT_SUCCESS    Function executed succesfully
 * \retval  EXIT_FAILURE    An error occured, function execution failed
 */
int port::set_port(std::string portName){
	m_port_name = portName;
    if(open_port(m_port_name)==EXIT_SUCCESS){
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}
/*!
 * \brief port::get_port
 * \details Function returns name of port
 * \return Name of port
 */
std::string port::get_port(){
	return m_port_name;
}
/*!
 * \brief port::~port
 * \details Destructor close the port when refference to port object is lost
 */
port::~port() {
    syslog(LOG_INFO,"Closing port");
    close_port();
}

