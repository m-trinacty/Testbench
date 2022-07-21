/*
 * Copyright (C) Aero4TE, s.r.o. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include <iostream>
#include <string>
// Linux headers
#include <termios.h> // Contains POSIX terminal control definitions

#ifndef PORT_H_
#define PORT_H_

/*!
 * \brief The port class
 * \details This class provides functions to work with ports. Constructors builds connection with
 *          port user code can then use only RW functions. Configuration is predefined for connection
 *          with ODrive.
 */
class port {

private:
    std::string m_port_name;    /*!< Private member with port name*/
    struct termios m_port_cfg;    /*!< Private member with port settings*/

    int m_serial_port;            /*!< Private member with port file descriptor*/

    int open_port(std::string port_name);
    int close_port();
    int set_port_attribs(int fd, int speed, int parity);
    int set_port_block(int fd, int shouldBlock);
    std::string char_arr_to_string(char * text,int size);
public:
	port();
    port(std::string portName);
    int write_port(std::string message);
    std::string read_port();
    int set_port(std::string portName);
    std::string get_port();
	virtual ~port();


};

#endif /* PORT_H_ */
