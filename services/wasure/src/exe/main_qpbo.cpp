#include <stdio.h>
#include "QPBO.h"
#include <fstream>
#include <iostream>
#include <vector>
#include "math.h"
double reg(double vv){
  if(vv == 0)
    vv += 0.0000001;
  if(vv == 1)
    return 0;
  return -log(vv);
  //  return -log(vv+0.000001);
}
  

int main(int argc, char** argv) 
{ 

  std::string vertex(argv[1]);
  std::string edge(argv[2]);



  int ids[2];
  double E[4];
  std::vector<std::vector<double>> v_ids;
  std::vector<std::vector<double>> v_edg;

  std::ifstream infile;
  infile.open(vertex);
  while (infile >> ids[0] >> E[0] >> E[1])
    {
      //      std::cerr << ids[0] << "\t :" << reg(E[0]) << "\t" << reg(E[1]) << std::endl;
      v_ids.push_back(std::vector<double>({ids[0],E[0],E[1]}));
    }
  std::cerr << std::endl;
  infile.close();

  infile.open(edge);
  while (infile >> ids[0] >> ids[1] >> E[0] >> E[1] >> E[2] >> E[3]){

      v_edg.push_back(std::vector<double>({(double)ids[0],(double)ids[1],E[0],E[1],E[2],E[3]}));
  }
  std::cerr << std::endl;
  infile.close();
  
  typedef double FLOAT;
  QPBO<FLOAT>* q;

  q = new QPBO<FLOAT>(v_ids.size(), v_edg.size()); // max number of nodes & edges
  q->AddNode(v_ids.size()); // add two nodes

  for(auto nd : v_ids){
    std::cerr << nd[0] << "\t :" << reg(nd[1]) << "\t" << reg(nd[2]) << std::endl;
    std::cerr << std::endl;
    q->AddUnaryTerm((int)nd[0],reg(nd[1]),reg(nd[2])); // add term 2*x

  }
  for(auto ee : v_edg){
    std::cerr << ee[0] << " " << ee[1] << "\t :" << reg(ee[2]) << "\t" << reg(ee[3]) << "\t" << reg(ee[4]) << "\t" << reg(ee[5]) << std::endl;
    q->AddPairwiseTerm((int)ee[0], (int)ee[1],reg(ee[2]), reg(ee[3]),reg(ee[4]), reg(ee[5])); // add term (x+1)*(y+2)
  }
  std::cerr << "=========" << std::endl;
  q->Solve();
  q->ComputeWeakPersistencies();

  bool is_first =true;
  for(auto vv : v_ids){
    std::cout << vv[0] << "," <<  q->GetLabel((int)vv[0]) << ";";
  }
  std::cout << std::endl;

  return 0;
}
