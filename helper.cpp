#include "helper.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <getopt.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <bits/stdc++.h>
#include <vector>


/*!
 * \brief       helper::check_num
 * \details     Function is checking if parameter passed in is number or not.
 * \param       number    Char array to check
 * \return      Function return bool value indicating if passed parameter is number.
 * \retval      true   Indicating that parameter is number
 * \retval      false  Indicating that parameter is not number
 */
bool helper::check_num(char *number)
{
    std::string s = number;
    std::string::const_iterator it = s.begin();

    while (it != s.end() && std::isdigit(*it)){
        ++it;
    }
    return !s.empty() && it == s.end();

}
/*!
 * \brief       helper::check_num
 * \details     Function is checking if parameter passed in is number or not.
 * \param       number    String to check
 * \return      Function return bool value indicating if passed parameter is number.
 * \retval      true   Indicating that parameter is number
 * \retval      false  Indicating that parameter is not number
 */
bool helper::check_num(std::string number)
{

    std::string::const_iterator it = number.begin();

    while (it != number.end() && std::isdigit(*it)){
        ++it;
    }
    return !number.empty() && it == number.end();
}
/*!
 * \brief       helper::check_file
 * \details     This function chcecks if file with given name is already existing
 * \note        Used for checking if log(for testbench data) file is already existing
 * \param       name File name to check
 * \return      Function return bool value indicating if file exist or not.
 * \retval      true File is existing
 * \retval      false File is not existing
 */
bool helper::check_file(const std::string &name)
{
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}

/*!
 * \brief       helper::split_str
 * \details     This function split by space given string into single words
 * \param       str String to split
 * \return      String vector with words included in parameter str
 */

std::vector<std::string> helper::split_str(std::string str)
{
    std::stringstream ss(str);
    std::string word;
    std::vector<std::string> out;
    while (ss >> word)
    {
        out.push_back(word);
    }
    return out;
}


int helper::pid_fd=-1;  /*!< Process file descriptor*/
char *helper::pid_file_name=NULL;   /*!< Process file name*/
/*!
 * \brief   helper::daemonize
 * \details Function fork of started process and does basic configuration of process
 *          to run it as daemon
 */
void helper::daemonize()
{
    pid_t pid = 0;
    int fd;

    /* Fork off the parent process */
    pid = fork();

    /* An error occurred */
    if (pid < 0) {
        syslog(LOG_ERR,"Error while forking");
        exit(EXIT_FAILURE);
    }

    /* Success: Let the parent terminate */
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    /* On success: The child process becomes session leader */
    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }

    /* Ignore signal sent from child to parent process */
    signal(SIGCHLD, SIG_IGN);

    /* Fork off for the second time*/
    pid = fork();

    /* An error occurred */
    if (pid < 0) {

        syslog(LOG_ERR,"Error while second fork");
        exit(EXIT_FAILURE);
    }

    /* Success: Let the parent terminate */
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    /* Set new file permissions */
    umask(0);

    /* Change the working directory to the root directory */
    /* or another appropriated directory */
    //chdir("/");

    /* Close all open file descriptors */
    for (fd = sysconf(_SC_OPEN_MAX); fd > 0; fd--) {
        close(fd);
    }

    /* Reopen stdin (fd = 0), stdout (fd = 1), stderr (fd = 2) */
    stdin = fopen("/dev/null", "r");
    stdout = fopen("/dev/null", "w+");
    stderr = fopen("/dev/null", "w+");

    /* Try to write PID of daemon to lockfile */
    if (pid_file_name != NULL)
    {
        char str[256];
        pid_fd = open(pid_file_name, O_RDWR|O_CREAT, 0640);
        if (pid_fd < 0) {
            /* Can't open lockfile */

            syslog(LOG_ERR,"Cant open lockfile");
            exit(EXIT_FAILURE);
        }
        if (lockf(pid_fd, F_TLOCK, 0) < 0) {
            /* Can't lock file */

            syslog(LOG_ERR,"Cant lock file");
            exit(EXIT_FAILURE);
        }
        /* Get current PID */
        sprintf(str, "%d\n", getpid());
        /* Write PID to lockfile */
        write(pid_fd, str, strlen(str));
    }
}
/*!
 * \brief   helper::cfg_syslog
 * \details This function sets log file for loggin information and error from program. Name of the file is
 *          TevogsTestbench and log file is located in /var/log
 */
void helper::cfg_syslog()
{
    openlog("TevogsTestbench", LOG_PID|LOG_CONS|LOG_NDELAY, LOG_USER);
}
