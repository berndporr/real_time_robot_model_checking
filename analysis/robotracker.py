# Based on the openCV demo "optical flow":
# https://github.com/opencv/opencv/blob/3.4/samples/python/tutorial_code/video/optical_flow/optical_flow.py

import numpy as np
import cv2 as cv
import argparse
import csv

minY = 180
minX = 350
width = 600
height = 775

def doCrop(img):
    return img[minY:minY+height, minX:minX+width]

parser = argparse.ArgumentParser(description='Lucas-Kanade Optical Flow calculation.')
parser.add_argument('image', type=str, help='path to image file without .mp4')
args = parser.parse_args()

cap = cv.VideoCapture(args.image+".mp4")

# params for ShiTomasi corner detection
feature_params = dict( maxCorners = 100,
                       qualityLevel = 0.5,
                       minDistance = 70,
                       blockSize = 7 )

# Parameters for lucas kanade optical flow
lk_params = dict( winSize  = (15, 15),
                  maxLevel = 2,
                  criteria = (cv.TERM_CRITERIA_EPS | cv.TERM_CRITERIA_COUNT, 10, 0.03))

# Create some random colors
color = np.random.randint(0, 255, (100, 3))

# Take first frame and find corners in it
ret, old_frame = cap.read()
old_frame = doCrop(old_frame)
cv.imwrite(args.image+"_1stframe.png",old_frame)

old_gray = cv.cvtColor(old_frame, cv.COLOR_BGR2GRAY)
p0 = cv.goodFeaturesToTrack(old_gray, mask = None, **feature_params)

# Create a mask image for drawing purposes
mask = np.zeros_like(old_frame)

f = open(args.image+"_coord.txt","w")

framenumber = 0

while(1):
    ret, frame = cap.read()
    if not ret:
        break

    frame = doCrop(frame)
    frame_gray = cv.cvtColor(frame, cv.COLOR_BGR2GRAY)

    # calculate optical flow
    p1, st, err = cv.calcOpticalFlowPyrLK(old_gray, frame_gray, p0, None, **lk_params)

    # Select good points
    if p1 is not None:
        good_new = p1[st==1]
        good_old = p0[st==1]

    f.write(str(framenumber))
    framenumber = framenumber + int(1)

    for i, (new, old) in enumerate(zip(good_new, good_old)):
        a, b = new.ravel()
        f.write("\t{}\t{}".format(a,b))
        c, d = old.ravel()
        mask = cv.line(mask, (int(a), int(b)), (int(c), int(d)), color[i].tolist(), 2)
        frame = cv.circle(frame, (int(a), int(b)), 5, color[i].tolist(), -1)
    img = cv.add(frame, mask)

    f.write("\n")

    cv.imshow('frame', img)
    k = cv.waitKey(30) & 0xff
    if k == 27:
        break

    # Now update the previous frame and previous points
    old_gray = frame_gray.copy()
    p0 = good_new.reshape(-1, 1, 2)

cv.destroyAllWindows()

f.close()

cv.imwrite(args.image+"_trace.png",mask)
