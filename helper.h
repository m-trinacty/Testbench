/*
 * Copyright (C) Aero4TE, s.r.o. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef HELPER_H
#define HELPER_H
#include <stddef.h>
#include <string>
#include <vector>
/*!
 * \brief The helper class
 * \details Class with static function to simplify some of needed functionalities, such as
 *          checking if string is number, checking if file exist etc.
 */
class helper
{
public:
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
