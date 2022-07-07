/*
 * Copyright (C) Aero4TE, s.r.o. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include "oDrive.h"

#include <string>
#include <memory>
#include <syslog.h>

#include <unistd.h>

#define	ERROR_COMMAND_WRITE   "ERROR could not write command"
#define	ERROR_COMMAND_READ   "ERROR could not read command"
#define INVALID_PROPERTY    "invalid property"
#define INVALID_COMMAND_FORMAT  "invalid command format"
#define UNKNOWN_COMMAND "unknown command"
#define PRINT_READ_VALUES   false

/*!
 * \brief       oDrive::set_min_endstop
 * \details     Enable or disable minimal endstop
 *
 * \note        This function is used in initial homing sequence before spining, after homing
 *              its disabled to allow spining
 *
 * \param       axis      int   Determine axis with motor, connected with minimal endstop
 * \param       enabled   bool  Parameter enable(true) or disable minimal endstop
 *
 * \return      Function returns return code
 *
 * \retval      EXIT_SUCCESS    Function executed succesfully
 * \retval      EXIT_FAILURE    An error occured, couldn't write command to port
 */
int oDrive::set_min_endstop(int axis, bool enabled)
{
    std::string enable=enabled?"1":"0";
    std::string command = "w axis"+ std::to_string(axis)+ ".min_endstop.config.enabled "+enable;
    if (m_oDrive_port->write_port(command)<0) {
        syslog(LOG_ERR,ERROR_COMMAND_WRITE);
        return EXIT_FAILURE;
    }
    usleep(100);
    return EXIT_SUCCESS;
}
/*!
 * \brief       oDrive::get_min_endstop
 * \details     Function gives information, if motor is homed on minimal endstop position
 *
 * \note        Used to determine if slowly spining motor hits  minimal endstop
 *
 * \param       axis      int   Determine axis with motor, connected with minimal endstop
 *
 * \return      Function returns if motor has hit minimal endstop
 *
 * \retval      true    Minimal endstop was hit
 * \retval      false   Minimal endstop was not hit
 */
bool oDrive::get_min_endstop(int axis)
{
    std::string out;
    std::string command = "r axis"+std::to_string(axis)+".min_endstop.endstop_state";
    if (m_oDrive_port->write_port(command)<0) {
        syslog(LOG_ERR,ERROR_COMMAND_WRITE);
        return EXIT_FAILURE;
    }
    usleep(100);
    out = m_oDrive_port->read_port();
    if (out == INVALID_PROPERTY ||out ==  INVALID_COMMAND_FORMAT ||out ==  UNKNOWN_COMMAND) {
        syslog(LOG_ERR,ERROR_COMMAND_READ);
        return EXIT_FAILURE;
    }
    bool homed = std::stoi(out)==1?true:false;
    return homed;
}
/*!
 * \brief       oDrive::get_curr
 * \details     Current on DC bus, calculated by ODrive, when positive, ODrive is consuming power
 *              from power supply, when negative, ODrive is sourcing power to power supply, sum of
 *              motor current and current on brake resistor.
 *
 * \note        Not used in application
 *
 * \param       axis      int   Axis is not used here, but needed to work, otherwise command will fail
 *
 * \return      Measured DC bus current
 * \retval      float   Measured DC bus current, positive or negative
 */
float oDrive::get_curr(int axis)
{
    std::string out;
    std::string command = "r axis"+std::to_string(axis)+".ibus";
    if (m_oDrive_port->write_port(command)<0) {
        syslog(LOG_ERR,ERROR_COMMAND_WRITE);
        return EXIT_FAILURE;
    }
    usleep(100);
    out = m_oDrive_port->read_port();
    if (out == INVALID_PROPERTY ||out ==  INVALID_COMMAND_FORMAT ||out ==  UNKNOWN_COMMAND) {
        syslog(LOG_ERR,ERROR_COMMAND_READ);
        return EXIT_FAILURE;
    }
    float current =std::stof(out);
    return current;
}
/*!
 * \brief       oDrive::get_curr_Iq
 * \details     Current on measured on motor axis, used to create motor torque.It is the current
 *              along the Q axis in Field Oriented Control Loop
 *
 * \note        Used in application to determine limits of maximal current and speed
 *
 * \param       axis      int   Axis of motor where Iq current is measured
 *
 * \return      Measured current along the Q axis in FOC
 * \retval      float   Measured Iq current
 */
float oDrive::get_curr_Iq(int axis)
{
    std::string out;
    std::string command = "r axis"+std::to_string(axis)+".motor.current_control.Iq_measured";
    if (m_oDrive_port->write_port(command)<0) {
        syslog(LOG_ERR,ERROR_COMMAND_WRITE);
        return EXIT_FAILURE;
    }
    usleep(100);
    out = m_oDrive_port->read_port();
    if (out == INVALID_PROPERTY ||out ==  INVALID_COMMAND_FORMAT ||out ==  UNKNOWN_COMMAND) {
        syslog(LOG_ERR,ERROR_COMMAND_READ);
        return EXIT_FAILURE;
    }
    float current =std::stof(out);
    return current;
}

