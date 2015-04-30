"""
    This processes an image of a leaf and extracts the ROI, using the following process:
    -the image is converted to gray-scale
    -a gaussian filter with a kernel of 5 and sigma = 3 is applied to the gray-scale image
    -an initial mask for the active contour method is calculated
    -after this, an active contour (chan-vese) is applied to the gray-scale image
    -the rectangular boundary of the active contour represents the ROI for the initial image
"""

import cv2
import numpy
import os
import time
import subprocess

from chanvese.make_mask import make_mask


def process_leaf(image_path, output_folder="."):
    time_s = time.time()

    file_name = os.path.split(image_path)[-1]
    print "[leaf] Processing %s" % image_path

    image = cv2.imread(image_path)

    if image is None:
        print "[leaf-error] Could not load image %s" % image_path

    print "Rows: %s Columns: %s Depth: %s" % image.shape

    gray_image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    gray_image = cv2.GaussianBlur(gray_image, (5, 5), 3)

    temp_img_path = os.path.join(os.path.dirname(__file__), "chanvese", file_name + '_temp.bmp')

    cv2.imwrite(temp_img_path, gray_image)
    init_mask_path = make_mask(temp_img_path)
   
    if os.name != 'nt':
        process = subprocess.Popen([os.path.join(os.path.dirname(__file__), "chanvese", "chanvese"),
                                                                "phi0:%s" % init_mask_path, "mu:0.0", "iterperframe:1000", "maxiter:1000",
                                                                temp_img_path, temp_img_path + "_animation.gif", temp_img_path + "_final.bmp"])
    else:
        process = subprocess.Popen([os.path.join(os.path.dirname(__file__), "chanvese", "chanvese.exe"),
                                                                "phi0:%s" % init_mask_path, "mu:0.0", "iterperframe:1000", "maxiter:1000",
                                                                temp_img_path, temp_img_path + "_animation.gif", temp_img_path + "_final.bmp"])
    process.wait()

    mask = cv2.imread(temp_img_path + "_final.bmp", 0)

    active_contour = cv2.bitwise_and(image, image, mask=mask)

    active_contour = cv2.cvtColor(active_contour, cv2.COLOR_BGR2GRAY)

    contours, tree = cv2.findContours(active_contour, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

    areas = [cv2.contourArea(c) for c in contours]
    max_index = numpy.argmax(areas)
    cnt = contours[max_index]

    x, y, w, h = cv2.boundingRect(cnt)

    image = image[y:y+h, x:x+w]

    cv2.imwrite(os.path.join(output_folder, file_name), image)

    os.remove(temp_img_path)
    os.remove(temp_img_path + "_animation.gif")
    os.remove(init_mask_path)
    os.remove(temp_img_path + "_final.bmp")

    print "%ssec" % (time.time() - time_s)
