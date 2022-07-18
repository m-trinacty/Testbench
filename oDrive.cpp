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
 * \return      Function return measured DC bus current
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
 * \return      Function return measured current along the Q axis in FOC
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
/*!
 * \brief       oDrive::oDrive
 * \details     Constructor injecting name of port, where is ODrive connected
 * \param       port_name   string  Name of port where is ODrive connected
 */
oDrive::oDrive(std::string port_name){
    m_oDrive_port = new port(port_name);
}

/*!
 * \brief       oDrive::command_console
 * \details     Endless while loop, waiting for user input with ASCII commands, loop can be
 *              exited by command "quit". Otherwise function interacts with commands
 *              "r" for reading f.e. "r axis0.min_endstop.endstop_state"
 *              "w" for writing f.e. "w axis0.min_endstop.config.enabled 1"
 *              "f" for feedback f.e."f motor"
 * \return      Function returns status code
 * \retval      int EXIT_SUCCES
 */
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
/*!
 * \brief       oDrive::set_axis_state
 * \details     Function sets state of axis, which determines motors behavior, state
 *              should be selected from enum axis_state
 *              f.e.  AXIS_STATE_CLOSED_LOOP_CONTROL(8) is used when motor spins.
 *
 * \param       axis      int Axis of motor, on which is state set
 * \param       state     int Number corresponding with state
 *
 * \return      Function return status code
 *
 * \retval      EXIT_SUCCESS    Function executed succesfully
 * \retval      EXIT_FAILURE    An error occured, couldn't write command to port
 */
int oDrive::set_axis_state(int axis,int state){
    std::string command = "w axis"+ std::to_string(axis)+ ".requested_state "+std::to_string(state);
    if (m_oDrive_port->write_port(command)<0) {
        syslog(LOG_ERR,ERROR_COMMAND_WRITE);
        return EXIT_FAILURE;
    }
    usleep(100);
    return EXIT_SUCCESS;
}
/*!
 * \brief       oDrive::set_input_mode
 * \details     Input mode define the way ODrive unit and motors are controlled, modes
 *              should be selected from enum input_mode
 *
 * \param       axis      int Axis of motor, on which is control mode set
 * \param       mode      int Number corresponding with control mode
 * \return      Function return status code
 *
 * \retval      EXIT_SUCCESS    Function executed succesfully
 * \retval      EXIT_FAILURE    An error occured, couldn't write command to port
 */
int oDrive::set_input_mode(int axis, int mode){
    std::string command = "w axis"+std::to_string(axis)+ ".controller.config.input_mode "+std::to_string(mode);
    if (m_oDrive_port->write_port(command)<0) {
        syslog(LOG_ERR,ERROR_COMMAND_WRITE);
        return EXIT_FAILURE;
    }
    usleep(100);
    return EXIT_SUCCESS;
}
/*!
 * \brief       oDrive::set_vel
 * \details     Function sets velocity in rps, which stands for rotation per second
 *              of motor
 * \note        Motor can do around 20 rps with load of testbench
 *
 * \param       axis      int   Axis of motor, on which is velocity is set
 * \param       vel       float Velocity in rps, when negative, motor spins in other direction
 *
 * \return      Function return status code
 *
 * \retval      EXIT_SUCCESS    Function executed succesfully
 * \retval      EXIT_FAILURE    An error occured, couldn't write command to port
 */
int oDrive::set_vel(int axis, float vel)
{
    std::string command= "w axis"+std::to_string(axis)+".controller.input_vel " +std::to_string(vel);
    if (m_oDrive_port->write_port(command)<0) {
        syslog(LOG_ERR,ERROR_COMMAND_WRITE);
        return EXIT_FAILURE;
    }
    usleep(100);
    return EXIT_SUCCESS;
}
/*!
 * \brief       oDrive::set_lockin_vel
 *
 * \details     Function sets velocity in rps, which stands for rotation per second
 *              of motor, lockin velocity is used to determine velocity when
 *              AXIS_STATE_LOCKIN_SPIN is used
 * \note        AXIS_STATE_LOCKIN_SPIN is not usually used
 *
 * \param       axis      int       Axis of motor, on which is velocity is set
 * \param       vel       float     Velocity in rps, when negative, motor spins in other direction
 *
 * \return      Function return status code
 *
 * \retval      EXIT_SUCCESS        Function executed succesfully
 * \retval      EXIT_FAILURE        An error occured, couldn't write command to port
 */

int oDrive::set_lockin_vel(int axis, float vel){
    std::string command = "w axis"+std::to_string(axis)+".config.general_lockin.vel "+std::to_string(vel);
    if (m_oDrive_port->write_port(command)<0) {
        syslog(LOG_ERR,ERROR_COMMAND_WRITE);
        return EXIT_FAILURE;
    }
    usleep(100);
    return EXIT_SUCCESS;
}
/*!
 * \brief       oDrive::clear_errors
 * \details     Functions remove errors arised on ODrive unit, f.e. when endstop is hit. This
 *              function allows removing this error and continue working with ODrive
 *
 * \param       axis      int       Axis is not used here, but needed to work, otherwise command will fail
 * \return      Function return status code
 *
 * \retval      EXIT_SUCCESS        Function executed succesfully
 * \retval      EXIT_FAILURE        An error occured, couldn't write command to port
 */
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
/*!
 * \brief       oDrive::get_axis_state
 * \details     Function return actual state of selected axis
 * \param       axis        int     Axis, from which is state returned
 *
 * \return      Axis state number, or error state
 *
 * \retval      axis_state  int     Actual state on axis
 * \retval      -EXIT_FAILURE int   An error occured, couldn't write command to port
 */