oDrive::oDrive() {
    // TODO Auto-generated constructor stub

}

oDrive::oDrive(std::string port_name){
    m_oDrive_port = new port(port_name);
}
int oDrive::command_console(){
    std::string out;
    std::cout << "********************************" << std::endl;
    std::cout << "| oDrive Tool on Toradex       |" << std::endl;
    std::cout << "| Please input ASCII comands   |" << std::endl;
    std::cout << "| To control oDrive            |" << std::endl;
    std::cout << "********************************" << std::endl;

    std::string input="aaaa";
    char startLetter= 'a';
    while(1){
        std::cout<<">>";
        getline(std::cin, input);
        startLetter = input[0];
        if(input=="quit"){
            break;
        }

        switch (startLetter) {
        case 'r':
            m_oDrive_port->write_port(input);
            usleep(100);
            out = m_oDrive_port->read_port();
            std::cout << out << std::endl;
            break;
        case 'w':
            m_oDrive_port->write_port(input);
            usleep(100);
            break;
        case 'f':
            m_oDrive_port->write_port(input);
            usleep(100);
            out = m_oDrive_port->read_port();
            std::cout << out << std::endl;
            break;
        default:
            m_oDrive_port->write_port(input);
            usleep(100);
            break;
        }
    }
    return EXIT_SUCCESS;
}
int oDrive::set_axis_state(int axis,int state){
    std::string command = "w axis"+ std::to_string(axis)+ ".requested_state "+std::to_string(state);
    if (m_oDrive_port->write_port(command)<0) {
        syslog(LOG_ERR,ERROR_COMMAND_WRITE);
        return EXIT_FAILURE;
    }
    usleep(100);
    return EXIT_SUCCESS;
}

int oDrive::set_input_mode(int axis, int mode){
    std::string command = "w axis"+std::to_string(axis)+ ".controller.config.input_mode "+std::to_string(mode);
    if (m_oDrive_port->write_port(command)<0) {
        syslog(LOG_ERR,ERROR_COMMAND_WRITE);
        return EXIT_FAILURE;
    }
    usleep(100);
    return EXIT_SUCCESS;
}

int oDrive::set_vel(int axis, float vel)
{
    std::string command= "w axis"+std::to_string(axis)+".controller.input_vel " +std::to_string(vel);
    m_oDrive_port->write_port(command);
    usleep(100);
    return EXIT_SUCCESS;
}
int oDrive::set_lockin_vel(int axis, float vel){
    std::string command = "w axis"+std::to_string(axis)+".config.general_lockin.vel "+std::to_string(vel);
    m_oDrive_port->write_port(command);
    usleep(100);
    return EXIT_SUCCESS;
}

int oDrive::clear_errors(int axis)
{
    std::string command = "w axis"+std::to_string(axis)+".error 0";
    if (m_oDrive_port->write_port(command)<0) {
        syslog(LOG_ERR,ERROR_COMMAND_WRITE);
        return EXIT_FAILURE;
    }
    usleep(100);
    return EXIT_SUCCESS;
}

int oDrive::get_axis_state(int axis)
{
    std::string out;
    std::string command = "r axis"+std::to_string(axis)+".current_state";
    if (m_oDrive_port->write_port(command)<0) {
        syslog(LOG_ERR,ERROR_COMMAND_WRITE);
        return EXIT_FAILURE;
    }
    usleep(100);
    out = m_oDrive_port->read_port();
    if (out == INVALID_PROPERTY ||out ==  INVALID_COMMAND_FORMAT ||out ==  UNKNOWN_COMMAND) {
        syslog(LOG_ERR,ERROR_COMMAND_READ);
        return EXIT_FAILURE;
    }
    int state = std::stoi(out);
    return state;
}

float oDrive::get_pos_est(int axis)
{
    std::string out;
    std::string command = "r axis"+std::to_string(axis)+".encoder.pos_estimate";
    if (m_oDrive_port->write_port(command)<0) {
        syslog(LOG_ERR,ERROR_COMMAND_WRITE);
        return EXIT_FAILURE;
    }
    usleep(100);
    out = m_oDrive_port->read_port();
    if (out == INVALID_PROPERTY ||out ==  INVALID_COMMAND_FORMAT ||out ==  UNKNOWN_COMMAND) {
        syslog(LOG_ERR,ERROR_COMMAND_READ);
        return EXIT_FAILURE;
    }
    float pos =std::stof(out);
    return pos;
}

float oDrive::get_pos_est_cnt(int axis)
{
    std::string out;
    std::string command = "r axis"+std::to_string(axis)+".encoder.pos_estimate_counts";
    if (m_oDrive_port->write_port(command)<0) {
        syslog(LOG_ERR,ERROR_COMMAND_WRITE);
        return EXIT_FAILURE;
    }
    usleep(100);
    out = m_oDrive_port->read_port();
    if (out == INVALID_PROPERTY ||out ==  INVALID_COMMAND_FORMAT ||out ==  UNKNOWN_COMMAND) {
        std::cout<<out<<std::endl;
        syslog(LOG_ERR,ERROR_COMMAND_READ);
        return EXIT_FAILURE;
    }
    float pos =std::stof(out);
    return pos;
}

