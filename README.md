# README #

This is app for controlling GNSS Tevogs Tesbench. This app runs as daemon on tevogs computer, has open port on 1500. For this purpose is created simple GUI to connect and control testbench.
Service is set to start at system startup and restart in every failure, which may be caused by emergency stop.
positions are logged into /var/log/tevogs/testbench.log

Parameters for app are set in Rounds per second of motor, for example 20 rps will result in about 40km/h of angular speed at the end of testbench, where reciever devices are placed.
Second parameter is timelength of spin, whic is set in seconds. 

WARNING testbench will turn into starting position to where is inductive sensor placed at startup and before every spinsession start, dont stand in the way of arms of testbench. Use emergency stop to get out of way. Then release the emergency stop WARNING

There is another version of this app which can be run from terminal.

![alt text](https://github.com/m-trinacty/Testbench/blob/master/testbench.jpg?raw=true)

### What is this repository for? ###

* Quick summary
* Set up
* Usage

### Prerequisities ###

* Odrive board 24V 3.6 
	-Firmware version : 0.5.2
* Configured Odrive, if it is not configured yet
	* Configuration file is in folder pythonconfig
	* odrivetool is required
	* connection directly to oDrive via USB
* SDK tdx-wayland 5.3.0 build from testbench layers

### Setup ###
* Build
* Deploy to toradex 
### Setup service ###
* Place oDriveApp-daemon.service in /etc/systemd/system
* Run "systemctl daemon-reload command"
* Run "systemctl start oDriveApp-daemon.service"
* Run "systemctl enable oDriveApp-daemon.service"

### Usage ###
* Power up testbench
* Connect to testbench via wifi 
* Start GUI app, whic automatically connects to port, it may sometimes take a while for port to be able to connect
* Spin testbench
