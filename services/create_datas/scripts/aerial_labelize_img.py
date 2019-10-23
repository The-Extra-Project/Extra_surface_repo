import cv2
import numpy as np
import math
import random
from matplotlib import pyplot as plt
from matplotlib import pyplot
from random import randint
import argparse
import pdb
import sys

def count_nblab(input_img) :
    img = cv2.imread(input_img)

    if img is None :
        print("ERRROR, " + input_img + " does not exist")
        sys.exit(10);
        
    bool_img = np.logical_or(np.logical_or(
        (img == [255,0,0]).all(axis=2),
        (img == [0,255,0]).all(axis=2)),
        (img == [0,0,255]).all(axis=2))
    # bool_img.dtype='uint8'
    # cv2.imwrite("/tmp/tmp.tif",bool_img*255);
    # pdb.set_trace()
    return bool_img.sum() 


def labelize_img(intput_img) :
    img = cv2.imread(intput_img)
    img_buff = img.copy()
    gray = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)

    gray = np.float32(gray)
    dst = cv2.cornerHarris(gray,2,3,0.04)

    #result is dilated for marking the corners, not important
    dst = cv2.dilate(dst,None)
    edges = cv2.Canny(img,100,200)
    img_buff[edges>0.01*edges.max()]=[0,0,255]
    img_buff[dst>0.30*dst.max()]=[255,0,0]

    nbp = 200
    nbs = 100
    S0 = img.shape[0]
    S1 = img.shape[1]
    p0 =  np.random.uniform(0, S0, size=nbp).astype(int)
    p1 =  np.random.uniform(0, S1, size=nbp).astype(int)
    
    for x in range(0,nbp):
        x0 = randint(0, S0)
        x1 = randint(0, S1)
        rr = randint(5, 50)
        for s in  range(0,nbs):
            v1 = randint(0, rr)
            v2 = random.uniform(0,3.14*2)
            s0 = int(x0 + math.cos(v2)*v1)
            s1 = int(x1 + math.sin(v2)*v1)
            if (s0 > 0 and s1 > 0 and
                s0 < S0 and s1 < S1 ) :
                if((img[s0,s1] == [0,0,0]).all()) : 
                    img_buff[s0,s1] = [0,255,0]

                    
    return img_buff


if __name__ == '__main__':
    ###### Input Param parsing / setting =============
    parser = argparse.ArgumentParser(description='conv_ori')
    parser.add_argument('--input_img', default='',
                        help='give the input img dir')
    parser.add_argument('--output_img', default='',
                        help='give the output  dir')

    args=parser.parse_args()
    inputs=vars(args)

    if  args.input_img and args.output_img  :
        inputs["input_img"]=args.input_img
        inputs["output_img"]=args.output_img
    else :
        print("Error!")
        print("--input_img --output_img  empty")
        quit()
    print("\n=== Params ===  \n" + "\n".join("{} ==> {}".format(k, v) for k, v in inputs.items()))

    img_buff = labelize_img(inputs["input_img"])
    cv2.imwrite(inputs["output_img"], img_buff);


    
