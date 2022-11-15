# from https://stackoverflow.com/questions/19780411/pygame-drawing-a-rectangle

# to draw rectangle
import math
import sys

import pygame
from pygame.locals import *


class Point:

    def __init__(self, x, y):
        self.x = x
        self.y = y

    def __str__(self):
        return f'({self.x}, {self.y})'

# Finds if 2 given line segments intersect or not
# From: https://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/

# Given three collinear points p, q, r, the function checks if
# point q lies on line segment 'pr'
def onSegment(p, q, r):
	if ( (q.x <= max(p.x, r.x)) and (q.x >= min(p.x, r.x)) and
		(q.y <= max(p.y, r.y)) and (q.y >= min(p.y, r.y))):
		return True
	return False

def orientation(p, q, r):
	# to find the orientation of an ordered triplet (p,q,r)
	# function returns the following values:
	# 0 : Collinear points
	# 1 : Clockwise points
	# 2 : Counterclockwise
	
	# See https://www.geeksforgeeks.org/orientation-3-ordered-points/amp/
	# for details of below formula.
	
	val = (float(q.y - p.y) * (r.x - q.x)) - (float(q.x - p.x) * (r.y - q.y))
	if (val > 0):
		
		# Clockwise orientation
		return 1
	elif (val < 0):
		
		# Counterclockwise orientation
		return 2
	else:
		
		# Collinear orientation
		return 0

# The main function that returns true if
# the line segment 'p1q1' and 'p2q2' intersect.
def doIntersect(p1,q1,p2,q2):
	
	# Find the 4 orientations required for
	# the general and special cases
	o1 = orientation(p1, q1, p2)
	o2 = orientation(p1, q1, q2)
	o3 = orientation(p2, q2, p1)
	o4 = orientation(p2, q2, q1)

	# General case
	if ((o1 != o2) and (o3 != o4)):
		return True

	# Special Cases

	# p1 , q1 and p2 are collinear and p2 lies on segment p1q1
	if ((o1 == 0) and onSegment(p1, p2, q1)):
		return True

	# p1 , q1 and q2 are collinear and q2 lies on segment p1q1
	if ((o2 == 0) and onSegment(p1, q2, q1)):
		return True

	# p2 , q2 and p1 are collinear and p1 lies on segment p2q2
	if ((o3 == 0) and onSegment(p2, p1, q2)):
		return True

	# p2 , q2 and q1 are collinear and q1 lies on segment p2q2
	if ((o4 == 0) and onSegment(p2, q1, q2)):
		return True

	# If none of the cases
	return False

def dot(vA, vB):
    return vA[0]*vB[0]+vA[1]*vB[1]

def getAngle(p1,p2,p3,p4):
    # Get nicer vector form
    lineA = ((p1.x,p1.y),(p2.x,p2.y))
    lineB = ((p3.x,p3.y),(p4.x,p4.y))
    vA = [(lineA[0][0]-lineA[1][0]), (lineA[0][1]-lineA[1][1])]
    vB = [(lineB[0][0]-lineB[1][0]), (lineB[0][1]-lineB[1][1])]
    # Get dot prod
    dot_prod = dot(vA, vB)
    # Get magnitudes
    magA = dot(vA, vA)**0.5
    magB = dot(vB, vB)**0.5
    # Get cosine value
    cos_ = dot_prod/magA/magB
    # Get angle in radians and then convert to degrees
    angle = math.acos(dot_prod/magB/magA)
    # Basically doing angle <- angle mod 360
    ang_deg = math.degrees(angle)%360
    return ang_deg

def calcCenterPoint(new_point, rad, coords):
    # https://stackoverflow.com/questions/51223685/create-circle-tangent-to-two-lines-with-radius-r-geometry
    
    p1 = coords[len(coords) - 2]
    p2 = coords[len(coords) - 1]
    p3 = new_point

    le1 = math.sqrt((p2.x-p1.x)**2 + (p2.y-p1.y)**2) # length of A1-B1 segment
    v1x = (p2.x-p1.x) / le1
    v1y = (p2.y-p1.y) / le1

    le2 = math.sqrt((p3.x-p2.x)**2 + (p3.y-p2.y)**2) # length of A1-B1 segment
    v2x = (p3.x-p2.x) / le2
    v2y = (p3.y-p2.y) / le2

    R = rad
    px1 = p1.x - v1y*R
    py1 = p1.y + v1x*R
    px2 = p2.x - v2y*R
    py2 = p2.y + v2x*R

    px1u = p1.x + v1y*R
    py1u = p1.y - v1x*R
    px2u = p2.x + v2y*R
    py2u = p2.y - v2x*R
    
    den = v1x*v2y - v2x*v1y

    k1 = (v2y*(px2-px1) - v2x*(py2-py1)) / den
    # k2 = (v1y*(px2-px1) - v1x*(py2-py1)) / den

    k1u = (v2y*(px2u-px1u) - v2x*(py2u-py1u)) / den
    # k2u = (v1y*(px2u-px1u) - v1x*(py2u-py1u)) / den
    
    tx1 = p1.x + k1*v1x
    ty1 = p1.y + k1*v1y
    # tx2 = p2.x + k2*v2x
    # ty2 = p2.y + k2*v2x

    if(onSegment(p1,Point(tx1,ty1),p2)):
        cx =  px1 + k1*v1x
        cy =  py1 + k1*v1y
    else:
        cx =  px1u + k1u*v1x
        cy =  py1u + k1u*v1y
    
        

    return Point(cx,cy)

