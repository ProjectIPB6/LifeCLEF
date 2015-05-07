import os
import sys

def process_folder():

	if len(sys.argv) == 1:
		print "Scriptul asteapta ca argument numele folder-ului de procesat!"

	observation_dictionary = {}

	folder_name = sys.argv[1]

	files = os.listdir(folder_name)

	for file_name in files:

		file_extension = file_name.split('.')[-1]

		if file_extension.lower() == 'xml':

			xml_file = open(os.path.join(folder_name, file_name), 'r')

			for line in xml_file:
				if line.find('<ObservationId>') != -1:
					observation_id = line.split('>')[1]
					observation_id = observation_id.split('<')[0]
					
					if observation_id not in observation_dictionary.keys():
						observation_dictionary[observation_id] = []
						observation_dictionary[observation_id].append(file_name.split('\\')[-1])
					else:
						observation_dictionary[observation_id].append(file_name.split('\\')[-1])

			xml_file.close()

	output_file = open('output.txt', 'w')

	print len(observation_dictionary)

	for key in observation_dictionary:
		output_file.write(key + '|' + str(observation_dictionary[key]).strip('[').strip(']').replace('\'', '').replace('.xml', '') + '\n')

if __name__ == '__main__':
	process_folder()