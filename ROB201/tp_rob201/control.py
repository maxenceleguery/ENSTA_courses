""" A set of robotics control functions """
import numpy as np
import sys

def getRightIndex(tab,value):
    min = sys.float_info.max
    index=0
    for i in range(len(tab)):
        if (min > abs(tab[i]-value)):
            min = abs(tab[i]-value)
            index=i
    return index

def reactive_obst_avoid(lidar):
    """
    Simple obstacle avoidance
    lidar : placebot object with lidar data
    """
    # TODO for TP1
    command = {"forward": 0.5,
               "rotation": 1}

    v1 = lidar.get_sensor_values()
    v2 = lidar.get_ray_angles()

    index_min = np.argmin(v1)
    angle_closer = v2[index_min]
    distance_closer = v1[index_min]

    index_left = getRightIndex(v2,np.pi/2)
    print(v1[index_left],v2[index_left])
    if v1[index_left] > 15:
        command["rotation"] = 0.1
    if v1[index_left] > 50:
        command["forward"] = 0.3
        command["rotation"] = 0.4
    if v1[index_left] < 15:
        command["rotation"] = -0.1

    if ( -np.pi/12 < angle_closer < np.pi/12 ):
        command["forward"] = 0.1
        command["rotation"] = -1
    elif (0 < angle_closer < np.pi/2.5 and distance_closer < 40):
        command["forward"] = 0.2
        command["rotation"] = -0.5
    elif (0 > angle_closer > -np.pi/2.5 and distance_closer < 40):
        command["forward"] = 0.2
        command["rotation"] = 0.5

    return command

def potential_field_control(lidar, pose, goal):
    """
    Control using potential field for goal reaching and obstacle avoidance
    lidar : placebot object with lidar data
    pose : [x, y, theta] nparray, current pose in odom or world frame
    goal : [x, y, theta] nparray, target pose in odom or world frame
    """
   # TODO for TP2

    command = {"forward": 0,
               "rotation": 0}

    return command
