#ifndef HELPER_H
#define HELPER_H
#include <stddef.h>
#include <string>

class Helper
{
public:
    Helper();
    static bool isNumber(char* number);
    static bool isNumber(std::string number);
    static void daemonize();
    static char *pid_file_name;
    static int pid_fd;
private:
};

#endif // HELPER_H
