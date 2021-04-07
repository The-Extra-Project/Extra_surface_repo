import numpy as np
import math
import random
from random import randint
import argparse
import pdb
import sys
import os
import re

import pymeshlab


def is_inside_bbox(bbox,pts) :
    return ((pts[0] > bbox[0] and pts[0] < bbox[1]) and
            (pts[1] > bbox[2] and pts[1] < bbox[3]) and
            (pts[2] > bbox[4] and pts[2] < bbox[5]))


if __name__ == '__main__':
    ###### Input Param parsing / setting =============
    parser = argparse.ArgumentParser(description='conv_ori')
    parser.add_argument('--ply_dir', default='',
                        help='give the input ply dir')
    parser.add_argument('--bbox', default='',
                        help='give the output  dir')
    parser.add_argument('--mode', default='intersect',
                        help='give the output  dir')

    args=parser.parse_args()
    inputs=vars(args)

    
    if  args.ply_dir and args.bbox  :
        inputs["ply_dir"]= os.path.expanduser(args.ply_dir)
        inputs["bbox"]=args.bbox
    else :
        print("Error!")
        print("--ply_dir --bbox  empty")
        quit()

    inputs["mode"] = args.mode
    
    print("\n=== Params ===  \n" + "\n".join("{} ==> {}".format(k, v) for k, v in inputs.items()))

    bb1 = [float(i) for i in args.bbox.split(" ")]
    list_name = []

    ms = pymeshlab.MeshSet()
    for file in os.listdir(inputs["ply_dir"]):
        if file.endswith(".ply"):
            full_path = os.path.join(inputs["ply_dir"], file)
            vv = os.popen("head -n 5 " + full_path).read()
            #vv = os.popen("head -n 5 " + os.path.expanduser(os.path.join(inputs["ply_dir"], file))).read()
            bb2 =  [float(i) for i in re.split(r'\s{1,}',list(filter(lambda x: "comment" in x , vv.split("\n")))[0])[2:][:-1]]
            if (len(bb2) == 0) :
                continue;
            print("bb2 =>" + str(bb2))
            if inputs["mode"] == "strict" :
                do_keep = True
            if inputs["mode"] == "intersect" :
                do_keep = False
            for x in range(2):
                for y in range(2):
                    for z in range(2):
                        pb = [bb2[x],bb2[2+y],bb2[4+z]]
                        is_in = is_inside_bbox(bb1,pb)
                        print(str(x) + " " + str(y) + " " + str(z) + " " + str(pb) + "->" + str(is_in))
                        if inputs["mode"] == "intersect" : 
                            do_keep = do_keep or is_in
                        if inputs["mode"] == "strict" :
                            do_keep = do_keep and is_in
            if do_keep :
                ms.load_new_mesh(full_path)
                
    ms.flatten_visible_layers()
    ms.save_current_mesh(inputs["ply_dir"] + "/bbox_" +
                         str(int(bb1[0])) + "x" + str(int(bb1[1])) + "_" +
                         str(int(bb1[1])) + "x" + str(int(bb1[2])) + "_" +
                         str(int(bb1[3])) + "x" + str(int(bb1[4])) + ".ply")
            




# print(str(is_inside_bbox(bb1,[-29,-29,29])))
