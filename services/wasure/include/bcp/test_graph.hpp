#ifndef TBMRF_H
#define TBMRF_H
#define MULT (1.0)

//#include "wasure_typedefs.hpp"
#include <stdio.h>      /* printf */
#include <iostream>      /* printf */

#include <vector>
#include "io/write.hpp"
#include "io/read.hpp"
#include "partitioner/grid_partitioner.hpp"
#include "scheduler/scheduler.hpp"
#include "DDT.hpp"

#include "graph.h"
#include "wasure_maths.hpp"



void main(int argc, char **argv)
{

    int  N =  4;
    int NF = 11;

    GraphType *g = new GraphType(N,NF*2 );
    double e0,e1;

    for( int i = 0; i < N; i++ )
    {
        g -> add_node();
    }



    g->add_tweights(1, 1,6);
    g->add_tweights(2, 4,6);
    g->add_tweights(3, 4,0);
    g->add_tweights(4, 6,1);

    g->add_edge(1, 2,4);
    g->add_edge(2, 3,1);
    g->add_edge(3, 4,1);


    double flow = g->maxflow();
    std::cerr << "\t\t flow value : " << flow << std::endl;
    for(int i = 0 ; i < N; i++)
        std::cout << "i:" << i << " => " << (g->what_segment(cid) == GraphType::SOURCE) << std::endl;

    delete g;

}



#endif
