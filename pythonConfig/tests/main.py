import sys
import time
import odrive
from odrive.enums import *
import fibre
# insert at 1, 0 is the script path (or '' in REPL)
sys.path.insert(1, '../../pythonConfig/odriveConfig')
from oDrive import D6374MotorOdrive
def testFunc():
    d6374_motor = D6374MotorOdrive(ax   is_num=0)
    d6374_motor._find
if __name__ == '__main__':
    testFunc()