import cv2
import numpy as np
import sys
import os


def is_red_ish(image, line, column):
    return image.item(line, column, 2) > image.item(line, column, 0) \
        and image.item(line, column, 2) > image.item(line, column, 1)


def is_blue_ish(image, line, column):
    return image.item(line, column, 0) > image.item(line, column, 1) \
        and image.item(line, column, 0) > image.item(line, column, 2)


def is_green_ish(image, line, column):
    return image.item(line, column, 1) > image.item(line, column, 0) \
        and image.item(line, column, 1) > image.item(line, column, 2)


def is_yellow_ish(image, line, column):
    return image.item(line, column, 0) < 100 \
        and image.item(line, column, 1) > 150 \
        and image.item(line, column, 2) > 150


def is_white_ish(image, line, column):
    return image.item(line, column, 0) > 175 \
        and image.item(line, column, 1) > 175 \
        and image.item(line, column, 2) > 175


def is_black_ish(image, line, column):
    return image.item(line, column, 0) < 100 \
        and image.item(line, column, 1) < 100 \
        and image.item(line, column, 2) < 100

def is_other(image, line, column):
    return False

def get_background_colors(image):

    height, width, yada = image.shape

    red = 0
    yellow = 0
    blue = 0
    green = 0
    white = 0
    black = 0
    other = 0

    for i in range(10):
        for j in range(width):
            if is_black_ish(image, i, j):
                black += 1
            elif is_white_ish(image, i, j):
                white += 1
            elif is_yellow_ish(image, i, j):
                yellow += 1
            elif is_green_ish(image, i, j):
                green += 1
            elif is_blue_ish(image, i, j):
                blue += 1
            elif is_red_ish(image, i, j):
                red += 1
            else:
                other += 1

    for i in range(height - 10, height):
        for j in range(width):
            if is_black_ish(image, i, j):
                black += 1
            elif is_white_ish(image, i, j):
                white += 1
            elif is_yellow_ish(image, i, j):
                yellow += 1
            elif is_green_ish(image, i, j):
                green += 1
            elif is_blue_ish(image, i, j):
                blue += 1
            elif is_red_ish(image, i, j):
                red += 1
            else:
                other += 1

    for i in range(height + 10, height - 10):
        for j in range(10):
            if is_black_ish(image, i, j):
                black += 1
            elif is_white_ish(image, i, j):
                white += 1
            elif is_yellow_ish(image, i, j):
                yellow += 1
            elif is_green_ish(image, i, j):
                green += 1
            elif is_blue_ish(image, i, j):
                blue += 1
            elif is_red_ish(image, i, j):
                red += 1
            else:
                other += 1

    for i in range(height + 10, height - 10):
        for j in range(width - 10, width):
            if is_black_ish(image, i, j):
                black += 1
            elif is_white_ish(image, i, j):
                white += 1
            elif is_yellow_ish(image, i, j):
                yellow += 1
            elif is_green_ish(image, i, j):
                green += 1
            elif is_blue_ish(image, i, j):
                blue += 1
            elif is_red_ish(image, i, j):
                red += 1
            else:
                other += 1

    total = red + yellow + blue + green + white + black + other

    """print 'Red: ' + str(red * 1.0 / total)
    print 'Yellow: ' + str(yellow * 1.0 / total)
    print 'Blue: ' + str(blue * 1.0 / total)
    print 'Green: ' + str(green * 1.0 / total)
    print 'White: ' + str(white * 1.0 / total)
    print 'Black: ' + str(black * 1.0 / total)
    print 'Other: ' + str(other * 1.0 / total)"""

    background_colors_dict = {}

    background_colors_dict['red'] = red * 1.0 / total
    background_colors_dict['yellow'] = yellow * 1.0 / total
    background_colors_dict['blue'] = blue * 1.0 / total
    background_colors_dict['green'] = green * 1.0 / total
    background_colors_dict['white'] = white * 1.0 / total
    background_colors_dict['black'] = black * 1.0 / total
    background_colors_dict['other'] = other * 1.0 / total

    maximum = 0

    for color in background_colors_dict:
        if background_colors_dict[color] > maximum:
            maximum = background_colors_dict[color]
            predominant_color = color

    background_colors_dict['max'] = predominant_color

    return background_colors_dict

"""def flower_is_white(image_name):

    image = cv2.imread(image_name)

    height, width, yada = image.shape

    for i in range(height):
        white_dots = 0
        for j in range(width):
            if is_blue_ish(image, i, j):
                white_dots += 1

        if white_dots > width / 5:
            return True

    return False"""

