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


inline int l2gid(int cid,std::vector<int> & tile_ids)
{
    return cid + tile_ids[2] + 2;
}

inline int g2lid(int cid,std::vector<int> & tile_ids)
{
    return cid - tile_ids[2] - 2;
}


inline int l2gidf(int cid,std::vector<int> & tile_ids)
{
    return cid + tile_ids[2];
}

inline int g2lidf(int cid,std::vector<int> & tile_ids)
{
    return cid - tile_ids[2];
}


template<typename DTW,typename D_MAP>
class tbmrf
{

public :

    typedef double coord_type;
    typedef double DATA_TYPE;
    typedef Graph<DATA_TYPE,DATA_TYPE,DATA_TYPE> GraphType;


    typedef typename DTW::Tile_cell_const_handle              Tile_cell_const_handle;
    typedef typename DTW::DT::Full_cell::Vertex_handle_iterator Vertex_h_iterator;
    typedef typename DTW::Cell_const_iterator                 Cell_const_iterator;
    typedef typename DTW::Facet_const_iterator                Facet_const_iterator;
    typedef typename DTW::Traits                              Traits;
    typedef typename Traits::Point                            Point;
    typedef typename Traits::Vertex_const_handle              Vertex_const_handle;
    typedef typename Traits::Cell_const_handle                Cell_const_handle;
    typedef typename Traits::Cell_handle                      Cell_handle;
    typedef typename Traits::Vertex_handle                    Vertex_handle;


    tbmrf(int nblabs, DTW * t,D_MAP * dm) :  NBL(nblabs), lambda(1), labs(NBL),pLabs(NBL),ly_idx(0),ld_idx(0)
    {
        D = Traits::D;
        tri = t;
        data_map = dm;
        for(int i = 0; i < NBL; i++)
        {
            labs[i] = i;
            pLabs[i] = i*(255.0/((double)NBL));
        }
    }

    //std::vector<Point> parse_points(std::string namefile, int D);
    //void dump_points(std::vector<Point> & points, std::string namefile, int D);


    //  virtual void get_obs_cut_pursuit(Cell_const_iterator fch,double & obs, double & weight);
    virtual double get_score_linear(Cell_const_iterator fch,int label,D_MAP & data_map) = 0;


    void update_simplex()
    {
        // for( auto cit = tri->cells_begin();
        //      cit != tri->cells_end(); ++cit ){
        //   if( tri->is_infinite(cit) )
        //     continue;
        //   cit->data().update_lab();
        // }
        std::cout << "update simplex void" << std::endl;
    }


    void scale_simplex()
    {
        std::cout << "scale simpelx void" << std::endl;
    }


    void init_lab()
    {
        // for( auto cit = tri->cells_begin();
        //      cit != tri->cells_end(); ++cit ){
        //   // if( tri->is_infinite(cit) )
        //   //   continue;
        //   cit->data().lab = 0;
        // }
        return;

    }

    // Init lab function

    void init_idx()
    {
        // Vertex_iterator fvit;
        // int acc = 0;

        // for( auto cit = tri->cells_begin();
        //      cit != tri->cells_end(); ++cit ){
        //   cit->data().opt_idx = acc++;
        // }
        // acc = 0;
        // for(fvit = tri->vertices_begin(); fvit != tri->vertices_end() ; ++fvit){
        //   // if( tri->is_infinite(fvit) )
        //   //   continue;
        //   Vertex_handle v = fvit;
        //   //v->data().opt_idx = acc;
        //   acc++;
        // }
        return;
    }



    void hello()
    {
        std::cout << "hello tbmrf : " << D << std::endl;
    }



    double get_volume(Cell_const_iterator & cci)
    {
        Tile_cell_const_handle fch = cci->full_cell();
        Vertex_h_iterator vht;
        std::list<Point> lp;
        for(vht = fch->vertices_begin() ;
                vht != fch->vertices_end() ;
                ++vht)
        {
            Vertex_handle v = *vht;
            lp.push_back(v->point());

        }
        return n_volume(lp,D);
    }



