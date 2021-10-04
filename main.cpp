/*
 * Copyright (C) Aero4TE, s.r.o. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */
#include <iostream>
#include <string>
#include <memory>
#include <chrono>

#include <sstream>
#include <iomanip>
#include <thread>
#include <fstream>

#include "oDrive.h"
#include "pps.h"

int main() {

    const std::string portName = "/dev/ttyACM0";
    std::unique_ptr<oDrive> odrive(new oDrive(portName));
    odrive->setAxisState(0,odrive->AXIS_STATE_CLOSED_LOOP_CONTROL);

    auto now=std::chrono::system_clock::now();
    auto nowTime =  std::chrono::system_clock::to_time_t(now);
    auto start = std::chrono::steady_clock::now();
    std::stringstream ss;
    std::stringstream fileName;
    fileName << std::put_time(std::localtime(&nowTime),"%Y-%m-%d_%X");
    std::ofstream spinLog("spinLog_"+fileName.str()+".log");

    ss<<std::put_time(std::localtime(&nowTime),"%Y-%m-%d %X");
    spinLog<<"TIME,POS_CIRCULAR,POS_ESTIMATE,POS_ESTIMATE_COUNTS;"<<std::endl;
    std::cout<<"sys_clock::now() = "<< ss.str()<<std::endl;
    std::cout<<"Setting velocity to 1"<<std::endl;
    float velocity = 5.0;
    odrive->setVelocity(0,velocity);
    spinLog<<"Velocity= "<<std::to_string(velocity)<<std::endl;
    auto timer = std::chrono::seconds(1);
    while(1)
    {

        if(std::chrono::steady_clock::now() - (start+timer) > std::chrono::seconds(1))
        {
            timer = timer+std::chrono::seconds(1);
            now=std::chrono::system_clock::now();
            nowTime =  std::chrono::system_clock::to_time_t(now);
            std::stringstream ss;
            ss<<std::put_time(std::localtime(&nowTime),"%Y-%m-%d_%X");
            std::cout <<"Time: "<< ss.str()<<std::endl;

            std::cout<<"________________________________________________"<<std::endl<<std::endl;
            std::cout<<"Pos Circular"<<std::endl;
            float posCircular=odrive->getPosCircular(0);
            std::cout<<posCircular<<std::endl;
            std::cout<<"______________________________"<<std::endl<<std::endl;

            std::cout<<"Pos Estimate"<<std::endl;
            float posEstimate=odrive->getPosEstimate(0);
            std::cout<<posEstimate<<std::endl;
            std::cout<<"______________________________"<<std::endl<<std::endl;

            std::cout<<"Pos Estimate Counts"<<std::endl;
            float posEstimateCounts =odrive->getPosEstimateCounts(0);
            std::cout<<posEstimateCounts<<std::endl;
            std::cout<<"______________________________"<<std::endl<<std::endl;
            spinLog<<ss.str()+", "+std::to_string(posCircular)+", "+std::to_string(posEstimate)+", "+std::to_string(posEstimateCounts)+";"<<std::endl;
        }
        if(std::chrono::steady_clock::now()- start> std::chrono::seconds(60))
        {
            break;
        }
    }
    odrive->setAxisState(0,odrive->AXIS_STATE_IDLE);

    //odrive->commandConsole();
    return 0;
};
