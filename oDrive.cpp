/*
 * Copyright (C) Aero4TE, s.r.o. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include "oDrive.h"

#include <string>
#include <memory>

#include <unistd.h>

#define	ERROR_COMMAND_WRITE   "ERROR could not write command"
#define	ERROR_COMMAND_READ   "ERROR could not read command"
#define INVALID_PROPERTY    "invalid property"
#define INVALID_COMMAND_FORMAT  "invalid command format"
#define UNKNOWN_COMMAND "unknown command"
#define PRINT_READ_VALUES   false


int oDrive::setMinEndstop(int axis, bool enabled)
{
    std::string enable=enabled?"1":"0";
    std::string command = "w axis"+ std::to_string(axis)+ ".min_endstop.config.enabled "+enable;
    if (m_oDrivePort->writeToPort(command)<0) {
        std::cout<<ERROR_COMMAND_WRITE<<std::endl;
        return EXIT_FAILURE;
    }
    usleep(100);
    return EXIT_SUCCESS;
}

bool oDrive::getMinEndstop(int axis)
{
    std::string out;
    std::string command = "r axis"+std::to_string(axis)+".min_endstop.endstop_state";
    if (m_oDrivePort->writeToPort(command)<0) {
        std::cout<<ERROR_COMMAND_WRITE<<std::endl;
        return EXIT_FAILURE;
    }
    usleep(100);
    out = m_oDrivePort->readFromPort();
    if (out == INVALID_PROPERTY ||out ==  INVALID_COMMAND_FORMAT ||out ==  UNKNOWN_COMMAND) {
        std::cout<<out<<std::endl;
        std::cout<<ERROR_COMMAND_READ<<std::endl;
        return EXIT_FAILURE;
    }
#if PRINT_READ_VALUES
    std::cout << out << std::endl;
#endif
    bool homed = std::stoi(out)==1?true:false;
    return homed;
}

float oDrive::getCurrent(int axis)
{
    std::string out;
    std::string command = "r axis"+std::to_string(axis)+".ibus";
    if (m_oDrivePort->writeToPort(command)<0) {
        std::cout<<ERROR_COMMAND_WRITE<<std::endl;
        return EXIT_FAILURE;
    }
    usleep(100);
    out = m_oDrivePort->readFromPort();
    if (out == INVALID_PROPERTY ||out ==  INVALID_COMMAND_FORMAT ||out ==  UNKNOWN_COMMAND) {
        std::cout<<ERROR_COMMAND_READ<<std::endl;
        return EXIT_FAILURE;
    }
#if PRINT_READ_VALUES

    std::cout<< out << std::endl;
#endif
    float current =std::stof(out);
    return current;
}

float oDrive::getIqMeasured(int axis)
{
    std::string out;
    std::string command = "r axis"+std::to_string(axis)+".motor.current_control.Iq_measured";
    if (m_oDrivePort->writeToPort(command)<0) {
        std::cout<<ERROR_COMMAND_WRITE<<std::endl;
        return EXIT_FAILURE;
    }
    usleep(100);
    out = m_oDrivePort->readFromPort();
    if (out == INVALID_PROPERTY ||out ==  INVALID_COMMAND_FORMAT ||out ==  UNKNOWN_COMMAND) {
        std::cout<<ERROR_COMMAND_READ<<std::endl;
        return EXIT_FAILURE;
    }
#if PRINT_READ_VALUES

    std::cout<< out << std::endl;
#endif
    float current =std::stof(out);
    return current;
}

oDrive::oDrive() {
    // TODO Auto-generated constructor stub

}

oDrive::oDrive(std::string portName){
    m_oDrivePort = new port(portName);
}
int oDrive::commandConsole(){
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
            m_oDrivePort->writeToPort(input);
            usleep(100);
            out = m_oDrivePort->readFromPort();
            std::cout << out << std::endl;
            break;
        case 'w':
            m_oDrivePort->writeToPort(input);
            usleep(100);
            break;
        case 'f':
            m_oDrivePort->writeToPort(input);
            usleep(100);
            out = m_oDrivePort->readFromPort();
            std::cout << out << std::endl;
            break;
        default:
            m_oDrivePort->writeToPort(input);
            usleep(100);
            break;
        }
    }
    return EXIT_SUCCESS;
}
int oDrive::setAxisState(int axis,int state){
    std::string command = "w axis"+ std::to_string(axis)+ ".requested_state "+std::to_string(state);
    if (m_oDrivePort->writeToPort(command)<0) {
        std::cout<<ERROR_COMMAND_WRITE<<std::endl;
        return EXIT_FAILURE;
    }
    usleep(100);
    return EXIT_SUCCESS;
}

int oDrive::setInputMode(int axis, int mode){
    std::string command = "w axis"+std::to_string(axis)+ ".controller.config.input_mode "+std::to_string(mode);
    if (m_oDrivePort->writeToPort(command)<0) {
        std::cout<<ERROR_COMMAND_WRITE<<std::endl;
        return EXIT_FAILURE;
    }
    usleep(100);
    return EXIT_SUCCESS;
}

int oDrive::setVelocity(int axis, float vel)
{
    std::string command= "w axis"+std::to_string(axis)+".controller.input_vel " +std::to_string(vel);
    m_oDrivePort->writeToPort(command);
    usleep(100);
    return EXIT_SUCCESS;
}
int oDrive::setLockinVelocity(int axis, float vel){
    std::string command = "w axis"+std::to_string(axis)+".config.general_lockin.vel "+std::to_string(vel);
    m_oDrivePort->writeToPort(command);
    usleep(100);
    return EXIT_SUCCESS;
}

int oDrive::clearErrors(int axis)
{
    std::string command = "w axis"+std::to_string(axis)+".error 0";
    if (m_oDrivePort->writeToPort(command)<0) {
        std::cout<<ERROR_COMMAND_WRITE<<std::endl;
        return EXIT_FAILURE;
    }
    usleep(100);
    return EXIT_SUCCESS;
}

int oDrive::getAxisState(int axis)
{
    std::string out;
    std::string command = "r axis"+std::to_string(axis)+".current_state";
    if (m_oDrivePort->writeToPort(command)<0) {
        std::cout<<ERROR_COMMAND_WRITE<<std::endl;
        return EXIT_FAILURE;
    }
    usleep(100);
    out = m_oDrivePort->readFromPort();
    if (out == INVALID_PROPERTY ||out ==  INVALID_COMMAND_FORMAT ||out ==  UNKNOWN_COMMAND) {
        std::cout<<ERROR_COMMAND_READ<<std::endl;
        return EXIT_FAILURE;
    }
#if PRINT_READ_VALUES

    std::cout<< out << std::endl;
#endif
    int state = std::stoi(out);
    return state;
}

float oDrive::getPosEstimate(int axis)
{
    std::string out;
    std::string command = "r axis"+std::to_string(axis)+".encoder.pos_estimate";
    if (m_oDrivePort->writeToPort(command)<0) {
        std::cout<<ERROR_COMMAND_WRITE<<std::endl;
        return EXIT_FAILURE;
    }
    usleep(100);
    out = m_oDrivePort->readFromPort();
    if (out == INVALID_PROPERTY ||out ==  INVALID_COMMAND_FORMAT ||out ==  UNKNOWN_COMMAND) {
        std::cout<<ERROR_COMMAND_READ<<std::endl;
        return EXIT_FAILURE;
    }
#if PRINT_READ_VALUES

    std::cout<< out << std::endl;
#endif
    float pos =std::stof(out);
    return pos;
}

float oDrive::getPosEstimateCounts(int axis)
{
    std::string out;
    std::string command = "r axis"+std::to_string(axis)+".encoder.pos_estimate_counts";
    if (m_oDrivePort->writeToPort(command)<0) {
        std::cout<<ERROR_COMMAND_WRITE<<std::endl;
        return EXIT_FAILURE;
    }
    usleep(100);
    out = m_oDrivePort->readFromPort();
    if (out == INVALID_PROPERTY ||out ==  INVALID_COMMAND_FORMAT ||out ==  UNKNOWN_COMMAND) {
        std::cout<<out<<std::endl;
        std::cout<<ERROR_COMMAND_READ<<std::endl;
        return EXIT_FAILURE;
    }
#if PRINT_READ_VALUES
    std::cout << out << std::endl;
#endif
    float pos =std::stof(out);
    return pos;
}

float oDrive::getPosCircular(int axis)
{
    std::string out;
    std::string command = "r axis"+std::to_string(axis)+".encoder.pos_circular";
    if (m_oDrivePort->writeToPort(command)<0) {
        std::cout<<ERROR_COMMAND_WRITE<<std::endl;
        return EXIT_FAILURE;
    }
    usleep(100);
    out = m_oDrivePort->readFromPort();
    if (out == INVALID_PROPERTY ||out ==  INVALID_COMMAND_FORMAT || out == UNKNOWN_COMMAND) {
        std::cout<<ERROR_COMMAND_READ<<std::endl;
        return EXIT_FAILURE;
    }
#if PRINT_READ_VALUES
    std::cout << out << std::endl;
#endif
    float pos =std::stof(out);
    return pos;
}

float oDrive::getPosCprCounts(int axis)
{
    std::string out;
    std::string command = "r axis"+std::to_string(axis)+".encoder.pos_circular";
    if (m_oDrivePort->writeToPort(command)<0) {
        std::cout<<ERROR_COMMAND_WRITE<<std::endl;
        return EXIT_FAILURE;
    }
    usleep(100);
    out = m_oDrivePort->readFromPort();
    if (out == INVALID_PROPERTY ||out ==  INVALID_COMMAND_FORMAT || out == UNKNOWN_COMMAND) {
        std::cout<<ERROR_COMMAND_READ<<std::endl;
        return EXIT_FAILURE;
    }
#if PRINT_READ_VALUES
    std::cout << out << std::endl;
#endif
    float pos =std::stof(out);
    return pos;
}
int oDrive::getLockinVelocity(int axis){
    std::string command = "r axis"+std::to_string(axis)+".config.general_lockin.vel";
    std::string out;
    if (m_oDrivePort->writeToPort(command)<0) {
        std::cout<<ERROR_COMMAND_WRITE<<std::endl;
        return EXIT_FAILURE;
    }
    usleep(100);
    out = m_oDrivePort->readFromPort();
    if (out == INVALID_PROPERTY || out ==  INVALID_COMMAND_FORMAT || out ==  UNKNOWN_COMMAND) {
        std::cout<<ERROR_COMMAND_READ<<std::endl;
        return EXIT_FAILURE;
    }
#if PRINT_READ_VALUES
    std::cout << out << std::endl;
#endif
    return EXIT_SUCCESS;
}

float oDrive::getVelocity(int axis)
{
    std::string command = "r axis"+std::to_string(axis)+".encoder.vel_estimate";
    std::string out;
    std::string delimiter = " ";
    float velocity;
    if(m_oDrivePort->writeToPort(command)<0){
        std::cout<<ERROR_COMMAND_WRITE<<std::endl;
        return 0;
    }
    usleep(100);
    out = m_oDrivePort->readFromPort();
    if (out == INVALID_PROPERTY || out == INVALID_COMMAND_FORMAT || out == UNKNOWN_COMMAND) {
        std::cout<<ERROR_COMMAND_READ<<std::endl;
        return EXIT_FAILURE;
    }
#if PRINT_READ_VALUES
    std::cout << out << std::endl;
#endif
    velocity =stof(out.substr(0,out.find(delimiter)));
    return velocity;
}

float oDrive::getPosInTurns(int axis){
    std::string command = "f "+std::to_string(axis);
    std::string out;
    std::string delimiter = " ";
    float pos;
    if(m_oDrivePort->writeToPort(command)<0){
        std::cout<<ERROR_COMMAND_WRITE<<std::endl;
        return 0;
    }
    usleep(100);
    out = m_oDrivePort->readFromPort();
    if (out == INVALID_PROPERTY || out == INVALID_COMMAND_FORMAT || out == UNKNOWN_COMMAND) {
        std::cout<<ERROR_COMMAND_READ<<std::endl;
        return EXIT_FAILURE;
    }
#if PRINT_READ_VALUES
    std::cout << out << std::endl;
#endif
    pos =stof(out.substr(0,out.find(delimiter)));
    return pos;
}
int oDrive::setPosInTurns(int axis,float pos){
    std::string command = "q "+std::to_string(axis)+" "+std::to_string(pos);
    std::string out;
    std::string delimiter = " ";
    if(m_oDrivePort->writeToPort(command)<0){
        std::cout<<ERROR_COMMAND_WRITE<<std::endl;
        return 0;
    }
    usleep(100);
    return EXIT_SUCCESS;
}

int oDrive::setPos(int axis, float pos)
{
    std::string command = "w axis"+std::to_string(axis)+ ".controller.input_vel "+std::to_string(pos);
    if (m_oDrivePort->writeToPort(command)<0) {
        std::cout<<ERROR_COMMAND_WRITE<<std::endl;
        return EXIT_FAILURE;
    }
    usleep(100);
    return EXIT_SUCCESS;
}

void oDrive::setHoming(int axis)
{
    std::string showMessageStart= "Searching home";
    std::cout<<"Current_state: "<<getAxisState(0)<<std::endl;
    std::string showMessageEnd= "Odrive homed";
    setMinEndstop(axis,true);
    setAxisState(axis,AXIS_STATE_HOMING);
    std::cout << showMessageStart<< std::endl;
    bool searching = false;
    while(!searching){
        searching =getMinEndstop(axis);
        usleep(100);
    }
    if(getMinEndstop(axis)){
        std::cout << showMessageEnd<< std::endl;
    }
}
oDrive::~oDrive() {
    delete m_oDrivePort;
    // TODO Auto-generated destructor stub
}