    double get_surface(Cell_const_iterator & cci, int idx)
    {
        Tile_cell_const_handle fch = cci->full_cell();
        Vertex_h_iterator vht;
        std::list<Point> lp;
        for(vht = fch->vertices_begin() ;
                vht != fch->vertices_end() ;
                ++vht)
        {
            Vertex_handle v = *vht;
            if(fch->index(v) == idx)
                continue;
            lp.push_back(v->point());

        }
        return n_surface<Point,Traits>(lp,D);
    }


    double get_score_quad(int ch1lab,int chnlab)
    {
        return fabs(ch1lab-chnlab);

    }




    // void opt_gc(int lalpha){


    //   int N = tri->number_of_cells();
    //   int NF = 0;

    //   for(Facet_const_iterator fit = tri->facets_begin();  fit != tri->facets_end(); ++fit){
    //     NF++;
    //   }



    //   GraphType *g = new GraphType(N,NF*2 );
    //   double e0,e1;

    //   int acc = 0;
    //   for( auto cit = tri->cells_begin();
    //        cit != tri->cells_end(); ++cit ){
    //     // if(tri->is_infinite(cit))
    //     //   continue;
    //     cit->data().opt_idx = acc++;
    //     g -> add_node();
    //   }


    //   for( auto cit = tri->cells_begin();
    //        cit != tri->cells_end(); ++cit ){
    //     Cell_const_iterator fch = cit;

    //     // if(tri->is_infinite(fch))
    //     //   continue;
    //     int lcurr = fch->data().lab;
    //     int cid = fch->data().opt_idx;


    //     e0 = get_score_linear(fch,lcurr);
    //     e1 = get_score_linear(fch,lalpha);

    //     //    if(1 == lcurr)

    //     if(tri->is_infinite(fch)){
    //       e0 = 0;
    //       e1 = 0;
    //     }
    //     g->add_tweights(cid, (e0 * MULT), (e1 * MULT));

    //   }



    //   DATA_TYPE E[4];
    //   for(Facet_const_iterator fit = tri->facets_begin();  fit != tri->facets_end(); ++fit){
    //     Cell_const_iterator fch = fit.full_cell();
    //     int idx = fit.index_of_covertex();
    //     Cell_const_iterator fchn = fch->neighbor(idx);
    //     if(tri->is_infinite(fch) && tri->is_infinite(fchn) )
    //       continue;
    //     Vertex_h_iterator vht;

    //     int c1Id = fch->data().opt_idx;
    //     int cnId = fchn->data().opt_idx;

    //     double surface = get_surface(fch,idx);
    //     double coef = lambda*surface;

    //     int ch1lab = fch->data().lab;
    //     int chnlab = fchn->data().lab;

    //     E[3] = get_score_quad(ch1lab,chnlab);
    //     E[2] = get_score_quad(ch1lab,lalpha);
    //     E[1] = get_score_quad(lalpha,chnlab);
    //     E[0] = get_score_quad(lalpha,lalpha);

    //     double E_x1 = E[0] - E[2];
    //     double E_bx2 = E[3] - E[2];
    //     // Quadratic term should be positif
    //     double E_quad = -E[0] + E[1] + E[2] - E[3];


    //     if(E_x1 > 0)
    //       g->add_tweights(c1Id, 0 , MULT*E_x1*coef);
    //     else
    //       g->add_tweights(c1Id,-1*MULT*E_x1*coef, 0);
    //     if(E_bx2 > 0)
    //       g->add_tweights(cnId, MULT*E_bx2*coef, 0 );
    //     else
    //       g->add_tweights(cnId,0, -1*MULT*E_bx2*coef);
    //     g->add_edge(c1Id, cnId,    /* capacities */ MULT*E_quad*coef ,0);
    //   }


    //   //std::cout << "\t Max flow algorithm ..." << std::endl;
    //   double flow = g->maxflow();
    //   //std::cout << "\t\t flow value : " << flow << std::endl;
    //   //std::cout << "\t\t Wow! such flow! much cut! good optimizing!" << std::endl;
    //   int nb_merge = 0;

