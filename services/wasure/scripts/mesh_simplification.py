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
    parser.add_argument('--output_dir', default='',
                        help='give the output  dir')
    args=parser.parse_args()
    inputs=vars(args)

    
    if  args.ply_dir  and args.output_dir  :
        inputs["ply_dir"]= os.path.expanduser(args.ply_dir)
        inputs["output_dir"] = os.path.expanduser(args.output_dir)

    else :
        print("Error!")
        print("--ply_dir --bbox  empty")
        quit()

    
    print("\n=== Params ===  \n" + "\n".join("{} ==> {}".format(k, v) for k, v in inputs.items()))
    list_name = []

    ms = pymeshlab.MeshSet()
    for ff in os.listdir(inputs["ply_dir"]):
        if ff.endswith(".ply"):
            if ff == "part-06555_1.ply" :
                continue
            full_path = os.path.join(inputs["ply_dir"], ff)
            new_path = inputs["output_dir"] + ff;
            print(new_path)
            if os.path.isfile(new_path)  :
                print("exists!")
            else :
                ms.load_new_mesh(full_path)
                ms.simplification_quadric_edge_collapse_decimation(targetperc = 0.1,preserveboundary = True)
                ms.permesh_color_scattering()
                ms.per_face_color_function(r=str(random.randint(0,255)),g=str(random.randint(0,255)),b=str(random.randint(0,255)))
                ms.save_current_mesh(new_path,save_face_color=True)
            




# print(str(is_inside_bbox(bb1,[-29,-29,29])))

