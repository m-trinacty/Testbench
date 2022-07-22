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


/*!
 * \brief server::conn_server
 *
 * \details Function makes connection after Server object is intialized. It uses template pattern
 *          to ensure correct order of function call
 */
void server::conn_server()
{
    if((m_ipAddr && !m_ipAddr[0])&& m_port>0){
        create_server();
        acpt_server();
    }
    else{

        syslog(LOG_ERR,"Cant create connection",errno);
    }
}
/*!
 * \brief server::dconn_server
 *
 * \details Function for disconnecting from client. Function simply use system call close() to close client socket
 */
void server::dconn_server()
{
    // Close the socket
    if(m_cliSock==-1)
    {

        syslog(LOG_ERR,"Cant close connection! Quitting",errno);
        exit(EXIT_FAILURE);
    }
    else{
        close(m_cliSock);
    }
}
/*!
 * \brief   server::handler_msg
 *
 * \details Function for handling incoming messages with recv() system call. Message is then stored to
 *          buffer, from which can be extracted by std::get_msg().
 *
 * \return  Function return status code
 *
 * \retval  EXIT_FAILURE    Error in recieving or Client disconnected
 * \retval  EXIT_SUCCESS    Function executed successfully
 */
int server::handler_msg()
{


    memset(buf, 0, 4096);

    // Wait for client to send data
    int bytes_recv = recv(m_cliSock, buf, 4096, 0);
    if (bytes_recv == -1)
    {

        syslog(LOG_ERR,"Error in recv(). Quitting",errno);
        return EXIT_FAILURE;
    }

    if (bytes_recv == 0)
    {

        syslog(LOG_INFO,"Client Disconnected",errno);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
/*!
 * \brief   server::send_msg
 *
 * \details Function for sending messages to connected client.
 *
 * \param   msg String message that has to be sent.
 *
 * \return  Function return status code
 *
 * \retval  EXIT_FAILURE    Error while sending or Client disconnected
 * \retval  EXIT_SUCCESS    Function executed successfully
 */
int server::send_msg(std::string msg)
{

    memset(buf, 0, 4096);
    strcpy(buf,msg.c_str());
    int bytes_sent = send(m_cliSock, buf,msg.length() + 1, 0);
    if (bytes_sent == -1)
    {

        syslog(LOG_ERR,"Error in send(). Quitting",errno);
        return EXIT_FAILURE;
    }

    if (bytes_sent == 0)
    {

        syslog(LOG_INFO,"Client Disconnected",errno);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
/*!
 * \brief   server::get
 * \details Functiont return file descriptor of client socket
 * \return  File descriptor or negative status code
 * \retval  File descriptor
 * \retval  -EXIT_FAILURE
 */
int server::get()
{

    if(m_gotFD){
        return m_cliSock;
    }
    return -EXIT_FAILURE;
}
/*!
 * \brief   server::get_msg
 * \details Function for observing message from char array buffer
 * \return  String message
 */
std::string server::get_msg(){
    std::string msg = buf;

    return msg;
}
/*!
 * \brief   server::create_server
 * \details Private function creates socket based on port and IP adress for communication.
 *          Also sets basic configuration. TCP Socket is available immeadiately after disconnection.
 */
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
/*!
 * \brief   server::acpt_server
 * \details This function waits for client to connect to listening server on device controlling ODrive.
 *          Needs to be called after server::create_server()
 */
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
