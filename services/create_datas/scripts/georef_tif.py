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
        with open(args.img_dir + '/datas.json') as f:
                datas = json.load(f)
        for image_dat in datas["images"]:
                filename =  image_dat["name"]
                bname = filename.split('.')[0]
                input_img = inputs["img_dir"] + "/" + filename
                lab_filename = inputs["img_dir"] + "/" + bname + "_lab.tif"
                geo_filename = inputs["img_dir"] + "/" + bname + "_geo.tif"

                if (os.path.isfile(lab_filename) ) :
                        # Opens source dataset
                        src_ds = gdal.Open(lab_filename)
                        format = "GTiff"
                        driver = gdal.GetDriverByName(format)

                        # Open destination dataset
                        dst_ds = driver.CreateCopy(geo_filename, src_ds,0)

                        # Specify raster location through geotransform array
                        # (uperleftx, scalex, skewx, uperlefty, skewy, scaley)
                        # Scale = size of one pixel in units of raster projection
                        # this example below assumes 100x100
                        gt = [image_dat["posx"]*5000,1,0, image_dat["posy"]*5000+ 5000,0,-1]

                        # Set location
                        dst_ds.SetGeoTransform(gt)

                        # Get raster projection
                        epsg = 4326
                        srs = osr.SpatialReference()
                        srs.ImportFromEPSG(epsg)
                        dest_wkt = srs.ExportToWkt()

                        # Set projection
                        dst_ds.SetProjection(dest_wkt)

                        # Close files
                        dst_ds = None
                        src_ds = None


