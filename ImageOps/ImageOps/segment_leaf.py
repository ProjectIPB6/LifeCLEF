"""
    This module should process each image of a leaf and extract the ROI, using the following process:
    -the image is converted to gray-scale
    -a gaussian filter with a kernel of 5 and sigma = 3 is applied to the gray-scale image
    -an initial mask for the active contour method is calculated
    -after active contour is applied to the gray-scale image, the rectangular boundary represents the ROI
    for the initial image
"""

import cv2


def segment_leaf(image_path):

    image = cv2.imread(image_path)
    if image is None:
        print "[error] Could not load image %s" % image_path
        return -1

    initial_roi_coords = (image.shape[0]/6, image.shape[1]/6, (image.shape[0]*5)/6, (image.shape[1]*5)/6)

    print "Rows: %s Columns: %s Depth: %s" % image.shape
    print "Initial ROI top-left / bottom-right coordinates: (%s, %s), (%s, %s)" % initial_roi_coords

    gray_image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    gray_image = cv2.GaussianBlur(gray_image, (5, 5), 3)

    cv2.imshow('gray_gauss', gray_image)

    roi_mask = [(initial_roi_coords[1], initial_roi_coords[0]),

                (initial_roi_coords[1], initial_roi_coords[0] + (initial_roi_coords[2] - initial_roi_coords[0]) / 4),
                (initial_roi_coords[1], initial_roi_coords[0] + (initial_roi_coords[2] - initial_roi_coords[0]) / 2),
                (initial_roi_coords[1], initial_roi_coords[2] - (initial_roi_coords[2] - initial_roi_coords[0]) / 4),

                (initial_roi_coords[1], initial_roi_coords[2]),

                (initial_roi_coords[1] + (initial_roi_coords[3] - initial_roi_coords[1]) / 4, initial_roi_coords[2]),
                (initial_roi_coords[1] + (initial_roi_coords[3] - initial_roi_coords[1]) / 2, initial_roi_coords[2]),
                (initial_roi_coords[3] - (initial_roi_coords[3] - initial_roi_coords[1]) / 4, initial_roi_coords[2]),

                (initial_roi_coords[3], initial_roi_coords[2]),

                (initial_roi_coords[3], initial_roi_coords[0] + (initial_roi_coords[2] - initial_roi_coords[0]) / 4),
                (initial_roi_coords[3], initial_roi_coords[0] + (initial_roi_coords[2] - initial_roi_coords[0]) / 2),
                (initial_roi_coords[3], initial_roi_coords[2] - (initial_roi_coords[2] - initial_roi_coords[0]) / 4),

                (initial_roi_coords[1] + (initial_roi_coords[3] - initial_roi_coords[1]) / 4, initial_roi_coords[0]),
                (initial_roi_coords[1] + (initial_roi_coords[3] - initial_roi_coords[1]) / 2, initial_roi_coords[0]),
                (initial_roi_coords[3] - (initial_roi_coords[3] - initial_roi_coords[1]) / 4, initial_roi_coords[0]),

                (initial_roi_coords[3], initial_roi_coords[0]),
                ]

    init_roi = image.copy()

    for point in roi_mask:
        cv2.circle(init_roi, point, 5, (0, 0, 255), 3)

    cv2.imshow('init_roi', init_roi)

    image_ipl = cv2.cv.CreateImageHeader((gray_image.shape[1], gray_image.shape[0]), cv2.cv.IPL_DEPTH_8U, 1)
    cv2.cv.SetData(image_ipl, gray_image, gray_image.dtype.itemsize * 1 * gray_image.shape[1])

    active_contour = cv2.cv.SnakeImage(image_ipl, roi_mask, -0.5, 0.4, 0.6, (9, 9),  # -0.4 0.5 0.5 7 7
                                       (cv2.cv.CV_TERMCRIT_ITER, 10000, 0.1))

    final_roi = image.copy()

    for point in active_contour:
        cv2.circle(final_roi, point, 5, (0, 0, 255), 3)

    cv2.imshow('final_roi', final_roi)

    min_x = min(active_contour, key=lambda t: t[0])[0]
    min_y = min(active_contour, key=lambda t: t[1])[1]

    max_x = max(active_contour, key=lambda t: t[0])[0]
    max_y = max(active_contour, key=lambda t: t[1])[1]

    image = image[min_y:max_y, min_x:max_x]

    cv2.imshow('final_image', image)

    print image_path.split('.')
    cv2.imwrite(image_path.split('.')[0] + "_leaf." + image_path.split('.')[1], image)

    cv2.waitKey(0)
    cv2.destroyAllWindows()
