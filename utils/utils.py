import glob
import os
from tqdm import tqdm
import cv2
from PIL import Image
import numpy as np
import copy
import sys

def _prepare_data(out_path, img_list):

    index = 0
    for img in img_list:
        out_img = os.path.join(out_path, '{:04d}.jpg'.format(index))
        os.system('cp ' + img + ' ' + out_img)
        # im = Image.open(out_img)
        # rd_img = cv2.imread(img)
        # img = cv2.cvtColor(rd_img, cv2.COLOR_BGR2BGRA)
        # #wt_img = cv2.resize(rd_img, (128,128))
        # cv2.imwrite(out_img, img)
        index += 1

    return 

def _add_image(img_to_add):

    kk_list = []

    for img_name in img_to_add:

        image = cv2.imread(img_name)
        image = ~image
        kk_list.append(image)
    
    kk_list = np.array(kk_list).max(0)
    return ~kk_list

def _add_image_step(img_to_add):

    kk_list = []

    for index, img_name in enumerate(img_to_add):

        image = cv2.imread(img_name)
        # image = ~image
        kk_list.append(copy.deepcopy(image))
        kk_list2 = np.array(kk_list).min(0)
        cv2.imwrite('{}.jpg'.format(index), kk_list2)

    return ~kk_list2


def combine_part_to_full(length_of_img, img_list):

    length_arr = np.array(length_of_img)
    length_arr = np.cumsum(length_arr)
    length_arr = np.insert(length_arr, 0, 0)

    print(length_arr)
    img_list = sorted(glob.glob('./B/*.png'))   

    output_images = []
    for index, i in enumerate(length_arr[:-1]):
        img_to_add = img_list[length_arr[index]:length_arr[index+1]]
        if index == 54:
            image = _add_image_step(img_to_add)
        else:
            image = _add_image(img_to_add)
        output_images.append(image)
    
    # for index, sg_img in enumerate(output_images):
    #     cv2.imwrite('{}.jpg'.format(index), sg_img)


def main(method):

    img_path = './imgs/imgs_part'
    out_path = './A'

    os.makedirs(out_path, exist_ok = True)
    
    img_num = 324

    img_list = []
    length_of_img = []
    for img_folder in sorted(glob.glob(img_path+'/*')):
        length_of_img.append(len(glob.glob(img_folder+'/*.jpg')))
        for img in sorted(glob.glob(img_folder+'/*.jpg')):
            img_list.append(img)

    if method == 'comb':
        combine_part_to_full(length_of_img,img_list)
    else:
        _prepare_data(out_path, img_list)

if __name__ == '__main__':

    main(sys.argv[1])
