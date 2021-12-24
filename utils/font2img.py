from PIL import Image, ImageFont, ImageDraw, ImageOps
from fontTools.ttLib import TTFont
from fontTools.unicode import Unicode
import os.path as osp
import numpy as np
import os
import glob

def has_glyph(font, glyph):
    for table in font['cmap'].tables:
        if ord(glyph) in table.cmap.keys():
            return True
    return False

def stroke2img(ttf_path, stroke, font_size=128):
    
    fg = "#000000"  # black foreground
    bg = "#FFFFFF"  # white background
    FONT_SIZE = font_size
    MAX_PADDING = 0
    
    font_object = ImageFont.truetype(ttf_path, FONT_SIZE) # Font has to be a .ttf file
    font = TTFont(ttf_path)

    if not has_glyph(font, stroke):
        print('{} does not exist in this font'.format(stroke))
        return None

    text_width, text_height = font_object.getsize(stroke)
    text_height = 128
    image = Image.new('RGBA', (text_width + MAX_PADDING*2, text_height + MAX_PADDING*2), color=bg)
    draw_pad = ImageDraw.Draw(image)

    draw_pad.text((MAX_PADDING, MAX_PADDING-12), stroke, font=font_object, fill=fg)

    image = image.convert("L") # Use this if you want to binarize image
    return image

def main(save_path='./', all_font=False):
    FONT_SIZE = 128
    MAX_PADDING = 0

    if all_font == False:
#        font_paths = ["./ttf/ink_pen_kai.ttf"]
        font_paths = ["./ttf/font1.ttf"]
        font_names = [font_paths[0].split('/')[-1]]
    else:
        font_paths = glob.glob('./ttf/*.ttf')
        font_names = []
        for exact_path in font_paths:
            font_names.append(exact_path.split('/')[-1])


    lines_graphics = open('./src/graphics.txt','r').readlines()
    fg = "#000000"  # black foreground
    bg = "#FFFFFF"  # white background

    char_list = []

    for line in lines_graphics:

        char_dict = eval(line)
        char_list.append(char_dict['character'])
    
    cnt = 1
    # Generate ‘一’ only
    # char_list = ['二']

    for idx, font_path in enumerate(font_paths):

        font_object = ImageFont.truetype(font_path, FONT_SIZE) # Font has to be a .ttf file
        font = TTFont(font_path)
        for char in char_list:
            # os.makedirs(char, exist_ok=True)
            text_width, text_height = font_object.getsize(char)
            text_height = 128
            image = Image.new('RGBA', (text_width + MAX_PADDING*2, text_height + MAX_PADDING*2), color=bg)
            draw_pad = ImageDraw.Draw(image)

            draw_pad.text((MAX_PADDING, MAX_PADDING-12), char, font=font_object, fill=fg)

            # file_name = osp.join("./"+char, char + "_" + font_names[idx] + ".png")
            file_name = osp.join('./imgs/imgs_comp_3', str(cnt) + '.jpg')

            image = image.convert("L") # Use this if you want to binarize image
            print(image.size)
            # inverse_img = np.array(ImageOps.invert(image).getdata())

            if not has_glyph(font, char):
                print('{} does not exist in this font'.format(char))
                continue
            else:
                cnt += 1
                image.save(file_name)
                
                if cnt == 1001:
                    break
        

if __name__ == '__main__':

    main(all_font=False)

