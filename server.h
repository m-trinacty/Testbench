/*
 * Copyright (C) Aero4TE, s.r.o. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */
#ifndef SERVER_H
#define SERVER_H
#include <iostream>
/*!
 * \brief   The server class
 * \details Class for server communication between device controlling ODrive and Controll station.
 *          Provided functions offers creating Socket connection on IP adress of device, and then handling
 *          coming messages and sending messages
 */
class server
{
public:
    server(std::string ipAdress, int port){
        m_ipAddr = &*ipAdress.begin();
        m_port = port;
    }

    void conn_server();
    void dconn_server();
    int handler_msg();
    int send_msg(std::string message);
    int get();
    std::string get_msg();
private:

    void create_server();
    void acpt_server();

    char buf[4096];     /*!< Char array as buffer for incoming messages*/
    char* m_ipAddr;     /*!< Char array for storing IP Addres*/
    int m_port;         /*!< Private member for storing port number*/
    int m_lstn;         /*!< Private member of File descriptor for listenning socket*/
    int m_cliSock;      /*!< Private member of File descriptor for client socket */
    bool m_gotFD=false; /*!< Bool value representing if File descriptor of client socket was properly set*/
};

#endif // SERVER_H
