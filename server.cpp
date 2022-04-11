#include "server.h"
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <memory>



void Server::createConnection()
{
    createSocket();
    acceptConnection();
}

void Server::closeConnection()
{
    // Close the socket
    if(m_clientSocket==-1)
    {
        std::cerr << "Cant close connection! Quitting" << std::endl;
        exit(EXIT_FAILURE);
    }
    else{
        //shutdown(m_clientSocket, SHUT_WR);
        close(m_clientSocket);
    }
}

int Server::handleMessage()
{

    memset(buf, 0, 4096);

    // Wait for client to send data
    int bytesReceived = recv(m_clientSocket, buf, 4096, 0);
    if (bytesReceived == -1)
    {
        std::cerr << "Error in recv(). Quitting" << std::endl;
        return 0;
    }

    if (bytesReceived == 0)
    {
        std::cout << "Client disconnected " << std::endl;
        return 0;
    }

    std::cout << std::string(buf, 0, bytesReceived) << std::endl;

    // Echo message back to client
    //send(m_clientSocket, buf, bytesReceived + 1, 0);
    return 1;
}

int Server::sendMessage(std::string message)
{

    memset(buf, 0, 4096);
    strcpy(buf,message.c_str());
    send(m_clientSocket, buf,message.length() + 1, 0);
    return 1;
}

int Server::getFD()
{

    if(m_gotFD){
        return m_clientSocket;
    }
    return -1;
}

std::string Server::getMessage(){
    std::string msg = buf;
    /*int i=0;
    while(i<4096||buf[i]!='\0')
    {
        msg=msg+buf[i];
    }*/

    return msg;
}

void Server::createSocket()
{

    // Create a socket
    m_listening = socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;

    if (m_listening == -1)
    {
        std::cerr << "Can't create a socket! Quitting" << std::endl;
        exit(EXIT_FAILURE);
    }

    int enable = 1;
    if (setsockopt(m_listening, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    {
        std::cerr << "Can't set a socket options! Quitting" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Bind the ip address and port to a socket
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(m_port);
    inet_pton(AF_INET, m_ipAdress, &hint.sin_addr);

    bind(m_listening, (sockaddr*)&hint, sizeof(hint));

    // Tell Winsock the socket is for listening
    listen(m_listening, SOMAXCONN);
}

void Server::acceptConnection()
{
    // Wait for a connection
    sockaddr_in client;
    socklen_t clientSize = sizeof(client);

    m_clientSocket = accept(m_listening, (sockaddr*)&client, &clientSize);
    if (m_clientSocket == -1)
    {
        std::cerr << "Cant accept client connection! Quitting" << std::endl;
        exit(EXIT_FAILURE);
    }

    m_gotFD=true;

    char host[NI_MAXHOST];      // Client's remote name
    char service[NI_MAXSERV];   // Service (i.e. port) the client is connect on

    memset(host, 0, NI_MAXHOST); // same as memset(host, 0, NI_MAXHOST);
    memset(service, 0, NI_MAXSERV);

    if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
    {
        std::cout << host << " connected on port " << service << std::endl;
    }
    else
    {
        inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
        std::cout << host << " connected on port " << ntohs(client.sin_port) << std::endl;
    }
    // Close listening socket
    close(m_listening);

}
