"""
    This module should process each image of a leaf and extract the ROI, using the following process:
    -the image is converted to gray-scale
    -a gaussian filter with a kernel of 5 and sigma = 3 is applied to the gray-scale image
    -an initial mask for the active contour method is calculated
    -after active contour (package chanvese) is applied to the gray-scale image,
    the rectangular boundary represents the ROI for the initial image
"""

import cv2
import numpy
from chanvese import chanvese
import os
import shutil
import time


def segment_leaf(image_folder, dest_folder=None):
    if dest_folder is None:
        if not os.path.exists(os.path.join(image_folder, 'leaf')):
            os.mkdir(os.path.join(image_folder, 'leaf'))
        dest_folder = os.path.join(image_folder, 'leaf')

    files = os.listdir(image_folder)

    for file_name in files:
        image_path = os.path.join(image_folder, file_name)
        if not os.path.isfile(image_path) or "_leaf.jpg" in file_name or ".xml" in file_name:
            continue
        if files.count(file_name[:-3] + "xml") + files.count(file_name[:-3] + "jpg") != 2:
            print "[error] Missing (image, xml) pair"
            continue
        else:
            xml_file = open(image_path[:-3] + "xml")
            if "<content>leaf</content>" not in xml_file.read().lower():
                continue
            else:
                print "[leaf] %s" % image_path

        time_s = time.time()

        shutil.copy(image_path, dest_folder)
        shutil.copy(image_path[:-4] + ".xml", dest_folder)

        image_path = os.path.join(dest_folder, file_name)

        image = cv2.imread(image_path)

        if image is None:
            print "[error] Could not load image %s" % image_path
            continue

        initial_roi_coords = (image.shape[0]/6, image.shape[1]/6, (image.shape[0]*5)/6, (image.shape[1]*5)/6)

        print "Rows: %s Columns: %s Depth: %s" % image.shape
        print "Initial ROI top-left / bottom-right coordinates: (%s, %s), (%s, %s)" % initial_roi_coords

        gray_image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        gray_image = cv2.GaussianBlur(gray_image, (5, 5), 3)

        cv2.imwrite('temp_.jpg', gray_image)
        a, _, _ = chanvese.chanvese_2('temp_.jpg', initial_roi_coords[1], initial_roi_coords[3],
                                      initial_roi_coords[0], initial_roi_coords[2], 300)
        os.remove('temp_.jpg')

        active_contour = cv2.bitwise_and(image, image, mask=numpy.array(a, dtype=numpy.uint8))

        cv2.imwrite(image_path[:-4] + "_leaf_ac.jpg", active_contour)

        active_contour = cv2.cvtColor(active_contour, cv2.COLOR_BGR2GRAY)

        contours, tree = cv2.findContours(active_contour, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

        areas = [cv2.contourArea(c) for c in contours]
        max_index = numpy.argmax(areas)
        cnt = contours[max_index]

        x, y, w, h = cv2.boundingRect(cnt)
        image = image[y:y+h, x:x+w]

        cv2.imwrite(image_path[:-4] + "_leaf.jpg", image)

        cv2.waitKey(0)
        cv2.destroyAllWindows()

        print "%ssec" % (time.time() - time_s)

if __name__ == "__main__":
    segment_leaf(r"C:\_Facultate\IP\PlantCLEF2015TrainTestData\test")