def mark_center(folder):
    files_list = os.listdir(folder)

    for photo in files_list:
        if photo is not '.' and photo is not '..':

            image = cv2.imread(folder + '\\' + photo, 0)
            #get_background_colors(folder + '\\' + photo)
            """height, width, lvl = image.shape

            h_milestone_1 = width / 3
            h_milestone_2 = (width / 3) * 2
            v_milestone_1 = height / 3
            v_milestone_2 = (height / 3) * 2

            for i in range(height):
                image.itemset((i, h_milestone_1, 0), 0)
                image.itemset((i, h_milestone_1, 1), 255)
                image.itemset((i, h_milestone_1, 2), 255)
                image.itemset((i, h_milestone_2, 0), 0)
                image.itemset((i, h_milestone_2, 1), 255)
                image.itemset((i, h_milestone_2, 2), 255)

            for i in range(width):
                image.itemset((v_milestone_1, i, 0), 0)
                image.itemset((v_milestone_1, i, 1), 255)
                image.itemset((v_milestone_1, i, 2), 255)
                image.itemset((v_milestone_2, i, 0), 0)
                image.itemset((v_milestone_2, i, 1), 255)
                image.itemset((v_milestone_2, i, 2), 255)"""

            cv2.imwrite('output2\\' + photo, image)

def make_borders(image_name, flower_color):
    image = cv2.imread(image_name)
    height, width, yada = image.shape

    top_red = height - 1
    bottom_red = 0
    left_red = width - 1
    right_red = 0

    if flower_color == 'black':
        comp_function = is_black_ish
    elif flower_color == 'yellow':
        comp_function = is_yellow_ish
    elif flower_color == 'green':
        comp_function = is_green_ish
    elif flower_color == 'blue':
        comp_function = is_blue_ish
    elif flower_color == 'white':
        comp_function = is_white_ish
    elif flower_color == 'red':
        comp_function = is_red_ish
    else:
        comp_function = is_other

    for i in range(height):
        for j in range(width):
            if comp_function(image, i, j):

                if i < top_red:
                    top_red = i
                elif i > bottom_red:
                    bottom_red = i

                if j < left_red:
                    left_red = j
                elif j > right_red:
                    right_red = j

    for i in range(height):
        image.itemset((i, left_red, 0), 0)
        image.itemset((i, left_red, 1), 255)
        image.itemset((i, left_red, 2), 255)

        image.itemset((i, right_red, 0), 0)
        image.itemset((i, right_red, 1), 255)
        image.itemset((i, right_red, 2), 255)

    for i in range(width):
        image.itemset((top_red, i, 0), 0)
        image.itemset((top_red, i, 1), 255)
        image.itemset((top_red, i, 2), 255)

        image.itemset((bottom_red, i, 0), 0)
        image.itemset((bottom_red, i, 1), 255)
        image.itemset((bottom_red, i, 2), 255)

    return image

def transform_image(picture_name):
    image = cv2.imread(picture_name)
    heigth, width, momo = image.shape

    for i in range(heigth):
        for j in range(width):
            if is_black_ish(image, i, j):
                image.itemset((i, j, 0), 0)
                image.itemset((i, j, 1), 0)
                image.itemset((i, j, 2), 0)
            elif is_white_ish(image, i, j):
                image.itemset((i, j, 0), 255)
                image.itemset((i, j, 1), 255)
                image.itemset((i, j, 2), 255)
            elif is_yellow_ish(image, i, j):
                image.itemset((i, j, 0), 0)
                image.itemset((i, j, 1), 255)
                image.itemset((i, j, 2), 255)
            elif is_green_ish(image, i, j):
                image.itemset((i, j, 0), 0)
                image.itemset((i, j, 1), 255)
                image.itemset((i, j, 2), 0)
            elif is_blue_ish(image, i, j):
                image.itemset((i, j, 0), 255)
                image.itemset((i, j, 1), 0)
                image.itemset((i, j, 2), 0)
            elif is_red_ish(image, i, j):
                image.itemset((i, j, 0), 0)
                image.itemset((i, j, 1), 0)
                image.itemset((i, j, 2), 255)
            else:
                image.itemset((i, j, 0), 0)
                image.itemset((i, j, 1), 128)
                image.itemset((i, j, 2), 255)

    return image

def change_color(image, predominant_color, color):

    heigth, width, lolo = image.shape

    if predominant_color == 'black':
        blue_green_red = [0,0,0]
    elif predominant_color == 'yellow':
        blue_green_red = [0, 255, 255]
    elif predominant_color == 'green':
        blue_green_red = [0, 255, 0]
    elif predominant_color == 'blue':
        blue_green_red = [255, 0, 0]
    elif predominant_color == 'white':
        blue_green_red = [255, 255, 255]
    elif predominant_color == 'red':
        blue_green_red = [0, 0, 255]
    elif predominant_color == 'other':
        blue_green_red = [0, 128, 255]

    if color == 'black':
        comp_function = is_black_ish
    elif color == 'yellow':
        comp_function = is_yellow_ish
    elif color == 'green':
        comp_function = is_green_ish
    elif color == 'blue':
        comp_function = is_blue_ish
    elif color == 'white':
        comp_function = is_white_ish
    elif color == 'red':
        comp_function = is_red_ish
    else:
        comp_function = is_other

    for i in range(heigth):
        for j in range(width):
            if comp_function(image, i, j):
                image.itemset((i, j, 0), blue_green_red[0])
                image.itemset((i, j, 1), blue_green_red[1])
                image.itemset((i, j, 2), blue_green_red[2])

    return image

