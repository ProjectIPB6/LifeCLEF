import cv2
import os


def make_mask(image_path):
    if not os.path.exists(image_path):
        return False
        
    image = cv2.imread(image_path)
    iroi = (image.shape[0]/6, image.shape[1]/6, (image.shape[0]*5)/6, (image.shape[1]*5)/6)

    print "Initial ROI top-left / bottom-right coordinates: (%s, %s), (%s, %s)" % iroi

    image[0:image.shape[0], 0:image.shape[1]] = -1
    image[iroi[0]:iroi[2], iroi[1]:iroi[3]] = 1
    image[iroi[0]+1:iroi[2]-1, iroi[1]+1:iroi[3]-1] = 0

    init_mask = os.path.join(os.path.split(image_path)[0], os.path.split(image_path)[1] + "_init.txt")

    f = open(init_mask, "w")

    if not f:
        return False

    img_list = image.tolist()
    for row in img_list:
        for col in row:
            val = str(col[0])
            if val == "255":
                val = "-1"
            f.write(val + " ")
        f.write("\n")
    f.close()

    return init_mask
	
if __name__ == "__main__":
	make_mask("img.bmp")