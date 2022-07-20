#ifndef SERVER_H
#define SERVER_H
#include <iostream>

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
    std::string msg_get();
private:

    void create_server();
    void acpt_server();

    char buf[4096];
    char* m_ipAddr;
    int m_port;
    int m_lstn;
    int m_cliSock;
    bool m_gotFD=false;
};

#endif // SERVER_H
