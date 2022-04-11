#ifndef SERVER_H
#define SERVER_H
#include <iostream>

class Server
{
public:
    Server(std::string ipAdress, int port){
        m_ipAdress = &*ipAdress.begin();
        m_port = port;
    }

    void createConnection();
    void closeConnection();
    int handleMessage();
    int sendMessage(std::string message);
    int getFD();
    std::string getMessage();
private:

    void createSocket();
    void acceptConnection();

    char buf[4096];
    char* m_ipAdress;
    int m_port;
    int m_listening;
    int m_clientSocket;
    bool m_gotFD=false;
};

#endif // SERVER_H
