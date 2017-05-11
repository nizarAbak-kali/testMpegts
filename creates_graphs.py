import os
from os import walk

image_name = "pipeline.svg"

path_to_dots = "/home/niaba/Documents/test/testMpegts/dots/"
dots = []
i = 0

for (dirpath, dirnames, filenames) in walk(path_to_dots):
    for file in filenames :

        if not str(file[-4:]) == ".dot":
            continue
        i += 1
        string = file.split('-')
        name = string[1].split(".")[0]
        name = name +str(i)+ image_name
        os.system("dot -Tsvg "+dirpath+file+" > "+dirpath+name)
