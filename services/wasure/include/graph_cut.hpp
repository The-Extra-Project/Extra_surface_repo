#ifndef IQ_GC_H
#define IQ_GC_H
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


typedef double coord_type;
typedef double DATA_TYPE;
typedef Graph<DATA_TYPE,DATA_TYPE,DATA_TYPE> GraphType;


struct g_edge
{
    int id1,id2;
    double e0,e1;
} ;
struct g_vert
{
    int id1;
    double e0,e1;
} ;




int gc_on_stream(std::istream & ifile,std::ostream & ofile)
{
    int nb_dat,tt;
    std::vector<g_edge> v_edges;
    std::vector<g_vert> v_verts;
    int acc = 0;
    while(ifile  >> tt)
    {
        std::cerr << "acc:" << acc++ << "tt:" << tt << std::endl;
        //      for(int i = 0; i < nb_dat;i++){
        //	ifile >> tt;
        switch(tt)
        {
        case 1 :
        {
            g_vert gv;
            ifile >> gv.id1 >> gv.e0 >> gv.e1;
            v_verts.push_back(gv);
            break;
        }
        case 2 :
        {
            g_edge ge;
            ifile >> ge.id1 >> ge.id2 >> ge.e0 >> ge.e1;
            v_edges.push_back(ge);
            break;
        }
        }
    }
    //    }
    int N = v_verts.size();
    int NF = v_edges.size();
    std::cerr << "N:" << N << " NF:" << NF <<  std::endl;
    GraphType *g = new GraphType(N,NF*2 );
    std::cerr << "init graph struct " << std::endl;
    for(int i = 0; i < v_verts.size(); i++)
    {
        g -> add_node();
    }
    for(auto vv = v_verts.begin();
            vv != v_verts.end(); vv++)
    {
        g->add_tweights(vv->id1, vv->e0, vv->e1);
    }
    for(auto ee = v_edges.begin();
            ee != v_edges.end(); ee++)
    {
        g->add_edge(ee->id1, ee->id2,ee->e0,ee->e1);
    }
    std::cerr << "\t Max flow algorithm ..." << std::endl;
    double flow = g->maxflow();
    std::cerr << "\t\t flow value : " << flow << std::endl;
    int nb_merge = 0;
    for(int i = 0; i < N; i++)
    {
        int lab = g->what_segment(i);
        ofile << i << " " << lab << std::endl;
    }
    delete g;
    std::cerr << "perfect!" << std::endl;
    return 0;
}


#endif