int oDrive::get_axis_state(int axis)
{
    std::string out;
    std::string command = "r axis"+std::to_string(axis)+".current_state";
    if (m_oDrive_port->write_port(command)<0) {
        syslog(LOG_ERR,ERROR_COMMAND_WRITE);
        return -EXIT_FAILURE;
    }
    usleep(100);
    out = m_oDrive_port->read_port();
    if (out == INVALID_PROPERTY ||out ==  INVALID_COMMAND_FORMAT ||out ==  UNKNOWN_COMMAND) {
        syslog(LOG_ERR,ERROR_COMMAND_READ);
        return -EXIT_FAILURE;
    }
    int state = std::stoi(out);
    return state;
}
/*!
 * \brief       oDrive::get_pos_est
 * \details     Function returns linear position estimate of the encoder, in turns. Also known
 *              as “multi-turn” position.
 * \param       axis        int     Axis, on which is motor connected and its position should be returned
 *
 * \return      Linear position estimate, or error state
 *
 * \retval      pos_estimate float  linear position estimate
 * \retval      EXIT_FAILURE        An error occured, couldn't write command to port
 */
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
/*!
 * \brief       oDrive::get_pos_est_cnt
 * \details     Function returns linear position estimate of the encoder, in counts.
 *              Equal to pos_estimate * cpr(set on encoder, default 8192)
 * \param       axis        int    Axis, on which is motor connected and its position should be returned
 *
 * \return      Linear position estimate counts, or error state
 *
 * \retval      pos_estimate_counts float Linear position estimate counts
 * \retval      EXIT_FAILURE        float An error occured, couldn't write command to port
 */
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
/*!
 * \brief       oDrive::get_pos_cir
 * \details     Function returns circular position estimate of the encoder, as a decimal
 *              from [0, 1). One turn of motor.
 * \param       axis        int    Axis, on which is motor connected and its position should be returned
 *
 * \return      Circular position estimate counts, or error state
 *
 * \retval      pos_circular float[0-1)    Circular position estimate.
 * \retval      EXIT_FAILURE float    An error occured, couldn't write command to port
 */
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
/*!
 * \brief       oDrive::get_pos_cpr_cnt
 *
 * \details     Function returns circular position estimate of the encoder, on the space [0, cpr).
 *              CPR stands for counts per revolution, by default it is set to 8192
 * \param       axis        int    Axis, on which is motor connected and its position should be returned
 *
 * \return      Circular position cpr counts, or error state
 *
 * \retval      pos_cpr_counts float Circular position cpr counts.
 * \retval      EXIT_FAILURE   float An error occured, couldn't write command to port
 */
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
/*!
 * \brief       oDrive::get_locking_vel
 * \details     Function return locking velocity in rps, which stands for rotation per second
 *              of motor, lockin velocity is used with AXIS_STATE_LOCKIN_SPIN state
 * \note        AXIS_STATE_LOCKIN_SPIN is not usually used
 *
 * \param       axis      int   Axis of motor, from which lockin velocity needs to be determined
 *
 * \return      Function return velocity, or error state
 *
 * \retval      vel         float Lockin velocity in rps.
 * \retval      EXIT_FAILURE float An error occured, couldn't write command to port
 */
float oDrive::get_locking_vel(int axis){
    std::string command = "r axis"+std::to_string(axis)+".config.general_lockin.vel";
    std::string out;
    std::string delimiter = " ";
    float velocity;
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
    velocity =stof(out.substr(0,out.find(delimiter)));
    return velocity;
}
/*!
 * \brief       oDrive::get_vel
 * \details     Function return locking velocity in rps, which stands for rotation per second
 *              of motor. This parameter determines velocity with INPUT_MODE_VEL_RAMP input mode and
 *              AXIS_STATE_CLOSED_LOOP_CONTROL
 *
 * \param       axis      int   Axis of motor, from which velocity needs to be determined
 *
 * \return      Function return velocity, or error state
 *
 * \retval      vel         float Velocity in rps, if negative velocity means opposite direction.
 * \retval      EXIT_FAILURE float An error occured, couldn't write command to port
 */
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
/*!
 * \brief       oDrive::get_pos_turns
 *
 * \details     Function returns circular position estimate of the encoder, on the space [0, cpr).
 *              CPR stands for counts per revolution, by default it is set to 8192
 * \param       axis        int    Axis, on which is motor connected and its position should be returned
 *
 * \return      Circular position cpr counts, or error state
 *
 * \retval      pos_cpr_counts float Circular position cpr counts.
 * \retval      EXIT_FAILURE   float An error occured, couldn't write command to port
 */
std::string oDrive::get_motor_feedback(int axis){
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
        return "EXIT_FAILURE";
    }
    return out;
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
}

