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



if __name__ == '__main__':
    ###### Input Param parsing / setting =============
    parser = argparse.ArgumentParser(description='conv_ori')
    parser.add_argument('--ply_dir', default='',
                        help='give the input ply dir')
    args=parser.parse_args()
    inputs=vars(args)

    
    if  args.ply_dir   :
        inputs["ply_dir"]= os.path.expanduser(args.ply_dir)
    else :
        print("Error!")
        print("--ply_dir --bbox  empty")
        quit()

    inputs["mode"] = args.mode
    
    print("\n=== Params ===  \n" + "\n".join("{} ==> {}".format(k, v) for k, v in inputs.items()))
    list_name = []

    ms = pymeshlab.MeshSet()
    for file in os.listdir(inputs["ply_dir"]):
        if file.endswith(".ply"):
            full_path = os.path.join(inputs["ply_dir"], file)
            vv = os.popen("head -n 5 " + full_path).read()
            ms.load_new_mesh(full_path)
            ms.point_cloud_simplification(samplenum = 1000)
            ms.save_current_mesh(inputs["ply_dir"] + "output.ply")
            




# print(str(is_inside_bbox(bb1,[-29,-29,29])))
