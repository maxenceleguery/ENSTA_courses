""" A simple robotics navigation code including SLAM, exploration, planning"""

import pickle

import cv2
import numpy as np
from matplotlib import pyplot as plt
import sys

import heapq


class TinySlam:
    """Simple occupancy grid SLAM"""

    def __init__(self, x_min, x_max, y_min, y_max, resolution):
        # Given : constructor
        self.x_min_world = x_min
        self.x_max_world = x_max
        self.y_min_world = y_min
        self.y_max_world = y_max
        self.resolution = resolution

        self.counter = 0
        self.path = []

        self.x_max_map, self.y_max_map = self._conv_world_to_map(
            self.x_max_world, self.y_max_world)

        self.occupancy_map = np.zeros(
            (int(self.x_max_map), int(self.y_max_map)))

        # Origin of the odom frame in the map frame
        self.odom_pose_ref = np.array([0, 0, 0],dtype=float)

    def _conv_world_to_map(self, x_world, y_world):
        """
        Convert from world coordinates to map coordinates (i.e. cell index in the grid map)
        x_world, y_world : list of x and y coordinates in m
        """
        x_map = (x_world - self.x_min_world) / self.resolution
        y_map = (y_world - self.y_min_world) / self.resolution

        if isinstance(x_map, float):
            x_map = int(x_map)
            y_map = int(y_map)
        elif isinstance(x_map, np.ndarray):
            x_map = x_map.astype(int)
            y_map = y_map.astype(int)

        return x_map, y_map

    def _conv_map_to_world(self, x_map, y_map):
        """
        Convert from map coordinates to world coordinates
        x_map, y_map : list of x and y coordinates in cell numbers (~pixels)
        """
        x_world = self.x_min_world + x_map *  self.resolution
        y_world = self.y_min_world + y_map *  self.resolution

        if isinstance(x_world, np.ndarray):
            x_world = x_world.astype(float)
            y_world = y_world.astype(float)

        return x_world, y_world

    def add_map_line(self, x_0, y_0, x_1, y_1, val):
        """
        Add a value to a line of points using Bresenham algorithm, input in world coordinates
        x_0, y_0 : starting point coordinates in m
        x_1, y_1 : end point coordinates in m
        val : value to add to each cell of the line
        """

        # convert to pixels
        x_start, y_start = self._conv_world_to_map(x_0, y_0)
        x_end, y_end = self._conv_world_to_map(x_1, y_1)

        if x_start < 0 or x_start >= self.x_max_map or y_start < 0 or y_start >= self.y_max_map:
            return

        if x_end < 0 or x_end >= self.x_max_map or y_end < 0 or y_end >= self.y_max_map:
            return

        # Bresenham line drawing
        dx = x_end - x_start
        dy = y_end - y_start
        is_steep = abs(dy) > abs(dx)  # determine how steep the line is
        if is_steep:  # rotate line
            x_start, y_start = y_start, x_start
            x_end, y_end = y_end, x_end
        # swap start and end points if necessary and store swap state
        if x_start > x_end:
            x_start, x_end = x_end, x_start
            y_start, y_end = y_end, y_start
        dx = x_end - x_start  # recalculate differentials
        dy = y_end - y_start  # recalculate differentials
        error = int(dx / 2.0)  # calculate error
        y_step = 1 if y_start < y_end else -1
        # iterate over bounding box generating points between start and end
        y = y_start
        points = []
        for x in range(x_start, x_end + 1):
            coord = [y, x] if is_steep else [x, y]
            points.append(coord)
            error -= abs(dy)
            if error < 0:
                y += y_step
                error += dx
        points = np.array(points).T

        # add value to the points
        self.occupancy_map[points[0], points[1]] += val

    def add_map_points(self, points_x, points_y, val):
        """
        Add a value to an array of points, input coordinates in meters
        points_x, points_y :  list of x and y coordinates in m
        val :  value to add to the cells of the points
        """
        x_px, y_px = self._conv_world_to_map(points_x, points_y)

        select = np.logical_and(np.logical_and(x_px >= 0, x_px < self.x_max_map),
                                np.logical_and(y_px >= 0, y_px < self.y_max_map))
        x_px = x_px[select]
        y_px = y_px[select]

        self.occupancy_map[x_px, y_px] += val


    def score(self, lidar, pose):
        """
        Computes the sum of log probabilities of laser end points in the map
        lidar : placebot object with lidar data
        pose : [x, y, theta] nparray, position of the robot to evaluate, in world coordinates
        """
        # TODO for TP4
        v1, v2 = lidar.get_sensor_values(), lidar.get_ray_angles()
        mask=[True if i%5==0 else False for i in range(len(v1))] #Taking only 1 out of 5 values for speedup
        v1,v2 = v1[mask],v2[mask]
        v1,v2 = v1[v1 < 0.95*lidar.max_range],v2[v1 < 0.95*lidar.max_range]
        xObs, yObs = pose[0]+v1*np.cos(v2+pose[2]), pose[1]+v1*np.sin(v2+pose[2])
        xObs, yObs = self._conv_world_to_map(xObs,yObs)
        #print(xObs,yObs)
        select = np.logical_and(np.logical_and(xObs >= 0, xObs < self.x_max_map),
                                np.logical_and(yObs >= 0, yObs < self.y_max_map))
        
        xObs=xObs[select]
        yObs=yObs[select]
        
        if len(xObs)>0:
            score = 1000*sum(self.occupancy_map[xObs,yObs])/len(xObs)
        else:
            score=0

        return score

    def get_corrected_pose(self, odom, odom_pose_ref=None):
        """
        Compute corrected pose in map frame from raw odom pose + odom frame pose,
        either given as second param or using the ref from the object
        odom : raw odometry position
        odom_pose_ref : optional, origin of the odom frame if given,
                        use self.odom_pose_ref if not given
        """
        # TODO for TP4
        if (odom_pose_ref==None).any():
            odom_pose_ref=self.odom_pose_ref

        #print(odom)

        corrected_pose = np.array([0,0,0],dtype=float)
        corrected_pose[0] = odom_pose_ref[0] + np.sqrt(odom[:2].dot(odom[:2]))*np.cos(odom[2]+odom_pose_ref[2])
        corrected_pose[1] = odom_pose_ref[1] + np.sqrt(odom[:2].dot(odom[:2]))*np.sin(odom[2]+odom_pose_ref[2])
        corrected_pose[2] = odom[2]+odom_pose_ref[2]

        return corrected_pose

    def localise(self, lidar, odom):
        """
        Compute the robot position wrt the map, and updates the odometry reference
        lidar : placebot object with lidar data
        odom : [x, y, theta] nparray, raw odometry position
        """
        # TODO for TP4
        sigma=np.array([8,8,10]) #Standard deviation of position and angle
        N=2000 #Maximum number of iteration
        N2=30 #Minimal number of improvement
        c=0
        c2=0
        threshold = 800 #Maximal theoretical score : 8000

        #Increasing threshold with iteration to maximize accuracy
        if self.counter > 10:
            threshold = 100*self.counter
        if self.counter > 70:
            threshold = 7000
        threshold*=1

        
        best_score = -sys.float_info.max
        tmp_odom_ref = None
        best_odom_ref = self.odom_pose_ref
        while (best_score<threshold and c2<N2) and c<N:
            #print(c,c2)
            tmp_odom_ref = np.random.normal(best_odom_ref, sigma)
            score = self.score(lidar,self.get_corrected_pose(odom,tmp_odom_ref))
            if score>best_score:
                best_score=score
                best_odom_ref=tmp_odom_ref
                c2+=1
            c+=1

        self.odom_pose_ref = best_odom_ref
        return best_score
                
        if best_score>threshold:
            self.odom_pose_ref = best_odom_ref
            return best_score
        else:
            return None
        """
        corrected_pose_vectorized = np.vectorize(self.get_corrected_pose,excluded=['odom'],signature='(n),(m)->(n)')

        for i in range(1):
            tmp_odom_ref = self.odom_pose_ref
            tmp_odom_refs = np.random.normal(tmp_odom_ref, [sigma]*N, (N,3))

            corrected_poses = corrected_pose_vectorized(odom,tmp_odom_refs)

            scores = []
            for pose in corrected_poses:
                scores.append(self.score(lidar,pose))

            best_index = np.argmax(scores)
            best_score = scores[best_index]
            best_odom_ref = tmp_odom_refs[best_index]
            self.odom_pose_ref = best_odom_ref

        if best_score>threshold:
            self.odom_pose_ref = best_odom_ref
            return best_score
        else:
            return None
        """

    def update_map(self, lidar, pose):
        """
        Bayesian map update with new observation
        lidar : placebot object with lidar data
        pose : [x, y, theta] nparray, corrected pose in world coordinates
        """
        # TODO for TP3
        pOccupation=0.95
        v1,v2 = lidar.get_sensor_values(), lidar.get_ray_angles()
        mask=[True if i%1==0 else False for i in range(len(v1))] #Taking only 1 out of 2 values for speedup
        v1,v2 = v1[mask],v2[mask]
        v1,v2 = v1[v1 < 0.95*lidar.max_range],v2[v1 < 0.95*lidar.max_range]
        if min(v1) > 30:
            v1_prime = v1[:]-25
        else:
            v1_prime = v1[:]
        v2_prime,v1_prime = v2[v1_prime > 0],v1_prime[v1_prime > 0]
        xObs, yObs = pose[0]+v1*np.cos(v2+pose[2]), pose[1]+v1*np.sin(v2+pose[2])
        xL, yL = pose[0]+v1_prime*np.cos(v2_prime+pose[2]), pose[1]+v1_prime*np.sin(v2_prime+pose[2])


        vectorized_add_map_line = np.vectorize(self.add_map_line,excluded=['x_0','y_0','val'])
        vectorized_add_map_line(x_0=pose[0],y_0=pose[1],x_1=xL,y_1=yL,val=np.log((1-pOccupation)/pOccupation))

        #"""
        vect_dir=np.array([xObs-pose[0],yObs-pose[1]])
        #print(vect_dir)
        vect_dir[0,:]=vect_dir[0,:]/np.sqrt(vect_dir[0,:]**2+vect_dir[1,:]**2)
        vect_dir[1,:]=vect_dir[1,:]/np.sqrt(vect_dir[0,:]**2+vect_dir[1,:]**2)

        vect_dir/=0.4
        #"""

        self.add_map_points(np.array([xObs]),np.array([yObs]),np.log(pOccupation/(1-pOccupation)))
        self.add_map_points(np.array([xObs-vect_dir[0,:],xObs+vect_dir[0,:]]),np.array([yObs-vect_dir[1,:],yObs+vect_dir[1,:]]),np.log(pOccupation/(1-pOccupation)))
        #self.add_map_points(np.array([xObs-2*vect_dir[0,:],xObs+2*vect_dir[0,:]]),np.array([yObs-2*vect_dir[1,:],yObs+2*vect_dir[1,:]]),np.log(pOccupation/(1-pOccupation)))

        self.occupancy_map[self.occupancy_map > 8],self.occupancy_map[self.occupancy_map < -8]= 8,-8

        if self.counter==0 or (self.counter > 190 and self.counter%100==0):
            self.path = self.plan(pose,[0,0,0])
        
        self.display(pose,self.path)
        #self.display2(pose)

        self.counter += 1

    def get_neighbors(self, current):
        """
        current : [x, y] nparray, corrected pose in map coordinates
        """
        offset=1
        current=np.array(current)
        neighbors = [tuple(current + np.array([i,j])) for i in [-offset,0,offset] for j in [-offset,0,offset]]
        neighbors.pop(4)
        toRemove=[]
        for i,n in enumerate(neighbors):
            if n[0] < 0 or n[0] >= self.x_max_map or n[1] < 0 or n[1] >= self.y_max_map or self.occupancy_map[n[0],n[1]] >= -2:
                toRemove.append(i)
        mask=[False if (i in toRemove) else True for i in range(len(neighbors))]
        neighbors=np.array(neighbors)[mask].tolist()
        for i,n in enumerate(neighbors):
            neighbors[i]=tuple(n)
        return neighbors

    def heuristic(self, a, b):
        """
        a,b : [x, y] nparray, corrected pose in map coordinates
        """
        a=np.array(a)
        b=np.array(b)
        return np.sqrt(np.dot(b-a,b-a)) 

    def reconstruct_path(self,cameFrom,current):
        total_path = [current]
        while current in cameFrom.keys():
            current = cameFrom[current]
            total_path.append(current)
        return total_path[::-1]       

    def plan(self, start, goal):
        """
        Compute a path using A*, recompute plan if start or goal change
        start : [x, y, theta] nparray, start pose in world coordinates
        goal : [x, y, theta] nparray, goal pose in world coordinates
        """
        # TODO for TP5
        print("Planning trajectory")
        h=[]
        cameFrom={}

        start_x_map, start_y_map = self._conv_world_to_map(start[0],start[1])
        goal_x_map, goal_y_map = self._conv_world_to_map(goal[0],goal[1]) 
        start_map = (start_x_map,start_y_map)
        goal_map = (goal_x_map,goal_y_map)

        gScore={start_map:0}
        fScore={start_map:self.heuristic(start_map,goal_map)}

        heapq.heappush(h,(self.heuristic(start_map,goal_map),start_map))

        while len(h) > 0:
            #print(len(h))
            current = heapq.heappop(h)[1]
            if current==goal_map:
                return self.reconstruct_path(cameFrom,current)
            for n in self.get_neighbors(current):
                tentative_gScore = gScore[current] + self.heuristic(current,n)
                if tentative_gScore < gScore.get(n,np.inf):
                    cameFrom[n]=current
                    gScore[n]=tentative_gScore
                    fScore[n]=tentative_gScore+self.heuristic(n,goal_map)
                    h_array=np.array(h,dtype=object)
                    if len(h_array)==0 or (not (n in h_array[:,1])):
                        heapq.heappush(h,(fScore[n],n))

    def display(self, robot_pose,path):
        """
        Screen display of map and robot pose, using matplotlib
        robot_pose : [x, y, theta] nparray, corrected robot pose
        """

        plt.cla()
        plt.imshow(self.occupancy_map.T, origin='lower',
                   extent=[self.x_min_world, self.x_max_world, self.y_min_world, self.y_max_world])
        plt.clim(-4, 4)
        plt.axis("equal")

        delta_x = np.cos(robot_pose[2]) * 10
        delta_y = np.sin(robot_pose[2]) * 10
        plt.arrow(robot_pose[0], robot_pose[1], delta_x, delta_y,
                  color='red', head_width=5, head_length=10, )
        
        path=np.array(path)
        path[:,0],path[:,1] = self._conv_map_to_world(path[:,0],path[:,1])
        plt.plot(path[:,0], path[:,1],color='black')

        # plt.show()
        plt.pause(0.001)

    def display2(self, robot_pose):
        """
        Screen display of map and robot pose,
        using opencv (faster than the matplotlib version)
        robot_pose : [x, y, theta] nparray, corrected robot pose
        """

        img = cv2.flip(self.occupancy_map.T, 0)
        img = img - img.min()
        img = img / img.max() * 255
        img = np.uint8(img)
        img2 = cv2.applyColorMap(src=img, colormap=cv2.COLORMAP_JET)

        pt2_x = robot_pose[0] + np.cos(robot_pose[2]) * 20
        pt2_y = robot_pose[1] + np.sin(robot_pose[2]) * 20
        pt2_x, pt2_y = self._conv_world_to_map(pt2_x, -pt2_y)

        pt1_x, pt1_y = self._conv_world_to_map(robot_pose[0], -robot_pose[1])

        # print("robot_pose", robot_pose)
        pt1 = (int(pt1_x), int(pt1_y))
        pt2 = (int(pt2_x), int(pt2_y))
        cv2.arrowedLine(img=img2, pt1=pt1, pt2=pt2,
                        color=(0, 0, 255), thickness=2)
        cv2.imshow("map slam", img2)
        cv2.waitKey(1)

    def save(self, filename):
        """
        Save map as image and pickle object
        filename : base name (without extension) of file on disk
        """

        plt.imshow(self.occupancy_map.T, origin='lower',
                   extent=[self.x_min_world, self.x_max_world,
                           self.y_min_world, self.y_max_world])
        plt.clim(-4, 4)
        plt.axis("equal")
        plt.savefig(filename + '.png')

        with open(filename + ".p", "wb") as fid:
            pickle.dump({'occupancy_map': self.occupancy_map,
                         'resolution': self.resolution,
                         'x_min_world': self.x_min_world,
                         'x_max_world': self.x_max_world,
                         'y_min_world': self.y_min_world,
                         'y_max_world': self.y_max_world}, fid)

    def load(self, filename):
        """
        Load map from pickle object
        filename : base name (without extension) of file on disk
        """
        # TODO

    def compute(self):
        """ Useless function, just for the exercise on using the profiler """
        # Remove after TP1
        return

        ranges = np.random.rand(3600)
        ray_angles = np.arange(-np.pi,np.pi,np.pi/1800)

        # Poor implementation of polar to cartesian conversion
        points = []
        for i in range(3600):
            pt_x = ranges[i] * np.cos(ray_angles[i])
            pt_y = ranges[i] * np.sin(ray_angles[i])
            points.append([pt_x,pt_y])
