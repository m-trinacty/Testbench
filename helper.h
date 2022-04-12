#ifndef HELPER_H
#define HELPER_H
#include <stddef.h>
#include <string>
#include <vector>

class Helper
{
public:
    Helper();
    static bool isNumber(char* number);
    static bool isNumber(std::string number);
    static std::vector<std::string> splitString(std::string str);
    static void daemonize();
    static void setSyslog();
    static char *pid_file_name;
    static int pid_fd;
private:
};

#endif // HELPER_H
