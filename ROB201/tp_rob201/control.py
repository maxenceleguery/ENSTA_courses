""" A set of robotics control functions """
import numpy as np
import sys

def getLeftIndex(tab,value):
    return min(range(len(tab)), key=lambda i: abs(tab[i]-value))


def reactive_obst_avoid(lidar):
    """
    Simple obstacle avoidance
    lidar : placebot object with lidar data
    """
    # TODO for TP1
    command = {"forward": 0.3,
               "rotation": 1}

    v1, v2 = lidar.get_sensor_values(), lidar.get_ray_angles()

    index_left = getLeftIndex(v2, np.pi/2)
    if v1[index_left] > 50:
        command["forward"], command["rotation"] = 0.2, 0.2
    elif v1[index_left] > 15:
        command["rotation"] = 0.1
    else:
        command["rotation"] = -0.1

    index_min = np.argmin(v1)
    angle_closer, distance_closer = v2[index_min], v1[index_min]

    if abs(angle_closer) < np.pi / 12:
        command["forward"], command["rotation"] = 0.05, -1
    elif abs(angle_closer) < np.pi / 2.5 and distance_closer < 40:
        command["forward"], command["rotation"] = 0.2, -0.5 if angle_closer > 0 else 0.5

    return command


def potential_field_control(lidar, pose, goal):
    """
    Control using potential field for goal reaching and obstacle avoidance
    lidar : placebot object with lidar data
    pose : [x, y, theta] nparray, current pose in odom or world frame
    goal : [x, y, theta] nparray, target pose in odom or world frame
    """
    # TODO for TP2

    command = {"forward": 0.05,
               "rotation": 0}

    K = 1
    gradient = (K/np.sqrt( (pose[0] - goal[0])**2 + (pose[1] - goal[1])**2 )) * (goal[:2] - pose[:2])
    dist = np.sqrt( (pose[0] - goal[0])**2 + (pose[1] - goal[1])**2 )
    print(dist)
    
    v1, v2 = lidar.get_sensor_values(), lidar.get_ray_angles()
    index_min = np.argmin(v1)
    angle_closer, distance_closer = v2[index_min], v1[index_min]
    xObs, yObs = distance_closer*np.cos(angle_closer), distance_closer*np.sin(angle_closer)
    Kobs=1
    gradientObstacle=(Kobs/distance_closer**3) * (1/distance_closer - 1/50) * (np.array([xObs,yObs]) - pose[:2])

    gradient+=10000*gradientObstacle


    if dist > 0.5:
        command["forward"] = min(0.01*np.log(dist+1),1) 
    else:
        command["forward"] = 0
        print("PARKED")
        return command

    if (pose[2] - np.arctan2(gradient[1],gradient[0]) > 0.2):
        command["rotation"] = -0.1
    elif (pose[2] - np.arctan2(gradient[1],gradient[0]) < -0.2):
        command["rotation"] = 0.1
    else:
        print("ALIGNED")

    #print(gradient, pose, goal)


    return command
