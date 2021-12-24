import cv2
import argparse
import numpy as np
from glob import glob
import random


def rotate_image(image, angle):
  image_center = tuple(np.array(image.shape[1::-1]) / 2)
  rot_mat = cv2.getRotationMatrix2D(image_center, angle, 1.0)
  result = cv2.warpAffine(image, rot_mat, image.shape[1::-1], flags=cv2.INTER_LINEAR, borderValue=(255,255,255))
  return result

def generate(dir_name):
    
    random.seed(20)
    num_sample = 5
    # canvas = np.full((128,128),255, np.uint8)
    # cv2.line(canvas, (20,64), (80,64), (0,0,0), 4)

    for img_name in glob(dir_name+'/*.jpg'):
        for i in range(num_sample):

            img = cv2.imread(img_name)
            angle = random.randint(0, 30)
            img = rotate_image(img, angle)
            img_new_name = img_name.replace('.jpg', '_' + str(i) + '.jpg')
            img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
            cv2.imwrite(img_new_name, img)


if __name__ == '__main__':


    dir_name = './dummy'
    # parser = argparse.ArgumentParser()
    # parser.add_argument('--save_path', type=string, default='./one_stroke', help='size of the batches')
    # parser.add_argument('--batchSize', type=int, default=1, help='size of the batches')
    # parser.add_argument('--batchSize', type=int, default=1, help='size of the batches')
    # parser.add_argument('--batchSize', type=int, default=1, help='size of the batches')
    
    # args = parser.parse_args()
    generate(dir_name)

    