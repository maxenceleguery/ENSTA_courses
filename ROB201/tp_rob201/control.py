""" A set of robotics control functions """
import numpy as np
import sys

def getIndex(tab,value):
    """
    Get the index of the closest value in the table
    """
    return min(range(len(tab)), key=lambda i: abs(tab[i]-value))


def reactive_obst_avoid(lidar):
    """
    Simple obstacle avoidance
    lidar : placebot object with lidar data
    """
    # TODO for TP1
    command = {"forward": 0.1,
               "rotation": 0}

    v1, v2 = lidar.get_sensor_values(), lidar.get_ray_angles()

    index_min = np.argmin(v1)
    left = min([r if abs(angle-np.pi/2)<np.pi/6 else sys.float_info.max for r,angle in zip(v1,v2)]) # Minimal distance in the left cone
    front = min([r if abs(angle-0)<np.pi/10 else sys.float_info.max for r,angle in zip(v1,v2)]) # Minimal distance in the front cone
    angle_closer, distance_closer = v2[index_min], v1[index_min]

    rotation=0
    if abs(angle_closer) < np.pi/4:
        rotation = -angle_closer/(np.pi/4)

    if distance_closer<75:

        distance_asserv = 50
        err=5
        #print(f"Left :{left:0.2f}, Front :{front:0.2f}")
        # The robot must remain at a predefined distance from the wall
        if left > distance_asserv+err:
            command["rotation"] = rotation
        elif left < distance_asserv-err:
            command["rotation"] = rotation
        else:
            command["rotation"] = 0

        # If a wall is reached, turn left if possible else right
        if front < distance_asserv+5:
            command["forward"] = 0.05
            if left < distance_asserv+2*err:
                command["rotation"] = -0.1
            else:
                command["rotation"] = 0.1

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
    # Attractive gradient (goal to reach)
    gradient = (K/np.sqrt( (pose[0] - goal[0])**2 + (pose[1] - goal[1])**2 )) * (goal[:2] - pose[:2])
    dist = np.sqrt( (pose[0] - goal[0])**2 + (pose[1] - goal[1])**2 )
    print(dist)
    
    v1, v2 = lidar.get_sensor_values(), lidar.get_ray_angles()
    index_min = np.argmin(v1)
    angle_closer, distance_closer = v2[index_min], v1[index_min]
    xObs, yObs = distance_closer*np.cos(angle_closer), distance_closer*np.sin(angle_closer)
    Kobs=1
    # Repulsive gradient (obstacles to avoid)
    gradientObstacle=(Kobs/distance_closer**3) * (1/distance_closer - 1/50) * (np.array([xObs,yObs]) - pose[:2])

    gradient+=30000*gradientObstacle


    if dist > 0.5:
        # Slowdown until the goal is reached
        command["forward"] = min(0.05*np.log(dist+1),1) 
    else:
        command["forward"] = 0
        print("PARKED")
        return command

    # Turn until aligned with the goal
    if (pose[2] - np.arctan2(gradient[1],gradient[0]) > 0.2):
        command["rotation"] = -0.15
    elif (pose[2] - np.arctan2(gradient[1],gradient[0]) < -0.2):
        command["rotation"] = 0.15
    else:
        print("ALIGNED")

    return command
