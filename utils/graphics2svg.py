import os.path as osp
import cv2
import os
from cairosvg import svg2png
from tqdm import tqdm
from svgpathtools import parse_path, disvg, wsvg

def list_to_str(list):

    rt_str = ''

    for ele in list:
        rt_str += ele
        rt_str += '\n'

    return rt_str

def _svg_2_img(out_path, path_list):
    """[summary]

    Args:
        svg ([type]): [description]
    """

    transform = [r'<g transform="scale(1, -1) translate(0, -900)">', r'</g>']

    for index in range(len(path_list)):

        img_name = osp.join(out_path, str(index)) + '.jpg'
        svg_name = osp.join(out_path, str(index)) + '.svg'
        paths = path_list[index]
        
        path_str = wsvg(paths,filename=svg_name,dimensions=(1024,1024))#,viewbox='0 0 1024 1024')
        path_str_list = path_str.split('\n')
        path_str_list.insert(2, transform[0])
        path_str_list.insert(-2, transform[1])
        path_str = list_to_str(path_str_list)
        svg2png(bytestring=path_str,write_to=img_name,background_color='white',output_width=128, output_height=128)

def _parse_strokes(strokes):
    """[summary]

    Args:
        strokes ([type]): [description]
    """

    path_list = []

    for stroke in strokes:

        path = parse_path(stroke)
        path_list.append(path)

    return path_list

def main():
    """[summary]
    """

    input_file = './src/graphics.txt'
    output_dir = './imgs/imgs_part'

    input_lines = open(input_file,'r').readlines()

    for line in tqdm(input_lines[:100]):

        char_info = eval(line)

        strokes = char_info['strokes']
        medians = char_info['medians']
        char = char_info['character']

        out_path = osp.join(output_dir, char)

        os.makedirs(out_path, exist_ok = True)

        svg = _parse_strokes(strokes)
        _svg_2_img(out_path, svg)


if __name__ == '__main__':
    main()