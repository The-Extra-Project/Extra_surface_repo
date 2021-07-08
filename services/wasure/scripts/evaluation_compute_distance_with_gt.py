import numpy as np
import math
import random
from random import randint
import argparse
import pdb
import sys
import os
import re
import json
import csv

import pymeshlab
import matplotlib.pyplot as plt


def is_ply_not_empty(fname) :
    with open(fname, "r") as a_file:
        for line in a_file:
            stripped_line = line.strip()
            if "vertex" in stripped_line : 
    	        return True
            if "end_header" in stripped_line : 
                return False
    return False
            
            

def check_float(potential_float):
    try:
        float(potential_float)
        return True
    except ValueError:
        return False


    
def get_params_from_string(ddr) :
    params = dict()
    nbrs = [s for s in ddr.split("_") if (s.isdigit() or check_float(s))]
    params["it"] = nbrs[-1]
    params["num_e"] = nbrs[0]
    params["ll"] = nbrs[1]
    # works in case [s for s in ss.split("ll_")[-1].split("_") if (s.isdigit() or check_float(s))][0]
    params["mult"] = nbrs[2]
    params["full_name"] = ddr
    return params



def stats_csv(inputs,field_name) :
    acc=0
    eval_dict = dict()
    gt_dirname = os.path.basename(os.path.dirname(inputs["ply_gt_path"]))
    stats_dir = inputs["output_dir"] + "/stats"
    print("create dir")
    if not os.path.exists(stats_dir) : 
        os.mkdir(stats_dir);
    print("start evaluation")
    for ddr in sorted(os.listdir(inputs["output_dir"])):
        it_dir_path = inputs["output_dir"] + ddr
        if ((not it_dir_path.endswith(".csv")) or (not field_name in it_dir_path)  ) :
            continue
        print("it dir path :" + it_dir_path)
        params = get_params_from_string(ddr)
        print(params)
        it = params["it"]
        num_e = params["num_e"]
        mm = params["mult"]

        if not mm in eval_dict :
            eval_dict[mm] = dict()
        
        res = dict()
        res['it'] = it;
        with open(it_dir_path, newline='') as csvfile:
            spamreader = csv.reader(csvfile, delimiter=',', quotechar='|')
            for row in spamreader:
                print(', '.join(row))
                it=row[0]
                res=row[1]
                eval_dict[mm][it]=res
    fig = plt.figure()
    for kk1, vv1 in eval_dict.items() :
        x=[]
        y=[]
        for kk2, vv2 in vv1.items() :
            x.append(int(kk2))
            y.append(float(vv2))
        plt.plot(x, y,label=kk1)

    plt.legend()
    fig.savefig(stats_dir + '/' + field_name + '.png')       

def compute_error(inputs) :
    acc=0
    eval_dict = dict()
    params_dict = dict()
    gt_dirname = os.path.basename(os.path.dirname(inputs["ply_gt_path"]))
    stats_dir = inputs["output_dir"] + "/stats_2"
    print("create dir")
    if not os.path.exists(stats_dir) : 
        os.mkdir(stats_dir);
    print("start evaluation")
    for ddr in sorted(os.listdir(inputs["output_dir"])):
        it_dir_path = inputs["output_dir"] + ddr
        if (not os.path.isdir(it_dir_path)) or (ddr == gt_dirname) or ("global" in ddr) or ("stats" in ddr) :
            continue

        ms = pymeshlab.MeshSet()
        params = get_params_from_string(ddr)
        print(params)
        it = params["it"]
        num_e = params["num_e"]
        mm = params["mult"]
        print("processing num_e:" + num_e + " it:" + it + " p:" + it_dir_path)
        nb_ply=0
        for file in os.listdir(it_dir_path) :
            if file.endswith(".ply"):
                full_path = os.path.join(it_dir_path, file)
                if is_ply_not_empty(full_path) :
                    ms.load_new_mesh(full_path)
                    nb_ply += 1;
        ms.flatten_visible_layers()
        ms.save_current_mesh(stats_dir + "/merged_" + str(it) + ".ply")
        ms.load_new_mesh(inputs["ply_gt_path"])
        res = ms.hausdorff_distance(sampledmesh = 0,targetmesh = 1,savesample = True,sampleface = True)
        res['it'] = it;

        if not mm in eval_dict :
            eval_dict[mm] = dict()
            params_dict[mm] = dict()
        eval_dict[mm][it]=res
        params_dict[mm] = params
        if False : 
            ms.set_current_mesh(nb_ply+1)
            ms.save_current_mesh(stats_dir + "/error_a_" + str(it) + "_" + str(it) + ".ply")
            ms.set_current_mesh(nb_ply+2)
            ms.save_current_mesh(stats_dir + "/error_b_" + str(it) + "_" + str(it) + ".ply")
        print(res)
        acc=acc+1

    # dump csv
    # ploting figure

    for kk1, vv1 in sorted(eval_dict.items()) :
        x=[]
        y=[]
        with open(inputs["output_dir"] + "/" + params_dict[kk1]["full_name"] + "_stats_meanerror.csv" , 'w') as csvfile :
            spamwriter = csv.writer(csvfile, delimiter=' ',
                            quotechar='|', quoting=csv.QUOTE_MINIMAL)
            for kk2, vv2 in sorted(vv1.items()) :
                spamwriter.writerow([kk2,vv2['mean']])

    # ploting figure
    fig = plt.figure()
    for kk1, vv1 in sorted(eval_dict.items()) :
        x=[]
        y=[]
        for kk2, vv2 in vv1.items() :
            x.append(int(kk2))
            y.append(float(vv2['mean'])) # Mean from meshlab params
        plt.plot(x, y,label=params_dict[kk1]["mult"])
    plt.legend()
    fig.savefig(stats_dir + '/mean.png')   

    with open(stats_dir + '/evaluation.json', 'w') as fp:
        json.dump(eval_dict, fp, sort_keys=True, indent=4)


        
if __name__ == '__main__':
    ###### Input Param parsing / setting =============
    parser = argparse.ArgumentParser(description='conv_ori')
    parser.add_argument('--output_dir', default='',
                        help='give the input ply dir')
    parser.add_argument('--ply_gt_path', default='',
                        help='give the input ply dir')

    args=parser.parse_args()
    inputs=vars(args)
    
    if  args.output_dir and args.ply_gt_path  :
        inputs["output_dir"]= os.path.expanduser(args.output_dir)
        inputs["ply_gt_path"]= os.path.expanduser(args.ply_gt_path)
    else :
        print("Error!")
        print("--output_dir or --ply_gt_path  empty")
        quit()
    print("\n=== Params ===  \n" + "\n".join("{} ==> {}".format(k, v) for k, v in inputs.items()))
    compute_error(inputs)
    #stats_csv(inputs,"stats_conv")
