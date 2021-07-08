import numpy as np
import math
import random
from random import randint
import argparse
import pdb
import sys
import os
import re
from datetime import datetime
import pymeshlab
import matplotlib.pyplot as plt


def get_cmap(n, name='hsv'):
    '''Returns a function that maps each index in 0, 1, ..., n-1 to a distinct                                                                                                                                                                
    RGB color; the keyword argument name must be a standard mpl colormap name.'''
    return plt.cm.get_cmap(name, n)

def is_inside_bbox(bbox,pts) :
    return ((pts[0] > bbox[0] and pts[0] < bbox[1]) and
            (pts[1] > bbox[2] and pts[1] < bbox[3]) and
            (pts[2] > bbox[4] and pts[2] < bbox[5]))



def merge_ply(inputs,bb1,rmin = 0,rmax = 1000000) :
    list_name = []
    cur_dir = os.path.basename(os.path.normpath(inputs["input_dir"]))
    ms = pymeshlab.MeshSet()
    step = 1000
    cmap = get_cmap(100,'tab20c')
    acc = 0
    for ff in os.listdir(inputs["input_dir"]):
        if ff.endswith(".ply") and acc >= rmin and acc < rmax:
            full_path = os.path.join(inputs["input_dir"], ff)
            print("load : " + str(acc) + " =>" + full_path)
            ms.load_new_mesh(full_path)
            cc = cmap(random.randint(0,100))
            ms.per_face_color_function(r=str(cc[0]*255),g=str(cc[1]*255),b=str(cc[2]*255))
        acc = acc +1
    ms.flatten_visible_layers()
    cur_date = datetime.now().strftime("%d_%m_%Y_%H_%M_%S")
    outname=inputs["output_dir"] + "/merged" + str(rmin) + "_" + str(rmax) + ".ply"
    print(outname)
    ms.save_current_mesh(outname)

if __name__ == '__main__':
    ###### Input Param parsing / setting =============                                                                                                                                                                                        
    parser = argparse.ArgumentParser(description='conv_ori')
    parser.add_argument('--input_dir', default='',
                        help='give the input ply dir')
    parser.add_argument('--bbox', default='',
                        help='give bbox')
    parser.add_argument('--mode', default='intersect',
                        help='give the mode, "intersect" => just touchingt, "strict" => the bbox is included')
    parser.add_argument('--output_dir', default='',
                        help='give the output  dir')

    args=parser.parse_args()
    inputs=vars(args)


    if  args.input_dir   :
        inputs["input_dir"]= os.path.expanduser(args.input_dir)
    else :
        print("Error, ")
        print("--input_dir is mandatory")
        quit()

    if  args.output_dir :
        inputs["output_dir"] = args.output_dir
    else :
        inputs["output_dir"] = args.input_dir

    if  args.bbox :
        inputs["bbox"]=args.bbox
    else :
        inputs["bbox"]="-100000 100000 -100000 100000 -100000 100000"
        inputs["mode"] = args.mode


    print("\n=== Params ===  \n" + "\n".join("{} ==> {}".format(k, v) for k, v in inputs.items()))

    bb1 = [float(i) for i in args.bbox.split(" ")]
#    merge_ply(inputs,bb1,0,2000);
    merge_ply(inputs,bb1,2000,4000);
    merge_ply(inputs,bb1,4000,6000);
    merge_ply(inputs,bb1,6000,8000);
