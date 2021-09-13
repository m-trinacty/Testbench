import sys
import time
import odrive
from odrive.enums import *
import fibre
#from fibre.protocol import ChannelBrokenException
#import ChannelBrokenException


class D6374MotorConfig:
    """
    Class for configuring an Odrive axis for a D6374 motor.
    Only works with one Odrive at a time.
    """

    # Hoverboard Kv
    D6374_KV = 150.0

    # Min/Max phase inductance of motor
    MIN_PHASE_INDUCTANCE = 0
    MAX_PHASE_INDUCTANCE = 0.001

    # Min/Max phase resistance of motor
    MIN_PHASE_RESISTANCE = 0
    MAX_PHASE_RESISTANCE = 0.5

    # Tolerance for encoder offset float
    ENCODER_OFFSET_FLOAT_TOLERANCE = 1.0

    def __init__(self, axis_num):
        """
        Finds odrive, erase its
        configuration, and grabbing specified axis object.

        :param axis_num: Which channel/motor on the odrive your referring to.
        :type axis_num: int (0 or 1)
        """

        self.axis_num = axis_num

        # Connect to Odrive
        print("Looking for ODrive...")
        self._find_odrive()
        print("Found ODrive.")

    def _find_odrive(self):
        # connect to Odrive
        self.odrv = odrive.find_any()
        self.odrv_axis = getattr(self.odrv, "axis{}".format(self.axis_num))

    def configure(self):
        """
        Configures the odrive device for a  motor.
        """

        # Erase pre-exsisting configuration
        print("Erasing pre-exsisting configuration...")
        try:
            self.odrv.erase_configuration()
        except:#ChannelBrokenException:
            pass

        self._find_odrive()

        # Set brake resistor 50WR5J, resistance is 0.5 Ohms
        self.odrv.config.enable_brake_resistor = True
        self.odrv.config.brake_resistance = 0.55
        # D6374 has 7 pole pairs
        self.odrv_axis.motor.config.pole_pairs = 7
        self.odrv_axis.motor.config.calibration_current = 5

        self.odrv_axis.motor.config.motor_type = MOTOR_TYPE_HIGH_CURRENT
        self.odrv_axis.motor.config.resistance_calib_max_voltage = 2
        self.odrv_axis.motor.config.requested_current_range = 25
        self.odrv_axis.motor.config.current_control_bandwidth = 100

        # KV Odrive D6374 = 150
        self.odrv_axis.motor.config.torque_constant = 8.27 / self.D6374_KV
        # ENCODER CALIBRATION, should be one time matter
        # CUI AMT 102V is incremental encoder
        self.odrv_axis.encoder.config.mode = ENCODER_MODE_INCREMENTAL

        #Counts per revolution is 4x ppr, here using highest resolution, so
        #ppr is 2048
        self.odrv_axis.encoder.config.cpr = 8192

        # 50.26548385620117 left as default
        # offset calibration displacement to get better calibration accuracy.
        self.odrv_axis.encoder.config.calib_scan_distance = 50.26548385620117



        self.odrv_axis.encoder.config.use_index = True

        #Encoder bandwith left as default
        self.odrv_axis.encoder.config.bandwidth = 1000

        #These are tuning values
        self.odrv_axis.controller.config.pos_gain = 15
        self.odrv_axis.controller.config.vel_gain = 0.02 * self.odrv_axis.motor.config.torque_constant * self.odrv_axis.encoder.config.cpr
        self.odrv_axis.controller.config.vel_integrator_gain = 0.1 * self.odrv_axis.motor.config.torque_constant * self.odrv_axis.encoder.config.cpr
        self.odrv_axis.controller.config.vel_limit = 144




        print("Saving manual configuration and rebooting...")
        try:
            self.odrv.save_configuration()
            print("Manual configuration saved.")
        except fibre.libfibre.ObjectLostError:
            pass
        time.sleep(2)
        self._find_odrive()

        input("Make sure the motor is free to move, then press enter...")

        print("Calibrating Odrive for D6374 motor (you should hear a "
              "beep)...")

        self.odrv_axis.requested_state = AXIS_STATE_MOTOR_CALIBRATION

        # Wait for calibration to take place
        time.sleep(10)

        if self.odrv_axis.motor.error != 0:
            print("Error: Odrive reported an error of {} while in the state "
                  "AXIS_STATE_MOTOR_CALIBRATION. Printing out Odrive motor data for "
                  "debug:\n{}".format(self.odrv_axis.motor.error,
                                      self.odrv_axis.motor))

            sys.exit(1)

        if self.odrv_axis.motor.config.phase_inductance <= self.MIN_PHASE_INDUCTANCE or \
                self.odrv_axis.motor.config.phase_inductance >= self.MAX_PHASE_INDUCTANCE:
            print("Error: After odrive motor calibration, the phase inductance "
                  "is at {}, which is outside of the expected range. Either widen the "
                  "boundaries of MIN_PHASE_INDUCTANCE and MAX_PHASE_INDUCTANCE (which "
                  "is between {} and {} respectively) or debug/fix your setup. Printing "
                  "out Odrive motor data for debug:\n{}".format(self.odrv_axis.motor.config.phase_inductance,
                                                                self.MIN_PHASE_INDUCTANCE,
                                                                self.MAX_PHASE_INDUCTANCE,
                                                                self.odrv_axis.motor))

            sys.exit(1)

        if self.odrv_axis.motor.config.phase_resistance <= self.MIN_PHASE_RESISTANCE or \
                self.odrv_axis.motor.config.phase_resistance >= self.MAX_PHASE_RESISTANCE:
            print("Error: After odrive motor calibration, the phase resistance "
                  "is at {}, which is outside of the expected range. Either raise the "
                  "MAX_PHASE_RESISTANCE (which is between {} and {} respectively) or "
                  "debug/fix your setup. Printing out Odrive motor data for "
                  "debug:\n{}".format(self.odrv_axis.motor.config.phase_resistance,
                                      self.MIN_PHASE_RESISTANCE,
                                      self.MAX_PHASE_RESISTANCE,
                                      self.odrv_axis.motor))

            sys.exit(1)

        # If all looks good, then lets tell ODrive that saving this calibration
        # to persistent memory is OK
        self.odrv_axis.motor.config.pre_calibrated = True
        print("Finding index for encoder...")
        self.odrv_axis.requested_state = AXIS_STATE_ENCODER_INDEX_SEARCH
        #Wait for odrive to find index
        time.sleep(5)


        print("Calibrating Odrive for encoder...")
        self.odrv_axis.requested_state = AXIS_STATE_ENCODER_OFFSET_CALIBRATION

        # Wait for calibration to take place
        time.sleep(10)

        if self.odrv_axis.encoder.error != 0 and self.odrv_axis.error != 0 \
                and (self.odrv_axis.encoder.config.direction == 1 or
                     self.odrv_axis.encoder.config.direction == -1):
            print("Error: Odrive reported an error of {} while in the state "
                  "AXIS_STATE_ENCODER_OFFSET_CALIBRATION. Printing out Odrive encoder "
                  "data for debug:\n{}".format(self.odrv_axis.encoder.error,
                                               self.odrv_axis.error,
                                               self.odrv_axis.encoder))

            sys.exit(1)

        # Let inform oDrive in every startup from now that encoder is calibrated
        self.odrv_axis.encoder.config.pre_calibrated = True

        # Let Odrive find index in every startup
        self.odrv_axis.config.startup_encoder_index_search = True

        # If offset_float isn't 0.5 within some tolerance, or its not 1.5 within
        # some tolerance, raise an error

        if not ((self.odrv_axis.encoder.config.phase_offset_float > 0.5 - self.ENCODER_OFFSET_FLOAT_TOLERANCE and
                 self.odrv_axis.encoder.config.phase_offset_float < 0.5 + self.ENCODER_OFFSET_FLOAT_TOLERANCE) or
                (self.odrv_axis.encoder.config.phase_offset_float > 1.5 - self.ENCODER_OFFSET_FLOAT_TOLERANCE and
                 self.odrv_axis.encoder.config.phase_offset_float < 1.5 + self.ENCODER_OFFSET_FLOAT_TOLERANCE)):
            print("Error: After odrive encoder calibration, the 'offset_float' "
                  "is at {}, which is outside of the expected range. 'offset_float' "
                  "should be close to 0.5 or 1.5 with a tolerance of {}. Either "
                  "increase the tolerance or debug/fix your setup. Printing out "
                  "Odrive encoder data for debug:\n{}".format(self.odrv_axis.encoder.config.phase_offset_float,
                                                              self.ENCODER_OFFSET_FLOAT_TOLERANCE,
                                                              self.odrv_axis.encoder))

            sys.exit(1)

        # If all looks good, then lets tell ODrive that saving this calibration
        # to persistent memory is OK
        self.odrv_axis.encoder.config.pre_calibrated = True

        # ANTICOGGING calibration
        # Put the controller in position control mode
        self.odrv_axis.controller.config.control_mode = CONTROL_MODE_POSITION_CONTROL
        self.odrv_axis.controller.config.input_mode = INPUT_MODE_PASSTHROUGH
        self.odrv_axis.requested_state = AXIS_STATE_CLOSED_LOOP_CONTROL

        # Temporarily increase the gain
        self.odrv_axis.controller.config.pos_gain = 200.0
        self.odrv_axis.controller.config.vel_integrator_gain = 0.0150
        self.odrv_axis.controller.config.vel_gain = 0.0005
        # THIS IS THE MAGIC - Start the calibration

        print("Starting anticogging calibration")
        self.odrv_axis.controller.start_anticogging_calibration()
        time.sleep(2)
        while self.odrv_axis.controller.config.anticogging.calib_anticogging:
            print("Anticogging calibration: " + str(self.odrv_axis.controller.config.anticogging.calib_anticogging))
            time.sleep(2)

        self.odrv_axis.controller.config.anticogging.pre_calibrated = True
        # Put updated values for anticogging calibration back to normal
        self.odrv_axis.controller.config.pos_gain = 15
        self.odrv_axis.controller.config.vel_gain = 0.02 * self.odrv_axis.motor.config.torque_constant * self.odrv_axis.encoder.config.cpr
        self.odrv_axis.controller.config.vel_integrator_gain = 0.1 * self.odrv_axis.motor.config.torque_constant * self.odrv_axis.encoder.config.cpr

        # Set control to velocity mode and input to ramp, then set ramp to 5turns/s
        self.odrv_axis.controller.config.control_mode = CONTROL_MODE_VELOCITY_CONTROL
        self.odrv_axis.controller.config.input_mode = INPUT_MODE_VEL_RAMP
        self.odrv_axis.controller.config.vel_ramp_rate = 5

        print("Saving calibration configuration and rebooting...")
        # self.odrv.save_configuration()
        # print("Calibration configuration saved.")
        try:
            self.odrv.save_configuration()
            print("Calibration configuration saved.")
        except fibre.libfibre.ObjectLostError:
            pass
        time.sleep(5)
        self._find_odrive()
        print("Setting values for Anticogging")

        try:
            print("Reboot Odrive.")
            self.odrv.reboot()
        except fibre.libfibre.ObjectLostError:
            pass
        time.sleep(4)
        self._find_odrive()
        print("Odrive configuration finished.")

    def mode_idle(self):
        """
        Puts the motor in idle (i.e. can move freely).
        """

        self.odrv_axis.requested_state = AXIS_STATE_IDLE

    def mode_close_loop_control(self):
        """
        Puts the motor in closed loop control.
        """

        self.odrv_axis.requested_state = AXIS_STATE_CLOSED_LOOP_CONTROL

    def move_input_pos(self, angle):
        """
        Puts the motor at a certain angle.

        :param angle: Angle you want the motor to move.
        :type angle: int or float
        """

        self.odrv_axis.controller.input_pos = angle / 360.0

    def move_input_vel(self, vel):
        """
        Puts the motor at a certain angle.

        :param vel: Velocity you want the motor to spin in turns/s.
        :type vel: int or float
        """

        self.odrv_axis.controller.input_vel = vel

if __name__ == "__main__":
    d6374_motor_config = D6374MotorConfig(axis_num=0)
    d6374_motor_config.configure()

    print("CONDUCTING MOTOR TEST")
    print("Placing motor in close loop control. If you move motor, motor will "
          "resist you.")
    d6374_motor_config.mode_close_loop_control()

    # Go from 0 to 360 degrees in increments of 30 degrees
    #for angle in range(0, 390, 30):
    #    print("Setting motor to {} degrees.".format(angle))
    #    hb_motor_config.move_input_pos(angle)
    #    time.sleep(5)
    #Spin motor for 10 seconds in speed of 2 turns per second, then other way same speed
    vel = 0.05
    print("Startin rotation in speed "+str(vel)+" turns per second for 5 seconds")
    d6374_motor_config.move_input_vel(vel)
    time.sleep(5)

    print("Startin rotation in speed "+str(vel)+" turns per second for 5 seconds in reverse")
    d6374_motor_config.move_input_vel(-vel)
    time.sleep(5)
    print("Placing motor in idle. If you move motor, motor will "
          "move freely")
    d6374_motor_config.mode_idle()