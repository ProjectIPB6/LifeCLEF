import unittest
import os
import sys
import cv2

from LifeCLEF_ImageProcessor import crop_image
from LifeCLEF_ImageProcessor import process_scan_leaf
from LifeCLEF_ImageProcessor import process_stem
from ExtractLeaf.segment_leaf import process_leaf

class TestLifeClef(unittest.TestCase):

	#Munteanu Ilie-Alexandru, B6

	#Verifica daca modulul de Extract Leaf (extern scriptului original)
	def test_extract_leaf(self):
		assert os.path.exists (os.path.join(os.path.dirname(os.path.abspath(__file__)), 'ExtractLeaf', 'segment_leaf.py'))

	#Verifica daca functia de crop returneaza o imagine cu width si height valide (mai mari decat 0)
	def test_crop_validity(self):
		image = cv2.imread('test.jpg')
		height, width, empty = image.shape
		cropped_image = crop_image(image, height, width)
		height, width, empty = image.shape
		self.assertTrue(height > 0)
		self.assertTrue(width > 0)

	#Turnea Diana, B6

	#Verifica felul in care se comporta modulul de procesat ScanLeaf daca se dau ca date de intrare path-uri invalide, HINT: Da Fail
	def test_process_scan_leaf(self):
		assert process_scan_leaf('C:\\Unexisting Folder\\UnexistingPicture.jpg', 'C:\\Unexisting Output Folder')

	#Verifica daca scriptul de procesare trunchi parseaza corect path-ul pentru a obtine numele imaginii
	def test_process_stem(self):
		self.assertTrue(process_stem(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'test.jpg'), 'C:\\output') == 'test.jpg')

if __name__ == '__main__':
	unittest.main()