float oDrive::get_pos_cir(int axis)
{
    std::string out;
    std::string command = "r axis"+std::to_string(axis)+".encoder.pos_circular";
    if (m_oDrive_port->write_port(command)<0) {
        syslog(LOG_ERR,ERROR_COMMAND_WRITE);
        return EXIT_FAILURE;
    }
    usleep(100);
    out = m_oDrive_port->read_port();
    if (out == INVALID_PROPERTY ||out ==  INVALID_COMMAND_FORMAT || out == UNKNOWN_COMMAND) {
        syslog(LOG_ERR,ERROR_COMMAND_READ);
        return EXIT_FAILURE;
    }
    float pos =std::stof(out);
    return pos;
}

float oDrive::get_pos_cpr_cnt(int axis)
{
    std::string out;
    std::string command = "r axis"+std::to_string(axis)+".encoder.pos_circular";
    if (m_oDrive_port->write_port(command)<0) {
        syslog(LOG_ERR,ERROR_COMMAND_WRITE);
        return EXIT_FAILURE;
    }
    usleep(100);
    out = m_oDrive_port->read_port();
    if (out == INVALID_PROPERTY ||out ==  INVALID_COMMAND_FORMAT || out == UNKNOWN_COMMAND) {
        syslog(LOG_ERR,ERROR_COMMAND_READ);
        return EXIT_FAILURE;
    }
    float pos =std::stof(out);
    return pos;
}
int oDrive::get_locking_vel(int axis){
    std::string command = "r axis"+std::to_string(axis)+".config.general_lockin.vel";
    std::string out;
    if (m_oDrive_port->write_port(command)<0) {
        syslog(LOG_ERR,ERROR_COMMAND_WRITE);
        return EXIT_FAILURE;
    }
    usleep(100);
    out = m_oDrive_port->read_port();
    if (out == INVALID_PROPERTY || out ==  INVALID_COMMAND_FORMAT || out ==  UNKNOWN_COMMAND) {
        syslog(LOG_ERR,ERROR_COMMAND_READ);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

float oDrive::get_vel(int axis)
{
    std::string command = "r axis"+std::to_string(axis)+".encoder.vel_estimate";
    std::string out;
    std::string delimiter = " ";
    float velocity;
    if(m_oDrive_port->write_port(command)<0){
        syslog(LOG_ERR,ERROR_COMMAND_WRITE);
        return 0;
    }
    usleep(100);
    out = m_oDrive_port->read_port();
    if (out == INVALID_PROPERTY || out == INVALID_COMMAND_FORMAT || out == UNKNOWN_COMMAND) {
        syslog(LOG_ERR,ERROR_COMMAND_READ);
        return EXIT_FAILURE;
    }
    velocity =stof(out.substr(0,out.find(delimiter)));
    return velocity;
}

float oDrive::get_pos_turns(int axis){
    std::string command = "f "+std::to_string(axis);
    std::string out;
    std::string delimiter = " ";
    float pos;
    if(m_oDrive_port->write_port(command)<0){
        syslog(LOG_ERR,ERROR_COMMAND_WRITE);
        return 0;
    }
    usleep(100);
    out = m_oDrive_port->read_port();
    if (out == INVALID_PROPERTY || out == INVALID_COMMAND_FORMAT || out == UNKNOWN_COMMAND) {
        syslog(LOG_ERR,ERROR_COMMAND_READ);
        return EXIT_FAILURE;
    }
#if PRINT_READ_VALUES
    std::cout << out << std::endl;
#endif
    pos =stof(out.substr(0,out.find(delimiter)));
    return pos;
}
int oDrive::set_pos_turns(int axis,float pos){
    std::string command = "q "+std::to_string(axis)+" "+std::to_string(pos);
    std::string out;
    std::string delimiter = " ";
    if(m_oDrive_port->write_port(command)<0){
        syslog(LOG_ERR,ERROR_COMMAND_WRITE);
        return 0;
    }
    usleep(100);
    return EXIT_SUCCESS;
}

int oDrive::set_pos(int axis, float pos)
{
    std::string command = "w axis"+std::to_string(axis)+ ".controller.input_vel "+std::to_string(pos);
    if (m_oDrive_port->write_port(command)<0) {
        syslog(LOG_ERR,ERROR_COMMAND_WRITE);
        return EXIT_FAILURE;
    }
    usleep(100);
    return EXIT_SUCCESS;
}

void oDrive::homing(int axis)
{
    set_min_endstop(axis,true);
    set_axis_state(axis,AXIS_STATE_HOMING);
    syslog(LOG_INFO,"Searching home");
    bool searching = false;
    while(!searching){
        searching =get_min_endstop(axis);
        usleep(100);
    }
    if(get_min_endstop(axis)){
        syslog(LOG_INFO,"Odrive homed");
    }
}
oDrive::~oDrive() {
    delete m_oDrive_port;
    // TODO Auto-generated destructor stub
}