    //   for( auto cit = tri->cells_begin();
    //        cit != tri->cells_end(); ++cit ){
    //     Cell_const_iterator fch = cit;
    //     if(g->what_segment(fch->data().opt_idx) == GraphType::SOURCE){
    //       fch->data().lab = lalpha;
    //       nb_merge++;
    //     }else{

    //     }

    //   }
    //   //std::cerr << "nb merges :" << nb_merge << std::endl;
    //   delete g;

    // }




    // void alpha_exp(){

    //   int IT_MAX = 10;
    //   std::vector<int>::iterator labsit;
    //   int acc = 0;
    //   for(int it = 0; it < IT_MAX;it++){
    //     for(labsit = labs.begin(); labsit != labs.end();++labsit){
    //       int lalpha = *labsit;
    //       //std::cout << "\t\t alpha exp label:" << lalpha << std::endl;
    //       opt_gc(lalpha);
    //       acc++;
    //     }
    //     if(NBL == 2) break;
    //   }
    // }


    void do_cut_pursuit()
    {
        return ;

    }







    void init_lab(DTW & tri)
    {
        for( auto cit = tri.cells_begin();
                cit != tri.cells_end(); ++cit )
        {
            (*cit)->data().lab = 0;
        }

    }


    void init_idx(DTW & tri)
    {
        //  Vertex_iterator fvit;
        //  int acc = 0;

        //  for( auto cit = tri.cells_begin();
        //       cit != tri.cells_end(); ++cit ){
        //    (*cit)->data().opt_idx = acc++;
        // }
        //  acc = 0;
        return;
    }


    void  do_cut_pursuit(DTW & tri)
    {

        return ;

    }


    void alpha_exp(DTW & tri,D_MAP & data_map)
    {

        for(auto lit = labs.begin(); lit != labs.end(); ++lit)
            std::cerr << "ll:"<< *lit << std::endl;

        if(NBL == 2)
        {
            opt_gc(labs.back(),tri,data_map);
            return;
        }

        int IT_MAX = 2;
        std::vector<int>::iterator labsit;
        int acc = 0;
        for(int it = 0; it < IT_MAX; it++)
        {
            for(labsit = labs.begin(); labsit != labs.end(); ++labsit)
            {
                int lalpha = *labsit;
                //std::cerr << "\t\t alpha exp label:" << lalpha << "/" << labs.size() << " it:" << it << "/" << IT_MAX <<  std::endl;
                opt_gc(lalpha,tri,data_map);
                acc++;
            }
        }
        ////std::cerr << "alpha exp finish" << std::endl;
    }


    void opt_gc(int lalpha,DTW & tri,D_MAP & data_map)
    {

        int  N =   tri.number_of_cells();
        int NF = 0;


        std::cerr << "init facet " << std::endl;
        for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit)
        {
            NF++;
        }

        GraphType *g = new GraphType(N,NF*2 );
        double e0,e1;

        int acc = 0;
        std::map<Cell_const_iterator,int> id_map;
        std::cerr << "init cell " << std::endl;
        for( auto cit = tri.cells_begin();
                cit != tri.cells_end(); ++cit )
        {
            // if(tri.is_infinite(cit))
            //   continue;
            id_map[cit] = acc++;
            g -> add_node();
        }


        std::cerr << "score cell " << std::endl;
        for( auto cit = tri.cells_begin();
                cit != tri.cells_end(); ++cit )
        {
            Cell_const_iterator fch = *cit;

            // if(tri->is_infinite(fch))
            //   continue;

            int cid = id_map[cit];
            int cccid = cit->cell_data().id;

            int lcurr = data_map[fch->tile()->id()].format_labs[cccid];
            e0 = get_score_linear(fch,lcurr,data_map);
            e1 = get_score_linear(fch,lalpha,data_map);
            //      std::cerr << "lcurr:" << lcurr << " lalpha:" << lalpha << std::endl;
            //      std::cerr << "e0   :" << e0    << "     e1:" << e1 << std::endl;
            //    if(1 == lcurr)
            // if(tri.is_infinite(fch)){
            //   e0 = 0;
            //   e1 = 0;
            // }
            g->add_tweights(cid, (e0 * MULT), (e1 * MULT));

        }


