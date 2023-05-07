"""
Robot controller definition
Complete controller including SLAM, planning, path following
"""
import numpy as np
import time

from place_bot.entities.robot_abstract import RobotAbstract
from place_bot.entities.odometer import OdometerParams
from place_bot.entities.lidar import LidarParams

from tiny_slam import TinySlam

from control import reactive_obst_avoid, potential_field_control


# Definition of our robot controller
class MyRobotSlam(RobotAbstract):
    """A robot controller including SLAM, path planning and path following"""

    def __init__(self,
                 lidar_params: LidarParams = LidarParams(),
                 odometer_params: OdometerParams = OdometerParams()):
        # Passing parameter to parent class
        super().__init__(should_display_lidar=False,
                         lidar_params=lidar_params,
                         odometer_params=odometer_params)

        # step counter to deal with init and display
        self.counter = 0

        # Init SLAM object
        self._size_area = (800, 800)
        self.tiny_slam = TinySlam(x_min=- self._size_area[0],
                                  x_max=self._size_area[0],
                                  y_min=- self._size_area[1],
                                  y_max=self._size_area[1],
                                  resolution=2)

        # storage for pose after localization
        self.corrected_pose = np.array([0, 0, 0])

    def control(self):
        """
        Main control function executed at each time step
        """
        self.counter += 1
        c=0
        score=0
        modulo=3 

        start=time.time()

        if self.counter in [1,2]: #Map's initialization
            self.tiny_slam.update_map(self.lidar(),self.odometer_values())

        if self.counter%modulo==0: #Localization and mapping during 1 out of modulo iterations
            score=self.tiny_slam.localise(self.lidar(),self.odometer_values())
            self.tiny_slam.update_map(self.lidar(),self.odometer_values())
            print(f"Score : {score:0.2f}")
            print(f"\t\t\t{modulo/(time.time()-start):.2f} FPS\r", end='')

        #Debug only
        #self.tiny_slam.update_map(self.lidar(),[self.true_position().x,self.true_position().y,self.true_angle()])            

        # Follow walls first and then go back to the origine
        if self.tiny_slam.counter < 2000000000:
            command = reactive_obst_avoid(self.lidar())
        else:
            x,y = self.tiny_slam._conv_map_to_world(self.tiny_slam.path[len(self.tiny_slam.path)//6][0],self.tiny_slam.path[len(self.tiny_slam.path)//6][1])
            goal = [x,y,0]
            command = potential_field_control(self.lidar(),self.odometer_values(),goal)

        return command
