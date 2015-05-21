# Puscasu Emanuel b3

import unittest
import os
import sys

from chanvese.make_mask import make_mask


class TestExtractLeaf(unittest.TestCase):
	def test_leafextract_path(self):
		# testeaza daca trimiterea unui parametru invalid returneaza False
		self.assertFalse(make_mask('uninsindasda'))

	def test_chanvese_path(self):
		# testeaza existenta folderului chanvese, necesar pentru functia activecontours
		assert os.path.exists(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'chanvese'))

	def test_chanvese_exe(self):
		# testeaza, in funtie de platforma, existenta executabilului pentru activecontours
		if sys.platform == 'win32':
			assert os.path.exists(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'chanvese', 'chanvese.exe'))
		else:
			assert os.path.exists(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'chanvese', 'chanvese'))

	def test_mask_file_path(self):
		# testeaza existenta fisierului de test pentru functia de mask
		assert os.path.exists(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'test.bmp'))

	def test_mask_output_good(self):
		# verifica output-ul corect al functiei de generare a mastii
		ret_val = make_mask(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'test.bmp'))
		self.assertEqual(ret_val, os.path.join(os.path.dirname(os.path.abspath(__file__)), 'test.bmp_init.txt'))
		self.assertEqual(open('test.bmp_init.txt').readline(), '-1 -1 -1 -1 -1 -1 -1 -1 -1 -1 \n')

	def test_mask_output_bad(self):
		# verifica output-ul in cazul trimiterii a unor parametri eronati la functia de generare a mastii
		ret_val = make_mask(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'qtest.bmp'))
		self.assertFalse(ret_val)

if __name__ == '__main__':
    unittest.main()