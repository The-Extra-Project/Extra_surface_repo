
#include <stdio.h>      /* printf */
#include <iostream>      /* printf */

#include <vector>
#include "io/write.hpp"
#include "io/read.hpp"
#include "partitioner/grid_partitioner.hpp"
#include "scheduler/scheduler.hpp"
#include "DDT.hpp"

#include "graph.h"
//#include "maths.hpp"

  typedef double coord_type;
  typedef double DATA_TYPE;
  typedef Graph<DATA_TYPE,DATA_TYPE,DATA_TYPE> GraphType;


int test1(){
  int  N =  4;
  int NF = 11;

  GraphType *g = new GraphType(N,NF*2 );
  double e0,e1;

  for( int i = 0; i < N; i++ ){
    g -> add_node(); 
  }

  g->add_tweights(0, 1,6);
  g->add_tweights(1, 4,6);
  g->add_tweights(2, 4,0);
  g->add_tweights(3, 6,1);    
    
  g->add_edge(0, 1,4,0);
  g->add_edge(1, 2,1,0);
  g->add_edge(2, 3,1,0);


  double flow = g->maxflow();
  std::cerr << "\t\t flow value : " << flow << std::endl;
  for(int i = 0 ; i < N;i++){
    int ii = i+1;
    std::cout << "i:" << ii << " => " << (g->what_segment(i) == GraphType::SOURCE) << std::endl;
  }

  delete g;


}

void add_tweight(GraphType *g ,int i,int e0,int e1){
  g->add_tweights(i,e0,e1);
  std::cout << 0 << " " << i+2 << " " << e0 << ",";
  std::cout << i+2 << " " << 1 << " " << e1 << ",";
}
void add_edge(GraphType *g ,int id1,int id2,int w1,int w2,bool is_last = false){
  g->add_edge(id1,id2,w1,w2);
  std::cout << id1 + 2 << " " << id2 + 2 << " " << w1 << ",";
  std::cout << id2 + 2<< " " << id1 +2 << " " << w2 ;
  if(!is_last)
    std::cout << ",";
}

int test2(int N){
  int NF = 1;

  GraphType *g = new GraphType(N,NF*2 );
  double e0,e1;
  std::vector<int> ve0(N);
  std::vector<int> ve1(N);

  
  for( int i = 0; i < N; i++ ){
    g -> add_node(); 
  }

  for( int i = 0; i < N; i++ ){
    int e0 = rand() % 100;
    int e1 = 100-e0;
    ve0[i] = e0;
    ve1[i] = e1;
    add_tweight(g,i,e0,e1);
  }

  int nbe = N/3;
  for(int i = 0; i < nbe; i ++){
    int id1 = rand()%N;
    int id2 = rand()%N;
    if(id1 == id2)
      continue;
    add_edge(g,id1,id2,(rand()%100)/10,(rand()%100)/10,i == (nbe-1));
  }
  std::cout << ";";
  
  std::cerr << "ARC:" << g->get_arc_num() << std::endl;
  double flow = g->maxflow();
  std::cerr << "\t\t flow value : " << flow << std::endl;
  auto nd = g->get_first_node();
  for(int i = 0 ; i < N;i++){
    int lab = (g->what_segment(i) == GraphType::SOURCE);
    double cap = nd->tr_cap;
    double cap1,cap2;
    if(cap > 0){
      cap1 = cap;
      cap2 = 0;
    }else{
      cap1 = 0;
      cap2 = -cap;
    }
    //    node_map[nd] = i;

        
    std::cerr << 0 << " " << i+2 << " " << ve0[i] << " " << lab  << " " << cap1 <<  std::endl;
    std::cerr << i+2 << " " << 1 << " " << ve1[i] << " " << lab << " " << cap2 << std::endl;
    
    std::cout << 0 << " " << i+2 << " " << ve0[i] << " " << lab  << " " << cap1 << ",";
    std::cout << i+2 << " " << 1 << " " << ve1[i] << " " << lab << " " << cap2;
    nd++;
    if(i < N-1)
      std::cout << "," ;
    
  }
  std::cout << ";";
  for (auto a= g->get_first_arc(); a < g->get_last_arc(); a++)
    {
      std::cout << a->sister->head - g->get_first_node() +2 << " " 
		<< a->head - g->get_first_node() + 2 << " " << 0 << " " << a->r_cap;

      if( a < g->get_last_arc() - 1)
	std::cout << "," ;
    }

  // std::cout << ";";

  // for(int i = 0 ; i < N;i++){
  //   int lab = (g->what_segment(i) == GraphType::SOURCE);
  //   //    std::cerr  << i << " " << ve0[i] << " " << ve1[i] << std::endl;
  //   if(i < N-1)
  //     std::cout << "," ;
  // }
  std::cout << ";";
  for(int i = 0 ; i < N;i++){
    int lab = (g->what_segment(i) == GraphType::SOURCE);
    std::cout << i+2 << " " << lab << " ";
    if(i < N-1)
      std::cout << "," ;
  }
  std::cout << ";";
  std::cout << flow ;
  std::cout << std::endl;

  //  std::cerr << "ARC:" << *g->get_first_arc()  << " " << *g->get_last_arc() << std::endl;
  
  

  // int acc = 0;
  // for (auto a= g->get_first_node(); a < g->get_last_node(); a++)
  //   {
  //     // std::cerr << "node:" << a << std::endl;
  //     // std::cerr << a->is_sink << std::endl;
  //     double cap = a->tr_cap;
  //     if(cap > 0)
  // 	std::cerr << "NODE: "  << acc++ << " " << cap << " " << 0 << std::endl;
  //     else
  // 	std::cerr << "NODE: "  << acc++ << " " << 0 << " " << -cap << std::endl;
  //   }
	
  
  delete g;
  return 0;

}



int main(int argc, char **argv){
  srand(time(NULL));  
  test2(atoi(argv[1]));
  return 0;
}

