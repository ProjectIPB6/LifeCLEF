import cv2
import numpy as np
import sys
import os
import shutil
from multiprocessing import Pool
from ExtractLeaf.segment_leaf import process_leaf


def crop_image(image, height, width):
    top = height - 1
    bottom = 0
    left = width - 1
    right = 0

    print "[crop] Top: %s Bottom: %s Left: %s Right: %s" % (top, bottom, left, right)

    for i in range(height):
        for j in range(width):
            if image.item(i, j, 0) != 0 and image.item(i, j, 1) != 0 and image.item(i, j, 2) != 0:
                if i < top:
                    top = i
                elif i > bottom:
                    bottom = i

                if j < left:
                    left = j
                elif j > right:
                    right = j

    print "[crop] Top: %s Bottom: %s Left: %s Right: %s" % (top, bottom, left, right)

    image = image[top:bottom, left:right]

    return image


def process_flower_fruit(flower_path_name, output_folder):

    flower_picture_name = os.path.split(flower_path_name)[-1]

    image = cv2.imread(flower_path_name)
    grey_image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    grey_image = cv2.GaussianBlur(grey_image, (5, 5), 3)
    ret, threshold = cv2.threshold(grey_image, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)

    contours, hierarchy = cv2.findContours(threshold, cv2.RETR_CCOMP, cv2.CHAIN_APPROX_NONE)

    height, width, depth = image.shape

    if len(contours) <= 50:
        image = cv2.bitwise_and(image, image, mask=threshold)
        print "[flower_fruit] " + flower_picture_name + ' a fost procesata de flower/fruit simplu'

    else:
        rect = (10, 10, width - 21, height - 21)
        mask = np.zeros(image.shape[:2], np.uint8)
        bgd_model = np.zeros((1, 65), np.float64)
        fgd_model = np.zeros((1, 65), np.float64)
        cv2.grabCut(image, mask, rect, bgd_model, fgd_model, 5, cv2.GC_INIT_WITH_RECT)
        mask2 = np.where((mask == 2) | (mask == 0), 0, 1).astype('uint8')
        image = image * mask2[:, :, np.newaxis]
        print "[flower_fruit] " + flower_picture_name + ' a fost procesata de flower/fruit complex'

    image = crop_image(image, height, width)
    cv2.imwrite(os.path.join(output_folder, flower_picture_name), image)


def process_scan_leaf(scan_leaf_path, output_folder):
    leaf_picture_name = os.path.split(scan_leaf_path)[-1]

    image = cv2.imread(scan_leaf_path)
    grey_image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    grey_image = cv2.GaussianBlur(grey_image, (5, 5), 3)
    ret, threshold = cv2.threshold(grey_image, 0, 255, cv2.THRESH_BINARY_INV + cv2.THRESH_OTSU)

    contours, hierarchy = cv2.findContours(threshold, cv2.RETR_CCOMP, cv2.CHAIN_APPROX_NONE)

    height, width, depth = image.shape

    if len(contours) <= 50:
        image = cv2.bitwise_and(image, image, mask=threshold)
        print "[leafscan] " + leaf_picture_name + ' a fost procesata de leaf scan simplu'

    else:
        rect = (10, 10, width - 21, height - 21)
        mask = np.zeros(image.shape[:2], np.uint8)
        bgd_model = np.zeros((1, 65), np.float64)
        fgd_model = np.zeros((1, 65), np.float64)
        cv2.grabCut(image, mask, rect, bgd_model, fgd_model, 5, cv2.GC_INIT_WITH_RECT)
        mask2 = np.where((mask == 2) | (mask == 0), 0, 1).astype('uint8')
        image = image * mask2[:, :, np.newaxis]
        print "[leafscan] " + leaf_picture_name + ' a fost procesata de leaf scan complex'

    image = crop_image(image, height, width)
    cv2.imwrite(os.path.join(output_folder, leaf_picture_name), image)


def process_stem(stem_path, output_folder):
    stem_picture_name = os.path.split(stem_path)[-1]

    image = cv2.imread(stem_path)
    height, width, depth = image.shape

    top = 0
    bottom = height - 1
    left = (width - 1) / 4
    right = (width - 1) * 3 / 4

    image = image[top:bottom, left:right]

    cv2.imwrite(os.path.join(output_folder, stem_picture_name), image)