def main():
    pygame.init()

    DISPLAY = pygame.display.set_mode((700,500),0,32)

    WHITE = (255,255,255)
    BLUE = (0,0,255)
    prev_point = Point(0,0)
    line_count = -1
    ANGLE_TOLERANCE = 20
    RADIUS_IN_PIXELS = 50
    coords = [] # stores coordinates as point values

    DISPLAY.fill(WHITE)

    while True:

        for event in pygame.event.get():
            if (event.type == pygame.KEYDOWN and event.key == pygame.K_ESCAPE) or event.type == QUIT:
                # before program ends
                file = open('drive_coords.txt', 'w')
                try:
                    # write to file
                    file.write(str(len(coords)) + "\n")
                    for i in range(len(coords)):
                        file.write(str(coords[i].x) + " " + str(coords[i].y))
                        if(i != len(coords)-1):
                            file.write("\n")
                        
                except:
                    print("Unable to open file")
                
                file.close()


                for i in range(len(coords)):
                    print("(" + str(coords[i].x) + ", " + str(coords[i].y) + ")")
                    
                pygame.quit()
                sys.exit()

            if event.type == pygame.MOUSEBUTTONDOWN:
                x,y = pygame.mouse.get_pos()
                new_point = Point(x,y)
                # check for double click
                if(new_point.x == prev_point.x and new_point.y == prev_point.y):
                    continue
                legal_line = True

                # new line
                p1 = Point(prev_point.x, prev_point.y)
                q1 = Point(new_point.x, new_point.y)
                
                
                if line_count > 1:
                    # check if new line lintersects with any other line
                    for i in range(line_count-1):
                        # temp line
                        p2 = coords[i]
                        q2 = coords[i+1]
                        if(doIntersect(p1, q1, p2, q2)):
                            legal_line = False

                    # check if angle between old and new line is more than 15 degrees
                    # finds angle in relation to x axis (for some reason)
                    angle = getAngle(new_point, prev_point, coords[line_count-1], prev_point)
                    print(angle)
                    if angle < ANGLE_TOLERANCE:
                        legal_line = False
                    
                if legal_line:
                    if line_count != -1:
                        pygame.draw.aaline(DISPLAY, BLUE, (prev_point.x, prev_point.y), (new_point.x, new_point.y))

                        # TODO draw circle
                        if line_count >= 1:
                            
                            centCoord = calcCenterPoint(new_point, RADIUS_IN_PIXELS, coords)
                            rect = Rect(centCoord.x-RADIUS_IN_PIXELS, centCoord.y-RADIUS_IN_PIXELS, RADIUS_IN_PIXELS*2, RADIUS_IN_PIXELS*2)
                            pygame.draw.arc(DISPLAY,BLUE,rect,0,360, 1)

                        pygame.display.flip()
                    coords.append(Point(new_point.x, new_point.y))
                    prev_point = Point(new_point.x, new_point.y)
                    line_count += 1
               
        pygame.display.flip()

main()

# TODO
#   angle cant be too sharp
#   radi - http://www.pygame.org/docs/ref/draw.html#pygame.draw.arc
#   somehow set scale
# 
# Resources:
# http://www.pygame.org/docs/ref/draw.html#pygame.draw.line
# https://www.geeksforgeeks.org/with-statement-in-python/
# https://www.pythontutorial.net/python-basics/python-write-text-file/
# https://www.w3schools.com/python/ref_list_extend.asp
# https://stackoverflow.com/questions/19780411/pygame-drawing-a-rectangle
# https://stackoverflow.com/questions/3838329/how-can-i-check-if-two-segments-intersect
# https://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/
# https://stackoverflow.com/questions/28260962/calculating-angles-between-line-segments-python-with-math-atan2