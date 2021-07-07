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


def get_cmap(n, name='hsv'):
    '''Returns a function that maps each index in 0, 1, ..., n-1 to a distinct                                                                                                                                                                
    RGB color; the keyword argument name must be a standard mpl colormap name.'''
    return plt.cm.get_cmap(name, n)


def is_inside_bbox(bbox,pts) :
    return ((pts[0] > bbox[0] and pts[0] < bbox[1]) and
            (pts[1] > bbox[2] and pts[1] < bbox[3]) and
            (pts[2] > bbox[4] and pts[2] < bbox[5]))



def merge_ply(inputs,bb1) :
    list_name = []
    cur_dir = os.path.basename(os.path.normpath(inputs["input_dir"])) 
    ms = pymeshlab.MeshSet()

    step = 1000
    cmap = get_cmap(100,'tab20c')
    for file in os.listdir(inputs["input_dir"]):
        if file.endswith(".ply"):
            full_path = os.path.join(inputs["input_dir"], file)
            vv = os.popen("head -n 5 " + full_path).read()
            #vv = os.popen("head -n 5 " + os.path.expanduser(os.path.join(inputs["input_dir"], file))).read()
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

            

            
            if  do_keep :
                ms.load_new_mesh(full_path)
                cc = cmap(random.randint(0,100))
                ms.per_face_color_function(r=str(cc[0]*255),g=str(cc[1]*255),b=str(cc[2]*255))
                
    ms.flatten_visible_layers()
    cur_date = datetime.now().strftime("%d_%m_%Y_%H_%M_%S")
    ms.save_current_mesh(inputs["output_dir"] + "/" + cur_dir + "bbox_" +
                         str(int(bb1[0])) + "x" + str(int(bb1[1])) + "_" +
                         str(int(bb1[2])) + "x" + str(int(bb1[3])) + "_" +
                         str(int(bb1[4])) + "x" + str(int(bb1[5])) + ".ply")

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
    #merge_ply(inputs,bb1);

    split_nb = 4
    for n in range(split_nb):
        for m in range(split_nb):
            llx = (bb1[1] - bb1[0])/split_nb
            new_bbp = [bb1[0] + n*llx,bb1[0] + (n+1)*llx,bb1[2] + m*llx,bb1[2] + (m+1)*llx,bb1[4],bb1[5]]
            merge_ply(inputs,new_bbp)

            




# print(str(is_inside_bbox(bb1,[-29,-29,29])))


## "<bbox>3800x4800:4500x5500:0x1000</bbox>"
# 4000 4500 4750 5250 0x1000