if __name__ == '__main__':

    if len(sys.argv) == 1:
        print "Scriptul asteapta ca argument numele folder-ului de procesat!"

    else:

        pool = Pool(processes=4)

        folder_name = sys.argv[1]

        output_folder = os.path.split(folder_name)[0]
        folder = os.path.split(folder_name)[-1]
        folder += '_output'

        output_folder = os.path.join(output_folder, folder)

        if not os.path.isdir(output_folder):
            print "[main] Se creeaza " + output_folder
            os.mkdir(output_folder)
            os.mkdir(os.path.join(output_folder, "flower"))
            os.mkdir(os.path.join(output_folder, "fruit"))
            os.mkdir(os.path.join(output_folder, "leafscan"))
            os.mkdir(os.path.join(output_folder, "leaf"))
            os.mkdir(os.path.join(output_folder, "stem"))
            os.mkdir(os.path.join(output_folder, "entire"))
            os.mkdir(os.path.join(output_folder, "branch"))

        files_o = os.listdir(output_folder)
        files = os.listdir(folder_name)

        for file_name in files:

            extension = file_name.split('.')[-1]

            file_name = os.path.join(folder_name, file_name)
            
            if extension == 'xml':
                picture_name = file_name.split('.')[-2]
                picture_name = os.path.split(picture_name)[-1]
                picture_name += '.jpg'
                picture_path = os.path.join(folder_name, picture_name)

                if os.path.isfile(picture_path):
                    xml_file = open(os.path.join(folder_name, file_name), 'r')

                    for line in xml_file:
                        if line.find('<Content>') != -1:
                            if line.find('<Content>Flower</Content>') != -1:
                                #print 'Flower: ', picture_path
                                pool.apply_async(process_flower_fruit, [picture_path, os.path.join(output_folder, "flower")])
				shutil.copyfile(file_name, os.path.join(output_folder, "flower", os.path.split(file_name)[-1]))
                            
                            if line.find('<Content>Fruit</Content>') != -1:
                                #print 'Flower/Fruit: ', picture_path
                                pool.apply_async(process_flower_fruit, [picture_path, os.path.join(output_folder, "fruit")])
				shutil.copyfile(file_name, os.path.join(output_folder, "fruit", os.path.split(file_name)[-1]))
				
                            elif line.find('<Content>LeafScan</Content>') != -1:
                                #print 'LeafScan: ', picture_path
                                pool.apply_async(process_scan_leaf, [picture_path, os.path.join(output_folder, "leafscan")])
				shutil.copyfile(file_name, os.path.join(output_folder, "leafscan", os.path.split(file_name)[-1]))

                            elif line.find('<Content>Leaf</Content>') != -1:
                                #print 'Leaf: ', picture_path
                                pool.apply_async(process_leaf, [picture_path, os.path.join(output_folder, "leaf")])
				shutil.copyfile(file_name, os.path.join(output_folder, "leaf", os.path.split(file_name)[-1]))

                            elif line.find('<Content>Stem</Content>') != -1:
                                #print 'Stem: ', picture_path
                                pool.apply_async(process_stem, [picture_path, os.path.join(output_folder, "stem")])
				shutil.copyfile(file_name, os.path.join(output_folder, "stem", os.path.split(file_name)[-1]))

                            elif line.find('<Content>Entire</Content>') != -1:
                                #print 'Entire: ', picture_path
                                shutil.copyfile(picture_path, os.path.join(output_folder, "entire", picture_name))
				shutil.copyfile(file_name, os.path.join(output_folder, "entire", os.path.split(file_name)[-1]))
				
                            elif line.find('<Content>Branch</Content>') != -1:
                                #print 'Branch: ', picture_path
                                shutil.copyfile(picture_path, os.path.join(output_folder, "branch", picture_name))
				shutil.copyfile(file_name, os.path.join(output_folder, "branch", os.path.split(file_name)[-1]))
        pool.close()
        pool.join()