        std::cerr << "score facet " << std::endl;
        DATA_TYPE E[4];
        for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit)
        {
            if(fit->is_infinite())
                continue;
            try
            {


                Cell_const_iterator tmp_fch = fit.full_cell();
                int tmp_idx = fit.index_of_covertex();
                Cell_const_iterator tmp_fchn = tmp_fch->neighbor(tmp_idx);

                if(!tri.tile_is_loaded(tmp_fch->main_id()) ||
                        !tri.tile_is_loaded(tmp_fchn->main_id()))
                    continue;

                Cell_const_iterator fch = tmp_fch->main();
                int idx = tmp_idx;
                Cell_const_iterator fchn = tmp_fchn->main();


                Vertex_h_iterator vht;

                int c1Id = id_map[fch];
                int cnId = id_map[fchn];

                // if( c1Id ==  cnId  || c1Id == 0 || cnId == 0 ){
                //   continue;
                // }


                int cccid = fch->cell_data().id;
                int cccidn = fchn->cell_data().id;
                double surface = get_surface(fch,idx);
                double coef = lambda*surface;

                int ch1lab = data_map[fch->tile()->id()].format_labs[cccid];
                int chnlab = data_map[fchn->tile()->id()].format_labs[cccidn];



                E[3] = get_score_quad(ch1lab,chnlab);
                E[2] = get_score_quad(ch1lab,lalpha);
                E[1] = get_score_quad(lalpha,chnlab);
                E[0] = get_score_quad(lalpha,lalpha);

                double E_x1 = E[0] - E[2];
                double E_bx2 = E[3] - E[2];
                // Quadratic term should be positif
                double E_quad = -E[0] + E[1] + E[2] - E[3];


                if(E_x1 > 0)
                    g->add_tweights(c1Id, 0, MULT*E_x1*coef);
                else
                    g->add_tweights(c1Id,-1*MULT*E_x1*coef, 0);
                if(E_bx2 > 0)
                    g->add_tweights(cnId, MULT*E_bx2*coef, 0 );
                else
                    g->add_tweights(cnId,0, -1*MULT*E_bx2*coef);
                g->add_edge(c1Id, cnId,    /* capacities */ MULT*E_quad*coef,0);

            }
            catch (...)
            {
                continue;
            }
        }


        std::cerr << "\t Max flow algorithm ..." << std::endl;
        double flow = g->maxflow();
        std::cerr << "\t\t flow value : " << flow << std::endl;
        int nb_merge = 0;

        for( auto cit = tri.cells_begin();
                cit != tri.cells_end(); ++cit )
        {
            Cell_const_iterator fch = *cit;
            int cid = id_map[fch];
            if(g->what_segment(cid) == GraphType::SOURCE)
            {
                int cccid = cit->cell_data().id;
                if( data_map.find(fch->tile()->id()) == data_map.end())
                    continue;
                data_map[fch->tile()->id()].format_labs[cccid] = lalpha;
                nb_merge++;
            }
            else
            {

            }

        }
        std::cerr << "nb merges :" << nb_merge << "/"<< N << std::endl;
        delete g;

    }



    void extract_graph(int lalpha,DTW & tri,D_MAP & data_map, std::vector<int> & tile_ids,std::ostream & ofile)
    {

        int sourceId = 0;
        int targetId = 1;

        int  N = tri.number_of_cells();
        int NF = 0;
        std::cerr << "init graph" << std::endl;
        for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit)
        {
            NF++;
        }

        double e0,e1;

        int acc = 0;
        std::map<Cell_const_iterator,int> id_map;

        std::cerr << "create ids" << std::endl;
        for( auto cit = tri.cells_begin();
                cit != tri.cells_end(); ++cit )
        {
            // if(tri.is_infinite(cit))
            //    continue;
            id_map[cit] = acc++;
        }


        std::cerr << "score simplex" << std::endl;
        for( auto cit = tri.cells_begin();
                cit != tri.cells_end(); ++cit )
        {
            Cell_const_iterator fch = *cit;

            // if(tri->is_infinite(fch))
            //    continue;

            int cid = id_map[cit];
            int cccid = cit->cell_data().id;

            int lcurr = data_map[fch->tile()->id()].format_labs[cccid];
            e0 = get_score_linear(fch,lcurr,data_map);
            e1 = get_score_linear(fch,lalpha,data_map);

            ofile << sourceId << " " <<  l2gid(cid,tile_ids) << " " << (e0 * MULT) << std::endl;
            ofile << l2gid(cid,tile_ids) << " " << targetId << " " <<  (e1 * MULT) << std::endl;

            //      std::cerr << "lcurr:" << lcurr << " lalpha:" << lalpha << std::endl;
            //      std::cerr << "e0   :" << e0    << "     e1:" << e1 << std::endl;
            //    if(1 == lcurr)
            // if(tri.is_infinite(fch)){
            //   e0 = 0;
            //   e1 = 0;
            // }

        }


        std::cerr << "score facet " << std::endl;
        DATA_TYPE E[4];
        for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit)
        {
            break;
            if(fit->is_infinite())
                continue;
            try
            {


                Cell_const_iterator tmp_fch = fit.full_cell();
                int tmp_idx = fit.index_of_covertex();
                Cell_const_iterator tmp_fchn = tmp_fch->neighbor(tmp_idx);

                if(!tri.tile_is_loaded(tmp_fch->main_id()) ||
                        !tri.tile_is_loaded(tmp_fchn->main_id()))
                    continue;

                Cell_const_iterator fch = tmp_fch->main();
                int idx = tmp_idx;
                Cell_const_iterator fchn = tmp_fchn->main();


                Vertex_h_iterator vht;

                int c1Id = id_map[fch];
                int cnId = id_map[fchn];

                // if( c1Id ==  cnId  || c1Id == 0 || cnId == 0 ){
                //   continue;
                // }


                int cccid = fch->cell_data().id;
                int cccidn = fchn->cell_data().id;
                double surface = get_surface(fch,idx);
                double coef = lambda*surface;

                int ch1lab = data_map[fch->tile()->id()].format_labs[cccid];
                int chnlab = data_map[fchn->tile()->id()].format_labs[cccidn];



                E[3] = get_score_quad(ch1lab,chnlab);
                E[2] = get_score_quad(ch1lab,lalpha);
                E[1] = get_score_quad(lalpha,chnlab);
                E[0] = get_score_quad(lalpha,lalpha);

                double E_x1 = E[0] - E[2];
                double E_bx2 = E[3] - E[2];
                // Quadratic term should be positif
                double E_quad = -E[0] + E[1] + E[2] - E[3];


                if(E_x1 > 0)
                {
                    //g->add_tweights(c1Id, 0 , MULT*E_x1*coef);
                    ofile << l2gid(c1Id,tile_ids) << " "  << targetId  << " " << MULT*E_x1*coef  << std::endl;
                }
                else
                {
                    //g->add_tweights(c1Id,-1*MULT*E_x1*coef, 0);
                    ofile << sourceId << " " << l2gid(c1Id,tile_ids) << " "  << -1*MULT*E_x1*coef  << std::endl;
                }
                if(E_bx2 > 0)
                {
                    //g->add_tweights(cnId, MULT*E_bx2*coef, 0 );
                    ofile << sourceId << " " << l2gid(cnId,tile_ids) << " "  << MULT*E_bx2*coef   << std::endl;
                }
                else
                {
                    // g->add_tweights(cnId,0, -1*MULT*E_bx2*coef);
                    ofile <<  l2gid(cnId,tile_ids) << " "  << targetId << " " << -1*MULT*E_bx2*coef  << std::endl;

                }
                // g->add_edge(c1Id, cnId,    /* capacities */ MULT*E_quad*coef ,0);
                ofile  << l2gid(c1Id,tile_ids) << " " << l2gid(cnId,tile_ids) << " " << MULT*E_quad*coef <<  std::endl;
            }
            catch (...)
            {
                continue;
            }
        }
    }




    int extract_factor_graph(int lalpha,DTW & tri,D_MAP & data_map, std::map<int,std::vector<int>> & tile_ids,std::ostream & ofile)
    {

        int sourceId = 0;
        int targetId = 1;

        int  N = tri.number_of_cells();
        int NF = 0;
        std::cerr << "init graph" << std::endl;
        for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit)
        {
            NF++;
        }

        double e0,e1;

        int acc = 0;
        std::map<int,int> id_map;

        std::vector<int> id2gid_vec;
        std::cerr << "create ids" << std::endl;
        for( auto cit = tri.cells_begin();
                cit != tri.cells_end(); ++cit )
        {
            id2gid_vec.push_back(cit->cell_data().id);
        }

        std::sort(id2gid_vec.begin(), id2gid_vec.end());
        // for( auto cit = tri.cells_begin();
        // 	 cit != tri.cells_end(); ++cit ){
        //   id_map[cit] = ii;
        // }

        for(int ii = 0 ; ii < id2gid_vec.size(); ii++)
        {
            id_map[id2gid_vec[ii]] = ii;
        }

        std::cerr << "score simplex" << std::endl;
        for( auto cit = tri.cells_begin();
                cit != tri.cells_end(); ++cit )
        {
            Cell_const_iterator fch = *cit;

            // if(tri->is_infinite(fch))
            //    continue;
            int ctid = cit->tile()->id();

            int cid = id_map[cit->cell_data().id];
            int lcurr = 0; //data_map[fch->tile()->id()].format_labs[cccid];
            e0 = get_score_linear(fch,lcurr,data_map);
            e1 = get_score_linear(fch,lalpha,data_map);
            ofile << "v" << " " <<  l2gidf(cid,tile_ids[ctid]) << " " << (e0 * MULT) << " " <<  (e1 * MULT) << std::endl;
            //ofile << "v" << " " <<  l2gidf(cid,tile_ids) << " " << cid << " " <<  cid << std::endl;
        }


        std::cerr << "score facet " << std::endl;
        DATA_TYPE E[4];
        for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit)
        {
            if(fit->is_infinite())
                continue;
            try
            {


                Cell_const_iterator tmp_fch = fit.full_cell();
                int tmp_idx = fit.index_of_covertex();
                Cell_const_iterator tmp_fchn = tmp_fch->neighbor(tmp_idx);

                if(!tri.tile_is_loaded(tmp_fch->main_id()) ||
                        !tri.tile_is_loaded(tmp_fchn->main_id()))
                    continue;

                Cell_const_iterator fch = tmp_fch->main();
                int idx = tmp_idx;
                Cell_const_iterator fchn = tmp_fchn->main();


                Vertex_h_iterator vht;

                int cccid = fch->cell_data().id;
                int cccidn = fchn->cell_data().id;
                int c1Id = id_map[cccid];
                int cnId = id_map[cccidn];
                double surface = get_surface(fch,idx);
                double coef = lambda*surface;

                int ch1tid = fch->tile()->id();
                int chntid = fchn->tile()->id();
                int ch1lab = data_map[ch1tid].format_labs[cccid];
                int chnlab = data_map[chntid].format_labs[cccidn];



                E[3] = get_score_quad(ch1lab,chnlab);
                E[2] = get_score_quad(ch1lab,lalpha);
                E[1] = get_score_quad(lalpha,chnlab);
                E[0] = get_score_quad(lalpha,lalpha);

                double E_x1 = E[0] - E[2];
                double E_bx2 = E[3] - E[2];
                // Quadratic term should be positif
                double E_quad = -E[0] + E[1] + E[2] - E[3];
                ofile << "e " << l2gidf(c1Id,tile_ids[ch1tid]) << " "  << l2gidf(cnId,tile_ids[chntid])  << " ";
                for(int i = 0 ; i < 4; i++)
                {
                    ofile << MULT*E[i]*coef;
                    if(i <3)
                        ofile << " ";
                }
                ofile << std::endl;

            }
            catch (...)
            {
                continue;
            }
        }
        return acc;
    }




    int extract_factor_graph_old(int lalpha,DTW & tri,D_MAP & data_map, std::map<int,std::vector<int>> & tile_ids,std::ostream & ofile)
    {

        int sourceId = 0;
        int targetId = 1;

        int  N = tri.number_of_cells();
        int NF = 0;
        std::cerr << "init graph" << std::endl;
        for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit)
        {
            NF++;
        }

        double e0,e1;

        int acc = 0;
        std::map<int,int> id_map;

        std::vector<int> id2gid_vec;
        std::cerr << "create ids" << std::endl;
        for( auto cit = tri.cells_begin();
                cit != tri.cells_end(); ++cit )
        {
            id2gid_vec.push_back(cit->cell_data().id);
        }

        std::sort(id2gid_vec.begin(), id2gid_vec.end());
        // for( auto cit = tri.cells_begin();
        // 	 cit != tri.cells_end(); ++cit ){
        //   id_map[cit] = ii;
        // }

        for(int ii = 0 ; ii < id2gid_vec.size(); ii++)
        {
            id_map[id2gid_vec[ii]] = ii;
        }

        std::cerr << "score simplex" << std::endl;
        for( auto cit = tri.cells_begin();
                cit != tri.cells_end(); ++cit )
        {
            Cell_const_iterator fch = *cit;

            // if(tri->is_infinite(fch))
            //    continue;
            int ctid = cit->tile()->id();

            int cid = id_map[cit->cell_data().id];
            int lcurr = 0; //data_map[fch->tile()->id()].format_labs[cccid];
            e0 = get_score_linear(fch,lcurr,data_map);
            e1 = get_score_linear(fch,lalpha,data_map);
            ofile << "v" << " " <<  l2gidf(cid,tile_ids[ctid]) << " " << (e0 * MULT) << " " <<  (e1 * MULT) << std::endl;
            //ofile << "v" << " " <<  l2gidf(cid,tile_ids) << " " << cid << " " <<  cid << std::endl;
        }


        std::cerr << "score facet " << std::endl;
        DATA_TYPE E[4];
        for(auto fit = tri.facets_begin();  fit != tri.facets_end(); ++fit)
        {
            if(fit->is_infinite())
                continue;
            try
            {


                Cell_const_iterator tmp_fch = fit.full_cell();
                int tmp_idx = fit.index_of_covertex();
                Cell_const_iterator tmp_fchn = tmp_fch->neighbor(tmp_idx);

                if(!tri.tile_is_loaded(tmp_fch->main_id()) ||
                        !tri.tile_is_loaded(tmp_fchn->main_id()))
                    continue;

                Cell_const_iterator fch = tmp_fch->main();
                int idx = tmp_idx;
                Cell_const_iterator fchn = tmp_fchn->main();


                Vertex_h_iterator vht;

                int cccid = fch->cell_data().id;
                int cccidn = fchn->cell_data().id;
                int c1Id = id_map[cccid];
                int cnId = id_map[cccidn];
                double surface = get_surface(fch,idx);
                double coef = lambda*surface;

                int ch1tid = fch->tile()->id();
                int chntid = fchn->tile()->id();
                int ch1lab = data_map[ch1tid].format_labs[cccid];
                int chnlab = data_map[chntid].format_labs[cccidn];



                E[3] = get_score_quad(ch1lab,chnlab);
                E[2] = get_score_quad(ch1lab,lalpha);
                E[1] = get_score_quad(lalpha,chnlab);
                E[0] = get_score_quad(lalpha,lalpha);

                double E_x1 = E[0] - E[2];
                double E_bx2 = E[3] - E[2];
                // Quadratic term should be positif
                double E_quad = -E[0] + E[1] + E[2] - E[3];
                ofile << "e " << l2gidf(c1Id,tile_ids[ch1tid]) << " "  << l2gidf(cnId,tile_ids[chntid])  << " ";
                for(int i = 0 ; i < 4; i++)
                {
                    ofile << MULT*E[i]*coef;
                    if(i <3)
                        ofile << " ";
                }
                ofile << std::endl;

            }
            catch (...)
            {
                continue;
            }
        }
        return acc;
    }



    int D,NBL;
    double lambda;
    int ly_idx,ld_idx;
    DTW * tri;
    D_MAP * data_map;


    std::vector<int>  labs;
    std::vector<double>  pLabs;



};



#endif
