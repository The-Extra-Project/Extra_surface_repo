#!/usr/bin/env python3


import random
from matplotlib import pyplot as plt
from matplotlib import pyplot
from random import randint

import numpy as np
import numpy.linalg
import glob
import argparse
import re
import subprocess
import os.path
import os
import pdb


#import cv2
#from aerial_labelize_img import labelize_img
#from aerial_labelize_img import count_nblab

from os.path import basename
from operator import itemgetter
from xml.dom.minidom import parseString
import subprocess

import json
import sys

def get_script_path():
        return os.path.dirname(os.path.realpath(sys.argv[0]))



if __name__ == '__main__':
        ###### Input Param parsing / setting =============
        parser = argparse.ArgumentParser(description='conv_ori')
        parser.add_argument('--img_dir', default='',
                            help='give the input img dir')
        parser.add_argument('--output_dir', default='',
                            help='give the output  dir')
        parser.add_argument('--bin_dir', default='',
                            help='get the c+ bin dir')

        args=parser.parse_args()
        inputs=vars(args)
        img2data_exe=args.bin_dir + "/create_data-stream-exe"


        if  args.img_dir and args.output_dir  :
            inputs["img_dir"]=args.img_dir
            inputs["output_dir"]=args.output_dir
        else :
            print("Error!")
            print("--img_dir or --output_dir  empty")
            quit()


        print("\n=== Params ===  \n" + "\n".join("{} ==> {}".format(k, v) for k, v in inputs.items()))

        nbl_tot = 0

        with open(args.img_dir + '/datas.json') as f:
                datas = json.load(f)

        for image_dat in datas["images"]:
                filename =  image_dat["name"]

                bname = filename.split('.')[0]
                input_img = inputs["img_dir"] + "/" + filename
                lab_filename = inputs["output_dir"] + "/" + bname + "_lab.tif"
                ply_filename = inputs["output_dir"] + "/" + bname + "_res.ply"
                print(filename)
                print(lab_filename)
                #img_lab = labelize_img(input_img)
                #cv2.imwrite(lab_filename,img_lab);
                if (os.path.isfile(lab_filename) ) :
                        subprocess.call([img2data_exe,
                                         "--name_img_in",lab_filename,
                                         "--x_origin",str(image_dat["posx"]*5000),
                                         "--y_origin",str(image_dat["posy"]*5000),
                                         "--name_ply_out",ply_filename,
                                         "--do_rand"])


