#ifndef LOCAL_H
#define LOCAL_H
#include "create_data_typedefs.hpp"
#include <iostream>
#include <stdio.h>      /* printf */
#include <fstream>
#include "CImg.h"

std::string remove_extension(const std::string& filename) {
  size_t lastdot = filename.find_last_of(".");
  if (lastdot == std::string::npos) return filename;
  return filename.substr(0, lastdot);
}


template <typename typedata>
inline void dump_value(typedata num,std::ofstream & fo){
  fo << num << " ";
  //fo.write(reinterpret_cast<const char *>(&num), sizeof(num));
}


int dump_points_ply_ori_lab(const std::vector<Point> & points,const std::vector<Point> & ori,const std::vector<int> & labs, std::string outputName, uint D){

  std::cout << "size:" << points.size() << std::endl;;
  if(points.size() == 0)
    return 0;
 
  std::ofstream fo;
  fo.open (outputName);

  fo << "ply" << std::endl;
  fo << "format ascii 1.0" << std::endl;
  fo << "element vertex " << points.size() << std::endl;
  fo << "property double x" << std::endl;
  fo << "property double y" << std::endl;
  if(D == 3){
    fo << "property double z" << std::endl;
  }
  fo << "property double x_origin" << std::endl;
  fo << "property double y_origin" << std::endl;
  if(D == 3){
    fo << "property double z_origin" << std::endl;
  }
  fo << "property int lab" << std::endl;
  fo << "end_header" << std::endl;
  fo << std::setprecision(10);
  fo.close();
  fo.open(outputName,std::ios_base::app);
 
  std::cout << "\t Dump points..." << std::endl;

  for(uint i = 0 ; i < points.size() ; i++){
    Point p1 = points[i];
    Point o1 = ori[i];
    int lab = labs[i];
    for(uint d = 0; d < D; d++)
      dump_value(p1[d],fo);
    for(uint d = 0; d < D; d++)
      dump_value(o1[d],fo);
    dump_value(lab,fo);
    fo << std::endl;

  }
  fo.close();

  return 0;

}

int dump_points_ply_ori(const std::vector<Point> & points,const std::vector<Point> & ori, std::string outputName, uint D){
  std::vector<int> labs(points.size(),0);
  return dump_points_ply_ori_lab(points,ori,labs,outputName,D);
}



int dump_points_ply(const std::vector<Point> & points, std::string outputName, uint D){

  std::cout << "size:" << points.size() << std::endl;;
  if(points.size() == 0)
    return 0;
 
  std::ofstream fo;
  fo.open (outputName);

  fo << "ply" << std::endl;
  //  fo << "format binary_little_endian 1.0" << std::endl;
  fo << "format ascii 1.0" << std::endl;
  fo << "element vertex " << points.size() << std::endl;
  fo << "property double x" << std::endl;
  fo << "property double y" << std::endl;
  if(D == 3){
    fo << "property double z" << std::endl;
  }
  fo << "end_header" << std::endl;
  fo << std::setprecision(10);
  fo.close();
  fo.open(outputName,std::ios_base::app | std::ios::binary);
 
  std::cout << "\t Dump points..." << std::endl;

  for(uint i = 0 ; i < points.size() ; i++){
    Point p1 = points[i];
    for(uint d = 0; d < D; d++)
      dump_value(p1[d],fo);
  }
  fo.close();

  return 0;

}

#endif