def get_flower_color(image, background_color):
    heigth, width, aim = image.shape

    colors = {}
    colors['black'] = 0
    colors['yellow'] = 0
    colors['green'] = 0
    colors['blue'] = 0
    colors['white'] = 0
    colors['red'] = 0
    colors['other'] = 0

    for i in range(heigth):
        for j in range(width):
            if is_black_ish(image, i, j):
                colors['black'] += 1
            elif is_white_ish(image, i, j):
                colors['white'] += 1
            elif is_yellow_ish(image, i, j):
                colors['yellow'] += 1
            elif is_green_ish(image, i, j):
                colors['green'] += 1
            elif is_blue_ish(image, i, j):
                colors['blue'] += 1
            elif is_red_ish(image, i, j):
                colors['red'] += 1
            else:
                colors['other'] += 1

    maximum = 0
    flower_color = 'unknown'

    for color in colors:
        if colors[color] > maximum and color is not background_color:
            maximum = colors[color]
            flower_color = color

    return flower_color



def get_roi_borders(picture_name):
    image = cv2.imread(picture_name)

    background_colors = get_background_colors(image)
    predominant_background_color = background_colors['max']
    background_colors.pop('max')

    for color in background_colors:
        if background_colors[color] > 0.1:
            image = change_color(image, predominant_background_color, color)

    flower_color = get_flower_color(image, predominant_background_color)

    original_image = make_borders(picture_name, flower_color)

    return original_image


def crop_roi(picture_name):
    image = cv2.imread(picture_name)
    grey_image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    grey_image = cv2.GaussianBlur(grey_image,(5,5),0)
    edges = cv2.Canny(grey_image, 100, 200)
    ret, threshold = cv2.threshold(edges,127,255,cv2.THRESH_OTSU)
    contours, hierarchy = cv2.findContours(threshold,cv2.RETR_CCOMP,cv2.CHAIN_APPROX_NONE)

    for contour in contours:
        if cv2.contourArea(contour) > 5000:
            cv2.drawContours(image=edges, contours=contour, contourIdx=-1, color=(255,255,255),thickness=-1)

    height, width, yolo = image.shape

    top = height - 1
    bottom = 0
    left = width - 1
    right = 0

    for i in range(height):
        for j in range(width):
            if edges.item(i, j) == 255:
                if i < top:
                    top = i
                elif i > bottom:
                    bottom = i

                if j < left:
                    left = j
                elif j > right:
                    right = j

    """if top < (10 / 100) * (height - 1):
        top = (10 / 100) * (height - 1)

    if bottom > (90 / 100) * (height - 1):
        bottom = (90 / 100) * (height - 1)

    if left < (10 / 100) * (width - 1):
        left = (10 / 100) * (width - 1)

    if right > (90 / 100) * (width - 1):
        right = (90 / 100) * (width - 1)"""

    """for i in range(height):
        image.itemset((i, left, 0), 0)
        image.itemset((i, left, 1), 255)
        image.itemset((i, left, 2), 255)

        image.itemset((i, right, 0), 0)
        image.itemset((i, right, 1), 255)
        image.itemset((i, right, 2), 255)

    for i in range(width):
        image.itemset((top, i, 0), 0)
        image.itemset((top, i, 1), 255)
        image.itemset((top, i, 2), 255)

        image.itemset((bottom, i, 0), 0)
        image.itemset((bottom, i, 1), 255)
        image.itemset((bottom, i, 2), 255)"""

    image = image[top:bottom, left:right]

    return image




if __name__ == '__main__':

    if len(sys.argv) > 1:
        folder_name = sys.argv[1]
        files = os.listdir(folder_name)

        for file_name in files:
            if file_name is not '.' and file_name is not '..':
                if file_name.split('.')[-1] == 'xml':
                    xml_file = open(os.path.join(folder_name, file_name), 'r')

                    for line in xml_file:
                        if line.find('<Content>') != -1:

                            if line.find('Flower') != -1 or line.find('Fruit') != -1:
                                photo_name = file_name.split('.')[0] + '.jpg'
                                cv2.imwrite('output_final\\' + photo_name, crop_roi(folder_name + '\\' + photo_name))

                    xml_file.close()
    else:
        print('Scriptul are nevoie de numele folder-ului ca parametru.')