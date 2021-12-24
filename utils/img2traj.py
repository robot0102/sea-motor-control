import numpy as np
import cv2
import os
import glob
import numpy as np
import scipy.ndimage.morphology as m
import scipy.special
import time, math, copy
import bezier
import matplotlib.pyplot as plt
from .skeltonize import *
import random
import shutil

def skeletonize(img, num_points=10):

    im = (img>128).astype(np.uint8)
    im = thinning(im)
    
    rects = []
    polys = traceSkeleton(im,0,0,im.shape[1],im.shape[0],num_points,999,rects)
    
    img_canvas = np.full((128,128),255, np.uint8)

    for l in polys:
        c = (0,0,0)
        for i in range(0,len(l)-1):
            cv2.line(img_canvas,(l[i][0],l[i][1]),(l[i+1][0],l[i+1][1]),c,2)

    cv2.imshow('',img_canvas);cv2.waitKey(0)

    return polys, img_canvas

def _extract_points(img):
    
    return skeletonize(~img)

if __name__ == "__main__":

    test_type = '/Users/yucunjun/Git/RoboDraw/一/一_ink_pen_kai.ttf.png'
    width, height = 128, 128
    offset = 3.0
    show_animation = True
    image = np.ones((height, width)) * 255
    
    if test_type == 'line':
        x1, y1 = 20, 20
        x2, y2 = 200, 200
        line_thickness = 1
        cv2.line(image, (x1, y1), (x2, y2), (0, 0, 0), thickness=line_thickness)
    elif test_type == 'circle':
        center_coordinates = (100, 100)
        radius = 20
        color = [0,0,0]
        thickness = 1
        cv2.circle(image, center_coordinates, radius, color, thickness)
    elif os.path.isdir(test_type):
        pls_lst = []
        # for folder in glob.glob(test_type+'/*'):
        os.makedirs(test_type.replace('imgs_comp', 'imgs_ske'),exist_ok=True)
        for file_name in sorted(glob.glob(test_type+'/*.jpg')):
            prefix = file_name[:-4] 
            log_name = prefix + '.txt'
            out_file = open(log_name, 'w')
            image = cv2.imread(file_name, cv2.IMREAD_GRAYSCALE)
            points, images = _extract_points(image)
            inverse = ~images
            if inverse.sum() == 0:
                print(file_name + ' is empty')
                shutil.copy(file_name, file_name.replace('imgs_part', 'imgs_ske'))
            else:
                cv2.imwrite(file_name.replace('imgs_part', 'imgs_ske'), images)
                # np.savetxt(out_file,points[0])
                # out_file.close()
                # pls_lst.append(points)
    else:
        image = cv2.imread(test_type, cv2.IMREAD_GRAYSCALE)
        points, image = _extract_points(image)
        file_name = test_type
        file_name = file_name.replace('png', 'jpg')
        cv2.imwrite(file_name, image)
        



