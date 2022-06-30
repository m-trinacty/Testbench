#ifndef HELPER_H
#define HELPER_H
#include <stddef.h>
#include <string>
#include <vector>

class Helper
{
public:
    Helper();
    static bool check_num(char* number);
    static bool check_num(std::string number);
    static bool check_file(const std::string& name);
    static std::vector<std::string> split_str(std::string str);
    static void daemonize();
    static void cfg_syslog();
    static char *pid_file_name;
    static int pid_fd;
private:
};

#endif // HELPER_H
