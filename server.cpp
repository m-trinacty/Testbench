#include "server.h"
#include <iostream>

#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <syslog.h>

#include <string.h>
#include <string>
#include <memory>



void server::conn_server()
{
    create_server();
    acpt_server();
}

void server::dconn_server()
{
    // Close the socket
    if(m_cliSock==-1)
    {

        syslog(LOG_ERR,"Cant close connection! Quitting",errno);
        //std::cerr << "Cant close connection! Quitting" << std::endl;
        exit(EXIT_FAILURE);
    }
    else{
        //shutdown(m_clientSocket, SHUT_WR);
        close(m_cliSock);
    }
}

int server::handler_msg()
{


    memset(buf, 0, 4096);

    // Wait for client to send data
    int bytesRcv = recv(m_cliSock, buf, 4096, 0);
    if (bytesRcv == -1)
    {

        syslog(LOG_ERR,"Error in recv(). Quitting",errno);
        //std::cerr << "Error in recv(). Quitting" << std::endl;
        return 0;
    }

    if (bytesRcv == 0)
    {

        syslog(LOG_INFO,"Client Disconnected",errno);
        return 0;
    }

    //std::cout << std::string(buf, 0, bytesRcv) << std::endl;

    // Echo message back to client
    //send(m_clientSocket, buf, bytesReceived + 1, 0);
    return 1;
}

int server::send_msg(std::string message)
{

    memset(buf, 0, 4096);
    strcpy(buf,message.c_str());
    send(m_cliSock, buf,message.length() + 1, 0);
    return 1;
}

int server::get()
{

    if(m_gotFD){
        return m_cliSock;
    }
    return -1;
}

std::string server::msg_get(){
    std::string msg = buf;
    /*int i=0;
    while(i<4096||buf[i]!='\0')
    {
        msg=msg+buf[i];
    }*/

    return msg;
}

void server::create_server()
{

    // Create a socket
    m_lstn = socket(AF_INET, SOCK_STREAM, 0);

    if (m_lstn == -1)
    {

        syslog(LOG_ERR,"Can't create a socket! Quitting");
        exit(EXIT_FAILURE);
    }

    int enable = 1;
    if (setsockopt(m_lstn, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    {

        syslog(LOG_ERR,"Can't socket options! Quitting");
        exit(EXIT_FAILURE);
    }

    // Bind the ip address and port to a socket
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(m_port);
    inet_pton(AF_INET, m_ipAddr, &hint.sin_addr);

    bind(m_lstn, (sockaddr*)&hint, sizeof(hint));

    // Tell Winsock the socket is for listening
    listen(m_lstn, SOMAXCONN);
}

void server::acpt_server()
{
    // Wait for a connection
    sockaddr_in client;
    socklen_t clientSize = sizeof(client);

    m_cliSock = accept(m_lstn, (sockaddr*)&client, &clientSize);
    if (m_cliSock == -1)
    {

        syslog(LOG_ERR,"Can't accept client connection! Quitting");
        exit(EXIT_FAILURE);
    }

    m_gotFD=true;

    char host[NI_MAXHOST];      // Client's remote name
    char service[NI_MAXSERV];   // Service (i.e. port) the client is connect on

    memset(host, 0, NI_MAXHOST); // same as memset(host, 0, NI_MAXHOST);
    memset(service, 0, NI_MAXSERV);

    if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
    {
        syslog(LOG_INFO,"%s connected on port %s",host,service);
        //std::cout << host << " connected on port " << service << std::endl;
    }
    else
    {
        inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);

        syslog(LOG_INFO,"%s connected on port %d",host,ntohs(client.sin_port));
        //std::cout << host << " connected on port " << ntohs(client.sin_port) << std::endl;
    }
    // Close listening socket
    close(m_lstn);

}
